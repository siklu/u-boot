/*
 * mvBoardSikluPhyBle.c
 *
 *  Created on: Oct 27, 2016
 *      Author: edwardk
 *
 *      Marvell PHY and Bluetooth Low Energy functionality
 */

#include <common.h>
#include <command.h>
#include "mvCommon.h"
#include "mvOs.h"
#include <siklu_api.h>

extern MV_STATUS mvEthPhyRegRead(MV_U32 phyAddr, MV_U32 regOffs, MV_U16 *data);
extern MV_STATUS mvEthPhyRegWrite(MV_U32 phyAddr, MV_U32 regOffs, MV_U16 data);

/*
 *
 */
MV_STATUS siklu_88e512_phy_read(__u32 phy_addr, __u32 bank, __u32 reg_addr, __u16* reg_val)
{
    MV_STATUS rc;
    mvEthPhyRegWrite(phy_addr, 22, bank); // Preset page
    rc = mvEthPhyRegRead(phy_addr, reg_addr, reg_val);

    return rc;

}
/*
 *
 */
MV_STATUS siklu_88e512_phy_write(__u32 phy_addr, __u32 bank, __u32 reg_addr, __u16 reg_val)
{
    MV_STATUS rc = 0;

    mvEthPhyRegWrite(phy_addr, 22, bank); // Preset page
    rc = mvEthPhyRegWrite(phy_addr, reg_addr, reg_val); // Write value

    return rc;
}

static int do_siklu_marvell_88e512_phy_access(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int rc = CMD_RET_SUCCESS;
    int ret;
    int phy_addr, bank, reg_addr, reg_val;
    char op = '-';

    phy_addr = reg_addr = reg_val = bank = -1;

    switch (argc)
    { // do not breaks inside !

    case 5:
        reg_val = simple_strtoul(argv[4], NULL, 16);
        op = 'w';
    case 4:
        reg_addr = simple_strtoul(argv[3], NULL, 16);
        if (op == '-')
            op = 'r';
    case 3:
        bank = simple_strtoul(argv[2], NULL, 16);
        if (op == '-')
            op = 'b'; // read full bank
    case 2:
        phy_addr = simple_strtoul(argv[1], NULL, 16);
        break;
    default:
        //                         1        2        3       4           5
        printf("Command format: sphy   <phy_addr> <bank> <reg_addr*> <reg_val**>\n");
        return CMD_RET_USAGE;
    }

    //printf(" phy_addr %x, bank %x, reg_addr %x, reg_val %x, op %c\n",
    //        phy_addr, bank, reg_addr, reg_val, op);

    switch (op)
    {
    case 'w': // write register
        ret = siklu_88e512_phy_write(phy_addr, bank, reg_addr, reg_val);
        if (ret != 0)
            printf(" Write FAIL\n");
        break;
    case 'r': // read register
    {
        __u16 reg_val;
        ret = siklu_88e512_phy_read(phy_addr, bank, reg_addr, &reg_val);
        if (ret == 0)
            printf("[%02x(%02d)] 0x%04x\n", reg_addr, reg_addr,reg_val);
    }
        break;
    case 'b': // read full block registers
    {
        __u16 reg_val;
        for (reg_addr = 0; reg_addr <= 26; reg_addr++)
        {
            ret = siklu_88e512_phy_read(phy_addr, bank, reg_addr, &reg_val);
            if (ret == 0)
                printf("[%02x(%02d)] 0x%04x\n", reg_addr,reg_addr, reg_val);
        }
    }
        break;
    default:
        return CMD_RET_USAGE;
    }

    return rc;
}

U_BOOT_CMD(sphy, 5, 1, do_siklu_marvell_88e512_phy_access, "On-board 88e512 PHY Access",
        "[phy_addr] [bank] [reg_addr*] [reg_val**] Read/Write On-board 88e512 PHY");
