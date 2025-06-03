/*
 *MISCDATA_DEF_H - use field defined on partition of 'miscdata'
 *
 *  Copyright (C) 2021 Unisoc Communications Inc.
 *  History:
 *      Tue Feb 9 10:10:36 2021 zhenxiong.lai
 *      Description
 *        
 */

#ifndef __MISCDATA_DEF_H__
#define __MISCDATA_DEF_H__

/*
 *Verified Boot功能
 *(8K~8K+512)
 *uboot读取miscdata专用
 *(9K~9K+1K) 配置uboot读取loglevel用          	9K~9K+32
 *           配置uboot读取开机模式信息用      			9K+32~9K+64
 *           存放sysdump 开关用         	    	9K+64~9K+288
 *           uboot读取flag专用                  9k+288~9k+352
 *           uboot读取selinux权限检查开关     		9k+352~9k+383
 *           存储虚拟board id用于匹配dtbo     		9k+384~9k+388
 *           下载时获取手机里的版本号用       			9K+512~9K+768
 *           uboot保存时间戳物料判定用        		9K+768~9K+800
 *           记录watchdog使能菜单           		9K+800~9K+832
 *客户自定义区
 *768K- 1024K
 */
#define MISCDATA_VERIFIED_BOOT		(8 * 1024)
#define PDT_INFO_LOCK_FLAG_OFFSET 	(MISCDATA_VERIFIED_BOOT)

/** UBOOT 专用区域 */
#define MISCDATA_UBOOT_BASE			(9 * 1024)

/* loglevel */
#define DEBUG_INFO_OFFSET			MISCDATA_UBOOT_BASE
#define DEBUG_INFO_LEN    			(32)

/* first mode */
#define SET_FIRST_MODE_MAGIC 		(0x53464d00)
#define SET_FIRST_MODE_OFFSET 		(MISCDATA_UBOOT_BASE + 32)
#define SET_FIRST_MDOE_LEN 			(0x4)

/* virtual board id */
#define SET_VIRTUAL_BOARD_ID_MAGIC	(0x5AA50000)
#define SET_VIRTUAL_BOARD_ID_OFFSET	(MISCDATA_UBOOT_BASE + 384)
#define SET_VIRTUAL_BOARD_ID_LEN	(0x4)

/* core0 startup flag */
#define CORE_STARTUP_FLAG_OFFSET	(MISCDATA_UBOOT_BASE + 388)
#define CORE_STARTUP_FLAG_LEN		(0x2)

/* mode startup block flags */
#define STARTUP_BLOCK_FLAG_OFFSET	(MISCDATA_UBOOT_BASE + 390)
#define STARTUP_BLOCK_FLAG_LEN		(0x4)

/*wdt enable flags */
#define WDTEN_MAGIC			(0xe551)
#define WDTEN_DATA_OFFSET		(MISCDATA_UBOOT_BASE + 800)
#define WDTEN_DATA_LEN			(0x20)

/*deepsleep wdt enable flags */
#define DSWDTEN_DATA_OFFSET             (MISCDATA_UBOOT_BASE + 832)
#define DSWDTEN_DATA_LEN		(0x20)

/* sysdump */
#define MISCDATA_DUMP_DATA_START 	 (9 * 1024 + 64)///fixme
#define FULLDUMP_PARTITION_MAGIC_LEN (10)


/* usr base */
#define MISCDATA_USR_BASE  		(512 * 1024)

/* ADC数据存储 */
#define ADC_DATA_OFFSET  		(MISCDATA_USR_BASE)
#define ADC_DATA_START  		(ADC_DATA_OFFSET)

/* 保存pac的创建时间 */
#define DATETIME_OFFSET 		(MISCDATA_USR_BASE + 5120)
#define DATETIME_LEN			(0x400)

#endif /* __MISCDATA_DEF_H__ */

