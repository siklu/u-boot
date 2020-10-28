#include <common.h>

#include "common_boot.h"
#include <siklu/siklu_board_generic.h>

#define BOOT_DIR "/boot"

void setup_bootargs(const char *bootargs) {
	char formatted_bootargs[1024];
	const char *mtdparts;
	const char *old_bootargs;
	struct siklu_board *board;
	const char *additional_board_bootargs;

	old_bootargs = env_get("bootargs");
	mtdparts = env_get("mtdparts");

	board = siklu_get_board();
	if(board && board->additional_bootargs) {
		additional_board_bootargs = board->additional_bootargs;
	} else {
		additional_board_bootargs = NULL;
	}

	snprintf(formatted_bootargs, sizeof(formatted_bootargs), "%s %s %s %s",
			bootargs, old_bootargs ? old_bootargs : "",
			mtdparts ? mtdparts : "",
			additional_board_bootargs ? additional_board_bootargs : "");
	env_set("bootargs", formatted_bootargs);
}

char *kernel_load_address(void)
{
	return env_get("kernel_addr_r");
}

char *kernel_path(void)
{
	if (IS_ENABLED(CONFIG_ARM64))
		return BOOT_DIR "/Image";
	else
		return BOOT_DIR "/zImage";
}

char *dtb_load_address(void)
{
	return env_get("fdt_addr_r");
}

char *dtb_path(void)
{
	static char dtpath[128];

	snprintf(dtpath, sizeof(dtpath), BOOT_DIR "/%s", env_get("fdtfile"));

	return dtpath;
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
