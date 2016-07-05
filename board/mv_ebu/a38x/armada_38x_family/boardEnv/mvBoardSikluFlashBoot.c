/*
 * mvBoardSikluFlashBoot.c
 *
 *  Created on: Jul 5, 2016
 *      Author: edwardk
 */


#include <common.h>
#include <command.h>






/*
 *
 *
 *
 *
 *
 */
static int do_siklu_boot(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
    int rc = -1;

    (void) cmdtp;
    (void) flag;
    (void) argc;
    (void) argv;
#if 0 // edikk open it
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
#endif // 0
    return rc;
}


/*
 *
 *
 */
static int do_siklu_ram_boot(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
    int rc = -1;
#if 0 // edikk open it

    ulong img_addr = 0;

    if (argc != 2)
    {
        printf(" Called with wrong args num %d\n", argc);
        return 0;
    }

    img_addr = simple_strtoul(argv[1], NULL, 16);
    rc = execute_siklu_boot_from_ram(img_addr);
#endif // 0
    return rc;
}


/*
 *
 *
 */
static int do_siklu_set_dflt_env(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
    int rc = CMD_RET_FAILURE; // the command isn't repeatable!

#if 0 // edikk open it

    extern uint32_t get_nand_part_offset_by_name(const char* name);
    extern int primary_format_mutual_env(uint32_t env_part_offs);

    uint32_t mut_env_in_nand_flash_start = get_nand_part_offset_by_name("env_var0");
    primary_format_mutual_env(mut_env_in_nand_flash_start);
#endif // 0
    return rc;
}

/*
 *  siklu_remark002
 */
static int do_siklu_show_mut_env_area(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
    int rc = CMD_RET_SUCCESS;

#if 0 // edikk open it

    extern void siklu_print_mut_env_area(void);
    siklu_print_mut_env_area();
#endif // 0
    return rc;
}










U_BOOT_CMD(siklu_boot, 5, 0, do_siklu_boot, "Boot Siklu software from FLASH storage", "[img2load* none/0/1] ");
U_BOOT_CMD(siklu_boot_ram, 5, 0, do_siklu_ram_boot, "Boot Siklu software from RAM", "[uimage addr] ");
U_BOOT_CMD(ssde, 5, 0, do_siklu_set_dflt_env, "Set system default environment", "Set system default environment");
U_BOOT_CMD(smeprint, 5, 0, do_siklu_show_mut_env_area, "Show Siklu Mutual Environment Area",
        "Show Siklu Mutual Environment Area");

