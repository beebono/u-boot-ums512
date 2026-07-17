/*
 * Copyright 2008 - 2009 Windriver, <www.windriver.com>
 * Author: Tom Rix <Tom.Rix@windriver.com>
 *
 * (C) Copyright 2014 Linaro, Ltd.
 * Rob Herring <robh@kernel.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <command.h>

int usb_fastboot_init(void);
int usb_fastboot_exit(void);

int do_fastboot(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int ret;

	ret = usb_fastboot_init();
	if (ret)
		return ret;

	usb_fastboot_exit();
	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	fastboot,	1,	0,	do_fastboot,
	"use USB Fastboot protocol",
	"\n"
	"    - run as a fastboot usb device"
);
