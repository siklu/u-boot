#include <common.h>
#include <siklu/siklu_board_generic.h>
#include <siklu/siklu_board_n366.h>
#include <dm/device.h>
#include <asm/io.h>

// get the board HW revision
int siklu_get_hw_revision(u32 *hw_revsion)
{
	int ret = 0;
	
	// siklu n366
	if (of_machine_is_compatible("siklu,n366"))
	{
		ret = siklu_n366_get_hw_revision (hw_revsion);
		if (ret)
		{
			return (ret);
		}
	}
	else
	{
		return -ENOSYS; // not implemented
	}

	return 0;
}

// print siklu HW revision
 void siklu_print_hw_revision (void)
{	
	int ret = 0;
	u32 hw_revision = 0;
	
	ret  = siklu_get_hw_revision(&hw_revision);

	printf ("HW revision: ");
	if (!ret)
	{
		printf ("%u\n", hw_revision);
	}
	else
	{
		printf("%s\n", ret == -ENOSYS  ? "Not implemented" : "Unknown");
	}
}

// get CPU info
int siklu_get_cpu_name (const char **cpu_name)
{
	int ret = 0;
	
	// siklu n366
	if (of_machine_is_compatible("siklu,n366"))
	{
		ret = siklu_n366_get_cpu_name(cpu_name);
		if (ret)
		{
			return ret;
		}
	}
	else
	{
		return -ENOSYS; // not implemented
	}

	return 0;
}


// get CPU info
int siklu_get_cpu_config_register(uint64_t *config_reg)
{
	int ret = 0;
	
	// siklu n366
	if (of_machine_is_compatible("siklu,n366"))
	{
		ret = siklu_n366_get_cpu_config_register(config_reg);
		if (ret)
		{
			return ret;
		}
	}
	else
	{
		return -ENOSYS; // not implemented
	}
	
	return 0;
}

static const struct siklu_board boards[] = {
	{ 
		.compatible = "siklu,n366",
		/* Limit memory to 2G on siklu N366 boards */
		/* Pin isolcpus 0,2,3 to not be used by default */
		.additional_bootargs = "mem=2G isolcpus=0,2,3"
	}
};

struct siklu_board *siklu_get_board(void) {
	int i;
	for(i = 0; i < ARRAY_SIZE(boards); ++i) {
		if(of_machine_is_compatible(boards[i].compatible)) {
			return &boards[i];
		}
	}

	return NULL;
}