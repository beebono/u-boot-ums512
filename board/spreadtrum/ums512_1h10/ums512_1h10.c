/*
 * (C) Copyright 2014
 * David Feng <fenghua@phytium.com.cn>
 * Sharma Bhupesh <bhupesh.sharma@freescale.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <asm/io.h>
#include <asm/arch/sprd_reg.h>
#include <common.h>
#include <malloc.h>
#include <boot_mode.h>
#include <adi_hal_internal.h>
#include <chipram_env.h>
#include <sprd_adc.h>
#include <sprd_led.h>
#include <sprd_sensor.h>
#include <sprd_battery.h>
#include <sprd_direct_acc_prot.h>
#include <otp_helper.h>
#include <clk.h>
#include <dm.h>
#include <i2c.h>
#include "sensor_board_info.h"

#define ADC_CHANNEL_FOR_NV    3
#define WCN_GPIO	54

DECLARE_GLOBAL_DATA_PTR;
phys_size_t real_ram_size = 0x40000000;

phys_size_t get_real_ram_size(void)
{
        return real_ram_size;
}

void enable_global_clocks(void)
{
	__raw_writel(BIT_AON_APB_MM_EB | BIT_AON_APB_GPU_EB, REG_AON_APB_APB_EB0 + 0x1000);
	while (__raw_readl(REG_PMU_APB_PWR_STATUS3_DBG) & BIT_PMU_APB_PD_MM_TOP_STATE(~0));
	sci_glb_set(REG_MM_AHB_AHB_EB, BIT_MM_AHB_CKG_EB);
}

static void thm_overheate_en(void)
{
	sci_glb_set(REG_AON_APB_OVERHEAT_CTRL, BIT_AON_APB_THM0_OVERHEAT_ALARM_ADIE_EN |
					BIT_AON_APB_THM1_OVERHEAT_ALARM_ADIE_EN |
					BIT_AON_APB_THM2_OVERHEAT_ALARM_ADIE_EN );
}

extern void setup_chipram_env(void);
int board_init(void)
{
#ifdef CONFIG_DIRECT_ACC_PROT
	direct_acc_prot_enable();
#endif
	setup_chipram_env();
#ifndef CONFIG_FPGA
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;
#ifdef CONFIG_CLK
	clk_init();
#endif
	/*config sensorhub*/
#ifdef CONFIG_SENSOR_HUB_UBOOT
	sprd_sensor_init();
#endif
	/*config serial console*/
#ifdef CONFIG_DM_SERIAL
	serial_init();
	console_init_f();
#endif
	ADI_init();
	/*FPGA forbiden*/
	misc_init();
	regulator_init();
	pmic_adc_Init();
	/*FPGA forbiden*/
	pin_init();
	sprd_eic_init();
	sprd_gpio_init();
	sprd_led_init();
	/*FPGA forbiden*/
	sprd_pmu_lowpower_init();
	TDPllRefConfig(1);
	enable_global_clocks();
	thm_overheate_en();
#else
	sci_glb_set(REG_AON_APB_APB_EB1, BIT_AON_APB_ANA_EB);
#endif
	return 0;
}
int dram_init(void)
{
#ifdef CONFIG_DDR_AUTO_DETECT
	ulong sdram_base = CONFIG_SYS_SDRAM_BASE;
	ulong sdram_size = 0;
	chipram_env_t * env = CHIPRAM_ENV_LOCATION;
	if (CHIPRAM_ENV_MAGIC != env->magic) {
		printf("Chipram magic wrong , ddr data may be broken\n");
		return 0;
	}

	real_ram_size = 0;

	if (env->cs_number == 1) {
		real_ram_size += env->cs0_size;
		debugf("dram cs0 size %x\n",env->cs0_size);
	} else if(env->cs_number == 2){
		real_ram_size += env->cs0_size;
		real_ram_size += env->cs1_size;
		debugf("dram cs0 size %x\ndram cs1 size %x\n",env->cs0_size, env->cs1_size);
	}

	//real_ram_size = get_ram_size((volatile void *)sdram_base, real_ram_size);
#else
	real_ram_size = REAL_SDRAM_SIZE;
#endif

	gd->ram_size = PHYS_SDRAM_1_SIZE;

	return 0;
}

#ifdef CONFIG_DUAL_DDR
void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = PHYS_SDRAM_2_SIZE;
}
#endif


int misc_init_r(void)
{
	/*reserver for future use*/
	return 0;
}

#define AW32257_I2C_BUS		4
#define AW32257_ADDR		0x6a
#define AW32257_REG_CONTROL	0x01
#define AW32257_REG_VOLTAGE	0x02
#define AW32257_REG_CURRENT	0x04
#define AW32257_REG_SAFETY	0x06
#define AW32257_SAFETY_LIMITS	0xb7
#define AW32257_VOLTAGE_CFG	((42 << 2) | 0x02)
#define AW32257_CURRENT_CFG	((5 << 3) | 2)
#define AW32257_CONTROL_CFG	0x38
#define AW32257_CEN_BIT		BIT(2)	/* REG01[2]: 1 disables charging */
#define BAT_NTC_ADC_CHANNEL	0	/* same channel the kernel calls bat-temp */
#define BAT_NTC_SAMPLES		15
#define BAT_TEMP_INVALID	(-30000)
#define BAT_TEMP_MIN_DC		0	/* deci-celsius */
#define BAT_TEMP_MAX_DC		600
#define BAT_TEMP_HYST_DC	30

static const struct {
	int uv;
	int temp_dc;
} bat_ntc_table[] = {
	{ 1095000, -200 }, { 986000, -150 }, { 878000, -100 }, { 775000, -50 },
	{  678000,    0 }, { 590000,   50 }, { 510000,  100 }, { 440000, 150 },
	{  378000,  200 }, { 324000,  250 }, { 278000,  300 }, { 238000, 350 },
	{  204000,  400 }, { 175000,  450 }, { 150000,  500 }, { 129000, 550 },
	{  111000,  600 }, {  96000,  650 },
};

static int aw32257_battery_temp_dc(void)
{
	int32_t raw[BAT_NTC_SAMPLES];
	int32_t sum = 0;
	int i, uv;

	if (pmic_adc_get_values(BAT_NTC_ADC_CHANNEL, ADC_SCALE_0,
				BAT_NTC_SAMPLES, raw))
		return BAT_TEMP_INVALID;

	for (i = 0; i < BAT_NTC_SAMPLES; i++)
		sum += raw[i];

	uv = sprd_chan_small_adc_to_vol(BAT_NTC_ADC_CHANNEL, ADC_SCALE_0, 0,
					sum / BAT_NTC_SAMPLES) * 1000;

	if (uv >= bat_ntc_table[0].uv)
		return bat_ntc_table[0].temp_dc;

	for (i = 1; i < ARRAY_SIZE(bat_ntc_table); i++) {
		int hi_uv = bat_ntc_table[i - 1].uv;
		int lo_uv = bat_ntc_table[i].uv;

		if (uv < lo_uv)
			continue;

		return bat_ntc_table[i].temp_dc +
		       (bat_ntc_table[i - 1].temp_dc - bat_ntc_table[i].temp_dc) *
		       (uv - lo_uv) / (hi_uv - lo_uv);
	}

	return bat_ntc_table[ARRAY_SIZE(bat_ntc_table) - 1].temp_dc;
}

static int aw32257_set_charging(int enable)
{
	struct udevice *chg;
	int ret, val;

	ret = i2c_get_chip_for_busnum(AW32257_I2C_BUS, AW32257_ADDR, 1, &chg);
	if (ret)
		return ret;

	val = dm_i2c_reg_read(chg, AW32257_REG_CONTROL);
	if (val < 0)
		return val;

	if (enable)
		val &= ~AW32257_CEN_BIT;
	else
		val |= AW32257_CEN_BIT;

	ret = dm_i2c_reg_write(chg, AW32257_REG_CONTROL, (u8)val);
	return ret < 0 ? ret : 0;
}

int aw32257_temp_guard(int blocked)
{
	int temp = aw32257_battery_temp_dc();
	int min = BAT_TEMP_MIN_DC, max = BAT_TEMP_MAX_DC;
	int block;

	if (temp == BAT_TEMP_INVALID) {
		printf("[uboot] aw32257: battery temperature unreadable, guard inactive\n");
		return blocked;
	}

	if (blocked) {
		min += BAT_TEMP_HYST_DC;
		max -= BAT_TEMP_HYST_DC;
	}

	block = (temp < min || temp > max);

	if (block != blocked) {
		if (aw32257_set_charging(!block)) {
			printf("[uboot] aw32257: failed to %s charging\n",
			       block ? "inhibit" : "resume");
			return blocked;
		}
		printf("[uboot] aw32257: battery %d.%dC, charging %s\n",
		       temp / 10, temp < 0 ? -(temp % 10) : temp % 10,
		       block ? "inhibited" : "resumed");
	}

	return block;
}

int aw32257_battery_temp(void)
{
	return aw32257_battery_temp_dc();
}

static void aw32257_init(void)
{
	struct udevice *chg;
	int ret;

	ret = i2c_get_chip_for_busnum(AW32257_I2C_BUS, AW32257_ADDR, 1, &chg);
	if (ret) {
		printf("[uboot] aw32257: i2c%d addr 0x%02x not found (%d)\n",
		       AW32257_I2C_BUS, AW32257_ADDR, ret);
		return;
	}

	ret = dm_i2c_reg_write(chg, AW32257_REG_SAFETY, AW32257_SAFETY_LIMITS);
	if (ret < 0) {
		printf("[uboot] aw32257: REG06 write failed (%d)\n", ret);
		return;
	}

	ret = dm_i2c_reg_read(chg, AW32257_REG_SAFETY);
	if (ret < 0)
		printf("[uboot] aw32257: REG06 read-back failed (%d)\n", ret);
	else if (ret == AW32257_SAFETY_LIMITS)
		printf("[uboot] aw32257: safety limits set, REG06=0x%02x (4.34V/1984mA)\n",
		       ret);
	else
		printf("[uboot] aw32257: REG06 locked, kept 0x%02x (wanted 0x%02x)\n",
		       ret, AW32257_SAFETY_LIMITS);

	ret = dm_i2c_reg_write(chg, AW32257_REG_VOLTAGE, AW32257_VOLTAGE_CFG);
	if (ret < 0)
		printf("[uboot] aw32257: REG02 write failed (%d)\n", ret);

	ret = dm_i2c_reg_write(chg, AW32257_REG_CURRENT, AW32257_CURRENT_CFG);
	if (ret < 0)
		printf("[uboot] aw32257: REG04 write failed (%d)\n", ret);

	ret = dm_i2c_reg_write(chg, AW32257_REG_CONTROL, AW32257_CONTROL_CFG);
	if (ret < 0)
		printf("[uboot] aw32257: REG01 write failed (%d)\n", ret);

	printf("[uboot] aw32257: charge cfg 4.34V/1240mA (REG02=0x%02x REG04=0x%02x REG01=0x%02x)\n",
	       AW32257_VOLTAGE_CFG, AW32257_CURRENT_CFG, AW32257_CONTROL_CFG);

	aw32257_temp_guard(0);
}

static void battery_init(void)
{
	/* Must run before anything else can touch the charger. */
	aw32257_init();

	sprdchg_common_cfg();
#ifdef CONFIG_SGM41512_CHARGE_IC
	sprdchg_sgm41512_init();
#endif
	sprdbat_init();
}

int board_late_init(void)
{

 boot_mode_t boot_role;
        extern chipram_env_t* get_chipram_env(void);
        chipram_env_t* cr_env = get_chipram_env();
        boot_role = cr_env->mode;

	boot_pwr_check();

#if !defined(CONFIG_FPGA)
#ifdef CONFIG_NAND_BOOT
	//extern int nand_ubi_dev_init(void);
	nand_ubi_dev_init();
	debugf("nand ubi init OK!\n");
#endif
	battery_init();
	debugf("CHG init OK!\n");
#endif
	board_keypad_init();
	return 0;
}


CBOOT_FUNC s_boot_func_array[CHECK_BOOTMODE_FUN_NUM] = {
	get_mode_from_bat_low,
	write_sysdump_before_boot_extend,
	get_mode_from_miscdata_boot_flag,
	/* 1 get mode from file*/
	get_mode_from_file_extend,
	/* 2 get mode from watch dog*/
	get_mode_from_watchdog,
	/*3 get mode from alarm register*/
	get_mode_from_alarm_register,
	/*0 get mode from calibration detect*/
	get_mode_from_pctool,
	/*4 get mode from charger*/
	get_mode_from_charger,
	/*5 get mode from keypad*/
	get_mode_from_keypad,
	/*6 get mode from gpio*/
	get_mode_from_gpio_extend,

	/*shutdown device*/
	//get_mode_from_shutdown,
	0
};



void board_boot_mode_regist(CBOOT_MODE_ENTRY *array)
{
	MODE_REGIST(CMD_NORMAL_MODE, normal_mode);
	MODE_REGIST(CMD_RECOVERY_MODE, recovery_mode);
	MODE_REGIST(CMD_FASTBOOT_MODE, fastboot_mode);
	MODE_REGIST(CMD_WATCHDOG_REBOOT, watchdog_mode);
	MODE_REGIST(CMD_AP_WATCHDOG_REBOOT, ap_watchdog_mode);
	MODE_REGIST(CMD_UNKNOW_REBOOT_MODE, unknow_reboot_mode);
	MODE_REGIST(CMD_PANIC_REBOOT, panic_reboot_mode);
	MODE_REGIST(CMD_AUTODLOADER_REBOOT, autodloader_mode);
	MODE_REGIST(CMD_SPECIAL_MODE, special_mode);
	MODE_REGIST(CMD_CHARGE_MODE, charge_mode);
	MODE_REGIST(CMD_ENGTEST_MODE,engtest_mode);
	/*MODE_REGIST(CMD_FACTORYTEST_MODE, factorytest_mode);*/
	MODE_REGIST(CMD_CALIBRATION_MODE, calibration_mode);
	MODE_REGIST(CMD_EXT_RSTN_REBOOT_MODE, normal_mode);
	MODE_REGIST(CMD_IQ_REBOOT_MODE, iq_mode);
	MODE_REGIST(CMD_ALARM_MODE, alarm_mode);
	MODE_REGIST(CMD_SPRDISK_MODE, sprdisk_mode);
	MODE_REGIST(CMD_AUTOTEST_MODE, autotest_mode);
	MODE_REGIST(CMD_APKMMI_MODE, apkmmi_mode);
	MODE_REGIST(CMD_UPT_MODE, upt_mode);
	MODE_REGIST(CMD_ABNORMAL_REBOOT_MODE, abnormal_reboot_mode);
	return ;
}

static const int wcn_crystal_type[] = {
	0,/*TSX*/
	1/*TCXO*/
};

extern int sprd_gpio_request(struct gpio_chip *chip, unsigned offset);
extern int sprd_gpio_direction_input(struct gpio_chip *chip, unsigned offset);
extern int sprd_gpio_get(struct gpio_chip *chip, unsigned offset);

static unsigned int gpio_state(unsigned int GPIO_NUM)
{
	int value = 0 ;

	sprd_gpio_request(NULL, GPIO_NUM);
	sprd_gpio_direction_input(NULL, GPIO_NUM);
	value = sprd_gpio_get(NULL,GPIO_NUM);

	return value > 0;
}

static int get_wcnid(void)
{
	unsigned int gpio_val = 0;

	gpio_val |= gpio_state(WCN_GPIO);

	return gpio_val;
}

/* Return wcn crystal type */
int sprd_get_wcn_crystal(void)
{
	int id = get_wcnid();

	return wcn_crystal_type[id];
}

static const int adc2deltanv_table[] = {
	-1, /* reserved */
	-1, /* reserved */
	-1, /* reserved */
	-1, /* reserved */
	-1, /* reserved */
	2, /* hw_ver02.nv */
	1, /* hw_ver01.nv */
	0 /* hw_ver00.nv */
};

static int get_adc_value_board(int channel)
{
	int adc_value= 0, vol=0, adc3_vol= 0, level= 0;

	// get ADC3 voltage mv
	adc3_vol = sprd_chan_adc_to_vol(3, 0, 0, 0); // mv

	// get ADC3 current 
      adc_value = pmic_adc_get_value_by_isen(channel,0,15,2000);
      vol = sprd_chan_small_adc_to_vol(channel, 0 , 0 ,adc_value);
	   
	debugf("The adc3_vol voltage is = %d, vol = %d\n", adc3_vol, vol);

	/*
		1.latam
			latam voltage 100k 47k adc_vol=647
			latam current isen=20uA 0k 47k  adc_vol=940
		2.row
			row voltage 100k 56k adc3_vol=643
			row current isen=20uA 0k 56k  adc_vol=1120
		3.wifionly
			row voltage 100k 20k adc_vol=302
			row current isen=20uA 0k 20k  adc_vol=400
	*/	
	if (adc3_vol <= 0 || adc3_vol >= 1200 || vol <= 0 || vol >= 1300) {
		errorf("adc3_vol or vol is out of ranger [0~1200]\n");
		return -1;
	}

	//wifionly 20k 0.3v  302 
	if ((adc3_vol >= 260 && adc3_vol <= 350) || (vol >= 350 && vol <=450))
	{
		level = 5;
		debugf("adc3_vol: %d, vol: %d, ver: wifionly\n", adc3_vol, vol);
	}
	// latam   47k 0.576v 575
	if ((adc3_vol >= 520 && adc3_vol <= 600) ||  (vol >= 890 && vol <=1000))
	{
		level = 7;
		debugf("adc3_vol: %d, vol: %d, ver: latam\n", adc3_vol, vol);
	}
	// row     56k 0.646v 643
	if ((adc3_vol >= 601 && adc3_vol <= 700) || (vol >= 1050 && vol <=1200))
	{
		level = 6;
		debugf("adc3_vol: %d, vol: %d, ver: row\n", adc3_vol, vol);
	}

	if(level < 0 || level > 7) {
		errorf("Adc value of rf band if is wrong, the rfboard.id will be set to -1\n");
		return -1;
	}

	return adc2deltanv_table[level];
}

/* Return rf band info */
int sprd_get_bandinfo(void)
{
	unsigned int adc_val = 0;

	adc_val = get_adc_value_board(ADC_CHANNEL_FOR_NV);
	debugf("rfboard.id = %d\n", adc_val);

	return adc_val;
}

#ifdef CONFIG_SENSOR_HUB_UBOOT
int sprd_sensor_bandlist(struct sensor_phypara **sensorlist)
{
	*sensorlist = sensor_board_phylist;
	return sizeof(sensor_board_phylist)/sizeof(sensor_board_phylist[0]);
}
#endif

/* check wifi only thought adc/gpio */
int sprd_get_wifi_mode(void)
{
	int boardid = 0;
/*modified according to the actual hardware*/

	// add by zhong start
	int adc_value=0, vol=0, adc33_vol = 0,level= 0;

	// get ADC3 voltage mv
	adc33_vol = sprd_chan_adc_to_vol(3, 0, 0, 0); // mv

	// get ADC3 current 
    adc_value = pmic_adc_get_value_by_isen(3,0,15,2000);
    vol = sprd_chan_small_adc_to_vol(3, 0 , 0 ,adc_value);

	debugf("The adc33_vol voltage is = %d, vol: %d\n", adc33_vol, vol);

	if (adc33_vol <= 0 || adc33_vol >= 1200 || vol <= 0 || vol >= 1300) {
		errorf("adc33_vol or vol  is out of ranger [0~1200]\n");
		return -1;
	}

	if ((adc33_vol >= 260 && adc33_vol <= 350) || (vol >= 350 && vol <=450))
	{
		debugf("adc33_vol: %d, vol: %d, ver: wifionly\n", adc33_vol, vol);
		return 1;
	}
  	else {
		debugf("adc33_vol: %d, vol: %d, ver: not wifionly\n", adc33_vol, vol);
		return 0;
	}
	// add by zhong end

#ifdef CONFIG_SSMH_WIFI_MODE
	boardid = get_adc_value_board();
	if (boardid == 0) {
		return 1;
	} else {
		return 0;
	}
#else
/* fixed to Wifionly */
#ifdef PRODUCT_WIFI_ONLY
	return 1;
#else
/* default setting */
	return 0;
#endif
#endif
}

/* check data only thought adc/gpio */
int sprd_get_data_mode(void)
{
	int boardid = 0;
/*modified according to the actual hardware*/
#ifdef CONFIG_SSMH_DATA_MODE
	boardid = get_adc_value_board();
	if (boardid == 1) {
		return 1;
	} else {
		return 0;
	}
#else
/* fixed to dataonly */
#ifdef PRODUCT_DATA_ONLY
	return 1;
#else
/* default setting */
	return 0;
#endif
#endif
}

/* check gpio/adc return “SINGLESIM”or “DUALSIM” */
char sprd_get_sim(void)
{
	int boardid = 0;
#ifdef CONFIG_SSMH_SIM_MODE
	boardid = get_adc_value_board();// modified according to the actual hardware
	if (boardid == 3) {
		return 1;
	} else {
		return 0;
	}
#else
/* fixed to SINGLESIM */
#ifdef PRODUCT_SINGLE_SIM
	return 1;
#else
/* default setting */
	return 0;
#endif
#endif
}
