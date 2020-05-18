#include <common.h>
#include <command.h>
//#include <dm.h>
//#include <spi.h>
//#include <ubi_uboot.h>
//#include <ubifs_uboot.h>

#include "common_boot.h"
const int GPIO_POE_PIN = 39;

static int do_siklu_poe_num_pairs_show(cmd_tbl_t * cmdtp, int flag, int argc,
	char * const argv[]) {
    int rc = CMD_RET_SUCCESS;
/*
    T_CPLD_LOGIC_DIP_MODE_REGS reg;
    u8 poe_pair1_exist, poe_pair2_exist;

    reg.uint8 = siklu_cpld_read (CONFIG_CPLD_DIP_MODE_REG_ADDR);

    poe_pair1_exist = reg.s.cfg_poe_pair1_exist;
    poe_pair2_exist = reg.s.cfg_poe_pair2_exist;
*/
    unsigned gpio_pin;
    int r = gpio_lookup_name("cpm_gpio18", NULL, NULL, &gpio_pin);
    printf("RES %d, PIN %d\n", r, gpio_pin);
    //int r = gpio_direction_input(GPIO_POE_PIN);
    //printf("Direction %d\n", r);
    //int r2 = gpio_get_value(GPIO_POE_PIN);
    //printf("Value %d\n", r2);

/*

    if (poe_pair1_exist && poe_pair2_exist)
	printf("No pairs\n");
    else if (poe_pair1_exist || poe_pair2_exist)
	printf("2-pairs\n");
    else
	printf("4-pairs\n");
*/
    return rc;
}
U_BOOT_CMD(spoe, 5, 0, do_siklu_poe_num_pairs_show,
	"Show POE number pairs Status", "Show POE number pairs Status");
