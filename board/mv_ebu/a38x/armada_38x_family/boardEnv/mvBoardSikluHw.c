/*
 * mvBoardSikluHw.c
 *
 *  Created on: Jun 9, 2016
 *      Author: edwardk
 */

#include <common.h>
#include <command.h>
#include "mvCommon.h"
#include "mvOs.h"
#include <siklu_api.h>
#include <i2c.h>
#include <spi_flash.h>

#define	MV_GPP_IN	0xFFFFFFFF	/* GPP input */
#define MV_GPP_OUT	0		/* GPP output */

extern MV_STATUS mvGppValueSet(MV_U32 group, MV_U32 mask, MV_U32 value);
extern MV_STATUS mvGppTypeSet(MV_U32 group, MV_U32 mask, MV_U32 value);
extern MV_U32 mvGppValueGet(MV_U32 group, MV_U32 mask);

DECLARE_GLOBAL_DATA_PTR;

/*
 *
 */
int mvSikluCpuGpioSetVal(int gppNum, int val)
{
    int rc = 0;
    int group = 0;

    if (gppNum >= 32)
    {
        group = 1;
        gppNum -= 32;
    }

    val = !!val;
    mvGppTypeSet(group, 1 << gppNum, (MV_GPP_OUT & (1 << gppNum))); // configure gppX as output
    mvGppValueSet(group, 1 << gppNum, (val << gppNum));

    return rc;

}
/*
 *
 */
int mvSikluCpuGpioSetDirection(int gppNum, int isOutput)
{
    int rc = 0;
    int group = 0;

    if (gppNum >= 32)
    {
        group = 1;
        gppNum -= 32;
    }

    if (isOutput)
    {
        mvGppTypeSet(group, 1 << gppNum, (MV_GPP_OUT & (1 << gppNum))); // configure gppX as output
    }
    else
    {
        mvGppTypeSet(group, 1 << gppNum, (MV_GPP_IN & (1 << gppNum))); // configure gppX as input
    }

    return rc;

}
/*
 *
 */
int mvSikluCpuGpioGetVal(int gppNum, int* val)
{
    int rc = 0;
    int group = 0;

    if (gppNum >= 32)
    {
        gppNum -= 32;
        group = 1;
    }

    *val = !!mvGppValueGet(group, 1 << gppNum);
    return rc;
}

#define CONFIG_PCA9557_BUS_NUM 1
#define CONFIG_PCA9557_DEV_ADDR	0x18

#define PCA9557_INPUT_PORT_REG		0x00
#define PCA9557_OUTPUT_PORT_REG		0x01
#define PCA9557_POLAR_INVERT_REG	0x02
#define PCA9557_CONFIG_REG			0x03

#define PCA9557_GPIO_POE_PARS			0 // input
#define PCA9557_GPIO_SFP_P3_TX_DIS		1 // output
#define PCA9557_GPIO_SFP_P2_TX_DIS		2 // output
#define PCA9557_GPIO_WG0_DIS			3 // output
#define PCA9557_GPIO_WG1_DIS			4 // output
#define PCA9557_GPIO_WLAN_DIS			5 // output
#define PCA9557_GPIO_BLE_DIS			6 // output
#define PCA9557_GPIO_VHV_DIS			7 // output

/*
 * I/O extender
 0	PoE_Pairs		In		PoE Pairs status (0: 2 pairs, 1: 4 pairs)
 1	PSE_P3_RST_n	Out		SFP_P3_TX_DIS	PSE ports 3 Rest or SFP port 3 Tx Disable
 2	PSE_P2_RST_n	Out		SFP_P2_TX_DIS	PSE ports 2 Rest or SFP port 2 Tx Disable
 3	WG0_DISABLE_n	Out		WiGig Modem 0 Chip Disable (active low)
 4	WG1_DISABLE_n	Out		WiGig Modem 1 Chip Disable (active low)
 5	WLAN_Disable	Out		WLAN (mini-PCIe) Disable (active low)
 6	BLE_RST_n		Out		BLE chip Reset (active low)
 7	VHV enable		Out		Enable 1.8V for eFuse burning
 */
static int mvSikluExtndrGpioConf(void)
{
    int rc = 0, count;

    mvSikluExtndrGpioSetDirection(0, 0);
    for (count = 1; count <= 7; count++)
    {
        mvSikluExtndrGpioSetDirection(count, 1);
    }

    int old_bus = i2c_get_bus_num();
    i2c_set_bus_num(CONFIG_PCA9557_BUS_NUM);

    i2c_reg_write(CONFIG_PCA9557_DEV_ADDR, PCA9557_POLAR_INVERT_REG, 0); // disable polarity inversion for all inputs
    i2c_reg_write(CONFIG_PCA9557_DEV_ADDR, PCA9557_OUTPUT_PORT_REG, 0x0); // set GPIO[7..1] be output val = 0 (all in reset)

    i2c_set_bus_num(old_bus);
    return rc;
}
/*
 * Configure SoC MPP pins siklu_remarkM11
 */
static int mvSikluCpuGpioConf(void)
{
    int rc = 0;

    // configure MPP6 GPHY port2 reset, active low output
    mvSikluCpuGpioSetDirection(6, 1);
    mvSikluCpuGpioSetVal(6, 1); // change to reset

    // configure MPP12 Power Led Yellow output
    mvSikluCpuGpioSetDirection(12, 1);

    // configure MPP13 SFP P2 exists input
    mvSikluCpuGpioSetDirection(13, 0);
    // configure MPP14 SFP P2 FAULT input
    mvSikluCpuGpioSetDirection(14, 0);
    // configure MPP15-18  HW ID input
    mvSikluCpuGpioSetDirection(15, 0);
    mvSikluCpuGpioSetDirection(16, 0);
    mvSikluCpuGpioSetDirection(17, 0);
    mvSikluCpuGpioSetDirection(18, 0);

    // configure MPP21 Powe LED Green output
    mvSikluCpuGpioSetDirection(21, 1);

    // configure MPP43 SFP P3 LOS	input
    mvSikluCpuGpioSetDirection(43, 0);

    // configure MPP44 WIGIG0 CHIP reset active low output
    mvSikluCpuGpioSetDirection(44, 1);
    mvSikluCpuGpioSetVal(44, 1); // change to reset!
    // configure MPP47 WIGIG1 CHIP reset active low output
    mvSikluCpuGpioSetDirection(47, 1);
    mvSikluCpuGpioSetVal(47, 1); // change to reset!
    // configure MPP48 RF LED Green output
    mvSikluCpuGpioSetDirection(48, 1);
    // configure MPP49 RF LED Yellow  	output
    mvSikluCpuGpioSetDirection(49, 1);

    // configure MPP50 PHY Port3 LED output
    mvSikluCpuGpioSetDirection(50, 1);
    mvSikluCpuGpioSetVal(50, 1); // Write '1' turns led off
    // configure MPP51 WIGIG2 CHIP reset active low output
    mvSikluCpuGpioSetDirection(51, 1);
    mvSikluCpuGpioSetVal(51, 1); // change to reset!
    // configure MPP52 RST Factory default input
    mvSikluCpuGpioSetDirection(52, 0);

    // configure MPP53  PHY P1 Reset output
    mvSikluCpuGpioSetDirection(53, 1);
    mvSikluCpuGpioSetVal(53, 1); // change to reset!
    // configure MPP54  SFP P3 Exists  input
    mvSikluCpuGpioSetDirection(54, 0);
    // configure MPP55 	SFP P3 Fault input
    mvSikluCpuGpioSetDirection(55, 0);

    return rc;
}

/*
 *  IIC PCA9557 GPI Extender for 8 IO
 */
int mvSikluExtndrGpioSetVal(int gpio, int val)
{
    int rc = 0;
    if (gpio >= 8)
        return -1;

    int old_bus = i2c_get_bus_num();
    i2c_set_bus_num(CONFIG_PCA9557_BUS_NUM);
    MV_U8 reg_val = i2c_reg_read(CONFIG_PCA9557_DEV_ADDR,
    PCA9557_OUTPUT_PORT_REG);
    if (val)
        reg_val |= (1 << gpio);
    else
        reg_val &= ~(1 << gpio);

    i2c_reg_write(CONFIG_PCA9557_DEV_ADDR, PCA9557_OUTPUT_PORT_REG, reg_val);
    i2c_set_bus_num(old_bus);

    return rc;
}
/*
 *  IIC PCA9557 GPI Extender for 8 IO
 */
int mvSikluExtndrGpioSetDirection(int gpio, int isOutput)
{
    int rc = 0;
    if (gpio >= 8)
        return -1;

    int old_bus = i2c_get_bus_num();
    i2c_set_bus_num(CONFIG_PCA9557_BUS_NUM);
    MV_U8 reg_val = i2c_reg_read(CONFIG_PCA9557_DEV_ADDR,
    PCA9557_CONFIG_REG);
    if (!isOutput)
        reg_val |= (1 << gpio);
    else
        reg_val &= ~(1 << gpio);

    i2c_reg_write(CONFIG_PCA9557_DEV_ADDR, PCA9557_CONFIG_REG, reg_val);
    i2c_set_bus_num(old_bus);

    return rc;
}
/*
 *  IIC PCA9557 GPI Extender for 8 IO
 */
int mvSikluExtndrCpuGpioGetVal(int gpio, int* val)
{
    int rc = 0;
    if (gpio >= 8)
        return -1;

    int old_bus = i2c_get_bus_num();
    i2c_set_bus_num(CONFIG_PCA9557_BUS_NUM);
    MV_U8 reg_val = i2c_reg_read(CONFIG_PCA9557_DEV_ADDR,
    PCA9557_INPUT_PORT_REG);
    if (reg_val & (1 << gpio))
        *val = 1;
    else
        *val = 0;
    i2c_set_bus_num(old_bus);
    return rc;
}

/* ##########################################################################################################
 ########################################################################################################## */
static int do_siklu_pca9557_access(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]) //
{
    int rc = CMD_RET_SUCCESS;
    MV_U8 reg_val;
    MV_U8 reg_addr;

    int old_bus = i2c_get_bus_num();
    i2c_set_bus_num(CONFIG_PCA9557_BUS_NUM);

    switch (argc)
    {
    case 1: // no args, print all registers
    {
        int count;
        for (count = PCA9557_INPUT_PORT_REG; count <= PCA9557_CONFIG_REG; count++)
        {
            reg_val = i2c_reg_read(CONFIG_PCA9557_DEV_ADDR, count);
            printf(" 0x%x: val 0x%x\n", count, reg_val);
        }
    }
        break;
    case 2: // read specif register
        reg_addr = simple_strtoul(argv[1], NULL, 16);
        reg_val = i2c_reg_read(CONFIG_PCA9557_DEV_ADDR, reg_addr);
        printf(" 0x%x: val 0x%x\n", reg_addr, reg_val);
        break;
    case 3: // write specif register
        reg_addr = simple_strtoul(argv[1], NULL, 16);
        reg_val = simple_strtoul(argv[2], NULL, 16);
        i2c_reg_write(CONFIG_PCA9557_DEV_ADDR, reg_addr, reg_val);
        break;
    }

    i2c_set_bus_num(old_bus);
    return rc;
}
/*
 *
 */
int mvSikluHwResetCntrl(SKL_MODULE_RESET_CNTRL_E dev, int isEna)
{
    int rc = 0;

    switch (dev)
    {
    case SKL_WIGIG0_RF_RESET:
        mvSikluExtndrGpioSetVal(PCA9557_GPIO_WG0_DIS, !isEna);
        break;
    case SKL_WIGIG1_RF_RESET:
        mvSikluExtndrGpioSetVal(PCA9557_GPIO_WG1_DIS, !isEna);
        break;
    case SKL_WIFI_RESET:
        mvSikluExtndrGpioSetVal(PCA9557_GPIO_WLAN_DIS, !isEna);
        break;
    case SKL_BLE_RESET:
        mvSikluExtndrGpioSetVal(PCA9557_GPIO_BLE_DIS, !isEna);
        break;
    case SKL_WIGIG0_CHIP_RESET:
        mvSikluCpuGpioSetVal(44, !isEna);
        break;
    case SKL_WIGIG1_CHIP_RESET:
        mvSikluCpuGpioSetVal(47, !isEna);
        break;
    case SKL_WIGIG2_CHIP_RESET:
        mvSikluCpuGpioSetVal(51, !isEna);
        break;
    case SKL_GPHY_0_RESET:
        mvSikluCpuGpioSetVal(53, !isEna);
        break;
    case SKL_GPHY_1_RESET:
        mvSikluCpuGpioSetVal(6, !isEna);
        break;
    case SKL_GPHY_2_RESET:
        mvSikluCpuGpioSetVal(50, !isEna);
        break;
    default:
        break;
    }

    return rc;
}

int arch_early_init_r(void)
{

    // configure IIC GPIO Extender
    mvSikluExtndrGpioConf();

    // configure CPU GPIO
    mvSikluCpuGpioConf();

    // siklu_remarkM11 - follow part remove after debug. we do not need PCIe enabled by default
    mvSikluHwResetCntrl(SKL_WIGIG0_RF_RESET, 0); // enable WIGIG radio ???
    mvSikluHwResetCntrl(SKL_WIGIG1_RF_RESET, 0); // enable WIGIG radio ???
    mvSikluHwResetCntrl(SKL_WIFI_RESET, 0);
    mvSikluHwResetCntrl(SKL_BLE_RESET, 1);

    mvSikluHwResetCntrl(SKL_WIGIG0_CHIP_RESET, 0);
    mvSikluHwResetCntrl(SKL_WIGIG1_CHIP_RESET, 0);
    mvSikluHwResetCntrl(SKL_WIGIG2_CHIP_RESET, 0);

    mvSikluHwResetCntrl(SKL_GPHY_0_RESET, 0);
    mvSikluHwResetCntrl(SKL_GPHY_1_RESET, 0);
    mvSikluHwResetCntrl(SKL_GPHY_2_RESET, 0);

    udelay(10000);

    return 0;
}
/*
 * set POWER and WLAN LEDs
 */
static int siklu_set_led_by_mpp(SKL_BOARD_LED_TYPE_E led, SKL_BOARD_LED_MODE_E mode)
{
    int rc = 0;

    if (led == SKL_LED_WLAN)
    { // mpp48 and mpp49
        mvSikluCpuGpioSetDirection(48, 1); // set output
        mvSikluCpuGpioSetDirection(49, 1); // set output
        switch (mode)
        {
        case SKL_LED_MODE_OFF: // mpp12='h', mpp21='h'
            mvSikluCpuGpioSetVal(48, 1);
            mvSikluCpuGpioSetVal(49, 1);
            break;
        case SKL_LED_MODE_GREEN: // mpp49='h', mpp48='l'
            mvSikluCpuGpioSetVal(49, 1);
            mvSikluCpuGpioSetVal(48, 0);
            break;
        case SKL_LED_MODE_YELLOW: // mpp49='l', mpp48='h'
            mvSikluCpuGpioSetVal(48, 1);
            mvSikluCpuGpioSetVal(49, 0);
            break;
        default:
            return -1;
        }

    }
    else if (led == SKL_LED_POWER) // mpp12 and mpp21
    {
        mvSikluCpuGpioSetDirection(12, 1); // set output
        mvSikluCpuGpioSetDirection(21, 1); // set output

        switch (mode)
        {
        case SKL_LED_MODE_OFF: // mpp12='h', mpp21='h'
            mvSikluCpuGpioSetVal(12, 1);
            mvSikluCpuGpioSetVal(21, 1);
            break;
        case SKL_LED_MODE_GREEN: // mpp12='h', mpp21='l'
            mvSikluCpuGpioSetVal(12, 1);
            mvSikluCpuGpioSetVal(21, 0);
            break;
        case SKL_LED_MODE_YELLOW: // mpp12='l', mpp21='h'
            mvSikluCpuGpioSetVal(12, 0);
            mvSikluCpuGpioSetVal(21, 1);
            break;
        default:
            return -1; // unsupported mode
            break;
        }
    }
    return rc;
}
/*
 * Set variable LED modes via 88e1512 PHY
 */
static int siklu_set_eth_led(SKL_BOARD_LED_TYPE_E led, SKL_BOARD_LED_MODE_E mode)
{
    __u32 phy_addr;
    __u32 bank = 3;
    __u32 reg_addr = 16; // see Datasheet
    __u16 reg_val = 0x101E;  // this is a default value of register 16(0x10) in bank 3

    int rc = 0;
    switch (led)
    {
    case SKL_LED_ETH0:
        phy_addr = 0;
        break;
    case SKL_LED_ETH1:
        phy_addr = 1;
        break;
    case SKL_LED_ETH2:
    default:
        return -1; // unsupported led
        break;
    }
    reg_val &= 0xFF00; // preset mask
    switch (mode)
    {
    case SKL_LED_MODE_OFF:
        reg_val |= 0x0088;
        break;
    case SKL_LED_MODE_GREEN:
        reg_val |= 0x0089;
        break;
    case SKL_LED_MODE_YELLOW:
        reg_val |= 0x0098;
        break;
    case SKL_LED_MODE_GREEN_BLINK:
        reg_val |= 0x008B;
        break;
    case SKL_LED_MODE_YELLOW_BLINK:
        reg_val |= 0x00B8;
        break;
    default:
        return -1; // unsupported mode
        break;

    }

    rc = siklu_88e512_phy_write(phy_addr, bank, reg_addr, reg_val);

    printf("%s() phy_addr %x, bank %x, reg_addr %x, reg_val %x, rc %d\n", __func__, phy_addr, bank, reg_addr, reg_val,
            rc); // edikk remove

    return rc;
}
/*
 *
 */
int siklu_set_led(SKL_BOARD_LED_TYPE_E led, SKL_BOARD_LED_MODE_E mode)
{
    int rc = 0;

    switch (led)
    {
    case SKL_LED_ETH0:
    case SKL_LED_ETH1:
    case SKL_LED_ETH2:
        rc = siklu_set_eth_led(led, mode);
        break;
    case SKL_LED_WLAN:
    case SKL_LED_POWER:
        rc = siklu_set_led_by_mpp(led, mode);
        break;
    case SKL_LED_BLE:
    default:
        return -1; // no handler!
        break;
    }
    return rc;
}

/*
 *
 */
static int do_siklu_pca9557_config(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]) //
{
    int rc = CMD_RET_SUCCESS;
    mvSikluExtndrGpioConf();

    return rc;
}

typedef enum
{
    BIST_MODE_DISABLED = 0, //
    BIST_MODE_ON = 1, //
    BIST_MODE_AND_MONITORING = 2, //
    BIST_MODE_LAST = BIST_MODE_AND_MONITORING, //
} BIST_MODE_E;

/*
 *
 */
static int do_siklu_board_bist_mode(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int rc = CMD_RET_FAILURE; // return false for prevent command be repeatable

    if (argc == 1) // show current mode
    {
        char *bist_state;
        bist_state = getenv(SIKLU_BIST_ENVIRONMENT_NAME);

        if (bist_state == NULL)
        {
            printf("No BIST mode\n");
        }
        else
        {
            BIST_MODE_E state = (BIST_MODE_E) simple_strtol(bist_state, NULL, 10);
            switch (state)
            {
            case BIST_MODE_DISABLED:
                printf("No BIST mode\n");
                break;
            case BIST_MODE_ON:
                printf("System in BIST mode\n");
                break;
            case BIST_MODE_AND_MONITORING:
                printf("System in BIST mode with Monitoring\n");
                break;
            default:
                printf("Wrong BIST mode! Disable BIST for future runs\n");
                setenv(SIKLU_BIST_ENVIRONMENT_NAME, NULL);
                saveenv();
                break;
            }
        }
    }

    else if (argc == 2)
    {
        // set new BIST mode
        BIST_MODE_E bist_mode = simple_strtoul(argv[1], NULL, 10);

        switch (bist_mode)
        {
        case BIST_MODE_DISABLED:
            printf("Disable BIST mode\n");
            setenv(SIKLU_BIST_ENVIRONMENT_NAME, NULL);
            break;
        case BIST_MODE_ON:
            printf("Set System in BIST mode\n");
            setenv(SIKLU_BIST_ENVIRONMENT_NAME, "1");
            break;
        case BIST_MODE_AND_MONITORING:
            printf("System in BIST mode with Monitoring\n");
            setenv(SIKLU_BIST_ENVIRONMENT_NAME, "2");
            break;
        default:
            printf("Wrong BIST mode! Disable BIST for future runs\n");
            setenv(SIKLU_BIST_ENVIRONMENT_NAME, NULL);
            break;
        }
        saveenv();
    }
    else
    {
        // wrong arguments
        printf("Wrong arguments\n");
        printf("Usage:\n%s\n", cmdtp->usage);
        return CMD_RET_FAILURE;
    }

    return rc;
}

static int do_siklu_board_diplay_hw_info(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int rc = CMD_RET_SUCCESS;
    struct spi_flash *flash = NULL;
    char buffer[32];
    int mpp15, mpp16, mpp17, mpp18;
    extern int seeprom_get_assembly_type_v1(char* assembly);
// octeon_model_get_string_buffer(cvmx_get_proc_id(), buffer);
// do not change format below! each new line should include "key" and "value" delimited by ":" !!!
//printf("Product name        : %s\n", siklu_get_board_product_name());
    seeprom_get_assembly_type_v1(buffer);
    printf("Board HW name       : %s\n", buffer);
//printf("CPU type            : 0x%02x  (%s)\n", siklu_get_cpu_type(), buffer);
//printf("Core clock          : %lld MHz\n", DIV_ROUND_UP(cvmx_clock_get_rate(CVMX_CLOCK_CORE), 1000000));
//printf("IO clock            : %lld MHz\n", divide_nint(cvmx_clock_get_rate(CVMX_CLOCK_SCLK), 1000000));
    printf("DDR clock           : %u MHz\n", gd->ddr_clk);
//printf("Board ID            : 0x%02x\n", siklu_get_board_hw_major()); // read from 4bits CPU GPIO
//printf("CPLD version        : 0x%02x\n", siklu_get_cpld_ver());
//printf("CPLD board version  : 0x%02x\n", siklu_get_cpld_board_ver());
//printf("Assembly version    : 0x%02x\n", siklu_get_assembly());
//printf("Num ETH ports       : %d\n", siklu_get_product_num_eth_ports());

    extern struct spi_flash *get_spi_flash_data(void);

    flash = get_spi_flash_data();
    if (flash)
        printf("SF                  : %s\n", flash->name);

// Display HW ID  MPP input pins 15-18  siklu_remarkM06
    mvSikluCpuGpioSetDirection(15, 0);
    mvSikluCpuGpioSetDirection(16, 0);
    mvSikluCpuGpioSetDirection(17, 0);
    mvSikluCpuGpioSetDirection(18, 0);

    mvSikluCpuGpioGetVal(15, &mpp15);
    mvSikluCpuGpioGetVal(16, &mpp16);
    mvSikluCpuGpioGetVal(17, &mpp17);
    mvSikluCpuGpioGetVal(18, &mpp18);
    printf("HW ID               : %x\n", (mpp18 << 3) | (mpp17 << 2) | (mpp16 << 1) | (mpp15 << 0));

    return rc;
}

/*
 * Notice: access internal Marvell CPU regs occured via offset - #define INTER_REGS_BASE         0xF1000000
 *
 *
 */
static int do_siklu_rtc_correction_factor(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int rc = CMD_RET_FAILURE; // return false for prevent command be repeatable
#define RTC_CLOCK_CORRECTION_REGISTER  0xA3818

    if (argc == 1) // show current mode
    {
        volatile unsigned int temp;
        temp = MV_REG_READ(RTC_CLOCK_CORRECTION_REGISTER);
        printf("%d %d\n", temp & (1 << 15), temp & ((1 << 15) - 1));
        rc = CMD_RET_SUCCESS;
    }
    else if (argc == 3) // set mode
    {
        volatile unsigned int mode, val;
        mode = simple_strtoul(argv[1], NULL, 10) & 0x1;
        val = simple_strtoul(argv[2], NULL, 10) & 0x7fff;
        MV_REG_WRITE(RTC_CLOCK_CORRECTION_REGISTER, (mode << 15) | val);
        printf(" Done\n");
        rc = CMD_RET_SUCCESS;
    }
    else
    {
        // wrong arguments
        printf("Wrong arguments\n");
        printf("Usage:\n%s\n", cmdtp->usage);
        rc = CMD_RET_FAILURE;
    }

    return rc;
}

/*
 *
 */
static int do_siklu_access_mrv_regs(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int rc = CMD_RET_FAILURE; // return false for prevent command be repeatable
    volatile unsigned int offs, val;

    if (argc == 2) // show register value
    {
        offs = simple_strtoul(argv[1], NULL, 16);
        val = MV_REG_READ(offs);
        printf("reg 0x%x, val 0x%x\n", offs, val);
        rc = CMD_RET_SUCCESS;
    }
    else if (argc == 3) // set register value
    {
        offs = simple_strtoul(argv[1], NULL, 16);
        val = simple_strtoul(argv[2], NULL, 16);
        MV_REG_WRITE(offs, val);
        rc = CMD_RET_SUCCESS;
    }
    else
    {
        // wrong arguments
        printf("Wrong arguments\n");
        printf("Usage:\n%s\n", cmdtp->usage);
        rc = CMD_RET_FAILURE;
    }

    return rc;
}

/*
 *
 */
static int do_siklu_poe_num_pairs_show(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    MV_U8 num_pairs;

    int old_bus = i2c_get_bus_num();
    i2c_set_bus_num(CONFIG_PCA9557_BUS_NUM);

    num_pairs = i2c_reg_read(CONFIG_PCA9557_DEV_ADDR, PCA9557_INPUT_PORT_REG) & 0x01;
    if (num_pairs)
        printf("\t4 pairs\n");
    else
        printf("\t2 pairs\n");

    i2c_set_bus_num(old_bus);
    return CMD_RET_SUCCESS;
}
/*
 * 
 */
static int do_siklu_push_button_stat_show(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int rc = CMD_RET_SUCCESS;
    int val;

    mvSikluCpuGpioSetDirection(52, 0);
    mvSikluCpuGpioGetVal(52, &val);
    if (val)
        printf("\tPB released\n");
    else
        printf("\tPB pressed\n");

    return rc;
}

static int do_siklu_marvell_mpp_control(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int rc = CMD_RET_SUCCESS;
    int mpp, val;

    mpp = simple_strtoul(argv[1], NULL, 10);
    val = !!simple_strtoul(argv[2], NULL, 10);

    switch (mpp)
    {
    case 12:
    case 21:
    case 48:
    case 49:
        mvSikluCpuGpioSetDirection(mpp, 1); // set output
        mvSikluCpuGpioSetVal(mpp, val);
        break;
    default:
        printf("Wrong MPP Value\n");
        break;
    }

    return rc;
}

static int do_siklu_board_led_control(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int rc = CMD_RET_SUCCESS;
    char led[30];
    char state[30];
    SKL_BOARD_LED_MODE_E _mode;
    SKL_BOARD_LED_TYPE_E _led;

    if (argc == 3)
    {
        strcpy(led, argv[1]);
        strcpy(state, argv[2]);
    }
    else
    {
        printf("sled [led] [state]\n");
        printf(" led:   ble/wlan/eth0/eth1/eth2/power\n");
        printf(" state: \n\to - off\n\tg - green\n\ty - yellow\n\tgb - green blink\n\tyb - yellow blink\n");
        return rc;
    }

    if (strcmp(led, "ble") == 0)
    {
        _led = SKL_LED_BLE;

    } //
    else if (strcmp(led, "wlan") == 0)
    {
        _led = SKL_LED_WLAN;
    } //
    else if (strcmp(led, "eth0") == 0)
    {
        _led = SKL_LED_ETH0;
    } //
    else if (strcmp(led, "eth1") == 0)
    {
        _led = SKL_LED_ETH1;
    } //
    else if (strcmp(led, "eth2") == 0)
    {
        _led = SKL_LED_ETH2;
    } //
    else if (strcmp(led, "power") == 0)
    {
        _led = SKL_LED_POWER;
    } //
    else
    {
        printf("Wrong LED type\n");
        return CMD_RET_USAGE;
    }

    if (strcmp(state, "o") == 0)
    {
        _mode = SKL_LED_MODE_OFF;
    }
    else if (strcmp(state, "g") == 0)
    {
        _mode = SKL_LED_MODE_GREEN;
    }
    else if (strcmp(state, "y") == 0)
    {
        _mode = SKL_LED_MODE_YELLOW;
    }
    else if (strcmp(state, "gb") == 0)
    {
        _mode = SKL_LED_MODE_GREEN_BLINK;
    }
    else if (strcmp(state, "yb") == 0)
    {
        _mode = SKL_LED_MODE_YELLOW_BLINK;
    }
    else
    {
        printf("Wrong LED mode\n");
        return CMD_RET_USAGE;
    }

    rc = siklu_set_led(_led, _mode);
    if (rc != 0)
    {
        printf(" Error or unsupported mode\n");
        rc = CMD_RET_SUCCESS; // return success here
    }

    return rc;
}

//############################################################################################
//############################################################################################
//############################################################################################
//############################################################################################

U_BOOT_CMD(spca9557, 7, 1, do_siklu_pca9557_access, "Read/Write PCA9557 IIC Extender", //
        "[reg] [val*] Read/Write PCA9557 IIC Extender");

U_BOOT_CMD(spca9557c, 7, 1, do_siklu_pca9557_config, "Config PCA9557 IIC Extender to default values", //
        "Config PCA9557 IIC Extender to default values");

U_BOOT_CMD(sbist, 5, 1, do_siklu_board_bist_mode, "Set board to BIST Mode", "0-off,1-bist,2-bist with monitoring");
U_BOOT_CMD(shw, 5, 1, do_siklu_board_diplay_hw_info, "Display Board HW info", " Display Board HW info");

U_BOOT_CMD(srtccf, 5, 1, do_siklu_rtc_correction_factor, "Show/Set Internal CPU RTC correction factor",
        " [Mode 0/1]* [Decimal Value]* Without params - show");

U_BOOT_CMD(smrvr, 5, 1, do_siklu_access_mrv_regs, "Access Marvell SoC registers", "[reg] [val*] Show/Set Reg val");

U_BOOT_CMD(spbs, 3, 1, do_siklu_push_button_stat_show, "Show Siklu board Push-Button Status",
        "Show Siklu board Push-Button Status");

U_BOOT_CMD(spoe, 3, 1, do_siklu_poe_num_pairs_show, "Show POE number pairs Status", "Show POE number pairs Status");

U_BOOT_CMD(smpp, 3, 1, do_siklu_marvell_mpp_control, "Control CPU MPP 12/21/48/49 Control",
        "[mpp_num] [0/1] Set 0/1 on required MPP number");

U_BOOT_CMD(sled, 3, 1, do_siklu_board_led_control, "Control Onboard LEDs", "[led] [state] Control Onboard LEDs");

