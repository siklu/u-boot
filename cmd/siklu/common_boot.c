#include <common.h>

#include "common_boot.h"

void setup_bootargs(const char *bootargs) {
	char formatted_bootargs[1024];
	const char *mtdparts;

	mtdparts = env_get("mtdparts");
	if (! mtdparts)
		mtdparts = "";

	snprintf(formatted_bootargs, sizeof(formatted_bootargs),
			 "%s %s %s", CONFIG_BOOTARGS, bootargs, mtdparts);

	env_set("bootargs", formatted_bootargs);
}

static char *boot_command(void)
{
	if (IS_ENABLED(CONFIG_ARM64))
		return "booti";
	else
		return "bootz";
}

int load_kernel_image(uintptr_t image_address, uintptr_t dtb_address) {
	char buff[256];
	int ret;
	
	snprintf(buff, sizeof(buff), "%s %p - %p", boot_command(),
			(void*)image_address, (void*)dtb_address);
	
	ret = run_command(buff, 0);
	
	/* If we are here, we could not run the command */
	if (ret) {
		printk(KERN_ERR "Could not load kernel from using command \"%s\"", 
				buff);
	}
	
	return ret;
}
