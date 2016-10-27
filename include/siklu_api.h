/*
 * siklu_api.h
 *
 *  Created on: Jun 9, 2016
 *      Author: edwardk
 */

#ifndef INCLUDE_SIKLU_API_H_
#define INCLUDE_SIKLU_API_H_


#define SIKLU_BIST_ENVIRONMENT_NAME "bist"
#define SIKLU_BIST_MUT_ENVIRONMENT_NAME "SK_bist" // siklu_remark002


typedef enum {
	SKL_WIGIG0_RF_RESET,  // edikk: mean RF output?
	SKL_WIGIG1_RF_RESET,  // edikk: mean RF output?
	SKL_WIFI_RESET,
	SKL_BLE_RESET,

	SKL_WIGIG0_CHIP_RESET,
	SKL_WIGIG1_CHIP_RESET,
	SKL_WIGIG2_CHIP_RESET,

	SKL_GPHY_0_RESET,
	SKL_GPHY_1_RESET,
	SKL_GPHY_2_RESET,

} SKL_MODULE_RESET_CNTRL_E;

typedef enum {
    SKL_LED_BLE,
    SKL_LED_WLAN,
    SKL_LED_ETH0,
    SKL_LED_ETH1,
    SKL_LED_ETH2,
    SKL_LED_POWER,
} SKL_BOARD_LED_TYPE_E;

typedef enum {
    SKL_LED_MODE_OFF,
    SKL_LED_MODE_GREEN,
    SKL_LED_MODE_YELLOW,
    SKL_LED_MODE_GREEN_BLINK,
    SKL_LED_MODE_YELLOW_BLINK
}  SKL_BOARD_LED_MODE_E;


// SoC GPIO pins
extern int mvSikluCpuGpioSetVal(int gpio, int val);
extern int mvSikluCpuGpioSetDirection(int gpio, int isOutput);
extern int mvSikluCpuGpioGetVal(int gpio, int* val);

// IIC Extender GPIO pins
extern int mvSikluExtndrGpioSetVal(int gpio, int val);
extern int mvSikluExtndrGpioSetDirection(int gpio, int isOutput);
extern int mvSikluExtndrCpuGpioGetVal(int gpio, int* val);


extern int mvSikluHwResetCntrl(SKL_MODULE_RESET_CNTRL_E, int isEna);


extern const char* siklu_mutable_env_get(const char *varname);
extern uint32_t get_nand_part_offset_by_name(const char* name);
extern int siklu_mutable_env_set(const char *varname, const char *varvalue, int save_if_diff_required);
extern const char* siklu_mutable_env_get(const char *varname);

extern int siklu_is_restore2fact_default(void);
extern void siklu_wait_user4prevent_card_reboot(void);

extern int siklu_88e512_phy_read(__u32 phy_addr, __u32 bank, __u32 reg_addr, __u16* reg_val);
extern int siklu_88e512_phy_write(__u32 phy_addr, __u32 bank, __u32 reg_addr, __u16 reg_val);

extern int siklu_set_led(SKL_BOARD_LED_TYPE_E led, SKL_BOARD_LED_MODE_E state);

#endif /* INCLUDE_SIKLU_API_H_ */
