/** @file
 * Copyright (c) 2019, Arm Limited or its affiliates. All rights reserved.
 * SPDX-License-Identifier : Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
**/

#include "val_interfaces.h"
#include "val_target.h"
#include "test_s008.h"
#ifdef ITS_TEST
#include "test_its_data.h"
#elif PS_TEST
#include "test_ps_data.h"
#endif

#define TEST_BUFF_SIZE 20
#define TEST_MAX_UINT32 0xFFFFFFFF

client_test_t test_s008_sst_list[] = {
    NULL,
    psa_sst_valid_offset_success,
    psa_sst_invalid_offset_failure,
    NULL,
};

static psa_storage_uid_t uid = UID_BASE_VALUE + 5;
static uint8_t read_buff[TEST_BUFF_SIZE];
static uint8_t write_buff[TEST_BUFF_SIZE] = {0x99, 0x01, 0x02, 0x03, 0x04, 0x23, 0xF6, 0x07, 0x08, \
                                  0x0D, 0x70, 0xA1, 0xFF, 0xFF, 0x14, 0x73, 0x46, 0x97, 0xE8, 0xDD};

int32_t psa_sst_invalid_offset_failure(security_t caller)
{
    uint32_t status, j, p_data_length = 0;

    /* Case where offset = data_size +1 , data_len 0. Also check nothing is returned in read buff */
    val->print(PRINT_TEST, "[Check 2] Try to access data with varying invalid offset\n", 0);
    memset(read_buff, 0, TEST_BUFF_SIZE);
    status = SST_FUNCTION(s008_data[6].api, uid, TEST_BUFF_SIZE+1, 0, read_buff, &p_data_length);
    TEST_ASSERT_NOT_EQUAL(status, s008_data[6].status, TEST_CHECKPOINT_NUM(8));
    TEST_ASSERT_EQUAL(p_data_length, 0, TEST_CHECKPOINT_NUM(9));
    for (j = 0; j < TEST_BUFF_SIZE; j++)
    {
        TEST_ASSERT_EQUAL(read_buff[j], 0x00, TEST_CHECKPOINT_NUM(10));
    }

    /* Case where offset = data_size  , data_len= 1  Also check nothing is returned in read buff*/
    status = SST_FUNCTION(s008_data[8].api, uid, TEST_BUFF_SIZE, 1, read_buff, &p_data_length);
    TEST_ASSERT_EQUAL(status, s008_data[8].status, TEST_CHECKPOINT_NUM(11));
    TEST_ASSERT_EQUAL(p_data_length, 0, TEST_CHECKPOINT_NUM(12));
    for (j = 0; j < TEST_BUFF_SIZE; j++)
    {
        TEST_ASSERT_EQUAL(read_buff[j], 0x00, TEST_CHECKPOINT_NUM(13));
    }

    /* Case where offset = 0  , data_len > data_size  Also check nothing is returned in read buff*/
    status = SST_FUNCTION(s008_data[10].api, uid, 0, TEST_BUFF_SIZE+1, read_buff, &p_data_length);
    TEST_ASSERT_EQUAL(status, s008_data[10].status, TEST_CHECKPOINT_NUM(14));
    TEST_ASSERT_EQUAL(p_data_length, 0, TEST_CHECKPOINT_NUM(15));
    for (j = 0; j < TEST_BUFF_SIZE; j++)
    {
        TEST_ASSERT_EQUAL(read_buff[j], 0x00, TEST_CHECKPOINT_NUM(16));
    }

    /* Try to access data with offset as MAX_UINT32 and length less than buffer size */
    status = SST_FUNCTION(s008_data[12].api, uid, TEST_MAX_UINT32, TEST_BUFF_SIZE/2, read_buff,
                          &p_data_length);
    TEST_ASSERT_NOT_EQUAL(status, s008_data[12].status, TEST_CHECKPOINT_NUM(17));

    /* Remove the UID */
    status = SST_FUNCTION(s008_data[13].api, uid);
    TEST_ASSERT_EQUAL(status, s008_data[13].status, TEST_CHECKPOINT_NUM(18));

    return VAL_STATUS_SUCCESS;
}

int32_t psa_sst_valid_offset_success(security_t caller)
{
    uint32_t status, data_len, offset = TEST_BUFF_SIZE;
    uint32_t p_data_length = 0;

    /* Set data for UID */
    status = SST_FUNCTION(s008_data[1].api, uid, TEST_BUFF_SIZE, write_buff, PSA_STORAGE_FLAG_NONE);
    TEST_ASSERT_EQUAL(status, s008_data[1].status, TEST_CHECKPOINT_NUM(1));

    /* Case where offset + datalen =  data_size */
    val->print(PRINT_TEST, "[Check 1] Try to access data with varying valid offset\n", 0);
    while (offset > 0)
    {
         data_len = TEST_BUFF_SIZE - offset;
         memset(read_buff, 0, TEST_BUFF_SIZE);
         status = SST_FUNCTION(s008_data[2].api, uid, offset, data_len, read_buff, &p_data_length);
         TEST_ASSERT_EQUAL(status, s008_data[2].status, TEST_CHECKPOINT_NUM(2));
         TEST_ASSERT_MEMCMP(read_buff, write_buff + offset, data_len, TEST_CHECKPOINT_NUM(3));
         TEST_ASSERT_EQUAL(p_data_length, data_len, TEST_CHECKPOINT_NUM(4));
         offset >>= 1;
     }

    offset = TEST_BUFF_SIZE - 2;
    data_len = 1;
    /* Case where offset + datalen <  data_size */
    while (offset > 0)
    {
         status = SST_FUNCTION(s008_data[4].api, uid, offset, data_len, read_buff, &p_data_length);
         TEST_ASSERT_EQUAL(status, s008_data[4].status, TEST_CHECKPOINT_NUM(5));
         TEST_ASSERT_MEMCMP(read_buff, write_buff + offset, data_len, TEST_CHECKPOINT_NUM(6));
         TEST_ASSERT_EQUAL(p_data_length, data_len, TEST_CHECKPOINT_NUM(7));
         offset >>= 1;
         data_len <<= 1;
     }

    return VAL_STATUS_SUCCESS;
}
