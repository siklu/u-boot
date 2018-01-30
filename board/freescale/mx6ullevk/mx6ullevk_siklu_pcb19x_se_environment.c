/*
 * mx6ullevk_siklu_pcb19x_se_environment.c
 *
 *  Created on: Jan 29, 2018
 *      Author: edwardk
 */

#include <common.h>
#include <cli.h>
#include <command.h>
#include <console.h>
#include <environment.h>
#include <search.h>
#include <errno.h>
#include <malloc.h>
#include <mapmem.h>
#include <watchdog.h>
#include <linux/stddef.h>
#include <asm/byteorder.h>
#include <asm/io.h>

#include "siklu_def.h"
#include "siklu_api.h"

typedef struct {
	u8 occup;
	char key[KEY_VAL_FIELD_SIZE];
	char val[KEY_VAL_FIELD_SIZE];
} key_val_pair_t;
static key_val_pair_t key_val_pair[SYSEEPROM_NUM_FIELDS];

#define VALID_KEY_WORD 0xDEADBEAF
typedef struct {
	union {
		struct {
			struct control_info {
				uint32_t valid_key;  // should be VALID_KEY_WORD
				uint32_t crc; /* CRC32 over data bytes	*/
				int data_size;
			} control_info;
			char data[1]; /* Environment data		*/
		} info;
		char buff[SIKLU_SF_ENV_SIZE];
	};
} sf_env_siklu_se_t __attribute__((aligned(16)));
static sf_env_siklu_se_t sf_env_siklu_se;
static sf_env_siklu_se_t* p_sf_env_siklu_se = &sf_env_siklu_se;

static const char siklu_default_environment_se[] = { //
		"SE_product_name=WIGIG_ST" ";" //
						"SE_mac=00:24:a4:00:de:ad" ";"//
						"SE_board_serial=F123456789" ";"//
						"SE_system_serial=0" ";"//
						"SE_port_map=c---" ";" "\0" //
		};

/*
 * called if SYSEEPROM data was damaged or not yet created
 */
int siklu_syseeprom_restore_default(void) {
	int rc = 0;

	// printf("%s() called, line %d\n", __func__, __LINE__); // edikk remove

	memset(p_sf_env_siklu_se->buff, 0, sizeof(sf_env_siklu_se_t));
	memcpy(p_sf_env_siklu_se->info.data, siklu_default_environment_se,
			sizeof(siklu_default_environment_se));
	p_sf_env_siklu_se->info.control_info.data_size = strlen(
			siklu_default_environment_se);
	p_sf_env_siklu_se->info.control_info.crc = crc32(0L,
			(unsigned char *) p_sf_env_siklu_se->info.data,
			p_sf_env_siklu_se->info.control_info.data_size);
	p_sf_env_siklu_se->info.control_info.valid_key = VALID_KEY_WORD;

	rc = siklu_sf_sys_eeprom_write(p_sf_env_siklu_se->buff,
			sizeof(sf_env_siklu_se_t));
	return rc;
}

static int siklu_parse_pair(char* buf, key_val_pair_t* p_key_val) {
	int rc = -1;
	char* key;
	char* val;

	memset(p_key_val, 0, sizeof(key_val_pair_t));

	key = buf;
	val = memchr(key, '=', KEY_VAL_FIELD_SIZE);
	if (val) {
		memcpy(p_key_val->key, key, (size_t) (val - key));
		val += 1; // next char after '='
		strcpy(p_key_val->val, val);
		// printf("%s()  key.val  %s.%s\n", __func__, p_key_val->key,	p_key_val->val); // edikk remove
		rc = 0;
	}
	return rc;
}

static int siklu_fill_pair(char* buf) {
	int rc = 0, i = 0;
	key_val_pair_t key_val_temp;

	rc = siklu_parse_pair(buf, &key_val_temp);
	if (rc != 0) {
		printf("%s() error on line %d\n", __func__, __LINE__);
		return rc;
	}

	for (i = 0; i < SYSEEPROM_NUM_FIELDS; i++) {
		key_val_pair_t* p_key_val = key_val_pair + i;

		if (p_key_val->occup) {
			// check does we already have same key
			if (strcmp(p_key_val->key, key_val_temp.key) == 0) {
				printf("We already have a key, update value and exit TBD\n");
				return 0;
			} else
				continue;
		}
		// new entry
		memcpy(p_key_val, &key_val_temp, sizeof(key_val_pair_t));
		p_key_val->occup = 1;
		// printf("%s() [%d] key.val  %s.%s\n", __func__, i, p_key_val->key, p_key_val->val); // edikk remove

		return rc;
	}

	printf("%s()  No free entries for buf: %s, i-%d\n", __func__, buf, i);

	return rc;
}

/*
 * Called once on power up
 */
static int siklu_fill_tupples_from_sf(sf_env_siklu_se_t* p_sf_env_siklu_se) {
	int rc = 0;
	int i = 0;
	char* s = p_sf_env_siklu_se->info.data;

	while (i < p_sf_env_siklu_se->info.control_info.data_size) {
		// find end of current env
		char* p = memchr(s, ';', 2 * KEY_VAL_FIELD_SIZE);
		if (p) {
			// ok we found current pair key-val
			char buf[2 * KEY_VAL_FIELD_SIZE];
			memset(buf, 0, sizeof(buf));
			memcpy(buf, s, (size_t) (p - s));
			// printf("find env - %s\n", buf); // edikk remove
			// fill tuple
			siklu_fill_pair(buf);

			i += (strlen(buf) + 1);
			s = p + 1;
		}
	}

	return rc;
}
/*
 * display SYSEEPROM parsed data in memory array
 */
int siklu_syseeprom_display(void) {
	int rc = 0, i;

	for (i = 0; i < SYSEEPROM_NUM_FIELDS; i++) {
		key_val_pair_t* p_key_val = key_val_pair + i;
		if (p_key_val->occup) {
			printf("%s=%s\n", p_key_val->key, p_key_val->val);
		}
	}
	return rc;
}

int siklu_syseeprom_get_val(const char* key, char* val) {
	int rc = 0, i;

	for (i = 0; i < SYSEEPROM_NUM_FIELDS; i++) {
		key_val_pair_t* p_key_val = key_val_pair + i;
		if (p_key_val->occup) {
			if (strncmp(p_key_val->key, key, KEY_VAL_FIELD_SIZE - 1) == 0) {
				strcpy(val, p_key_val->val);
				return 0;
			}
		}
	}
	// not founf :-(
	strcpy(val, "");
	rc = -1;
	return rc;
}
/*
 *
 */
int siklu_syseeprom_set_val(const char* key, const char* val) {
	int rc = 0, i;

	// 1st step does the key already exists
	for (i = 0; i < SYSEEPROM_NUM_FIELDS; i++) {
		key_val_pair_t* p_key_val = key_val_pair + i;
		if (p_key_val->occup) {
			if (strncmp(p_key_val->key, key, KEY_VAL_FIELD_SIZE - 1) == 0) {
				// ok it already exists, update value and exit
				if ((!val) || strlen(val) == 0) {			// clear entry
					memset(p_key_val, 0, sizeof(key_val_pair_t));
				} else
					// udate entry
					strcpy(p_key_val->val, val);
				return 0;
			} else
				continue;
		}
	}
	// if not create new
	for (i = 0; i < SYSEEPROM_NUM_FIELDS; i++) {
		key_val_pair_t* p_key_val = key_val_pair + i;
		if (!p_key_val->occup) {
			strncpy(p_key_val->key, key, KEY_VAL_FIELD_SIZE - 1);
			strncpy(p_key_val->val, val, KEY_VAL_FIELD_SIZE - 1);
			p_key_val->occup = 1;
			return 0;
		}
	}

	// no free entries
	printf("%s()   no free entries for key %s\n", __func__, key);
	rc = -1;

	return rc;
}

static int siklu_syseeprom_build_image(sf_env_siklu_se_t* p_sf_env_siklu_se) {
	int rc = 0, i, offs = 0;
	memset(p_sf_env_siklu_se, 0, sizeof(sf_env_siklu_se));

	for (i = 0; i < SYSEEPROM_NUM_FIELDS; i++) {
		key_val_pair_t* p_key_val = key_val_pair + i;
		if (p_key_val->occup) {
			offs += sprintf(p_sf_env_siklu_se->info.data + offs, "%s=%s;",
					p_key_val->key, p_key_val->val);
		}
	}
	p_sf_env_siklu_se->info.control_info.data_size = offs;
	p_sf_env_siklu_se->info.control_info.crc = crc32(0L,
			(unsigned char *) p_sf_env_siklu_se->info.data,
			p_sf_env_siklu_se->info.control_info.data_size);
	p_sf_env_siklu_se->info.control_info.valid_key = VALID_KEY_WORD;
	return rc;
}

int siklu_syseeprom_udate(void) {
	int rc = 0;

	memset(p_sf_env_siklu_se, 0, sizeof(sf_env_siklu_se));
	rc = siklu_syseeprom_build_image(p_sf_env_siklu_se);
	if (rc == 0)
		rc = siklu_sf_sys_eeprom_write(p_sf_env_siklu_se->buff,
				sizeof(sf_env_siklu_se_t));
	return rc;
}

/*
 *
 */
int siklu_syseeprom_init(void) {
	int rc = -1;
	int err_line = 0;
	u32 crc;

	// clear tupples and structure
	memset(key_val_pair, 0, sizeof(key_val_pair));
	memset(p_sf_env_siklu_se, 0, sizeof(sf_env_siklu_se));

	// read sf syseeprom area to struct
	rc = siklu_sf_sys_eeprom_read(sf_env_siklu_se.buff,
			sizeof(sf_env_siklu_se));
	if (rc != 0) {
		err_line = __LINE__;
		goto _bad_data;
	}

	// check valid key
	if (p_sf_env_siklu_se->info.control_info.valid_key != VALID_KEY_WORD) {
		rc = -1;
		err_line = __LINE__;
		goto _bad_data;
	}
	// check data length
	if (p_sf_env_siklu_se->info.control_info.data_size >= SIKLU_SF_ENV_SIZE) {
		rc = -1;
		err_line = __LINE__;
		goto _bad_data;
	}

	// check crc
	crc = crc32(0L, (unsigned char *) p_sf_env_siklu_se->info.data,
			p_sf_env_siklu_se->info.control_info.data_size);
	if (crc != p_sf_env_siklu_se->info.control_info.crc) {
		rc = -1;
		err_line = __LINE__;
		goto _bad_data;
	}

	// parce data, fill tuples
	rc = siklu_fill_tupples_from_sf(p_sf_env_siklu_se);
	if (rc != 0) {
		err_line = __LINE__;
		goto _bad_data;
	}
	rc = 0;
	return rc;
	_bad_data: //
	printf("%s() Error on line %d\n", __func__, err_line);
	return rc;

}

