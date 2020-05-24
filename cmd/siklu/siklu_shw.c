#include <common.h>
#include <linux/mtd/rawnand.h>
#include <siklu/siklu_device_specific_information.h>
#include <siklu/siklu_board_hw_revision.h>

// This function should not be called unless no specific board function with the same mame exists
// If you get here you should write one. e.g, in  siklu/cmd/board/n366/siklu_board_hw_revision.c 
__weak int siklu_get_hw_revision(unsigned int *hw_revision)
{
	// not implemented
	return ENOSYS;
}

// This function should not be called unless no specific CPU function with the same mame exists
// If you get here you should write one. e.g, in  arch/arm/mach-mvebu/armada8k/siklu_device_specific_information.c for the ARMADA 8040 
__weak int siklu_get_cpu_config_register(int *config_reg)
{
	// not implemented
	return ENOSYS;
}



// show HW revision
void show_hw_revision (void)
{
	int ret = CMD_RET_SUCCESS;
	unsigned int hw_revision = 0;

	printf("HW revision: ");

	ret = siklu_get_hw_revision (&hw_revision);
	if (ret == CMD_RET_SUCCESS)
	{
		 printf("%u\n",hw_revision);
	}
	else
	{
		(ret == ENOSYS) ? printf("Not implemented\n") : printf("Unknown\n");
	}

}

// show board model
static void show_board_model (void)
{
	const char *model;

	model = fdt_getprop(gd->fdt_blob, 0, "model", NULL);

	printf("Model: ");
	model ? printf("%s\n", model) : printf("Unknown\n");
}


// show nand
static void show_nand_info (void)
{
	int ret = CMD_RET_SUCCESS;
	struct mtd_info *mtd = NULL;
	struct nand_chip *chip = NULL;
	struct nand_flash_dev *type = NULL;
	
	printf("NAND: ");

	mtd = get_mtd_device(NULL, 0);
	if (mtd)
	{
		type = nand_flash_ids;
		chip = mtd_to_nand(mtd);

		u8 id_data[8];

		/* Send the command for reading device ID */
		ret = nand_readid_op(chip, 0, id_data, 2);
		if (ret)
		{	
			printf("Unknown\n");
			return;
		}

		/* Read manufacturer and device IDs */
		u8 maf_id = id_data[0];
		u8 dev_id = id_data[1];

		/* Try to identify manufacturer */
		int maf_idx = 0;

		for (maf_idx = 0; nand_manuf_ids[maf_idx].id != 0x0; maf_idx++) {
			if (nand_manuf_ids[maf_idx].id == maf_id)
				break;
		}
		
		// nand name
		printf("Name: %s ", nand_manuf_ids[maf_idx].name);
		
		// nand model

#ifdef CONFIG_SYS_NAND_ONFI_DETECTION
	
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
#else
		if (chip->jedec_version)
			printf("%s, ",chip->jedec_params.model);
		else
			printf("%s, ",type->name);
#endif

		// nand size
		printf("Size: ");
	 	chip ? printf("%d MIB, ", (int)(chip->chipsize >> 20)) : printf("Unknown\n");

		// nand Manufacturer ID 		
		printf("Manufacturer ID: 0x%02x, ",maf_id);

		// nand Chip ID		
		printf("Chip ID: 0x%02x\n", dev_id);
	}
	else
	{	
		printf("Unknown\n");
		return;
	}

}

// show DRAM (DDR)
static void show_dram_info (void)
{
	puts("DRAM: ");
	print_size(gd->ram_size, "\n");
}


// show SF (SPI flash)
static void show_sf_info (void)
{
	run_command("sf probe", 0);
}



// show CPU 
static void show_cpu_info (void)
{
	int ret = CMD_RET_SUCCESS;

	printf("CPU: ");

	const char *cpu_model = fdt_getprop(gd->fdt_blob, 0, "compatible", NULL);
//	const char *cpu_model = fdt_getprop(gd->fdt_blob, 0, "machine_is_compatible", NULL);

	printf("Name: ");
	cpu_model ? printf("%s, ",cpu_model) : printf("Unknown, ");

	int config_reg = 0;
	printf("Config register: ");	

	ret = siklu_get_cpu_config_register(&config_reg);
	if (ret == CMD_RET_SUCCESS)
	{	
		printf("0x%x\n", config_reg);
	}
	else
	{
		ret == 	ENOSYS ? printf("Not implemented\n") : printf("Unknown\n");
	}

}


// main siklu show command function
static int do_siklu_shw(cmd_tbl_t *cmdtp, int flag, int argc,
						char *const argv[])
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
	siklu_shw,		//name
	1,			//max params
	0,			//rep
	do_siklu_shw,		//func
	"dispaly HW info",	//help
	"" 			//usage
);
