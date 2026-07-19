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
#include <asm/u-boot-arm.h>

#include <asm/arch-sharkl5pro/chip_sharkl5pro/aon_apb.h>
#include <asm/arch-sharkl5pro/chip_sharkl5pro/pmu_apb.h>
#include "cp_boot.h"

#define STOCK_UBOOT_PARTITION	"uboot_b"
#define DHTB_HEADER_SIZE	0x200
#define DHTB_MAGIC		0x42544844	/* "DHTB", little-endian */
#define DHTB_DATA_SIZE_OFF	0x30
#define STOCK_STAGE_ADDR	(CONFIG_SYS_SDRAM_BASE + 0x00100000)
#define STOCK_TRAMP_ADDR	(CONFIG_SYS_SDRAM_BASE + 0x01000000)
#define STOCK_MAX_SIZE		(8 * 1024 * 1024)

DECLARE_GLOBAL_DATA_PTR;

extern void lcd_printf(const char *fmt, ...);
extern char stock_tramp[];
extern char stock_tramp_end[];

asm(
"	.pushsection	.text.stock_tramp, \"ax\"\n"
"	.globl	stock_tramp\n"
"	.type	stock_tramp, %function\n"
"stock_tramp:\n"
"1:	cbz	x2, 2f\n"
"	ldrb	w4, [x1], #1\n"
"	strb	w4, [x0], #1\n"
"	sub	x2, x2, #1\n"
"	b	1b\n"
"2:	br	x3\n"
"	.globl	stock_tramp_end\n"
"stock_tramp_end:\n"
"	.popsection\n"
);

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

static int chainload_stock_uboot(void)
{
	void (*tramp)(void *, void *, unsigned long, void *) =
		(void (*)(void *, void *, unsigned long, void *))STOCK_TRAMP_ADDR;
	static char hdr[DHTB_HEADER_SIZE] __aligned(64);
	char *stage = (char *)STOCK_STAGE_ADDR;
	uint64_t data_size;
	ulong tramp_len;

	if (common_raw_read(STOCK_UBOOT_PARTITION,
			    (uint64_t)DHTB_HEADER_SIZE, (uint64_t)0, hdr)) {
		printf("[uboot] cannot read %s header\n",
		       STOCK_UBOOT_PARTITION);
		return -1;
	}

	if (*(uint32_t *)hdr != DHTB_MAGIC) {
		printf("[uboot] %s: bad DHTB magic 0x%08x\n",
		       STOCK_UBOOT_PARTITION, *(uint32_t *)hdr);
		return -1;
	}

	data_size = *(uint64_t *)(hdr + DHTB_DATA_SIZE_OFF);
	if (!data_size || data_size > STOCK_MAX_SIZE) {
		printf("[uboot] %s: implausible payload size %llu (max %u)\n",
		       STOCK_UBOOT_PARTITION, (unsigned long long)data_size,
		       STOCK_MAX_SIZE);
		return -1;
	}

	printf("[uboot] staging stock U-Boot (%llu bytes) at 0x%08lx\n",
	       (unsigned long long)data_size, (ulong)STOCK_STAGE_ADDR);

	/* Payload begins right after the DHTB header. Read into scratch first. */
	if (common_raw_read(STOCK_UBOOT_PARTITION, data_size,
			    (uint64_t)DHTB_HEADER_SIZE, stage)) {
		printf("[uboot] %s: payload read failed\n",
		       STOCK_UBOOT_PARTITION);
		return -1;
	}

	/* Park the copy-and-jump stub in scratch, clear of the TEXT_BASE dest. */
	tramp_len = (ulong)stock_tramp_end - (ulong)stock_tramp;
	memcpy((void *)STOCK_TRAMP_ADDR, (void *)stock_tramp, tramp_len);

	/* Never returns on success. Flush the log while we still can. */
	diag_log_dump();

	/* Clean slate for stock u-boot. */
	cleanup_before_linux();

	tramp((void *)CONFIG_SYS_TEXT_BASE, stage, (unsigned long)data_size,
	      (void *)CONFIG_SYS_TEXT_BASE);

	/* If control ever comes back, the jump failed. */
	return -1;
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

	if (sd_present && !charger_connected()) {
		for (fi = 0; fi < ARRAY_SIZE(paths); fi++) {
			if (!file_exists("mmc", "1:1", paths[fi], FS_TYPE_ANY))
				continue;

			printf("[uboot] found %s, booting audio DSP\n",
			       paths[fi]);

			/* Boot audio DSP right before we jump to the kernel. */
			modem_entry();

			if (!try_sysboot(1, 1, paths[fi]))
				return 0;

			/* Config was there, but boot failed. Bail to stock and hope for the best. */
			printf("[uboot] sysboot failed after DSP boot\n");
			break;
		}
	}

	printf("[uboot] chainloading stock\n");

	chainload_stock_uboot();

	/* chainload_stock_uboot() only returns on failure. */
	printf("[uboot] chainload failed!\n");
	logo_display(LOGO_NORMAL_POWER, BACKLIGHT_ON, LCD_DISPLAY_ENABLE);
	lcd_printf("Chainload failed... :<\n");
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
