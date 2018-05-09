/*
 * pcf8523.c
 *
 *  Created on: Aug 13, 2015
 *      Author: edwardk
 */

#include <common.h>
#include <command.h>
#include <rtc.h>
#include <i2c.h>



#if defined(CONFIG_CMD_DATE)

#define CONFIG_SYS_RTC_PCF8523_BUS_NUM CONFIG_SYS_RTC_BUS_NUM

/*---------------------------------------------------------------------*/
#undef DEBUG_RTC
// #define DEBUG_RTC

#ifdef DEBUG_RTC
#define DEBUGR(fmt,args...) printf(fmt ,##args)
#else
#define DEBUGR(fmt,args...)
#endif
/*---------------------------------------------------------------------*/

#ifndef CONFIG_SYS_I2C_RTC_ADDR
# define CONFIG_SYS_I2C_RTC_ADDR    0x68
#endif

/*
 * RTC register addresses
 */

#define REG_CONTROL1 0x00
#define REG_CONTROL1_CAP_SEL (1 << 7)
#define REG_CONTROL1_STOP    (1 << 5)

#define REG_CONTROL2 0x01

#define REG_CONTROL3 0x02
#define REG_CONTROL3_PM_BLD (1 << 7) /* battery low detection disabled */
#define REG_CONTROL3_PM_VDD (1 << 6) /* switch-over disabled */
#define REG_CONTROL3_PM_DSM (1 << 5) /* direct switching mode */
#define REG_CONTROL3_PM_MASK 0xe0
#define REG_CONTROL3_BLF (1 << 2) /* battery low bit, read-only */

#define REG_SECONDS  0x03
#define REG_SECONDS_OS (1 << 7)

#define REG_MINUTES  0x04
#define REG_HOURS    0x05
#define REG_DAYS     0x06
#define REG_WEEKDAYS 0x07
#define REG_MONTHS   0x08
#define REG_YEARS    0x09

#define REG_TMR_CLKOUT_CTRL 0x0f


static uchar rtc_read(uchar reg);
static void rtc_write(uchar reg, uchar val);


/*    internal oscillator capacitor selection for quartz
      crystals with a corresponding load capacitance
        bit 7 value:      0- 7 pF
                          1- 12.5 pF
*/
static int pcf8523_select_capacitance(int high)
{
    uchar value;
    int err = 0;

    value = rtc_read( REG_CONTROL1);
    if (!high)
        value &= ~REG_CONTROL1_CAP_SEL;
    else
        value |= REG_CONTROL1_CAP_SEL;
    rtc_write(REG_CONTROL1, value);
    return err;
}

static int pcf8523_set_pm(uchar pm)
{
    uchar value;
    int err = 0;

    value = rtc_read(REG_CONTROL3);
    value = (value & ~REG_CONTROL3_PM_MASK) | (pm<<5);
    rtc_write(REG_CONTROL3, value);
    return err;
}

static int pcf8523_stop_rtc(void)
{
    uchar value;
    int err = 0;

    DEBUGR("%s() Called, line %d\n", __func__, __LINE__);
    value = rtc_read(REG_CONTROL1);
    value |= REG_CONTROL1_STOP;
    rtc_write( REG_CONTROL1, value);
    return err;
}

static int pcf8523_start_rtc(void)
{
    uchar value;
    int err = 0;
    DEBUGR("%s() Called, line %d\n", __func__, __LINE__);
    value = rtc_read(REG_CONTROL1);
    value &= ~REG_CONTROL1_STOP;
    rtc_write( REG_CONTROL1, value);
    return err;
}

/*
 * Get the current time from the RTC
 */
int rtc_get(struct rtc_time *tmp)
{
    int rc = 0;
    uchar sec, min, hour, mday, wday, mon, year;

    int old_bus = i2c_get_bus_num();

    sec = rtc_read(REG_SECONDS);
    min = rtc_read(REG_MINUTES);
    hour = rtc_read(REG_HOURS);
    wday = rtc_read(REG_WEEKDAYS);
    mday = rtc_read(REG_DAYS);
    mon = rtc_read(REG_MONTHS);
    year = rtc_read(REG_YEARS);

    if (sec & REG_SECONDS_OS)
    {
        /*
         * If the oscillator was stopped, try to clear the flag. Upon
         * power-up the flag is always set, but if we cannot clear it
         * the oscillator isn't running properly for some reason. The
         * sensible thing therefore is to return an error, signalling
         * that the clock cannot be assumed to be correct.
         */
        sec &= ~REG_SECONDS_OS;
        rtc_write(REG_SECONDS, sec);

    }

    tmp->tm_sec = bcd2bin(sec & 0x7f);
    tmp->tm_min = bcd2bin(min & 0x7f);
    tmp->tm_hour = bcd2bin(hour & 0x3f);
    tmp->tm_mday = bcd2bin(mday & 0x3f);
    tmp->tm_wday = wday & 0x7;
    tmp->tm_mon = bcd2bin(mon & 0x1f);
    tmp->tm_year = bcd2bin(year) + (bcd2bin(year) >= 70 ? 1900 : 2000);

    DEBUGR("Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n", tmp->tm_year, tmp->tm_mon, tmp->tm_mday,
            tmp->tm_wday, tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

    i2c_set_bus_num(old_bus);

    return rc;
}

/*
 * Set the RTC
 */
int rtc_set(struct rtc_time *tmp)
{
    int old_bus = i2c_get_bus_num();

    DEBUGR("Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n", tmp->tm_year, tmp->tm_mon, tmp->tm_mday,
            tmp->tm_wday, tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

    if (tmp->tm_year < 1970 || tmp->tm_year > 2069)
        printf("WARNING: year should be between 1970 and 2069!\n");

    pcf8523_stop_rtc();
    rtc_write(REG_SECONDS, bin2bcd(tmp->tm_sec));
    rtc_write(REG_MINUTES, bin2bcd(tmp->tm_min));
    rtc_write(REG_HOURS, bin2bcd(tmp->tm_hour));
    rtc_write(REG_DAYS, bin2bcd(tmp->tm_mday));
    rtc_write(REG_WEEKDAYS, tmp->tm_wday);
    rtc_write( REG_MONTHS, bin2bcd(tmp->tm_mon));
    rtc_write( REG_YEARS, bin2bcd(tmp->tm_year % 100));

    //additional configuration required for right chip mode. same setup configured in linux
    rtc_write( REG_TMR_CLKOUT_CTRL, 0x38); // disable clock out

    // set capacitor value according to board type

    pcf8523_select_capacitance(0); // set 7pF
    //  pcf8523_select_capacitance(1); // if not SIKLU board set 12.5pF

    rtc_write( REG_CONTROL2, 0); // disable all alarms


#if 0    // do not call the function, instead it set directly full register value + disable interrupts
    pcf8523_set_pm(5);   //battery switch-over function is enabled in direct switching mode;
    //battery low detection function is disabled
#else
    (void)pcf8523_set_pm; // prevent warning
	/*
		30 jan 2018. Value 0b100 required to solve problem with batteries V > 3.5V (overcharged)
		Write the same value as in CVMX UBOOT. See
			main_cvmx/sdk_310n/bootloader/u-boot_PCB15x_stage3_SDK310/drivers/rtc/pcf8523.c
			in http://pax/software/main_cvmx
	*/
    rtc_write( REG_CONTROL3, 0x80);

#endif

    pcf8523_start_rtc();
    i2c_set_bus_num(old_bus);
    return 0;
}

/*
 * Reset the RTC. We setting the date back to 1970-01-01.
 * We also enable the oscillator output on the SQW/OUT pin and program
 * it for 32,768 Hz output. Note that according to the datasheet, turning
 * on the square wave output increases the current drain on the backup
 * battery to something between 480nA and 800nA.
 */
void rtc_reset(void)
{
    struct rtc_time tmp;

    DEBUGR("%s() Called, line %d\n", __func__, __LINE__);
    tmp.tm_year = 1970;
    tmp.tm_mon = 1;
    tmp.tm_mday = 1;
    tmp.tm_hour = 0;
    tmp.tm_min = 0;
    tmp.tm_sec = 0;

    rtc_set(&tmp);

    DEBUGR("RTC:   %4d-%02d-%02d %2d:%02d:%02d UTC\n", tmp.tm_year, tmp.tm_mon, tmp.tm_mday, tmp.tm_hour, tmp.tm_min,
            tmp.tm_sec);

    return;
}

/*
 * Helper functions
 */

static uchar rtc_read(uchar reg)
{
    uchar val;
    int old_bus = i2c_get_bus_num();
    i2c_set_bus_num(CONFIG_SYS_RTC_PCF8523_BUS_NUM);
    val = (i2c_reg_read(CONFIG_SYS_I2C_RTC_ADDR, reg));
    i2c_set_bus_num(old_bus);
    return val;
}

static void rtc_write(uchar reg, uchar val)
{
    int old_bus = i2c_get_bus_num();
    i2c_set_bus_num(CONFIG_SYS_RTC_PCF8523_BUS_NUM);
    i2c_reg_write(CONFIG_SYS_I2C_RTC_ADDR, reg, val);
    i2c_set_bus_num(old_bus);
}

uchar pcf8523_read(uchar reg)
{
    return rtc_read(reg);
}
void pcf8523_write(uchar reg, uchar val)
{
    return rtc_write(reg, val);
}

static int do_siklu_rtc_display_regs(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int rc = CMD_RET_SUCCESS;


    printf( " 0x%2x - 0x%2x\n", 0, pcf8523_read(0));
    printf( " 0x%2x - 0x%2x\n", 1, pcf8523_read(1));
    printf( " 0x%2x - 0x%2x\n", 2, pcf8523_read(2));

    return rc;
}


U_BOOT_CMD(srtcrr, 5, 1, do_siklu_rtc_display_regs, "Display PCF8523 Control Regs", " Display PCF8523 Control Regs");




#endif
