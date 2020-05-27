#include <common.h>
#include <asm/io.h>
#include <asm/arch/soc.h>
#include <siklu/siklu_board_n366.h>
#include <siklu/common_gpio.h>

#define N366_NUM_OF_HW_REVISION_BITS 4

#define N366_HW_REVISION_GPIO_NAME_BIT_0 	"cpm_gpio126"  //  cpm bank 1 gpio 26 (CP0_MPP[58])
#define N366_HW_REVISION_GPIO_NAME_BIT_1 	"cpm_gpio127"  //  cpm bank 1 gpio 27 (CP0_MPP[59])
#define N366_HW_REVISION_GPIO_NAME_BIT_2 	"cps_gpio07"   //  cps bank 0 gpio 7  (CP1_MPP[7])
#define N366_HW_REVISION_GPIO_NAME_BIT_3 	"cpm_gpio10"   //  cpm bank 1 gpio 0  (CP0_MPP[32])


/* Armada 7k/8k */
#define SAR0_REG	(MVEBU_REGISTER(0x2400200))
#define CPU_NAME  	"marvell,armada8040"

static const char *hw_rev_gpio_bits_array[N366_NUM_OF_HW_REVISION_BITS]={
	N366_HW_REVISION_GPIO_NAME_BIT_0, //LSB
	N366_HW_REVISION_GPIO_NAME_BIT_1, 
	N366_HW_REVISION_GPIO_NAME_BIT_2, 
	N366_HW_REVISION_GPIO_NAME_BIT_3};

// get the siklu board HW revision
int siklu_n366_get_hw_revision (int *hw_revision)
{
	int ret = CMD_RET_SUCCESS;
	int val = 0;

	*hw_revision = 0;

	for (int i=0; i<N366_NUM_OF_HW_REVISION_BITS; i++)
	{
		ret=siklu_read_gpio_by_name (hw_rev_gpio_bits_array[i], &val);
		if (ret != CMD_RET_SUCCESS)
		{
			return ret;	
		}
		
		*hw_revision += val << i;
	}
		
	return CMD_RET_SUCCESS;
}


// get CPU config register
int siklu_n366_get_cpu_config_register(uint64_t *config_reg)
{
	*config_reg = readl(SAR0_REG);
	return CMD_RET_SUCCESS;
}

// get CPU name
int siklu_n366_get_cpu_name(const char **cpu_name)
{
	static const char *static_cpu_name= CPU_NAME;
	*cpu_name = static_cpu_name;
	return CMD_RET_SUCCESS;
}

