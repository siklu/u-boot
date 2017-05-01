/*
 * mvBoardSikluMutualEnvCntrl.c
 *
 *  Created on: Jul 6, 2016
 *      Author: edwardk
 *
 *      based on SDK310 siklu_mute_env_ctrl.c file
 */



#include <common.h>
#include <linux/ctype.h>
#include <nand.h>
#include <siklu_api.h>



static const char* trailer = "SKLTRAILER";
static const char* final = "FINALSKL";

#define ENV_STRING_MAX_VAL 0x100



static int siklu_mutable_env_power_up_set(const char *varname, const char *varvalue);
static int siklu_mutable_env_init(void);
static int _siklu_mutable_env_set(const char *varname, const char *varvalue);

#define MAX_MUT_ENV_STRING_SIZE 50
typedef struct
{
    char name[MAX_MUT_ENV_STRING_SIZE];
    char value[MAX_MUT_ENV_STRING_SIZE];
} mut_env_pair_S;

#define MAX_MUT_ENV_AVAIL 10
static mut_env_pair_S mut_env_pair[MAX_MUT_ENV_AVAIL];
static uint32_t mut_env_in_nand_flash_start = 0; //  = get_nand_part_offset_by_name("env_var0");
static int is_mutable_env_initiated = 0;

/*
 *   1          2 3                                    4          5
 *   #SKLTRAILER#3#SK_accept 1;SK_primary_image 2;ccc 3#2489927954#FINALSKL
 */
static int is_mutual_env_valid(char* buff)
{

    size_t i;
    size_t buf_len = strlen(buff);
    int count = 0;

    // printf("%s() process env string:   %s\n", __func__, buff);
    // check first character is '#'
    if (buff[0] != '#')
    {
        printf("%s() FAIL on line %d\n", __func__, __LINE__);
        return -1;
    }

    // 1. check next string after '#' is trailer
    if (strncmp(buff + 1, trailer, strlen(trailer)))
    {
        printf("%s() FAIL on line %d\n", __func__, __LINE__);
        return -1;
    }

    // check after trailer char '#' and after it digit in range [0..9]   (TBD)

    // find 5th character '#'
    for (i = 0; i < buf_len; i++)
    {
        if (buff[i] == '#')
            count++;
        if (count == 5)
            break;
    }
    if (i == buf_len)
    {
        printf("%s() FAIL on line %d\n", __func__, __LINE__);
        return -1;
    }
    if (count != 5)
    {
        printf("%s() FAIL on line %d\n", __func__, __LINE__);
        return -1;
    }

    // after it exists string final
    if (strncmp(buff + 1 + i, final, strlen(final)))
    {
        printf("%s() FAIL on line %d\n", __func__, __LINE__);
        return -1;
    }
    return 0;
}

#define MUTUAL_ENV_STRING_STEP 0x1000 // 4k
#define NUM_ENV_COPIES 5

/*
 *
 *
 */
static int siklu_update_mutual_env(uint32_t env_part_offs)
{
    int rc = 0;
    // erase section before write
    rc = run_command("nand erase.part env_var0", 0);
    const char* primary_image = siklu_mutable_env_get("SK_primary_image");
    const char *try_count = siklu_mutable_env_get("SK_try_count");
    const char* accept = siklu_mutable_env_get("SK_accept");
    const char* sbist =  siklu_mutable_env_get(SIKLU_BIST_MUT_ENVIRONMENT_NAME);
    char buff1[ENV_STRING_MAX_VAL];
    char buff2[ENV_STRING_MAX_VAL];
    int env_count = 0;
    int offs = 0;
    int i;
    uint32_t nand_write_addr;
    uint32_t crc;

    memset(buff1, 0, sizeof(buff1));
    memset(buff2, 0, sizeof(buff2));

    if (primary_image)
    {
        env_count++;
        offs += sprintf(buff1 + offs, "SK_primary_image %s;", primary_image);
    }
    if (try_count)
    {
        env_count++;
        offs += sprintf(buff1 + offs, "SK_try_count %s;", try_count);
    }
    if (accept)
    {
        env_count++;
        offs += sprintf(buff1 + offs, "SK_accept %s;", accept);
    }
    if (sbist)
    {
        env_count++;
        offs += sprintf(buff1 + offs, "%s %s;", SIKLU_BIST_MUT_ENVIRONMENT_NAME, sbist);
    }

    crc = crc32(0, (uint8_t*) buff1, (uint32_t) strlen(buff1));

    sprintf(buff2, "#%s#%d#%s#%x#%s", trailer, env_count, buff1, crc, final);
    //printf("New mutual environment:    %s\n", buff1); //
    nand_write_addr = env_part_offs;
    for (i = 0; i < NUM_ENV_COPIES; i++)
    {
        char cmd[100];
        memset(cmd, 0, sizeof(cmd));
        sprintf(cmd, "nand write %p %x %x", buff2, nand_write_addr, strlen(buff2) + 1);
        //printf("Execute command:    %s\n", cmd);
        rc = run_command(cmd, 0);
        if (rc != 0)
        {
            printf(" Error on line %d\n", __LINE__);
            break;
        }
        nand_write_addr += MUTUAL_ENV_STRING_STEP;
    }
    return rc;
}

/*
 * by default preset SK_primary_image to '0'
 */
int primary_format_mutual_env(uint32_t env_part_offs)
{

    char buff1[100];
    char buff2[100];
    int rc = 0;
    int i;
    uint32_t crc;
    uint32_t nand_write_addr;

    printf("Primary format mutual environment\n"); //
    // erase section before write
    rc = run_command("nand erase.part env_var0", 0);

    _siklu_mutable_env_set("SK_primary_image", "0");
    //printf("%s()   Called\n", __func__);
    sprintf(buff1, "SK_primary_image 0;");
    crc = crc32(0, (uint8_t*) buff1, (uint32_t) strlen(buff1));
    sprintf(buff2, "#%s#1#%s#%x#%s", trailer, buff1, crc, final);

    nand_write_addr = env_part_offs;
    for (i = 0; i < NUM_ENV_COPIES; i++)
    {
        char cmd[100];
        memset(cmd, 0, sizeof(cmd));
        sprintf(cmd, "nand write %p %x %x", buff2, nand_write_addr, strlen(buff2) + 1);
        //printf("Execute command:    %s\n", cmd);
        rc = run_command(cmd, 0);
        if (rc != 0)
        {
            printf(" Error on line %d\n", __LINE__);
            break;
        }
        nand_write_addr += MUTUAL_ENV_STRING_STEP;
    }
    return rc;
}

/*
 * returns
 *  0 -  Valid environment exists and copied to a buf
 *  -1 - No valid environment
 *
 */
static int siklu_find_valid_env(char* buff, uint32_t start_nand_offs)
{
    int i, rc = -1;

    // find right environment string
    uint32_t nand_read_addr = start_nand_offs;
    for (i = 0; i < NUM_ENV_COPIES; i++)
    {
        char cmd[100];

        sprintf(cmd, "nand read %p %x %x", buff, nand_read_addr, ENV_STRING_MAX_VAL - 1);
        //printf("Execute command:    %s\n", cmd);
        rc = run_command(cmd, 0);
        if (rc != 0)
        {
            printf(" Error on line %d\n", __LINE__);
            break;
        }
        // protect buffer
        buff[ENV_STRING_MAX_VAL - 1] = 0;
        if (is_mutual_env_valid((char*) buff) == 0)
        {
            // printf(" Find valid environment on NAND offset %x\n", nand_read_addr);
            rc = 1;
            break;
        }
        nand_read_addr += MUTUAL_ENV_STRING_STEP;
    }

    return rc;
}

/*
 *   1          2 3                                    4          5
 *   #SKLTRAILER#3#SK_accept 1;SK_primary_image 2;ccc 3#2489927954#FINALSKL
 */
static int siklu_parse_mutial_env_string2tokens(char* buff)
{
    int rc = 0;
    char env_temp[ENV_STRING_MAX_VAL];
    char crc[ENV_STRING_MAX_VAL];
    size_t i;
    size_t buf_len = strlen(buff);
    int count = 0;
    int num_env_vars = 0;
    int start_env_offs, end_env_offs;
    int start_crc_offs, end_crc_offs;

    memset(env_temp, 0, sizeof(env_temp));
    memset(crc, 0, sizeof(crc));

    start_env_offs = end_env_offs = 0;
    start_crc_offs = end_crc_offs = 0;

    // extract number after 2nd '#'
    for (i = 0; i < buf_len; i++)
    {
        if (buff[i] == '#')
            count++;
        if ((count == 2) && (num_env_vars == 0))
        {
            num_env_vars = buff[i + 1] - '0';
            //printf("%s() num_env_vars=%d\n", __func__, num_env_vars);
        }

        if ((count == 3) && (start_env_offs == 0))
        {
            start_env_offs = i + 1;
            //printf("%s() find  start_env_offs=%d\n", __func__, start_env_offs);
        }

        if ((count == 4) && (end_env_offs == 0))
        {
            end_env_offs = i;
            start_crc_offs = i + 1;
            //printf("%s() find  end_env_offs=%d, start_crc_offs %d\n", __func__, end_env_offs, start_crc_offs);
        }

        if ((count == 5) && (end_crc_offs == 0))
        {
            end_crc_offs = i;
            //printf("%s() find  end_crc_offs=%d\n", __func__, end_crc_offs);
            break;
        }

    }

    // check parsing validity
    if ((i >= buf_len) || (count != 5) || (!start_env_offs) || (!end_env_offs) || (!start_crc_offs) || (!end_crc_offs))
    {
        printf("%s() Parse environment error line %d\n", __func__, __LINE__);
        return -1;
    }

    //printf("%s()  count %d, start_env_offs %d, end_env_offs %d, start_crc_offs %d,end_crc_offs %d, num_env_vars %d\n",
    //        __func__, count, start_env_offs, end_env_offs, start_crc_offs, end_crc_offs, num_env_vars);

    // copy environment in a temp string
    memcpy(env_temp, buff + start_env_offs, end_env_offs - start_env_offs);
    memcpy(crc, buff + start_crc_offs, end_crc_offs - start_crc_offs);

    uint32_t calc_crc = crc32(0, (uint8_t*) env_temp, (uint32_t) strlen(env_temp));

    //printf("%s()  <env_temp> %s, <crc> %s, calculated crc 0x%x\n", __func__, env_temp, crc, calc_crc);
    uint32_t crc_from_env = simple_strtoul(crc, NULL, 16);
    if (crc_from_env != calc_crc)
    {
        printf("%s() Wrong CRC! calculated 0x%x, presents 0x%x\n", __func__, calc_crc, crc_from_env);
        return -1;
    }

    // parsae individual env variables
    i = 0;
    int env_len = strlen(env_temp);
    int env_offs = 0;
    // printf("Mutual environment:\n"); //
    while (num_env_vars > 0)
    {
        // first env var starts from offset 0;
        for (; i < env_len; i++)
        {
            if (env_temp[i] == ';')
            {
                env_temp[i] = 0;
                //printf("%s()   Find environment variable:   \"%s\"\n", __func__, env_temp + env_offs);
                char* f = env_temp + env_offs;      // first
                char* s = f + 1;                    // second

                for (; s < env_temp + i; s++)
                {
                    if (*s == ' ')
                    {
                        *s = 0;
                        s++;
                        //printf("   %s = %s\n", f, s);
                        // setenv(f, s); // store mutual environments in a global environment space
                        siklu_mutable_env_power_up_set(f, s);
                    }
                }
                env_offs = i + 1;
            }
        }
        num_env_vars--;
    }
    return rc;
}

/*
 * returns:     1 - required update environment
 *              0 - no need update
 *
 */
static int siklu_boot_process_control(void)
{
    int rc = 0;
    const char* primary_image = siklu_mutable_env_get("SK_primary_image");
    // preset by default "SK_primary_image" environment
    if (!primary_image)
    {
        printf("%s() SK_primary_image isn't set! restore it\n", __func__);
         _siklu_mutable_env_set("SK_primary_image", "0");
        return 1;
    }
    else
    {
        if (!((primary_image[0] == '0') || (primary_image[0] == '1')))
        {
            printf("%s() Error \"SK_primary_image\" value %s. restore default\n", __func__, primary_image);
            _siklu_mutable_env_set("SK_primary_image", "0");
            primary_image = siklu_mutable_env_get("SK_primary_image");
            rc = 1;
        }
    }

    /* check presence "SK_bist" mutual environment variable
      * if it presence with value '1' or '2'  configure sbist mode
      */
    const char* mut_bist = siklu_mutable_env_get(SIKLU_BIST_MUT_ENVIRONMENT_NAME); // string presences in mutual environment
    const char *uboot_bist = getenv(SIKLU_BIST_ENVIRONMENT_NAME);// string presences in regular uboot environment
    if ((mut_bist)&&(!uboot_bist) && ( ((strcmp (mut_bist, "1") == 0)) || (strcmp (mut_bist, "2") == 0))) {
        // we fall here if bist mode set in mutual environment but doesn't set un regular environment
    	// this occurs only in the case that bist mode configured via hwtest utility!
        char cmd[100];
    	printf("The System enters BIST mode due to SW demand\n"); // siklu_remark002
        sprintf(cmd, "sbist %s", mut_bist);
        rc = run_command(cmd, 0);
        // now we should remove BIST flag from mutual environment
        siklu_mutable_env_set(SIKLU_BIST_MUT_ENVIRONMENT_NAME, NULL,1);
    }

    // check presents an value of "SK_try_count" variable
    const char *try_count = siklu_mutable_env_get("SK_try_count");
    if (!try_count)
    {
        // regular boot process, no need swap
        return 0;
    }
    if (*try_count == '0')
    {
        // need swap when the value reach ZERO!
        printf("SK_try_count reaches 0, swap partitions\n");
        if (primary_image[0] == '0')
        {
            _siklu_mutable_env_set("SK_primary_image", "1");
        }
        else if (primary_image[0] == '1')
        {
            _siklu_mutable_env_set("SK_primary_image", "0");
        }
        // delete the environment
        _siklu_mutable_env_set("SK_try_count", NULL);
        rc = 1;
    }
    else
    {
        // decrement try_count
        char new_try_count[MAX_MUT_ENV_STRING_SIZE];
        *new_try_count = *try_count - 1;
        *(new_try_count + 1) = 0;
        _siklu_mutable_env_set("SK_try_count", new_try_count);
        rc = 1;
    }

     return rc;
}

static int siklu_mutable_env_init(void)
{

    static int rc = 0;
    char buff[ENV_STRING_MAX_VAL];
    int is_valid_env = 0; // is environment valid flag by default FALSE

    //  printf("%s()  called line %d\n",__func__, __LINE__);
    if (is_mutable_env_initiated) {
    	//  printf("%s()  called line %d\n",__func__, __LINE__);
        return rc;
    }

    // printf("%s()  called, process required\n",__func__);
    memset(mut_env_pair, 0, sizeof(mut_env_pair));
    //  preset mtdpartitions to default map
    rc = run_command("mtdparts default", 0);

    mut_env_in_nand_flash_start = get_nand_part_offset_by_name("env_var0");
    if (mut_env_in_nand_flash_start == (uint32_t) -1)
    {
        printf("\"env_var0\" partition is omitted!\n");
        rc = -1;
        return -1;
    }

    is_valid_env = siklu_find_valid_env(buff, mut_env_in_nand_flash_start);

    if (!is_valid_env)
    {
        printf("No valid environment!\n");
        rc = -1;
        return -1;
    }

    // parse environment on tokens
    // printf("%s()   Parse environment string: \"%s\"\n", __func__,buff );
    rc = siklu_parse_mutial_env_string2tokens(buff);
    if (rc != 0)
    {
    	printf("%s() Parse tokens fail\n",__func__);
        primary_format_mutual_env(mut_env_in_nand_flash_start);
        return 0;
    }
    else
    {
    	;//printf("%s() Buffer: \"%s\"\n",__func__, buff);
    }

    is_mutable_env_initiated = 1; // call it only once
    rc = 0;
    return 0;
}

/*
 * controls 3 mutable environment variables located in env_var0
 * entry point for a process
 */
int siklu_mutable_env_cntrl(int stage)
{
    int rc = 0;

    // printf("%s()   Called line %d\n", __func__, __LINE__);  //

    rc = siklu_mutable_env_init();
    if (rc != 0)
    {
        // wrong mutual environment requires rescue restore process!
        printf("Wrong environment! Rescue process required\n");
        return -1;
    }

    // update system behavior depend on env
    rc = siklu_boot_process_control();
    if (rc)
    {
        // printf("%s() called, rc = %d\n", __func__, rc);
        rc = siklu_update_mutual_env(mut_env_in_nand_flash_start);
    }
    else
    {
        ; //printf("%s()  No need update environment\n",__func__);
    }
    // printf("%s()   Called line %d\n", __func__, __LINE__);  //
    return 0;
}

int siklu_mutable_env_show(void)
{
    int rc = 0;
    int count = 0;

    printf(" == Mutual Environment ==\n");
    for (count = 0; count < MAX_MUT_ENV_AVAIL; count++)
    {
        mut_env_pair_S* pair = &mut_env_pair[count];
        if (strlen(pair->name) == 0)
            continue;
        if (strlen(pair->value) > 0)
        {
            printf("%s=%s\n", pair->name, pair->value);
        }
    }
    return rc;
}

/*
 * set and store! an environment
 * ret value:   1 - mutual environment store area should be updated
 *              0 - don't need update
 */
static int _siklu_mutable_env_set(const char *varname, const char *varvalue)
{

    int count = 0;

    for (count = 0; count < MAX_MUT_ENV_AVAIL; count++)
    {
        mut_env_pair_S* pair = &mut_env_pair[count];
        if (strlen(pair->name) == 0)
            continue;

        if (strcmp(varname, pair->name) == 0) // we found required environment
        {
            if ((varvalue == NULL) || (strcmp(varvalue, "") == 0))
            {
                memset(pair, 0, sizeof(*pair));
            }
            else
            {
                if (strcmp(pair->value, varvalue) == 0)
                {
                    return 0;
                }
                else
                {
                    strncpy(pair->value, varvalue, MAX_MUT_ENV_STRING_SIZE - 1);
                }
            }
            return 1; // required store
        }
    }
    if ((varvalue == NULL) || (strcmp(varvalue, "") == 0))
    {
        // unknown environment should be deleted. do nothing
        return 0;
    }
    // this is a new environment
    for (count = 0; count < MAX_MUT_ENV_AVAIL; count++)
    {
        mut_env_pair_S* pair = &mut_env_pair[count];
        if (strlen(pair->name) == 0)
        { // store new environment here
            strncpy(pair->name, varname, MAX_MUT_ENV_STRING_SIZE - 1);
            strncpy(pair->value, varvalue, MAX_MUT_ENV_STRING_SIZE - 1);
            return 1;
        }
    }
    printf("Error! max number mutual environments enriched!\n");
    return 0;
}

int siklu_mutable_env_set(const char *varname, const char *varvalue, int save_if_diff_required)
{

    if (!is_mutable_env_initiated)
        return 0;

    int rc = _siklu_mutable_env_set(varname, varvalue);
    if ((save_if_diff_required) && (rc == 1))
    {
        siklu_update_mutual_env(mut_env_in_nand_flash_start);
    }
    return 0;
}

/*
 * initiates environments in a RAM only once on power-up
 */
static int siklu_mutable_env_power_up_set(const char *varname, const char *varvalue)
{

    // siklu_mutable_env_init();
    _siklu_mutable_env_set(varname, varvalue);
    return 0;
}

const char* siklu_mutable_env_get(const char *varname)
{
    int count = 0;
    if (!is_mutable_env_initiated)
        return NULL;

    // siklu_mutable_env_init();
    for (count = 0; count < MAX_MUT_ENV_AVAIL; count++)
    {
        mut_env_pair_S* pair = &mut_env_pair[count];
        if (strlen(pair->name) == 0)
            continue;
        if (strcmp(varname, pair->name) == 0) // we found required environment
        {
            return pair->value;
        }
    }
    return NULL;
}
/*
 *  siklu_remark002
 */
void siklu_print_mut_env_area(void)
{
    char buff[ENV_STRING_MAX_VAL];
    int is_valid_env = 0; // is environment valid flag by default FALSE

    mut_env_in_nand_flash_start = get_nand_part_offset_by_name("env_var0");
    if (mut_env_in_nand_flash_start == (uint32_t) -1)
    {
        printf("\"env_var0\" partition is omitted!\n");
        return;
    }
    is_valid_env = siklu_find_valid_env(buff, mut_env_in_nand_flash_start);
    if (!is_valid_env)
    {
        printf("No valid environment!\n");
        return;
    }

    int i, k;
    for (k = 0; k < ((sizeof(buff)) / 0x10); k++)
    {
        printf("\n[0x%02x]: ", k * 0x10);
        for (i = 0; i < 0x10; i++)
            printf("%02x ", buff[(k * 0x10) + i] & 0xFF);
        printf(" - ");
        for (i = 0; i < 0x10; i++)
        {
            char* p = &buff[(k * 0x10) + i];
            *p &= 0xFF;
            printf("%c", isprint(*p) ? (*p) : ('.'));
        }
    }
    printf("\n");

}
