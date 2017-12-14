/*
 * watchdog.c - driver for i.mx on-chip watchdog
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <asm/io.h>
#include <watchdog.h>
#include <asm/arch/imx-regs.h>
#include <fsl_wdog.h>





static int do_siklu_wdog_reg_show(cmd_tbl_t * cmdtp, int flag, int argc,
		char * const argv[]) {
	int rc = CMD_RET_SUCCESS;
	struct watchdog_regs *wdog = (struct watchdog_regs *)WDOG1_BASE_ADDR;

		printf(" wcr  0x%04x\n", wdog->wcr);/* Control */
		printf(" wsr  0x%04x\n", wdog->wsr);/* Service */
		printf(" wrsr 0x%04x\n", wdog->wrsr);/* Reset Status */

	return rc;
}


#ifdef CONFIG_IMX_WATCHDOG
void hw_watchdog_reset(void)
{
	struct watchdog_regs *wdog = (struct watchdog_regs *)WDOG1_BASE_ADDR;

	writew(0x5555, &wdog->wsr);
	writew(0xaaaa, &wdog->wsr);
}

void hw_watchdog_init(void)
{
	struct watchdog_regs *wdog = (struct watchdog_regs *)WDOG1_BASE_ADDR;
	u16 timeout;

	/*
	 * The timer watchdog can be set between
	 * 0.5 and 128 Seconds. If not defined
	 * in configuration file, sets 128 Seconds
	 */
#ifndef CONFIG_WATCHDOG_TIMEOUT_MSECS
#define CONFIG_WATCHDOG_TIMEOUT_MSECS 128000
#endif
	timeout = (CONFIG_WATCHDOG_TIMEOUT_MSECS / 500) - 1;
	writew(WCR_WDZST | WCR_WDBG | WCR_WDE | WCR_WDT | WCR_SRS |
		WCR_WDA | SET_WCR_WT(timeout), &wdog->wcr);
	hw_watchdog_reset();
}
#endif

void __attribute__((weak)) reset_cpu(ulong addr)
{
	struct watchdog_regs *wdog = (struct watchdog_regs *)WDOG1_BASE_ADDR;

	clrsetbits_le16(&wdog->wcr, WCR_WT_MSK, WCR_WDE);

	writew(0x5555, &wdog->wsr);
	writew(0xaaaa, &wdog->wsr);	/* load minimum 1/2 second timeout */
	while (1) {
		/*
		 * spin for .5 seconds before reset
		 */
	}
}


U_BOOT_CMD(wdogs, 5, 0, do_siklu_wdog_reg_show, "Show WDOG Regs",
		" Show WDOG Regs");

