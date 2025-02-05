/** @file
 * Copyright (c) 2018-2019, Arm Limited or its affiliates. All rights reserved.
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
#include "test_c002.h"
#include "test_data.h"
#include "val_crypto.h"

client_test_t test_c002_crypto_list[] = {
    NULL,
    psa_import_key_test,
    NULL,
};

static int g_test_count = 1;

int32_t psa_import_key_test(security_t caller)
{
    int32_t               i, status;
    uint8_t               data[BUFFER_SIZE];
    size_t                length;
    const uint8_t        *key_data;
    psa_key_type_t        get_key_type;
    int                   num_checks = sizeof(check1)/sizeof(check1[0]);
    psa_key_attributes_t  attributes = PSA_KEY_ATTRIBUTES_INIT;
    psa_key_attributes_t  get_attributes = PSA_KEY_ATTRIBUTES_INIT;

    if (num_checks == 0)
    {
        val->print(PRINT_TEST, "No test available for the selected crypto configuration\n", 0);
        return RESULT_SKIP(VAL_STATUS_NO_TESTS);
    }

    /* Initialize the PSA crypto library*/
    status = val->crypto_function(VAL_CRYPTO_INIT);
    TEST_ASSERT_EQUAL(status, PSA_SUCCESS, TEST_CHECKPOINT_NUM(1));

    /* Set the key data buffer to the input base on algorithm */
    for (i = 0; i < num_checks; i++)
    {
        val->print(PRINT_TEST, "[Check %d] ", g_test_count++);
        val->print(PRINT_TEST, check1[i].test_desc, 0);

        /* Setting up the watchdog timer for each check */
        status = val->wd_reprogram_timer(WD_CRYPTO_TIMEOUT);
        TEST_ASSERT_EQUAL(status, VAL_STATUS_SUCCESS, TEST_CHECKPOINT_NUM(2));

        if (PSA_KEY_TYPE_IS_RSA(check1[i].key_type))
        {
            if (check1[i].key_type == PSA_KEY_TYPE_RSA_KEY_PAIR)
            {
                if (check1[i].expected_bit_length == BYTES_TO_BITS(384))
                    key_data = rsa_384_keypair;
                else if (check1[i].expected_bit_length == BYTES_TO_BITS(256))
                    key_data = rsa_256_keypair;
                else
                    return VAL_STATUS_INVALID;
            }
            else
            {
                if (check1[i].expected_bit_length == BYTES_TO_BITS(384))
                    key_data = rsa_384_keydata;
                else if (check1[i].expected_bit_length == BYTES_TO_BITS(256))
                    key_data = rsa_256_keydata;
                else
                    return VAL_STATUS_INVALID;
            }
        }
        else if (PSA_KEY_TYPE_IS_ECC(check1[i].key_type))
        {
            if (PSA_KEY_TYPE_IS_ECC_KEY_PAIR(check1[i].key_type))
                key_data = ec_keypair;
            else
                key_data = ec_keydata;
        }
        else
            key_data = check1[i].key_data;

        /* Setup the attributes for the key */
        val->crypto_function(VAL_CRYPTO_SET_KEY_TYPE, &attributes, check1[i].key_type);
        val->crypto_function(VAL_CRYPTO_SET_KEY_BITS, &attributes, check1[i].attr_bits);
        val->crypto_function(VAL_CRYPTO_SET_KEY_USAGE_FLAGS, &attributes, check1[i].usage);

        /* Import the key data into the key slot */
        status = val->crypto_function(VAL_CRYPTO_IMPORT_KEY, &attributes, key_data,
                 check1[i].key_length, &check1[i].key_handle);
        TEST_ASSERT_EQUAL(status, check1[i].expected_status, TEST_CHECKPOINT_NUM(3));

        /* If failure is expected, continue with the next data set */
        if (check1[i].expected_status != PSA_SUCCESS)
            continue;

        /* Get the attributes of the imported key and check if it matches the given value */
        status = val->crypto_function(VAL_CRYPTO_GET_KEY_ATTRIBUTES, check1[i].key_handle,
                 &get_attributes);
        TEST_ASSERT_EQUAL(status, PSA_SUCCESS, TEST_CHECKPOINT_NUM(4));

        val->crypto_function(VAL_CRYPTO_GET_KEY_TYPE, &attributes, &get_key_type);
        TEST_ASSERT_EQUAL(get_key_type, check1[i].key_type, TEST_CHECKPOINT_NUM(5));

        if (check1[i].attr_bits != 0)
            TEST_ASSERT_EQUAL(get_attributes.bits, check1[i].attr_bits, TEST_CHECKPOINT_NUM(6));
        else
            TEST_ASSERT_EQUAL(get_attributes.bits, check1[i].expected_bit_length,
                              TEST_CHECKPOINT_NUM(7));

        /* Export a key in binary format */
        status = val->crypto_function(VAL_CRYPTO_EXPORT_KEY, check1[i].key_handle, data,
                                      BUFFER_SIZE, &length);
        TEST_ASSERT_EQUAL(status, PSA_SUCCESS, TEST_CHECKPOINT_NUM(7));

        /* Check the attributes of the exported key */
        TEST_ASSERT_EQUAL(length, check1[i].expected_key_length, TEST_CHECKPOINT_NUM(8));

        if (PSA_KEY_TYPE_IS_UNSTRUCTURED(check1[i].key_type))
        {
            TEST_ASSERT_MEMCMP(data, check1[i].key_data, length, TEST_CHECKPOINT_NUM(9));
        }
        else if (PSA_KEY_TYPE_IS_RSA(check1[i].key_type) || PSA_KEY_TYPE_IS_ECC(check1[i].key_type))
        {
            TEST_ASSERT_MEMCMP(key_data, data, length, TEST_CHECKPOINT_NUM(10));
        }
        else
        {
            return VAL_STATUS_INVALID;
        }

        /* Reset the key attributes and check if psa_import_key fails */
        val->crypto_function(VAL_CRYPTO_RESET_KEY_ATTRIBUTES, &attributes);

        val->crypto_function(VAL_CRYPTO_DESTROY_KEY, check1[i].key_handle);
        TEST_ASSERT_EQUAL(status, PSA_SUCCESS, TEST_CHECKPOINT_NUM(11));

        status = val->crypto_function(VAL_CRYPTO_IMPORT_KEY, &attributes, key_data,
                 check1[i].key_length, &check1[i].key_handle);
        TEST_ASSERT_EQUAL(status, PSA_ERROR_INVALID_ARGUMENT, TEST_CHECKPOINT_NUM(12));

        TEST_ASSERT_EQUAL(check1[i].key_handle, 0, TEST_CHECKPOINT_NUM(13));
    }

    return VAL_STATUS_SUCCESS;
}
