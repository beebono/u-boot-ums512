#include <common.h>
#include <command.h>
#include <cli.h>
#include <sprd_common_rw.h>
#include <sprd_battery.h>
#include <boot_mode.h>
#include <logo_bin.h>
#include <mmc.h>
#include <part.h>
#include <fs.h>
#include <lcd.h>
#include <video_font.h>
#include <dm.h>
#include <dm/uclass.h>
#include <dm/device-internal.h>
#include <i2c.h>
#include <asm/u-boot-arm.h>

#include <asm/arch-sharkl5pro/chip_sharkl5pro/aon_apb.h>
#include <asm/arch-sharkl5pro/chip_sharkl5pro/pmu_apb.h>
#include "cp_boot.h"

/* Charge screen tuning. */
#define CHARGE_SPLASH_MS	3000	/* how long the "Charging..." text stays lit */
#define LONG_PRESS_MS		2000	/* power-key hold that means "boot the OS" */
#define CHARGE_POLL_MS		50		/* button poll cadence while blanked */
#define CHARGE_TEXT_SCALE	5		/* pixel-doubling factor for charge-screen text */
#define CHARGE_HEARTBEAT_MS	30000	/* diagnostic log cadence while blanked */
#define CHARGE_TEMP_POLL_MS	15000	/* charge temperature guard cadence, matches the kernel */
#define CHARGE_DIAG			0		/* 1: log pack state periodically (costs an eMMC write each) */
#define CHARGE_CHG_DUMP		0		/* 1: dump the charger IC's registers (LOCKS REG06, see below) */
#define CHG_SLAVE_ADDR		0x6a	/* charger@6a: AW32257, confirmed via REG03 */
#define CHG_REG_COUNT		11		/* AW32257 map: REG00..REG0A */

/* power_button_pressed() returns KEY_PRESSED (0) while the key is held down. */
#define PWRKEY_DOWN		0

DECLARE_GLOBAL_DATA_PTR;

extern void lcd_clear(void);
extern int fg_color, bg_color;
extern int power_button_pressed(void);
extern void power_down_devices(unsigned pd_cmd);
extern void stop_watchdog(void);
extern int aw32257_temp_guard(int blocked);
extern int aw32257_battery_temp(void);

static void diag_log_dump(void)
{
#if defined(CONFIG_SPRD_LOG) && defined(CONFIG_LOG_2_EMMC)
	if (!p_log_buffer || !p_log_buffer->addr || !p_log_buffer->used)
		return;

	if (common_raw_write(UBOOT_LOG_PARTITION,
			     (uint64_t)p_log_buffer->used, (uint64_t)0,
			     (uint64_t)LAST_LOG_PARTITION_OFFSET,
			     (char *)p_log_buffer->addr))
		printf("[uboot] uboot_log dump failed\n");
#endif
}

/*
 * Blit an ASCII string to the framebuffer, centered, scaled up by pixel
 * doubling (the console font is a fixed 8x16, far too small on this panel).
 * 32bpp only, which is what this board runs (LCD_COLOR32).
 */
static void charge_text_big(const char *s, int scale)
{
	u32 *fb = (u32 *)(uintptr_t)gd->fb_base;
	int w = panel_info.vl_col;
	int h = panel_info.vl_row;
	int gw = VIDEO_FONT_WIDTH * scale;
	int gh = VIDEO_FONT_HEIGHT * scale;
	int len = strlen(s);
	int x0 = (w - len * gw) / 2;
	int y0 = (h - gh) / 2;
	int i, row, col, sy, sx;

	if (x0 < 0)
		x0 = 0;
	if (y0 < 0)
		y0 = 0;

	for (i = 0; s[i]; i++) {
		const unsigned char *glyph =
			&video_fontdata[(unsigned char)s[i] * VIDEO_FONT_HEIGHT];
		int cx = x0 + i * gw;

		for (row = 0; row < VIDEO_FONT_HEIGHT; row++) {
			unsigned char bits = glyph[row];

			for (col = 0; col < VIDEO_FONT_WIDTH; col++) {
				u32 px = (bits & (0x80 >> col)) ?
					 (u32)fg_color : (u32)bg_color;

				for (sy = 0; sy < scale; sy++) {
					u32 *p = fb + (y0 + row * scale + sy) * w
						 + cx + col * scale;
					for (sx = 0; sx < scale; sx++)
						p[sx] = px;
				}
			}
		}
	}
}

/*
 * Clear to a solid background and draw a big centered message. Brings the
 * panel up on first use with the backlight off, so the stock logo never
 * flashes before our text; subsequent calls just repaint and relight.
 */
static int panel_up;

static void lcd_banner(const char *msg)
{
	if (!panel_up) {
		logo_display(LOGO_NORMAL_POWER, BACKLIGHT_OFF, LCD_DISPLAY_ENABLE);
		panel_up = 1;
	}
	lcd_clear();	/* wipe prior text and lay down the background color */
	charge_text_big(msg, CHARGE_TEXT_SCALE);
	lcd_sync();	/* flush dcache + push framebuffer to the panel (SWDISPC) */
	logo_display(LOGO_NORMAL_POWER, BACKLIGHT_ON, 0);
}

/* Show the current pack charge. Panel must already be up. */
static void charge_draw(void)
{
	char buf[32];

	snprintf(buf, sizeof(buf), "Charging... %d%%",
		 sprdfgu_read_live_capacity_permille() / 10);
	lcd_banner(buf);
}

#if CHARGE_DIAG

static void charge_heartbeat(ulong ms)
{
	int temp = aw32257_battery_temp();

	printf("[uboot] chg hb ms=%lu vbat=%umV cur=%dmA ocv=%dmV bat=%d.%dC cap=%d.%d%% saved=%d.%d%% chg=%d\n",
	       ms, sprdfgu_read_vbat_vol(),
	       sprdfgu_read_cur_ma(), sprdfgu_read_ocv_mv(),
	       temp / 10, temp < 0 ? -(temp % 10) : temp % 10,
	       sprdfgu_read_live_capacity_permille() / 10,
	       sprdfgu_read_live_capacity_permille() % 10,
	       sprdfgu_read_saved_capacity_permille() / 10,
	       sprdfgu_read_saved_capacity_permille() % 10,
	       charger_connected());
	diag_log_dump();
}
#else
static inline void charge_heartbeat(ulong ms) { }
#endif

#if CHARGE_CHG_DUMP
static void charger_reg_dump(void)
{
	struct udevice *bus;
	struct uclass *uc;
	int ret;

	ret = uclass_get(UCLASS_I2C, &uc);
	if (ret) {
		printf("[uboot] chg dump: no i2c uclass (%d)\n", ret);
		return;
	}

	uclass_foreach_dev(bus, uc) {
		struct udevice *chip;
		u8 reg[CHG_REG_COUNT];
		int i, val;

		if (device_probe(bus))
			continue;

		if (i2c_get_chip(bus, CHG_SLAVE_ADDR, 1, &chip))
			continue;

		for (i = 0; i < CHG_REG_COUNT; i++) {
			val = dm_i2c_reg_read(chip, i);
			if (val < 0)
				break;
			reg[i] = (u8)val;
		}

		if (i != CHG_REG_COUNT) {
			printf("[uboot] chg dump: bus %d addr 0x%02x no ack at reg %d\n",
			       bus->seq, CHG_SLAVE_ADDR, i);
			continue;
		}

		printf("[uboot] chg dump: bus %d addr 0x%02x regs %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
		       bus->seq, CHG_SLAVE_ADDR, reg[0], reg[1], reg[2], reg[3],
		       reg[4], reg[5], reg[6], reg[7], reg[8], reg[9], reg[10]);
		/* REG03: vendor [7:5], part number [4:3] (10b = AW32257), revision [2:0]. */
		printf("[uboot] chg dump: ic_info=0x%02x vendor=%d part=%d rev=%d\n",
		       reg[3], reg[3] >> 5, (reg[3] >> 3) & 0x3, reg[3] & 0x7);
		/* The two ceilings that actually bound charging, both in REG06. */
		printf("[uboot] chg dump: isafe_code=%d vsafe_code=%d voreg_code=%d ichg_code=%d\n",
		       reg[6] >> 4, reg[6] & 0xf, reg[2] >> 2, (reg[4] >> 3) & 0xf);
	}

	diag_log_dump();
}
#else
static inline void charger_reg_dump(void) { }
#endif

/* Blank the panel (backlight off only; leave the panel initialized). */
static void charge_blank(void)
{
	logo_display(LOGO_NORMAL_POWER, BACKLIGHT_OFF, 0);
}

/*
 * Offline charge screen. Entered when the device was plugged in while powered
 * off. Shows a brief "Charging..." splash, blanks the panel, then polls the
 * power key: a short press flashes the current charge, a long hold returns 0
 * to continue into the normal boot. Losing the charger powers off. Only
 * returns on a long hold; otherwise it loops or shuts the device down.
 */
static int charge_screen(void)
{
	ulong entry = get_timer(0);
	ulong last_hb = entry;
	ulong last_temp = entry;
	int temp_blocked;

	stop_watchdog();

	charger_reg_dump();

	temp_blocked = aw32257_temp_guard(0);

	/* Flash the charge splash (lcd_banner brings the panel up on first use). */
	charge_draw();
	mdelay(CHARGE_SPLASH_MS);
	charge_blank();
	charge_heartbeat(get_timer(entry));

	for (;;) {
		if (get_timer(last_temp) >= CHARGE_TEMP_POLL_MS) {
			last_temp = get_timer(0);
			temp_blocked = aw32257_temp_guard(temp_blocked);
		}

		if (get_timer(last_hb) >= CHARGE_HEARTBEAT_MS) {
			last_hb = get_timer(0);
			charge_heartbeat(get_timer(entry));
		}

		/* Unplugged: nothing left to do here, shut down. */
		if (!charger_connected()) {
			printf("[uboot] charger removed, powering off\n");
			diag_log_dump();
			power_down_devices(0);
		}

		if (power_button_pressed() == PWRKEY_DOWN) {
			ulong t0 = get_timer(0);
			int held_long = 0;

			/* Time the hold; break out early once it's a long press. */
			while (power_button_pressed() == PWRKEY_DOWN) {
				if (get_timer(t0) >= LONG_PRESS_MS) {
					held_long = 1;
					break;
				}
				mdelay(20);
			}

			if (held_long) {
				printf("[uboot] power held, leaving charge mode\n");
				lcd_banner("Booting...");
				return 0;
			}

			/* Short press: show charge, then blank again. */
			charge_draw();
			mdelay(CHARGE_SPLASH_MS);
			charge_blank();
		}

		mdelay(CHARGE_POLL_MS);
	}
}

static int try_sysboot(int dev, int part, const char *path)
{
	char cmd[160];
	int ret;

	printf("[uboot] try mmc %d:%d %s\n", dev, part, path);

	/* flush the log before sysboot: a successful boot never returns */
	diag_log_dump();

	snprintf(cmd, sizeof(cmd),
		 "sysboot mmc %d:%d any ${pxefile_addr_r} %s",
		 dev, part, path);
	ret = run_command(cmd, 0);
	if (!ret)
		return 0;

	printf("[uboot] miss mmc %d:%d %s (%d)\n", dev, part, path, ret);
	return ret;
}

static int do_extlinux_scan(cmd_tbl_t *cmdtp, int flag, int argc,
			    char * const argv[])
{
	static const char * const paths[] = {
		"/extlinux/extlinux.conf",
		"/boot/extlinux/extlinux.conf",
	};
	int sd_present, fi;

	printf("[uboot] extlinux diagnostic scan\n");

	sd_present = board_sd_init() != NULL;
	if (sd_present) {
		printf("[uboot] sd card registered\n");
	} else {
		printf("[uboot] no sd card found\n");
	}

	/*
	 * Decide whether this is a "plugged in while off" wake that should land
	 * on the charge screen, versus a deliberate power-on that should boot.
	 *
	 * The stock shutdown-charge flag is useless here: MISCDATA_SHUTDOWN_
	 * CHARGE_FLAG_* is undefined for this board, so the flag reads as always
	 * set. Instead use the live charger state plus the power key: a charger
	 * insertion auto-powers-on with the key released (-> charge screen),
	 * whereas a user power-press is still held this early in boot (-> boot).
	 * Unplugged never enters the charge screen.
	 *
	 * charge_screen() only returns if the user then holds power to boot; a
	 * short press just reports charge, and losing the charger powers off.
	 */
	if (sd_present && charger_connected() &&
	    power_button_pressed() != PWRKEY_DOWN) {
		printf("[uboot] charger present, key released: charge screen\n");
		charge_screen();
	}

	if (sd_present) {
		for (fi = 0; fi < ARRAY_SIZE(paths); fi++) {
			if (!file_exists("mmc", "1:2", paths[fi], FS_TYPE_ANY))
				continue;

			printf("[uboot] found %s, booting audio DSP\n",
			       paths[fi]);

			/*
			 * Show our own "Booting..." banner (not the stock logo)
			 * so it's clear we're on the custom firmware, then set up
			 * the audio DSP right before we jump to the kernel.
			 */
			lcd_banner("Booting...");
			memset_dsp_share_memory();

			if (!try_sysboot(1, 2, paths[fi]))
				return 0;

			/* Config was there, but boot failed. Nothing to fall back to. */
			printf("[uboot] sysboot failed after DSP boot\n");
			break;
		}
	}

	/*
	 * This U-Boot is hosted on the SD card and boots mainline directly, so
	 * there is no stock to hand back to. Stop here so a failed or missing
	 * mainline boot is visible instead of being masked.
	 */
	printf("[uboot] no mainline boot; halting\n");
	lcd_banner("No OS found");
	diag_log_dump();
	while (1)
		;

	return 1;
}

U_BOOT_CMD(
	extlinux_scan, 1, 0, do_extlinux_scan,
	"scan likely mmc boot partitions for extlinux.conf with LCD diagnostics",
	""
);
