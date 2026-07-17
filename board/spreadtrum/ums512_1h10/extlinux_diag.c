#include <common.h>
#include <command.h>
#include <cli.h>
#include <sprd_common_rw.h>
#include <boot_mode.h>
#include <logo_bin.h>
#include <mmc.h>
#include <part.h>

extern void lcd_printf(const char *fmt, ...);
extern int drv_lcd_init(void);

/*
 * Persist the printf capture buffer (CONFIG_SPRD_LOG fills p_log_buffer from
 * every printf, see common/console.c) to offset 0 of the uboot_log partition,
 * so the boot log can be read back over FDL: spd_dump ... r uboot_log.
 *
 * The init_write_log()/write_log() macros are no-ops unless DEBUG is defined,
 * and write_uboot_last_log() bails out when get_boot_role() reads DOWNLOAD, so
 * write the raw buffer directly with none of those gates.
 */
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
 * Locate a partition by GPT name (1-based partition number, or -1). GPT
 * numbering on this device: splloader is outside the GPT, prodnv = 1,
 * boot_a = 42, boot_b = 43 — but look up by name so a repartition can't
 * silently break the boot.
 */
static int find_part_by_name(int dev, const char *want)
{
	block_dev_desc_t *desc = get_dev("mmc", dev);
	disk_partition_t info;
	int p, found = -1;

	if (!desc)
		return -1;

	for (p = 1; p < 80; p++) {
		if (get_partition_info(desc, p, &info))
			break;
		if (!strcmp((char *)info.name, want)) {
			found = p;
			break;
		}
	}

	printf("[uboot] mmc%d '%s' = part %d\n", dev, want, found);
	return found;
}

static int try_sysboot(int dev, int part, const char *path)
{
	char cmd[160];
	int ret;

	printf("[uboot] try mmc %d:%d %s\n", dev, part, path);
	lcd_printf("boot mmc%d:%d\n", dev, part);
	lcd_printf("%s\n", path);

	/* flush the log before sysboot: a successful boot never returns */
	diag_log_dump();

	snprintf(cmd, sizeof(cmd),
		 "sysboot mmc %d:%d any ${pxefile_addr_r} %s",
		 dev, part, path);
	ret = run_command(cmd, 0);
	if (!ret)
		return 0;

	printf("[uboot] miss mmc %d:%d %s (%d)\n", dev, part, path, ret);
	lcd_printf("miss mmc%d:%d\n", dev, part);
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
	diag_log_dump();
	/*
	 * lcd_printf output is only visible after a DPU layer flip + backlight,
	 * which only logo_display() does (drv_lcd_init alone just probes the
	 * panel). Same pattern as fastboot_mode().
	 */
	logo_display(LOGO_NORMAL_POWER, BACKLIGHT_ON, LCD_DISPLAY_ENABLE);
	lcd_printf("extlinux scan\n");

	/*
	 * The boot path only registers the eMMC host (mmc dev 0); SDIO0
	 * (microSD) is normally brought up only by the sysdump/SD-log paths.
	 * Register it here so the scan can probe it as mmc dev 1.
	 */
	sd_present = board_sd_init() != NULL;
	if (sd_present) {
		printf("[uboot] sd card registered\n");
		lcd_printf("Found an SD Card\n");
	} else {
		printf("[uboot] no sd card found\n");
		lcd_printf("No SD Card found\n");
	}
	/* persist everything the LCD/panel probe just printed */
	diag_log_dump();

	/* microSD first (removable-media override), then the eMMC boot slots
	 * located by GPT name */
	if (sd_present) {
		for (fi = 0; fi < ARRAY_SIZE(paths); fi++)
			if (!try_sysboot(1, 1, paths[fi]))
				return 0;
	}

	{
		int pa = find_part_by_name(0, "boot_a");
		int pb = find_part_by_name(0, "boot_b");

		for (fi = 0; fi < ARRAY_SIZE(paths); fi++) {
			if (pa > 0 && !try_sysboot(0, pa, paths[fi]))
				return 0;
			if (pb > 0 && !try_sysboot(0, pb, paths[fi]))
				return 0;
		}
	}

	printf("[uboot] extlinux scan failed\n");
	lcd_printf("extlinux failed\n");
	diag_log_dump();
	return CMD_RET_FAILURE;
}

U_BOOT_CMD(
	extlinux_scan, 1, 0, do_extlinux_scan,
	"scan likely mmc boot partitions for extlinux.conf with LCD diagnostics",
	""
);
