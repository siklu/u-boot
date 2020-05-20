#include <common.h>
#include <command.h>
#include <asm/gpio.h>

const char N366_SPOE_NAME[] = "cpm_gpio18";
#if 1
#define SPOE_NAME N366_SPOE_NAME
#else
#error Need to be implemented
#endif

static int do_siklu_poe_num_pairs_show(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	int ret = CMD_RET_FAILURE;
	unsigned gpio_no;
	const char * failure = NULL;
	int pairs;
	if(gpio_lookup_name(SPOE_NAME, NULL, NULL, &gpio_no))
	{
		failure = "gpio_lookup_name";
		goto error;
	}

	if(gpio_request(gpio_no, "spoe"))
	{
		failure = "gpio_request";
		goto error;
	}

	if(gpio_direction_input(gpio_no))
	{
		failure = "gpio_direction_input";
		goto error_free_gpio;
	}

	pairs = gpio_get_value(gpio_no);

	if (pairs == 0)
		printf("2-pairs\n");
	else if(pairs == 1)
		printf("4-pairs\n");
	else /*Expected -1 */
	{
		failure = "gpio_get_value";
		goto error_free_gpio;
	}

	ret = CMD_RET_SUCCESS;

error_free_gpio:
	gpio_free(gpio_no);
error:
	if(ret != CMD_RET_SUCCESS)
		printf("Failed on %s", failure);
	return ret;
}

U_BOOT_CMD(spoe, 5, 0, do_siklu_poe_num_pairs_show,
	"Show POE number pairs Status", "Show POE number pairs Status");
