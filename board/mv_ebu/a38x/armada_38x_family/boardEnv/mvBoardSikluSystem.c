/*
 * mvBoardSikluSystem.c
 *
 *  Created on: Jul 6, 2016
 *      Author: edwardk
 */




#include <common.h>
#include <linux/ctype.h>
#include <nand.h>
#include <siklu_api.h>


static int is_event_rest2fact_dflt = 0; // by default no


/*
 * TODO port and complete restore to factory default process.
 */
int siklu_is_restore2fact_default(void)
{
    return is_event_rest2fact_dflt;
}


/*
 * After execute rescue restore uimage the card should execute
 * reboot. Follow procedure wait 4 seconds (TBD) user input for
 * prevent Card reboot.
 *
 *
 */
void siklu_wait_user4prevent_card_reboot(void)
{
    int count = 0;
    printf("\nPress any key for prevent System restart...\n");
    for (count = 0; count < 40; count++)
    {
        printf("\b\b\b\b\b\b\b\b -> %d", count);
        if (tstc())
        { /* we got a key press   */
            (void) getc(); /* consume input    */
            puts("\b\b\b\b\b\b\b\b\n");
            return;
        }
        mdelay(100);
    }
    puts("\b\b\b\b\b\b\b\b\n");
    run_command("reset", 0);
}

