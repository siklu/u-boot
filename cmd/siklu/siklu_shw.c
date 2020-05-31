#include <common.h>
#include <linux/mtd/rawnand.h>
#include <linux/mtd/spi-nor.h>
#include <siklu/siklu_board_generic.h>

// show HW revision
void show_hw_revision (void)
{
	int ret = CMD_RET_SUCCESS;
	int hw_revision = 0;

	printf("HW revision: ");

	ret = siklu_get_hw_revision (&hw_revision);
	if (ret == CMD_RET_SUCCESS)
	{
		printf ("%u\n",hw_revision);
	}
	else
	{
		printf("%s \n", ret == ENOSYS  ? "Not implemented" : "Unknown");
	}
}

// show board model
static void show_board_model (void)
{
	const char *model;

	model = fdt_getprop(gd->fdt_blob, 0, "model", NULL);

	printf("Model: %s\n", model ? model : "Unknown");
}

// show nand
static void show_nand_info (void)
{
	struct nand_flash_dev *type;
	struct mtd_info *mtd = NULL;
	struct nand_chip *chip = NULL;
	int nand_maf_id;
	int nand_dev_id;

	printf("NAND: ");

	mtd = get_mtd_device(NULL, 0);
	if (mtd)
	{
		chip = mtd_to_nand(mtd);
		if (chip == NULL)
		{
			goto ERROR_LABEL;
		}

		// save global data flags
		unsigned long save_flags = gd->flags;

		// disable console to a ignore internal prints of nand_get_flash_type
		gd->flags |= (GD_FLG_SILENT | GD_FLG_DISABLE_CONSOLE);

		type = nand_get_flash_type(mtd, chip, &nand_maf_id, &nand_dev_id, NULL);
		if (type == NULL)
		{
			goto ERROR_LABEL;
		}

		// restore previous flags
		gd->flags = save_flags;


		/* Try to identify manufacturer */
		int maf_idx = 0;

		for (maf_idx = 0; nand_manuf_ids[maf_idx].id != 0x0; maf_idx++) {
			if (nand_manuf_ids[maf_idx].id == nand_maf_id)
				break;
		}
		
		// nand name
		printf("Name: %s ", nand_manuf_ids[maf_idx].name);
		
		// nand model

		if (chip->onfi_version)
		{
			printf("%s, ",chip->onfi_params.model);
		}
		else if (chip->jedec_version)
		{
			printf("%s, ",chip->jedec_params.model);
		}
		else
		{
			printf("%s, ",type->name);
		}

		// nand size
		printf("Size: ");
	 	chip ? printf("%d MIB, ", (int)(chip->chipsize >> 20)) : printf("Unknown\n");

		// nand Manufacturer ID 		
		printf("Manufacturer ID: 0x%02x, ",nand_maf_id);

		// nand Chip ID		
		printf("Chip ID: 0x%02x\n", nand_dev_id);
		return;
	}

ERROR_LABEL:
	printf("Unknown\n");
}


// show DRAM (DDR)
static void show_dram_info (void)
{
	puts("DRAM: ");
	print_size(gd->ram_size, "\n");
}


// show SF (NOR)
static void show_sf_info (void)
{
	printf("SF: ");
	
	struct spi_nor *nor = get_mtd_device_nm("nor0")->priv; 
	if (nor == NULL)
	{
		printf("Unknown\n");
		return;
	}

	printf("%s with page size ", nor->name);
	print_size(nor->page_size, ", erase size ");
	print_size(nor->erase_size, ", total ");
	print_size(nor->size, "");
	printf ("\n");
}



// show CPU 
static void show_cpu_info (void)
{
	int ret = CMD_RET_SUCCESS;

	printf("CPU: ");

	//name 
	printf("Name: ");

	const char *cpu_name = NULL;

	ret = siklu_get_cpu_name(&cpu_name);
	if ((ret == CMD_RET_SUCCESS) && cpu_name)
	{
		printf("%s, ",cpu_name);
	}
	else
	{
		printf("%s, ", ret == ENOSYS  ? "Not implemented" : "Unknown");
	}
	
	//config register
	uint64_t config_reg = 0;
	printf("config register: ");	

	ret = siklu_get_cpu_config_register(&config_reg);
	if (ret == CMD_RET_SUCCESS)
	{
		printf("0x%llx\n",config_reg);
	}
	else
	{
		printf("%s\n", ret == ENOSYS  ? "Not implemented" : "Unknown");
	}
}


// main shw command function
static int do_shw(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	show_hw_revision();
	show_board_model();
	show_dram_info();
	show_nand_info();
	show_sf_info();
	show_cpu_info();

	return CMD_RET_SUCCESS;
}



U_BOOT_CMD(
	shw,					//name
	1,					//max params
	0,					//rep
	do_shw,					//func
	"dispaly HW info (siklu command)",	//help
	"" 					//usage
);
