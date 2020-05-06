#include <common.h>

#include "common_boot.h"

void setup_bootargs(const char *bootargs) {
	char formatted_bootargs[1024];
	const char *mtdparts;
	const char *old_bootargs;

	old_bootargs = env_get("bootargs");
	mtdparts = env_get("mtdparts");

	snprintf(formatted_bootargs, sizeof(formatted_bootargs), "%s %s %s",
			bootargs, old_bootargs ? old_bootargs : "",
			mtdparts ? mtdparts : "");
	env_set("bootargs", formatted_bootargs);
}

char *kernel_load_address(void)
{
	return env_get("kernel_addr_r");
}

char *dtb_load_address(void)
{
	return env_get("fdt_addr_r");
}

static char *boot_command(void)
{
	if (IS_ENABLED(CONFIG_ARM64))
		return "booti";
	else
		return "bootz";
}

int load_kernel_image(void) {
	char buff[256];
	int ret;
	
	snprintf(buff, sizeof(buff), "%s %s - %s", boot_command(),
			kernel_load_address(), dtb_load_address());
	
	ret = run_command(buff, 0);
	
	/* If we are here, we could not run the command */
	if (ret) {
		printk(KERN_ERR "Could not load kernel from using command \"%s\"", 
				buff);
	}
	
	return ret;
}
