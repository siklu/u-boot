#include <common.h>

#include <malloc.h>
#include <memalign.h>
#include <command.h>
#include <env.h>
#include <env_internal.h>
#include "common_mtd.h"

#ifdef CONFIG_CMD_SAVEENV
static int env_mtd_nor_save(void)
{
	ALLOC_CACHE_ALIGN_BUFFER(env_t, env_new, 1);
	int ret;
	struct part_info *part;
	
	part = siklu_get_part_by_label(CONFIG_SIKLU_ENV_PART);
	if (! part) {
		printf("Error: Could not find partition \"%s\"\n", CONFIG_SIKLU_ENV_PART);
		return -ENODEV;
	}

	ret = env_export(env_new);
	if (ret)
		return ret;

	ret = siklu_write_to_mtd(part, (const char*)env_new, sizeof(*env_new));
	if (ret) {
		printf("Error: Failed to write environemt\n");
		return -EIO;
	}

	puts("done\n");
	return 0;
}
#endif /* CONFIG_CMD_SAVEENV */

static int env_mtd_nor_load(void)
{
	char *buf;
	struct part_info *part;
	
	part = siklu_get_part_by_label(CONFIG_SIKLU_ENV_PART);
	if (! part) {
		printf("Error: Could not find partition \"%s\"\n", CONFIG_SIKLU_ENV_PART);
		return -ENODEV;
	}
	
	/* TOOD: check part size */
	buf = siklu_read_from_mtd(part);
	if (! buf) {
		printf("Error: Could not read from \"%s\"\n", CONFIG_SIKLU_ENV_PART);
		return -EIO;
	}
	
	return env_import(buf, 1);
}

U_BOOT_ENV_LOCATION(ubi) = {
	.location	= ENVL_SIKLU_NOR_MTD,
	ENV_NAME("MTD NOR")
	.load		= env_mtd_nor_load,
	.save		= env_save_ptr(env_mtd_nor_save),
};
