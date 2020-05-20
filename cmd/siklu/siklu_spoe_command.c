#include <common.h>
#include <command.h>

#include "siklu_spoe_command.h"

static int do_siklu_poe_num_pairs_show(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	enum PoE_PAIRS pairs = poe_num_pairs();

	if (pairs == TWO_PAIRS)
		printf("2-pairs\n");
	else if(pairs == FOUR_PAIRS)
		printf("4-pairs\n");
	else if(pairs == UNKNOWN_PAIRS)
		printf("Unknown\n");

	return pairs != UNKNOWN_PAIRS ? CMD_RET_SUCCESS : CMD_RET_FAILURE;
}

U_BOOT_CMD(spoe, 5, 0, do_siklu_poe_num_pairs_show,
	"Show POE number pairs Status", "Show POE number pairs Status");
