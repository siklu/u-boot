#include <common.h>
#include <asm/io.h>
#include <asm/arch/soc.h>
#include <siklu/siklu_device_specific_information.h>

/* Armada 7k/8k */
#define SAR0_REG	(MVEBU_REGISTER(0x2400200))

// get CPU config register
int siklu_get_cpu_config_register(int *config_reg)
{
	*config_reg = readl(SAR0_REG);
	return CMD_RET_SUCCESS;
}
