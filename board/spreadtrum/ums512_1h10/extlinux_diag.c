#include <common.h>
#include <command.h>
#include <cli.h>

extern void lcd_printf(const char *fmt, ...);

static int try_sysboot(int dev, int part, const char *path)
{
	char cmd[160];
	int ret;

	printf("[uboot] try mmc %d:%d %s\n", dev, part, path);
	lcd_printf("boot mmc%d:%d", dev, part);
	lcd_printf("%s", path);

	snprintf(cmd, sizeof(cmd),
		 "sysboot mmc %d:%d any ${pxefile_addr_r} %s",
		 dev, part, path);
	ret = run_command(cmd, 0);
	if (!ret)
		return 0;

	printf("[uboot] miss mmc %d:%d %s (%d)\n", dev, part, path, ret);
	lcd_printf("miss mmc%d:%d", dev, part);
	return ret;
}

static int do_extlinux_scan(cmd_tbl_t *cmdtp, int flag, int argc,
			    char * const argv[])
{
	static const int devs[] = { 0, 1, 2, 3 };
	static const int parts[] = { 42, 43 };
	static const char * const paths[] = {
		"/extlinux/extlinux.conf",
		"/boot/extlinux/extlinux.conf",
	};
	int di, pi, fi;

	printf("[uboot] extlinux diagnostic scan\n");
	lcd_printf("extlinux scan");

	for (di = 0; di < ARRAY_SIZE(devs); di++) {
		for (pi = 0; pi < ARRAY_SIZE(parts); pi++) {
			for (fi = 0; fi < ARRAY_SIZE(paths); fi++) {
				if (!try_sysboot(devs[di], parts[pi], paths[fi]))
					return 0;
			}
		}
	}

	printf("[uboot] extlinux scan failed\n");
	lcd_printf("extlinux failed");
	return CMD_RET_FAILURE;
}

U_BOOT_CMD(
	extlinux_scan, 1, 0, do_extlinux_scan,
	"scan likely mmc boot partitions for extlinux.conf with LCD diagnostics",
	""
);
