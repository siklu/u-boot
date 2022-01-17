/*
 * mvBoardSikluNandTest.c
 *
 *  Created on: Apr 22, 2018
 *      Author: edwardk
 */

#include <common.h>
#include <command.h>
#include "mvCommon.h"
#include "mvOs.h"
#include <siklu_api.h>
#include "siklu_board_system.h"

#define REFERENCE_RAND_ARRAY_ADDR	0x1000000
#define READ_BACK_RAND_ARRAY_ADDR	0x2000000
#define RAND_ARRAY_SIZE		0x10000 // 64k

extern unsigned int rand(void);
extern void srand(unsigned int seed);

/*
 *
 */
static int init_rand_array(void) {
	int count;
	uint32_t* p = (uint32_t*) REFERENCE_RAND_ARRAY_ADDR;

	srand(get_timer(0));

	for (count = 0; count < (RAND_ARRAY_SIZE / sizeof(int)); count++) {
		*p = rand();
	}
	return 0;
}

/*
 *
 *
 *
 */
static int do_siklu_nand_test(cmd_tbl_t *cmdtp, int flag, int argc,
		char * const argv[]) {

	int ret = 0, count;
	int is_error = 0;
#define NAND_TEST_EXEC_TIMES	100000
	uint32_t num_tests = NAND_TEST_EXEC_TIMES;


    if (argc == 2) // show register value
    {
    	num_tests = simple_strtoul(argv[1], NULL, 10);
    }

	// Execute N times write-read-compare
	for (count = 0; count < num_tests; count++) {
		char cmd[100];

		if (ctrlc())
			break;

		printf(" === cycle %d ===\n", count);

		//=======      Step #1 Create in RAM test Image
		init_rand_array();

		//=======     Erase partition before write
		sprintf(cmd, "nand erase.part %s", "env_var1");
		// printf("Execute command:    %s\n", cmd); // edikk
		ret = run_command(cmd, 0);
		if (ret != 0) {
			printf(" Error on line %d\n", __LINE__);
			is_error = 1;
			break;
		}

		sprintf(cmd, "nand write 0x%x %s 0x%x", REFERENCE_RAND_ARRAY_ADDR,
				"env_var1",
				RAND_ARRAY_SIZE);
		//=======      Step #2 Write test Image
		// printf("Execute command:    %s\n", cmd); // edikk
		ret = run_command(cmd, 0);
		if (ret != 0) {
			printf(" Error on line %d\n", __LINE__);
			is_error = 1;
			break;
		}

		//=======      Step #3 Read test Image
		sprintf(cmd, "nand read 0x%x %s 0x%x", READ_BACK_RAND_ARRAY_ADDR,
				"env_var1",
				RAND_ARRAY_SIZE);
		// printf("Execute command:    %s\n", cmd); // edikk
		ret = run_command(cmd, 0);
		if (ret != 0) {
			printf(" Error on line %d\n", __LINE__);
			is_error = 1;
			break;
		}

		//=======      Step #4 Compare

		if (memcmp((void*) REFERENCE_RAND_ARRAY_ADDR,
				(void*) READ_BACK_RAND_ARRAY_ADDR, RAND_ARRAY_SIZE) != 0) {
			printf(
					" ERROR: Original and target arrays are different! count %d\n",
					count);
			is_error = 1;
			break;
		}
	}

	if (is_error == 0) {
		printf("Test Ok\n");
	}

	return ret;
}

U_BOOT_CMD(snandt, 3, 1, do_siklu_nand_test, "Board NAND Test",
		"[num_loops]* - Board NAND Test");
