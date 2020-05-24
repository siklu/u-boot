#include <common.h>
#include <asm/io.h>
#include <asm/arch/soc.h>
#include <siklu/siklu_device_specific_information.h>

/* Armada 7k/8k */
#define SAR0_REG	(MVEBU_REGISTER(0x2400200))
#define CPU_NAME  	"marvell,armada8040"

// get CPU config register
int siklu_get_cpu_config_register(int *config_reg)
{
	*config_reg = readl(SAR0_REG);
	return CMD_RET_SUCCESS;
}

// get CPU name
int siklu_get_cpu_name(const char **cpu_name)
{
	static const char *static_cpu_name= CPU_NAME;
	*cpu_name = static_cpu_name;
	return CMD_RET_SUCCESS;
}


