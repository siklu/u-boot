/*
 * mvBoardSikluFlashBoot.c
 *
 *  Created on: Jul 5, 2016
 *      Author: edwardk
 *
 *      based on file siklu_flash_boot in SDK310
 *
 */

#include <common.h>
#include <command.h>
#include <version.h>
#include <siklu_api.h>

#define BOOT_DEBUG // edikk remove


#ifdef BOOT_DEBUG
static inline int _run_command(const char *cmd, int flag)
{
	int rc;
	printf(" *** Run command: %s\n", cmd);
	rc = run_command(cmd, flag);
	printf(" ***                  rc %d\n", rc);
	return rc;

}
#else
# define _run_command(a,b) run_command(a,b)
#endif

#define BOOT_FROM_IMAGE_IN_ENV    (-1)
#define ADDR_IN_RAM4ACTIVE_UIMAGE 0x5000000
#define MAX_ACTIVE_UIMAGE_SIZE    22000000 // 22M

#define INITRAMFS_SIZE      0x1800000
#define KERNEL_ADDR        "20000000"

static ulong ramd_addr = 0;
static ulong ramd_size = INITRAMFS_SIZE;


static int rescue_restore_boot_image(void);

static int is_image_valid(ulong load_addr) {
	const image_header_t *hdr = (image_header_t *) load_addr;
	// printf("%s() check image on address 0x%lx\n", __func__, load_addr);
	return image_check_magic(hdr) && //
			image_check_hcrc(hdr) && //
			image_check_dcrc(hdr);
}

/*
 *  copy image from ubi partition to RAM
 *  validate SW image
 */
static int validate_sw_image(int img2load) {
	int rc = 0;
	char buf[512];

	//
	//
	sprintf(buf, "mtdparts default");
	rc = _run_command(buf, 0);
	if (rc != 0) {
		printf(" Execute command \"%s\" FAIL\n", buf);
		return -1;
	}
	//
	sprintf(buf, "ubi part uimage%d", img2load);
	rc = _run_command(buf, 0);
	if (rc != 0) {
		printf(" Execute command \"%s\" FAIL\n", buf);
		return -1;
	}
	//
	sprintf(buf, "ubi read 0x%x vol_uimage%d 0x%x", ADDR_IN_RAM4ACTIVE_UIMAGE,
			img2load, MAX_ACTIVE_UIMAGE_SIZE);
	rc = _run_command(buf, 0);
	if (rc != 0) {
		printf(" Execute command \"%s\" FAIL\n", buf);
		return -1;
	}
	//
	sprintf(buf, "iminfo %x", ADDR_IN_RAM4ACTIVE_UIMAGE);
	rc = _run_command(buf, 0);
	if (rc != 0) {
		printf(" Execute command \"%s\" FAIL\n", buf);
		return -1;
	}

	return rc;
}
/*
 *
 */
static int allocate_mem_space(void) {
	int rc = 0;
//#define RAMD_MIN_ADDR  0x21000000
//#define RAMD_MAX_ADDR  0x30000000
//#define RAMD_ALIGNMENT 0x100000

#define RAMD_SIZE	0x2000000
#define RAMD_ADDR   0x21000000

	// ramd_addr = cvmx_bootmem_phy_named_block_alloc(ramd_size + 0x1000, RAMD_MIN_ADDR, RAMD_MAX_ADDR, RAMD_ALIGNMENT, "skl_initrd", 0);
	ramd_addr = RAMD_ADDR;
	printf(" namedalloc for <skl_initrd> addr 0x%x, size 0x%x(%d)\n",
			(uint) ramd_addr, (uint) ramd_size, (uint) ramd_size);
	return rc;
}
static int unpack_uimage(uint uimage_ram_addr) {
	int rc = 0;
	char buf[256];

	// extract kernel to RAM locations
	sprintf(buf, "imx %x 2 " KERNEL_ADDR, uimage_ram_addr);
	rc = _run_command(buf, 0);
	if (rc != 0) {
		printf(" Execute command \"%s\" FAIL\n", buf);
		return -1;
	}
	// ROOTFS is SQUASHFS compressed image, no need to decompress it
	// simply extract and copy it to dest address
	int part = 1; // This is root file system part number inside a siklu uimage!!!!
	ulong data = 0;   // root file system offset relative to start of image
	ulong len = 0;    // root file system  size
	image_header_t *hdr = (image_header_t *) uimage_ram_addr; //addr;
	image_multi_getimg(hdr, part, &data, &len);
	if ((data == 0) || (len == 0)) {
		printf(" Get ROOTFS param FAIL\n");
		return -1;
	}
	uint32_t ramd_hex_addr = 0;
	ramd_hex_addr = ramd_addr; //simple_strtoul(INITRAMFS_ADDR, NULL, 16);
	memcpy((char*) ramd_hex_addr, (char*) data, len + 100);
	return rc;
}

/*
 * The function doesn't return!
 */
static int run_linux_code(int is_system_in_bist) {
	int rc = 0;
	char buf[600];
	int i = 0;
	const char* mtd_str = getenv("user_mtdparts");

	if (mtd_str) {
		// use user predefined MTD partition table
	} else {
		mtd_str = MTDPARTS_DEFAULT;
	}
	// edikk !!!!----------------------------  build a command line ---------------------------------------------------------------------
	//
	i +=
			sprintf(buf + i,
					"bootoctlinux " KERNEL_ADDR " numcores=1 coremask=0x1 endbootargs rd_name=skl_initrd mtdparts=%s ",
					mtd_str);

	if (is_system_in_bist) { // add string to command line says about BIST mode
		const char *bist_state = getenv(SIKLU_BIST_ENVIRONMENT_NAME);
		i += sprintf(buf + i, "bist=%s ", bist_state);
	}

	if (siklu_is_restore2fact_default()) {
		i += sprintf(buf + i, "rfd=on "); // mean ResetFactoryDefault=ON
	}

	i += sprintf(buf + i, "ver=%s.%s.%srevv ", SIKLU_U_BOOT_VERSION,
			U_BOOT_SVNVERSION_STR, U_BOOT_DATE);

	// run the command line
	rc = _run_command(buf, 0);
	if (rc != 0) {
		printf(" Execute command \"%s\" FAIL\n", buf);
		return -1;
	}
	return rc;
}

/*
 *
 *
 */
static int execute_siklu_boot(int forced_image) {
	int rc = 0;
	int img2load = 0; // set by default
	const char *bist_state = getenv(SIKLU_BIST_ENVIRONMENT_NAME);   //
	int is_system_in_bist;
	// char buf[512];

	if (bist_state)
		is_system_in_bist = 1;
	else
		is_system_in_bist = 0;

	if (forced_image != BOOT_FROM_IMAGE_IN_ENV)
		img2load = forced_image;
	else {
		const char *primary_image_s = siklu_mutable_env_get("SK_primary_image"); // getenv("SK_primary_image");
		if (primary_image_s) {
			img2load = !!simple_strtoul(primary_image_s, NULL, 10);
		}
	}
	rc = validate_sw_image(img2load);

	if (rc < 0) // validating fail, attempt to load next uimage
			{
		img2load = !img2load;
		rc = validate_sw_image(img2load);
		if (rc < 0) { // both images are wrong!
			printf("%s()  Both uimage files are wrong\n", __func__);
			rc = rescue_restore_boot_image();
			if (rc >= 0) {
				siklu_wait_user4prevent_card_reboot();
				return 0; //
			}

		}
	}
	printf("\nTrying %s uimage... \n", (img2load == 0) ? ("1st") : ("2nd"));
	// allocate pools for NPU, kernel and rootfs
	rc = allocate_mem_space();
	if (rc != 0) {
		printf(" Execute command allocate_mem_space FAIL\n");
		return -1;
	}
	rc = unpack_uimage(ADDR_IN_RAM4ACTIVE_UIMAGE);
	if (rc != 0) {
		printf(" ERROR, exit\n");
		return -1;
	}

	rc = run_linux_code(is_system_in_bist); // the function doesn't return, jump to linux here!
	if (rc != 0) {
		printf(" ERROR, exit\n");
		return -1;
	}

	return rc;
}

static int execute_siklu_boot_from_ram(ulong img_addr) {
	int rc = 0;

	const char *bist_state = getenv(SIKLU_BIST_ENVIRONMENT_NAME);   //
	int is_system_in_bist;
	// char buf[512];

	if (bist_state)
		is_system_in_bist = 1;
	else
		is_system_in_bist = 0;

	printf("\nTrying load uimage from address 0x%x... \n", (uint) img_addr);
	// allocate pools for NPU, kernel and rootfs
	rc = allocate_mem_space();
	if (rc != 0) {
		printf(" Execute command allocate_mem_space FAIL\n");
		return -1;
	}
	rc = unpack_uimage(img_addr);
	if (rc != 0) {
		printf("Unpack ERROR, exit\n");
		return -1;
	}

	rc = run_linux_code(is_system_in_bist); // the function doesn't return, jump to linux here!
	if (rc != 0) {
		printf(" ERROR, exit\n");
		return -1;
	}

	return rc;
}
/*
 * ask for uimage file from default TFTP server,
 * program it in first uimage partition
 * preset SK_primary image environment
 * reboot
 *
 */
static int rescue_restore_boot_image(void) {
	static int rc = 0;
	char buf[200];
	ulong faddr = 0;
	ulong fsize = 0;
	char *s;

	printf("%s()  Execute RESQUE RESTORE and REBOOT\n", __func__);
	// execute 'dhcp' command, get network parameters, get uimage file name, server IP
	sprintf(buf, "dhcp");
	rc = _run_command(buf, 0);
	if (rc != 0) {
		printf("Execution command \"%s\" FAIL.\n", buf);
		return -1;
	}

	sprintf(buf, "tftp");
	rc = _run_command(buf, 0);
	if (rc != 0) {
		printf("Execution command \"%s\" FAIL.\n", buf);
		return -1;
	}

	// check file size and load address
	if ((s = getenv("fileaddr")) == NULL) {
		printf("File address isn't set\n");
		return -1;
	}
	faddr = simple_strtoul(s, NULL, 16);
	if ((s = getenv("filesize")) == NULL) {
		printf("File size isn't defined\n");
		return -1;
	}
	fsize = simple_strtoul(s, NULL, 16);

	// check that received over tftp image is valid
	if (!is_image_valid(faddr)) {
		printf("Received image is wrong\n");
		return -1;
	}

	// prepare NAND flash, execute commands:
	//      nand  erase.chip;mtdpart default
	//      ubi part uimage0;ubi create vol_uimage0;ubi part uimage1;ubi create vol_uimage1
	sprintf(buf, "nand  erase.chip;mtdpart default");
	rc = _run_command(buf, 0);
	if (rc != 0) {
		printf("Execution command \"%s\" FAIL.\n", buf);
		return -1;
	}
	sprintf(buf, "ubi part uimage1;ubi create vol_uimage1"); // start create from vol_uimage1
	rc = _run_command(buf, 0);
	if (rc != 0) {
		printf("Execution command \"%s\" FAIL.\n", buf);
		return -1;
	}
	sprintf(buf, "ubi part uimage0;ubi create vol_uimage0"); // after this create vol_uimage0
	rc = _run_command(buf, 0);
	if (rc != 0) {
		printf("Execution command \"%s\" FAIL.\n", buf);
		return -1;
	}

	// program image to uimage0 mtd partition
	//      Execute command: ubi write 0x1000000  vol_uimage0    ${filesize}
	sprintf(buf, "ubi write 0x%lx  vol_uimage0   0x%lx", faddr, fsize);
	rc = _run_command(buf, 0);
	if (rc != 0) {
		printf("Execution command \"%s\" FAIL.\n", buf);
		return -1;
	}

	// preset default mutual environment...
	{
		extern uint32_t get_nand_part_offset_by_name(const char* name);
		extern int primary_format_mutual_env(uint32_t env_part_offs);

		uint32_t mut_env_in_nand_flash_start = get_nand_part_offset_by_name(
				"env_var0");
		primary_format_mutual_env(mut_env_in_nand_flash_start);
	}

	// save default regular environment
	sprintf(buf, "env save");
	rc = _run_command(buf, 0);
	if (rc != 0) {
		printf("Execution command \"%s\" FAIL.\n", buf);
		return -1;
	}

	return 0;
}

/*
 *
 *
 *
 *
 *
 */
static int do_siklu_boot(cmd_tbl_t * cmdtp, int flag, int argc,
		char * const argv[]) {
	int rc = -1;

	(void) cmdtp;
	(void) flag;
	(void) argc;
	(void) argv;

	if (siklu_mutable_env_get("SK_primary_image") /*  getenv("SK_primary_image") */== 0)
	{
		printf("No SK_primary_image environment!... The SW should be restored\n");
		rc = rescue_restore_boot_image();
		if (rc >= 0)
		{
			siklu_wait_user4prevent_card_reboot();
		}
	}

	switch (argc)
	{
		case 2:  // command called with 1 parameter
		{
			int img = simple_strtoul(argv[1], NULL, 10);
			if ((img >= 0) && (img <= 1))
			rc = execute_siklu_boot(img);
		}
		break;
		default:
		execute_siklu_boot(BOOT_FROM_IMAGE_IN_ENV);
		break;
	}

	return rc;
}

/*
 *
 *
 */
static int do_siklu_ram_boot(cmd_tbl_t * cmdtp, int flag, int argc,
		char * const argv[]) {
	int rc = -1;


	ulong img_addr = 0;

	if (argc != 2)
	{
		printf(" Called with wrong args num %d\n", argc);
		return 0;
	}

	img_addr = simple_strtoul(argv[1], NULL, 16);
	rc = execute_siklu_boot_from_ram(img_addr);

	return rc;
}

/*
 *
 *
 */
static int do_siklu_set_dflt_env(cmd_tbl_t * cmdtp, int flag, int argc,
		char * const argv[]) {
	int rc = CMD_RET_FAILURE; // the command isn't repeatable!



	extern uint32_t get_nand_part_offset_by_name(const char* name);
	extern int primary_format_mutual_env(uint32_t env_part_offs);

	uint32_t mut_env_in_nand_flash_start = get_nand_part_offset_by_name("env_var0");
	primary_format_mutual_env(mut_env_in_nand_flash_start);

	return rc;
}

/*
 *  siklu_remark002
 */
static int do_siklu_show_mut_env_area(cmd_tbl_t * cmdtp, int flag, int argc,
		char * const argv[]) {
	int rc = CMD_RET_SUCCESS;



	extern void siklu_print_mut_env_area(void);
	siklu_print_mut_env_area();

	return rc;
}

U_BOOT_CMD(siklu_boot, 5, 0, do_siklu_boot,
		"Boot Siklu software from FLASH storage", "[img2load* none/0/1] ");
U_BOOT_CMD(siklu_boot_ram, 5, 0, do_siklu_ram_boot,
		"Boot Siklu software from RAM", "[uimage addr] ");
U_BOOT_CMD(ssde, 5, 0, do_siklu_set_dflt_env, "Set system default environment",
		"Set system default environment");
U_BOOT_CMD(smeprint, 5, 0, do_siklu_show_mut_env_area,
		"Show Siklu Mutual Environment Area",
		"Show Siklu Mutual Environment Area");

