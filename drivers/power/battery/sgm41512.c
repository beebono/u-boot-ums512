
#include "sprd_chg_helper.h"
#include <dm.h>
#include <errno.h>
#include <common.h>
#include <fdtdec.h>
#include <asm/io.h>
#include <i2c.h>
#include <clk.h>

/******************************************************************************
* Register addresses
******************************************************************************/
#define SGM41512_CON0      0x00
#define SGM41512_CON1      0x01
#define SGM41512_CON2      0x02
#define SGM41512_CON3      0x03
#define SGM41512_CON4      0x04
#define SGM41512_CON5      0x05
#define SGM41512_CON6      0x06
#define SGM41512_CON7      0x07
#define SGM41512_CON8      0x08
#define SGM41512_CON9      0x09
#define SGM41512_CON10      0x0A
#define	SGM41512_CON11		0x0B
#define SGM41512_REG_NUM 12


/******************************************************************************
* Register bits
******************************************************************************/
	//CON0
#define CON0_EN_HIZ_MASK   0x01
#define CON0_EN_HIZ_SHIFT  7
	
#define	CON0_STAT_IMON_CTRL_MASK	0x03
#define	CON0_STAT_IMON_CTRL_SHIFT 5
	
#define CON0_IINLIM_MASK   0x1F
#define CON0_IINLIM_SHIFT  0
	
	//CON1
#define CON1_PFM_MASK     0x01
#define CON1_PFM_SHIFT    7
	
#define CON1_WDT_RST_MASK     0x01
#define CON1_WDT_RST_SHIFT    6
	
#define CON1_OTG_CONFIG_MASK	0x01
#define CON1_OTG_CONFIG_SHIFT	5
	
#define CON1_CHG_CONFIG_MASK        0x01
#define CON1_CHG_CONFIG_SHIFT       4
	
#define CON1_SYS_MIN_MASK        0x07
#define CON1_SYS_MIN_SHIFT       1
	
#define	CON1_MIN_VBAT_SEL_MASK	0x01
#define	CON1_MIN_VBAT_SEL_SHIFT	0
	
	//CON2
#define CON2_BOOST_LIM_MASK   0x01
#define CON2_BOOST_LIM_SHIFT  7
	
#define	CON2_Q1_FULLON_MASK		0x01
#define	CON2_Q1_FULLON_SHIFT	6
	
#define CON2_ICHG_MASK    0x3F
#define CON2_ICHG_SHIFT   0
	
	//CON3
#define CON3_IPRECHG_MASK   0x0F
#define CON3_IPRECHG_SHIFT  4
	
#define CON3_ITERM_MASK           0x0F
#define CON3_ITERM_SHIFT          0
	
	//CON4
#define CON4_VREG_MASK     0x1F
#define CON4_VREG_SHIFT    3
	
#define	CON4_TOPOFF_TIMER_MASK 0x03
#define	CON4_TOPOFF_TIMER_SHIFT 1
	
#define CON4_VRECHG_MASK    0x01
#define CON4_VRECHG_SHIFT   0
	
	//CON5
#define CON5_EN_TERM_MASK      0x01
#define CON5_EN_TERM_SHIFT     7
	
#define CON5_WATCHDOG_MASK     0x03
#define CON5_WATCHDOG_SHIFT    4
	
#define CON5_EN_TIMER_MASK      0x01
#define CON5_EN_TIMER_SHIFT     3
	
#define CON5_CHG_TIMER_MASK           0x01
#define CON5_CHG_TIMER_SHIFT          2
	
#define CON5_TREG_MASK     0x01
#define CON5_TREG_SHIFT    1
	
	
	//CON6
#define	CON6_OVP_MASK		0x03
#define	CON6_OVP_SHIFT		6
	
#define	CON6_BOOSTV_MASK	0x3
#define	CON6_BOOSTV_SHIFT	4
	
#define	CON6_VINDPM_MASK	0x0F
#define	CON6_VINDPM_SHIFT	0
	
	//CON7
#define	CON7_FORCE_DPDM_MASK	0x01
#define	CON7_FORCE_DPDM_SHIFT	7
	
#define CON7_TMR2X_EN_MASK      0x01
#define CON7_TMR2X_EN_SHIFT     6
	
#define CON7_BATFET_Disable_MASK      0x01
#define CON7_BATFET_Disable_SHIFT     5
	
#define	CON7_BATFET_DLY_MASK		0x01
#define	CON7_BATFET_DLY_SHIFT		3
	
#define	CON7_BATFET_RST_EN_MASK		0x01
#define	CON7_BATFET_RST_EN_SHIFT	2
	
#define	CON7_VDPM_BAT_TRACK_MASK	0x03
#define	CON7_VDPM_BAT_TRACK_SHIFT	0
	
	//CON8
#define CON8_VBUS_STAT_MASK      0x07
#define CON8_VBUS_STAT_SHIFT     5
	
#define CON8_CHRG_STAT_MASK           0x03
#define CON8_CHRG_STAT_SHIFT          3
	
#define CON8_PG_STAT_MASK           0x01
#define CON8_PG_STAT_SHIFT          2
	
#define CON8_THERM_STAT_MASK           0x01
#define CON8_THERM_STAT_SHIFT          1
	
#define CON8_VSYS_STAT_MASK           0x01
#define CON8_VSYS_STAT_SHIFT          0
	
	//CON9
#define CON9_WATCHDOG_FAULT_MASK      0x01
#define CON9_WATCHDOG_FAULT_SHIFT     7
	
#define CON9_OTG_FAULT_MASK           0x01
#define CON9_OTG_FAULT_SHIFT          6
	
#define CON9_CHRG_FAULT_MASK           0x03
#define CON9_CHRG_FAULT_SHIFT          4
	
#define CON9_BAT_FAULT_MASK           0x01
#define CON9_BAT_FAULT_SHIFT          3
	
#define CON9_NTC_FAULT_MASK           0x07
#define CON9_NTC_FAULT_SHIFT          0
	
	//CON10
#define	CON10_VBUS_GD_MASK				0x01
#define	CON10_VBUS_GD_SHIFT				7
	
#define	CON10_VINDPM_STAT_MASK			0x01
#define	CON10_VINDPM_STAT_SHIFT			6
	
#define	CON10_IINDPM_STAT_MASK			0x01
#define	CON10_IINDPM_STAT_SHIFT			5
	
#define	CON10_TOPOFF_ACTIVE_MASK		0x01
#define	CON10_TOPOFF_ACTIVE_SHIFT		3
	
#define	CON10_ACOV_STAT_MASK			0x01
#define	CON10_ACOV_STAT_SHIFT			2
	
#define	CON10_VINDPM_INT_MASK			0x01
#define	CON10_VINDPM_INT_SHIFT			1
	
#define	CON10_INT_MASK_MASK				0x03
#define	CON10_INT_MASK_SHIFT			0
	
	//CON11
#define CON11_REG_RST_MASK     0x01
#define CON11_REG_RST_SHIFT    7
	
	
#define CON11_PN_MASK		0x0F
#define CON11_PN_SHIFT		3
	
#define CON11_Rev_MASK           0x03
#define CON11_Rev_SHIFT          0


#define I2C_SPEED			(100000)
#define SLAVE_ADDR 		(0x6b)

#define GETARRAYNUM(array) (ARRAY_SIZE(array))
/*BQ25601 REG04 ICHG[6:0]*/
const unsigned int CS_VTH[] = {
	0, 6000, 12000, 18000, 24000,
	30000, 36000, 42000, 48000, 54000,
	60000, 66000, 72000, 78000, 84000,
	90000, 96000, 102000, 108000, 114000,
	120000, 126000, 132000, 138000, 144000,
	150000, 156000, 162000, 168000, 174000,
	180000, 186000, 192000, 198000, 204000,
	210000, 216000, 222000, 228000
};
const unsigned int INPUT_CS_VTH[] = {
	10000, 20000, 30000, 40000,
	50000, 60000, 70000, 80000,
	90000, 100000, 110000, 120000,
	130000, 140000, 150000, 160000,
	170000, 180000, 190000, 200000,
	210000, 220000, 230000, 250000,
	260000, 270000, 280000, 290000,
	300000, 310000, 320000
};

unsigned char sgm41512_get_vendor_id(void);

#ifdef CONFIG_DM_SGM41512
struct sprd_sgm41512_dm_data {
	int i2c_num;
};

static struct sprd_sgm41512_dm_data dm_data = {
	.i2c_num = -1,
};

struct udevice *charger;

static int sprd_dm_sgm41512_i2c_init(void)
{
	int ret;
	u8 value;

	if (dm_data.i2c_num != -1) {
		ret = i2c_get_chip_for_busnum(dm_data.i2c_num,
					      SLAVE_ADDR, 1,
					      &charger);
		if (ret) {
			pr_err("%s: i2c%d failed to get\n",
			       __func__, dm_data.i2c_num);
			return ret;
		}
	} else {
		pr_err("%s:failed to set i2c bus num\n", __func__);
		return -EINVAL;
	}

	ret = dm_i2c_set_bus_speed(dev_get_parent(charger), I2C_SPEED);
	if (ret) {
		pr_err("%s: failed to set i2c%d speed\n",
		       __func__, dm_data.i2c_num);
		return ret;
	}
	return 0;
}

static int sgm41512_write_reg(int reg, u8 val)
{
	dm_i2c_reg_write(charger, reg, val);
	return 0;
}

static int sgm41512_read_reg(int reg, u8 *value)
{
	u8 val;
	int ret;

	ret = dm_i2c_reg_read(charger, reg);
	if (ret < 0) {
		pr_err("%s: reg(0x%x), ret(%d)\n", __func__, reg, ret);
		return ret;
	}

	*value = (u8)ret;
	debugf("######sgm41512readreg reg = %d value = %d/%x\n", reg, ret, ret);
	return 0;
}
static int sgm41512_update_bits(u8 reg, u8 mask, u8 data, u8 shift)
{
	u8 val;
	int ret;

	ret = sgm41512_read_reg(reg, &val);
	if (ret < 0){
		debugf("###################### sgm41512_read error !!!!!!!!!######################\n");
		return ret;
	}
	val &= ~(mask << shift);
	val |= (data << shift);
	return sgm41512_write_reg(reg, val);
}

void SGM41512_set_en_hiz(u32 val)
{
	unsigned int ret = 0;
	ret = sgm41512_update_bits(SGM41512_CON0,
						CON0_EN_HIZ_MASK,
						val, CON0_EN_HIZ_SHIFT);
}
void SGM41512_set_chg_config(u32 val)
{
	unsigned int ret = 0;
	ret = sgm41512_update_bits(SGM41512_CON1,
						CON1_CHG_CONFIG_MASK,
						val, CON1_CHG_CONFIG_SHIFT);
}

void SGM41512_set_vindpm(u32 val)
{
	unsigned int ret = 0;
	ret = sgm41512_update_bits(SGM41512_CON6,
							CON6_VINDPM_MASK,
							val, CON6_VINDPM_SHIFT);
}
void SGM41512_set_reg_rst(u32 val)
{
	unsigned int ret = 0;
	ret = sgm41512_update_bits(SGM41512_CON11,
							CON11_REG_RST_MASK,
							val, CON11_REG_RST_SHIFT);
}

void SGM41512_set_wdt_rst(u32 val)
{
	unsigned int ret = 0;
	ret = sgm41512_update_bits(SGM41512_CON1,
								CON1_WDT_RST_MASK,
								val, CON1_WDT_RST_SHIFT);
}
void SGM41512_set_sys_min(u32 val)
{
	unsigned int ret = 0;
	ret = sgm41512_update_bits(SGM41512_CON1,
									CON1_SYS_MIN_MASK,
									val, CON1_SYS_MIN_SHIFT);
}
void SGM41512_set_iprechg(u32 val)
{
	unsigned int ret = 0;
	ret = sgm41512_update_bits(SGM41512_CON3,
									CON3_IPRECHG_MASK,
									val, CON3_IPRECHG_SHIFT);
}
void SGM41512_set_iterm(u32 val)
{
	unsigned int ret = 0;
	ret = sgm41512_update_bits(SGM41512_CON3,
										CON3_ITERM_MASK,
										val, CON3_ITERM_SHIFT);
}
void SGM41512_set_vreg(u32 val)
{
	unsigned int ret = 0;
	ret = sgm41512_update_bits(SGM41512_CON4,
										CON4_VREG_MASK,
										val, CON4_VREG_SHIFT);
}
void SGM41512_set_pfm(u32 val)
{
	unsigned int ret = 0;
	ret = sgm41512_update_bits(SGM41512_CON1,
										CON1_PFM_MASK,
										val, CON1_PFM_SHIFT);
}
void SGM41512_set_rdson(u32 val)
{
	unsigned int ret = 0;
	ret = sgm41512_update_bits(SGM41512_CON2,
										CON2_Q1_FULLON_MASK,
										val, CON2_Q1_FULLON_SHIFT);
}
void SGM41512_set_batlowv(u32 val)
{
	unsigned int ret = 0;
	ret = sgm41512_update_bits(SGM41512_CON1,
										CON1_MIN_VBAT_SEL_MASK,
										val, CON1_MIN_VBAT_SEL_SHIFT);
}
void SGM41512_set_vrechg(u32 val)
{
	unsigned int ret = 0;
	ret = sgm41512_update_bits(SGM41512_CON4,
										CON4_VRECHG_MASK,
										val, CON4_VRECHG_SHIFT);
}
void SGM41512_set_en_term(u32 val)
{
	unsigned int ret = 0;
	ret = sgm41512_update_bits(SGM41512_CON5,
										CON5_EN_TERM_MASK,
										val, CON5_EN_TERM_SHIFT);
}
void SGM41512_set_watchdog(u32 val)
{
	unsigned int ret = 0;
	ret = sgm41512_update_bits(SGM41512_CON5,
										CON5_WATCHDOG_MASK,
										val, CON5_WATCHDOG_SHIFT);
}
void SGM41512_set_en_timer(u32 val)
{
	unsigned int ret = 0;
	ret = sgm41512_update_bits(SGM41512_CON5,
										CON5_EN_TIMER_MASK,
										val, CON5_EN_TIMER_SHIFT);
}
void SGM41512_set_int_mask(u32 val)
{
	unsigned int ret = 0;
	ret = sgm41512_update_bits(SGM41512_CON10,
										CON10_INT_MASK_MASK,
										val, CON10_INT_MASK_SHIFT);
}
void sgm41512_set_ovp(u32 val)
{
	unsigned int ret = 0;
	ret = sgm41512_update_bits(SGM41512_CON6,
										CON6_OVP_MASK,
										val, CON6_OVP_SHIFT);
}

void SGM41512_set_ichg(u32 val)
{
	unsigned int ret = 0;
	ret = sgm41512_update_bits(SGM41512_CON2,
									CON2_ICHG_MASK,
									val, CON2_ICHG_SHIFT);
}
void sgm41512_set_iinlim(u32 val)
{
	unsigned int ret = 0;
	ret = sgm41512_update_bits(SGM41512_CON0,
									CON0_IINLIM_MASK,
									val, CON0_IINLIM_SHIFT);
}

#else
uint16_t sprd_sgm41512_i2c_init(void)
{
	i2c_set_bus_num(CONFIG_SPRDCHG_I2C_BUS);
	i2c_init(I2C_SPEED, SLAVE_ADDR);
	return 0;
}
static int sgm41512_write_reg(int reg, u8 val)
{
	i2c_reg_write(SLAVE_ADDR,reg,val);
   	return 0;
}

int sgm41512_read_reg(int reg, u8 *value)
{
	int ret;
	ret = i2c_reg_read(SLAVE_ADDR , reg);
	if (ret < 0) {
		debugf("%s reg(0x%x), ret(%d)\n", __func__, reg, ret);
		return ret;
	}

	ret &= 0xff;
	*value = ret;
	debugf("######sgm41512readreg reg  = %d value =%d/%x\n",reg, ret, ret);
	return 0;
}
#endif

static void sgm41512_set_value(BYTE reg, BYTE reg_bit,BYTE reg_shift, BYTE val)
{
	BYTE tmp;
	int ret;

	ret = sgm41512_read_reg(reg, &tmp);
	if (ret < 0)
		return;

	tmp = tmp & (~reg_bit) |(val << reg_shift);

	sgm41512_write_reg(reg,tmp);
}

static BYTE sgm41512_get_value(BYTE reg, BYTE reg_bit, BYTE reg_shift)
{
	BYTE data = 0;
	BYTE ret = 0 ;
	ret = sgm41512_read_reg(reg, &data);
	ret = (data & reg_bit) >> reg_shift;

	return ret;
}
#ifdef CONFIG_FASTBOOT_SECURITY_DOWNLOAD
extern bool get_hmd_configs();
extern int s_is6000F;
extern uint32_t sprdfgu_read_vbat_vol(void);
#endif
void sprdchg_sgm41512_start_chg(int type)
{
#ifdef CONFIG_FASTBOOT_SECURITY_DOWNLOAD
	unsigned int vbat_vol = sprdfgu_read_vbat_vol();
	get_hmd_configs();
	printf("%s: s_is6000F = %d, vbat_vol = %d!\n", __func__, s_is6000F, vbat_vol);
	if( s_is6000F && vbat_vol > 3600){
		printf("## not charge in uboot!");
	}else

#endif
	{
		printf("## charge in uboot!");
		sprd_charge_pd_control(true);
		SGM41512_set_en_hiz(0x0);
		SGM41512_set_chg_config(1);
	}
}
 void sprdchg_sgm41512_stop_charging(int value)
{
	printf("stop charge\n");
	sprd_charge_pd_control(false);
}
void sprdchg_sgm41512_ic_init(void)
{
	unsigned char vendor_id;
	vendor_id = sgm41512_get_vendor_id();
	int ret;

#ifdef CONFIG_DM_SGM41512
	ret = sprd_dm_sgm41512_i2c_init();
#else
	ret = sprd_sgm41512_i2c_init();
#endif
	if (ret) {
		pr_err("%s:sgm41512 i2c init fail\n", __func__);
		return;
	}
	SGM41512_set_en_hiz(0x0);
	SGM41512_set_vindpm(0x4); //VIN DPM check 4.5V
	SGM41512_set_reg_rst(0x0);
	SGM41512_set_wdt_rst(0x1); //Kick watchdog
	SGM41512_set_sys_min(0x5); //Minimum system voltage 3.5V
	SGM41512_set_iprechg(0x7); //Precharge current 480mA
	SGM41512_set_iterm(0x4); //Termination current 180mA
	SGM41512_set_vreg(0x11); //CV=4.4V
	SGM41512_set_batlowv(0x0); //BATLOWV 3.0V
	SGM41512_set_vrechg(0x0); //VRECHG 0.1V (4.108V)
	SGM41512_set_en_term(0x1); //Enable termination	
	SGM41512_set_watchdog(0x0); //WDT 40s
	SGM41512_set_en_timer(0x0); //Disable charge timer
	SGM41512_set_int_mask(0x0); //Disable fault interrupt
	sgm41512_set_ovp(0x01);
}

void sprdchg_sgm41512_reset_timer(void)
{
	printf("sgm41512 reset rimer\n");
	//sgm41512_set_value(FAN5405_REG_CONTROL0, FAN5405_TMR_RST_OTG,FAN5405_TMR_RST_OTG_SHIFT, RESET32S);
}

unsigned char sgm41512_get_vendor_id(void)
{
	return sgm41512_get_value(SGM41512_CON11,
				  CON11_PN_MASK,
				  CON11_PN_SHIFT);
}
static unsigned int bmt_find_closest_level(const unsigned int *pList,
		unsigned int number,
		unsigned int level)
{
	unsigned int i;
	unsigned int max_value_in_last_element;

	if (pList[0] < pList[1])
		max_value_in_last_element = 1;
	else
		max_value_in_last_element = 0;

	if (max_value_in_last_element == 1) {
		for (i = (number - 1); i != 0;
		     i--) {	/* max value in the last element */
			if (pList[i] <= level) {
				printf("zzf_%d<=%d, i=%d\n",
					pList[i], level, i);
				return pList[i];
			}
		}

		pr_info("Can't find closest level\n");
		return pList[0];
		/* return CHARGE_CURRENT_0_00_MA; */
	} else {
		/* max value in the first element */
		for (i = 0; i < number; i++) {
			if (pList[i] <= level)
				return pList[i];
		}

		pr_info("Can't find closest level\n");
		return pList[number - 1];
		/* return CHARGE_CURRENT_0_00_MA; */
	}
}
static unsigned int charging_parameter_to_value(const unsigned int
		*parameter, const unsigned int array_size,
		const unsigned int val)
{
	unsigned int i;

	printf("array_size = %d\n", array_size);

	for (i = 0; i < array_size; i++) {
		if (val == *(parameter + i))
			return i;
	}

	pr_info("NO register value match\n");
	/* TODO: ASSERT(0);    // not find the value */
	return 0;
}

void sprdchg_sgm41512_set_cur(uint32_t current_value)
{

	unsigned int set_chr_current;
	unsigned int array_size;
	unsigned int register_value;

	debugf("charge_current_value = %d\n", current_value);
	current_value /= 10;
	array_size = GETARRAYNUM(CS_VTH);
	set_chr_current = bmt_find_closest_level(CS_VTH, array_size,
			  current_value);
	register_value = charging_parameter_to_value(CS_VTH, array_size,
			 set_chr_current);
	//pr_info("&&&& charge_register_value = %d\n",register_value);
	debugf("%s register_value = %d\n", __func__,
		register_value);
	SGM41512_set_ichg(register_value);
	return 0;
}

static int
sgm41512_charger_set_limit_current(u32 current_value)
{
	unsigned int set_chr_current;
	unsigned int array_size;
	unsigned int register_value;

	current_value /= 10;
	pr_info("#### limit current = %d\n", current_value);
	array_size = GETARRAYNUM(INPUT_CS_VTH);
	set_chr_current = bmt_find_closest_level(INPUT_CS_VTH, array_size,
			  current_value);
	register_value = charging_parameter_to_value(INPUT_CS_VTH, array_size,
			 set_chr_current);
	pr_info("####  limit reg value = %d\n",	register_value);
	sgm41512_set_iinlim(0x1f);

	return 0;
}

static int sprdchg_sgm41512_cmd(enum sprdchg_cmd cmd,int value)
{
	switch (cmd) {
	case CHG_SET_LIMIT_CURRENT:
		sgm41512_charger_set_limit_current(value);
		break;
	case CHG_SET_CURRENT:
		sprdchg_sgm41512_set_cur(value);
		break;
	default:
		break;
	}

	return 0;
}

static struct sprdchg_ic_operations sprd_extic_op ={
	.ic_init = sprdchg_sgm41512_ic_init,
	.chg_start = sprdchg_sgm41512_start_chg,
	.chg_stop = sprdchg_sgm41512_stop_charging,
	.timer_callback = sprdchg_sgm41512_reset_timer,
	.chg_cmd = sprdchg_sgm41512_cmd,
};

#ifdef CONFIG_DM_SGM41512
static int sprd_sgm41512_probe(struct udevice *dev)
{
	u32 i2c_id;
	int ret;

	ret = dev_read_u32(dev, "sprd,sgm41512-i2c-bus", &i2c_id);
	if (ret) {
		pr_err("%s:failed to get i2c-bus ret = %d!\n", __func__, ret);
		return ret;
	}
	debugf("i2c_id = %d\n", i2c_id);
	dm_data.i2c_num = i2c_id;
	return 0;
}

static int sprd_dm_sgm41512_init(void)
{
	struct udevice *devp;
	int ret;

	ret = uclass_get_device(UCLASS_CHARGER, 0, &devp);
	if (ret) {
		pr_err("%s:failed to get device ret = %d", __func__, ret);
		return ret;
	}
	sprd_dm_sgm41512_i2c_init();
	return 0;
}
#endif

void sprdchg_sgm41512_init(void)
{
	BYTE data = 0;
	unsigned char vendor_id = 0;
	int ret;

	printf("fan5405init\n");

#ifdef CONFIG_DM_SGM41512
		ret = sprd_dm_sgm41512_init();
#else
		ret = sprd_sgm41512_i2c_init();
#endif
	if (ret) {
		pr_err("sgm41512 i2c init failed\n");
		return;
	}
	printf("sgm41512 register charge ops!\n");
	sprdchg_register_ops(&sprd_extic_op);
}

#ifdef CONFIG_DM_SGM41512
static const struct udevice_id sprd_sgm41512_ids[] = {
	{.compatible = "sprd,sgm41512-charger"},
	{ }
};

U_BOOT_DRIVER(sgm41512) = {
	.name = "sprd-sgm41512",
	.id = UCLASS_CHARGER,
	.of_match = sprd_sgm41512_ids,
	.probe = sprd_sgm41512_probe,
};

UCLASS_DRIVER(charger)= {
	.name = "charger",
	.id = UCLASS_CHARGER,
};
#endif

