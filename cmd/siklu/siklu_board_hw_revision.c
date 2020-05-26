#include <common.h>

// Siklu HW revision
static int siklu_saved_hw_revision = -1;

// a getter for the calculated and saved HW revision
int siklu_get_saved_hw_revision(int *saved_hw_revision)
{
	int ret = CMD_RET_SUCCESS;

	(siklu_saved_hw_revision != -1) ? (*saved_hw_revision = siklu_saved_hw_revision) : (ret = CMD_RET_FAILURE);

	return ret;
}


// calculate and save the siklu HW revision
int calculate_and_save_siklu_hw_revision (void)
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
			siklu_saved_hw_revision = -1;
			printf ("error in calculating HW revision for this board\n");
			ret = CMD_RET_FAILURE;
		}
	}
	else
	{
		printf ("no HW revision calculation is implemented for this board\n");
		ret = CMD_RET_FAILURE;
	}

	return ret;
}


