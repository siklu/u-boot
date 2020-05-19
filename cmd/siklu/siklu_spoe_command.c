#include <common.h>
#include <command.h>
#include <asm-generic/gpio.h>

#include "common_boot.h"

static int do_siklu_poe_num_pairs_show(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
    unsigned gpio_no;
    unsigned need_release = 0;
    if(gpio_lookup_name("cpm_gpio18", NULL, NULL, &gpio_no))
	goto Error;

    if(gpio_request(gpio_no, "spoe"))
	goto Error;
    need_release = 1;

    if(gpio_direction_input(gpio_no))
	goto Error;

    int pairs = gpio_get_value(gpio_no);
    gpio_free(gpio_no);

    if (pairs == 0)
	printf("2-pairs\n");
    else if(pairs == 1)
	printf("4-pairs\n");
    else //Expected -1
	goto Error;

    return CMD_RET_SUCCESS;

Error:
    if(need_release)
	gpio_free(gpio_no);
    printf("Failed");
    return CMD_RET_FAILURE;
}

U_BOOT_CMD(spoe, 5, 0, do_siklu_poe_num_pairs_show,
	"Show POE number pairs Status", "Show POE number pairs Status");
