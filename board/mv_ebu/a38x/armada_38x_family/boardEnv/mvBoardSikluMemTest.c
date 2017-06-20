/*
 * mvBoardSikluMemTest.c
 *
 *  Created on: Jun 6, 2016
 *      Author: edwardk
 */

#include <common.h>
#include <command.h>

#define PRINT_TEST_PROGRESS_INTERVAL 0x100000

typedef enum {
	EH2000_MEM_TEST_FIRST = 1,
	EH2000_MEM_TEST_RUNNING_ONE = EH2000_MEM_TEST_FIRST,
	EH2000_MEM_TEST_RUNNING_ZERO,
	EH2000_MEM_TEST_CHESS,
	EH2000_MEM_TEST_RANDOM_PTRN,
	EH2000_MEM_TEST_ADDR_2_DATA,
	EH2000_MEM_TEST_LAST
// should be last
} EH2000_MEM_TEST_E;

static inline void flash_all_dcache(void) //
{
	flush_dcache_all();
	invalidate_dcache_all(); // both functions work

}

static long a = 1; // could be made the seed value
static int limit = 0xFFFF;

void rand2_init(int seed) {
	a = seed;
}

//
// returns random integer from 1 to lim (Gerhard's generator)
//
int rand2(void) {

	a = (a * 32719 + 3) % 32749;
	return ((a % limit) + 1);
}

/*
 *
 */
static int eh2000_memtest_running_one(ulong start_addr, ulong end_addr,
		int is_cycling) //
{
	int rc = 0;
	volatile uint* temp_addr;
	int pass_num = 1;

	do {
		if (pass_num > 1)
			printf(" Pass %d\n", pass_num);

		for (temp_addr = (uint*) start_addr; temp_addr < (uint*) end_addr;
				temp_addr++) {

			int i;
			for (i = 0; i < 32; i++) { // running one
				*temp_addr = 1 << i;
				// printf("\rPattern %08X  Writing..."	"%12s" "\b\b\b\b\b\b\b\b\b\b", *temp_addr, "");
				flash_all_dcache();
				if (*temp_addr != 1 << i) {
					printf(
							" Test FAIL on address %p. Read 0x%x, Expected 0x%x\n",
							temp_addr, *temp_addr, 1 << i);
					rc = -1;
					break;
				}
			}
			if (((uint) temp_addr % 0x2000) == 0)
				printf(".");
		}
		if (ctrlc()) // break on Ctrl-C
		{
			break;
		}
		pass_num++;
	} while (is_cycling);

	if (rc == -1)
		printf("\n  Test FAIL\n");
	else
		printf("\n  Test OK\n");

	return rc;
}
/*
 *
 */
static int eh2000_memtest_running_zero(ulong start_addr, ulong end_addr,
		int is_cycling) //
{
	int rc = 0;
	volatile uint* temp_addr;
	int pass_num = 1;

	do {
		if (pass_num > 1)
			printf(" Pass %d\n", pass_num);

		for (temp_addr = (uint*) start_addr; temp_addr < (uint*) end_addr;
				temp_addr++) {

			int i;
			for (i = 0; i < 32; i++) { // running zero
				uint temp = ~(1 << i);
				*temp_addr = temp;
				flash_all_dcache();
				// printf("\rPattern %08X  Writing..." "%12s" "\b\b\b\b\b\b\b\b\b\b", *temp_addr, "");
				if (*temp_addr != temp) {
					printf(
							" Test FAIL on address %p. Read 0x%x, Expected 0x%x\n",
							temp_addr, *temp_addr, temp);
					rc = -1;
					break;
				}
			}
			if (((uint) temp_addr % 0x2000) == 0)
				printf(".");
		}
		if (ctrlc()) // break on Ctrl-C
		{
			break;
		}
		pass_num++;
	} while (is_cycling);

	if (rc == -1)
		printf("\n  Test FAIL\n");
	else
		printf("\n  Test OK\n");

	return rc;
}
/*
 *
 */
static int eh2000_memtest_chess(ulong start_addr, ulong end_addr,
		int is_cycling) {
	int rc = 0;
	volatile uint* temp_addr;
	int pass_num = 1;
	printf("Chess memory test. range [%08lx - %08lx] %s\n\n", start_addr,
			end_addr, (is_cycling) ? ("periodic") : ("one time"));

	do {
		int i = 0;
		// fill area

		if (pass_num > 1)
			printf(" Pass %d\n", pass_num);

		for (temp_addr = (uint*) start_addr; temp_addr < (uint*) end_addr;
				temp_addr++) {

			if ((i % 2) == 0)
				*temp_addr = 0xAAAAAAAA;
			else
				*temp_addr = 0x55555555;

			if (((uint) temp_addr % PRINT_TEST_PROGRESS_INTERVAL) == 0)
				printf(".");
			i++;
		}
		flash_all_dcache();
		//----------// check data
		i = 0;
		printf("\nCheck data...");
		for (temp_addr = (uint*) start_addr; temp_addr < (uint*) end_addr;
				temp_addr++) {

			if ((i % 2) == 0) {
				if (*temp_addr != 0xAAAAAAAA) {
					printf(
							" Test FAIL on address %p. Read 0x%x, Expected 0x%x\n",
							temp_addr, *temp_addr, 0xAAAAAAAA);
					rc = -1;
					break;

				}
			} else {
				if (*temp_addr != 0x55555555) {
					printf(
							" Test FAIL on address %p. Read 0x%x, Expected 0x%x\n",
							temp_addr, *temp_addr, 0x55555555);
					rc = -1;
					break;
				}
			}
			i++;
		}

		if (ctrlc()) // break on Ctrl-C
			break;
		pass_num++;
	} while (is_cycling);

	if (rc == -1)
		printf("  Test FAIL\n");
	else
		printf("  Test OK\n");

	return rc;
}
/*
 *
 */
static int eh2000_memtest_random(ulong start_addr, ulong end_addr,
		int is_cycling) {
	int rc = 0;
	volatile ushort* temp_addr;
	int pass_num = 1;
	ushort seed_offs;
	printf("Random memory fill test.  range [%08lx - %08lx] %s ###\n\n",
			start_addr, end_addr, (is_cycling) ? ("periodic") : ("one time"));

	seed_offs = 0x100; // TBD cvmx_clock_get_count(CVMX_CLOCK_RCLK) & 0xFFFF;

	do {
		if (pass_num > 1) {
			printf(" Pass %d\n", pass_num);
		}

		rand2_init(seed_offs + pass_num);
		// fill area
		for (temp_addr = (ushort*) start_addr; temp_addr < (ushort*) end_addr;
				temp_addr++) {
			*temp_addr = rand2();

			if (((uint) temp_addr % PRINT_TEST_PROGRESS_INTERVAL) == 0)
				printf(".");
		}
		flash_all_dcache();
		//----------// check data
		rand2_init(seed_offs + pass_num);
		printf("\nCheck data...");
		for (temp_addr = (ushort*) start_addr; temp_addr < (ushort*) end_addr;
				temp_addr++) {

			int res = rand2();
			if (*temp_addr != res) {
				printf(" Test FAIL on address %p. Read 0x%x, Expected 0x%lx\n",
						temp_addr, *temp_addr, (ulong) res);
				rc = -1;
				break;
			}
		}

		if (ctrlc()) // break on Ctrl-C
			break;
		pass_num++;
	} while (is_cycling);

	if (rc == -1)
		printf("  Test FAIL\n");
	else
		printf("  Test OK\n");

	return rc;

	return rc;
}
/*
 *
 */
static int eh2000_memtest_addr2data(ulong start_addr, ulong end_addr,
		int is_cycling) {
	int rc = 0;
	volatile uint* temp_addr;
	int pass_num = 1;

	printf("Address to data memory test.  range [%08lx - %08lx] %s\n\n",
			start_addr, end_addr, (is_cycling) ? ("periodic") : ("one time"));

	do {
		if (pass_num > 1)
			printf(" Pass %d\n", pass_num);

		for (temp_addr = (uint*) start_addr; temp_addr < (uint*) end_addr;
				temp_addr++) {

			*temp_addr = (ulong) temp_addr;

			if (((uint) temp_addr % PRINT_TEST_PROGRESS_INTERVAL) == 0)
				printf(".");

		}
		flash_all_dcache();
		//----------// check data
		// i = 0;
		printf("\nCheck data...");
		for (temp_addr = (uint*) start_addr; temp_addr < (uint*) end_addr;
				temp_addr++) {

			if (*temp_addr != (ulong) temp_addr) {
				printf(" Test FAIL on address %p. Read 0x%x, Expected 0x%lx\n",
						temp_addr, *temp_addr, (ulong) temp_addr);
				rc = -1;
				break;
			}
		}

		if (ctrlc()) // break on Ctrl-C
			break;
		pass_num++;
	} while (is_cycling);

	if (rc == -1)
		printf("  Test FAIL\n");
	else
		printf("  Test OK\n");

	return rc;
}

/*
 * do_siklu_memory_ddr3_tests()
 * "[test type] [start addr] [end addr] [is cyclic]"
 * "test type - 1 running '1', 2 running '0', 3 - chess, 4 - random pattern, 5 - addrees to data\n");
 */
static int do_siklu_memory_ddr3_tests(cmd_tbl_t *cmdtp, int flag, int argc,
		char * const argv[]) __attribute__ ((unused));
static int do_siklu_memory_ddr3_tests(cmd_tbl_t *cmdtp, int flag, int argc,
		char * const argv[]) //
{
	int rc = 0;
	ulong start_addr, end_addr;
	EH2000_MEM_TEST_E test_type = EH2000_MEM_TEST_ADDR_2_DATA;
	int is_cyclic = 0;

	// default values
	start_addr = 0x100000;
	end_addr = 0x200000;

	switch (argc) {
	case 5:
		is_cyclic = simple_strtoul(argv[4], NULL, 10);
		// do not put break here!
	case 4:
		start_addr = simple_strtoul(argv[2], NULL, 16);
		end_addr = simple_strtoul(argv[3], NULL, 16);
		// do not put break here!
	case 2:
		test_type = simple_strtoul(argv[1], NULL, 10);
		break;
	default:
		return cmd_usage(cmdtp);
		break;
	}

	// check parameters
	if ((test_type < EH2000_MEM_TEST_FIRST)
			|| (test_type >= EH2000_MEM_TEST_LAST)) {
		printf(" Unknown test type %d\n", test_type);
		return 1;
	}

	start_addr &= 0xFFFFFFFC; // Alignment to 32 bit
	end_addr &= 0xFFFFFFFC;

	if (end_addr <= start_addr) {
		printf(" End address less than start\n");
		return 1;
	}

	switch (test_type) {
	case EH2000_MEM_TEST_RUNNING_ONE:
		rc = eh2000_memtest_running_one(start_addr, end_addr, is_cyclic);
		break;
	case EH2000_MEM_TEST_RUNNING_ZERO:
		rc = eh2000_memtest_running_zero(start_addr, end_addr, is_cyclic);
		break;
	case EH2000_MEM_TEST_CHESS:
		rc = eh2000_memtest_chess(start_addr, end_addr, is_cyclic);
		break;
	case EH2000_MEM_TEST_RANDOM_PTRN:
		rc = eh2000_memtest_random(start_addr, end_addr, is_cyclic);
		break;
	case EH2000_MEM_TEST_ADDR_2_DATA:
		rc = eh2000_memtest_addr2data(start_addr, end_addr, is_cyclic);
		break;
	default:
		break;

	}

	return rc;
}

U_BOOT_CMD(smemt, 7, 1, do_siklu_memory_ddr3_tests,
		"Private Memory Tests", //
		"[test type] [start addr] [end addr] [is cyclic]" "\n test type: 1 running '1', 2 running '0', 3 - chess, 4 - random pattern, 5 - addrees to data\n");
