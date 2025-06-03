#include <common.h>
#include <malloc.h>
#include "key_map.h"
#include <boot_mode.h>
#include <asm/arch/mfp.h>
#include <asm/arch/common.h>
#include <asm/arch/sprd_eic.h>

#define SPRD_VOLUMEDOWN_GPIO 124

#ifdef CONFIG_FASTBOOT_SECURITY_DOWNLOAD
extern int fb_check_secboot_enable(void);
#ifdef CONFIG_ERASE_SPL_AUTO_DOWNLOAD
extern int erase_spl_enter_download_mode(void);
#endif
#endif
#define GPIO_SW_ID3  82
int sw_key = KEY_VOLUMEDOWN;

int lcd_low_bat;
extern int low_bat;

#ifdef CONFIG_GET_HW_VERSION
/*SW_ID1 SW_ID2 SW_ID3*/
int hw_gpio_pin[3] = {38, 39, 82};
int hw_gpio_status[3] = {0};

void get_hw_version_init(void)
{
	int i = 0;

	for(i=0; i<3; i++)
		sprd_gpio_request(NULL, hw_gpio_pin[i]);
}

int get_hw_version(void)
{
	int i = 0;
	for(i=0; i<3; i++)
		hw_gpio_status[i] = sprd_gpio_get(NULL, hw_gpio_pin[i]);

	//debugf("hw %d %d %d\n", hw_gpio_status[0], hw_gpio_status[1], hw_gpio_status[2]);

	if(hw_gpio_status[2] > 0) // hw v1.0
		return 1;
	else
		return 0;
}
#endif

void board_keypad_init(void)
{
	sprd_gpio_request(NULL, SPRD_VOLUMEDOWN_GPIO);
	sprd_gpio_direction_input(NULL, SPRD_VOLUMEDOWN_GPIO);

	sprd_gpio_request(NULL, 124);
	sprd_gpio_direction_input(NULL, 124);
/*
	sprd_gpio_request(NULL, 125);
	sprd_gpio_direction_input(NULL, 125);
*/

#ifdef CONFIG_GET_HW_VERSION
	get_hw_version_init();
	if(get_hw_version() > 0)
		sw_key = KEY_VOLUMEUP;
	else
		sw_key = KEY_VOLUMEDOWN;
#endif

	printf("[gpio keys] init!\n");
	return;
}

unsigned char board_key_scan(void)
{
	uint32_t key_code = KEY_RESERVED;
	int gpio_volumeup = -1;
	int gpio_volumedown = -1;

	sprd_eic_request(EIC_KEY2_7S_RST_EXT_RSTN_ACTIVE);
	udelay(3000);
	gpio_volumeup = sprd_eic_get(EIC_KEY2_7S_RST_EXT_RSTN_ACTIVE);
	debugf("gpio_volumeup = %d\n",gpio_volumeup);
	if(gpio_volumeup < 0)
		errorf("[eic keys] volumeup : sprd_eic_get return ERROR!\n");
	if(gpio_volumeup > 0) {
		key_code = KEY_VOLUMEUP;
		debugf("[eic keys] volumeup pressed!\n");
	 }	

	gpio_volumedown = sprd_gpio_get(NULL, SPRD_VOLUMEDOWN_GPIO);
	debugf("gpio_volumedown = %d\n",gpio_volumedown);
	if(gpio_volumedown < 0)
		errorf("[eic keys] volumedown : sprd_eic_get return ERROR!\n");
	if(gpio_volumedown == 0) {
		key_code = KEY_VOLUMEDOWN;
		debugf("[eic keys] volumedown pressed!\n");
	}

	if (KEY_RESERVED == key_code)
		debugf("[gpio keys] no key pressed!\n");

	return key_code;
}

unsigned int check_key_boot(unsigned char key)
{
    /* Fixme, an example of the combination of keys for enter download */
    if(sw_key == key) {
#if 0
#ifdef CONFIG_FASTBOOT_SECURITY_DOWNLOAD
		debugf("befor fb_check_secboot_enable! low_bat=%d\n", low_bat);
		if (fb_check_secboot_enable()) {
			debugf("after fb_check_secboot_enable, and fb_check_secboot_enable is true!\n");
#ifdef CONFIG_ERASE_SPL_AUTO_DOWNLOAD
			if (0 != erase_spl_enter_download_mode()) {
				debugf("erase partition splloader and splloader_bak fail!\n");
				debugf("enter old autodloader_mode!\n");
			} else
#endif
			{
				errorf("not support!!!\n");
			}
		} else
#endif
#endif
	
	    if(1 == low_bat)
	    {
		lcd_low_bat = 1;
		return 0;	
	    }  
	    return CMD_FASTBOOT_MODE;
    } 
    else {
      debugf("check_key_boot not sw_key (%d)\n", sw_key);
      return 0;
    }
}

