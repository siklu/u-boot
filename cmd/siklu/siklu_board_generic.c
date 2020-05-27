#include <common.h>
#include <siklu/siklu_board_generic.h>
#include <siklu/siklu_board_n366.h>
#include <dm/device.h>


// Siklu HW revision 
static int siklu_saved_hw_revision = ILLEGAL_HW_REVISION;

// a getter for the calculated and saved HW revision
int siklu_get_saved_hw_revision(void)
{
	return siklu_saved_hw_revision;
}


// calculate and save the siklu HW revision
int siklu_calculate_and_save_siklu_hw_revision (void)
{
	int ret = CMD_RET_SUCCESS;
	
	// siklu n366
	if (of_machine_is_compatible("siklu,n366"))
	{
		ret = siklu_n366_get_hw_revision (&siklu_saved_hw_revision);
		if (ret == CMD_RET_SUCCESS)
		{
			printf("Siklu board HW revision: %u\n", siklu_saved_hw_revision);
		}
		else
		{
			siklu_saved_hw_revision = ILLEGAL_HW_REVISION;
			printf ("error in calculating HW revision for this board\n");
			return (ret);
		}
	}
	else
	{
		printf ("no HW revision calculation is implemented for this board\n");
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}


// get CPU info
int siklu_get_cpu_name (const char **cpu_name)
{
	int ret = CMD_RET_SUCCESS;
	
	// siklu n366
	if (of_machine_is_compatible("siklu,n366"))
	{
		ret = siklu_n366_get_cpu_name(cpu_name);
		if (ret != CMD_RET_SUCCESS)
		{
			return ret;
		}
	}
	else
	{
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}


// get CPU info
int siklu_get_cpu_config_register(uint64_t *config_reg)
{
	int ret = CMD_RET_SUCCESS;
	
	// siklu n366
	if (of_machine_is_compatible("siklu,n366"))
	{
		ret = siklu_n366_get_cpu_config_register(config_reg);
		if (ret != CMD_RET_SUCCESS)
		{
			return ret;
		}
	}
	else
	{
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}
