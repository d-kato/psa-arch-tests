
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
#include "test_c040.h"
#include "test_data.h"

client_test_t test_c040_crypto_list[] = {
    NULL,
    psa_asymmetric_decrypt_test,
    psa_asymmetric_decrypt_negative_test,
    NULL,
};

static bool_t is_buffer_empty(uint8_t *buffer, size_t size)
{
    int i;

    for (i = 0; i < size; i++)
    {
        if (buffer[i] != 0)
            return FALSE;
    }

    return TRUE;
}

static int         g_test_count = 1;
static uint8_t     output[SIZE_128B];

int32_t psa_asymmetric_decrypt_test(security_t caller)
{
    int                     num_checks = sizeof(check1)/sizeof(check1[0]);
    int32_t                 i, status;
    const uint8_t           *key_data;
    uint8_t                 *salt;
    size_t                  length;
    psa_key_attributes_t    attributes = PSA_KEY_ATTRIBUTES_INIT;

    if (num_checks == 0)
    {
        val->print(PRINT_TEST, "No test available for the selected crypto configuration\n", 0);
        return RESULT_SKIP(VAL_STATUS_NO_TESTS);
    }

    /* Initialize the PSA crypto library*/
    status = val->crypto_function(VAL_CRYPTO_INIT);
    TEST_ASSERT_EQUAL(status, PSA_SUCCESS, TEST_CHECKPOINT_NUM(1));

    for (i = 0; i < num_checks; i++)
    {
        val->print(PRINT_TEST, "[Check %d] ", g_test_count++);
        val->print(PRINT_TEST, check1[i].test_desc, 0);

         /* Setting up the watchdog timer for each check */
        status = val->wd_reprogram_timer(WD_CRYPTO_TIMEOUT);
        TEST_ASSERT_EQUAL(status, VAL_STATUS_SUCCESS, TEST_CHECKPOINT_NUM(2));

        memset(output, 0, sizeof(output));

        /* Set the key data based on key type */
        if (PSA_KEY_TYPE_IS_RSA(check1[i].key_type))
        {
            if (check1[i].key_type == PSA_KEY_TYPE_RSA_KEY_PAIR)
            {
                if (check1[i].expected_bit_length == BYTES_TO_BITS(384))
                    key_data = rsa_384_keypair;
                else if (check1[i].expected_bit_length == BYTES_TO_BITS(256))
                    key_data = rsa_256_keypair;
                else if (check1[i].expected_bit_length == BYTES_TO_BITS(128))
                    key_data = rsa_128_keypair;
                else
                    return VAL_STATUS_INVALID;
            }
            else
            {
                if (check1[i].expected_bit_length == BYTES_TO_BITS(384))
                    key_data = rsa_384_keydata;
                else if (check1[i].expected_bit_length == BYTES_TO_BITS(256))
                    key_data = rsa_256_keydata;
                else if (check1[i].expected_bit_length == BYTES_TO_BITS(128))
                    key_data = rsa_128_keydata;
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
        val->crypto_function(VAL_CRYPTO_SET_KEY_ALGORITHM, &attributes, check1[i].key_alg);
        val->crypto_function(VAL_CRYPTO_SET_KEY_USAGE_FLAGS, &attributes, check1[i].usage);

        /* Import the key data into the key slot */
        status = val->crypto_function(VAL_CRYPTO_IMPORT_KEY, &attributes, key_data,
                 check1[i].key_length, &check1[i].key_handle);
        TEST_ASSERT_EQUAL(status, PSA_SUCCESS, TEST_CHECKPOINT_NUM(3));

        if (is_buffer_empty(check1[i].salt, check1[i].salt_length) == TRUE)
        {
            salt = NULL;
            check1[i].salt_length = 0;
        }
        else
            salt = check1[i].salt;

        /* Decrypt a short message with a private key */
        status = val->crypto_function(VAL_CRYPTO_ASYMMTERIC_DECRYPT, check1[i].key_handle,
                    check1[i].key_alg, check1[i].input, check1[i].input_length, salt,
                    check1[i].salt_length, output, check1[i].output_size, &length);
        TEST_ASSERT_EQUAL(status, check1[i].expected_status, TEST_CHECKPOINT_NUM(6));

        if (check1[i].expected_status != PSA_SUCCESS)
        {
            /* Destroy the key */
            status = val->crypto_function(VAL_CRYPTO_DESTROY_KEY, check1[i].key_handle);
            TEST_ASSERT_EQUAL(status, PSA_SUCCESS, TEST_CHECKPOINT_NUM(7));

            continue;
        }

        /* Check if the output length matches with the expected length */
        TEST_ASSERT_EQUAL(length, check1[i].expected_output_length, TEST_CHECKPOINT_NUM(8));

        /* Check if the output matches with the expected data */
        TEST_ASSERT_MEMCMP(output, check1[i].expected_output, length, TEST_CHECKPOINT_NUM(9));

        /* Destroy the key */
        status = val->crypto_function(VAL_CRYPTO_DESTROY_KEY, check1[i].key_handle);
        TEST_ASSERT_EQUAL(status, PSA_SUCCESS, TEST_CHECKPOINT_NUM(10));

        /* Reset the key attributes and check if psa_import_key fails */
        val->crypto_function(VAL_CRYPTO_RESET_KEY_ATTRIBUTES, &attributes);
    }

    return VAL_STATUS_SUCCESS;
}

int32_t psa_asymmetric_decrypt_negative_test(security_t caller)
{
    int                     num_checks = sizeof(check2)/sizeof(check2[0]);
    int32_t                 i, status;
    uint8_t                 *salt;
    size_t                  length;

    /* Initialize the PSA crypto library*/
    status = val->crypto_function(VAL_CRYPTO_INIT);
    TEST_ASSERT_EQUAL(status, PSA_SUCCESS, TEST_CHECKPOINT_NUM(1));

    for (i = 0; i < num_checks; i++)
    {

        /* Setting up the watchdog timer for each check */
        status = val->wd_reprogram_timer(WD_CRYPTO_TIMEOUT);
        TEST_ASSERT_EQUAL(status, VAL_STATUS_SUCCESS, TEST_CHECKPOINT_NUM(2));

        if (is_buffer_empty(check2[i].salt, check2[i].salt_length) == TRUE)
        {
            salt = NULL;
            check2[i].salt_length = 0;
        }
        else
            salt = check1[i].salt;

        val->print(PRINT_TEST, "[Check %d] Test psa_asymmetric_decrypt - Invalid key handle\n",
                                                                                 g_test_count++);
        /* Encrypt a short message with a public key */
        status = val->crypto_function(VAL_CRYPTO_ASYMMTERIC_DECRYPT, check2[i].key_handle,
                    check2[i].key_alg, check2[i].input, check2[i].input_length, salt,
                    check2[i].salt_length, output, check2[i].output_size, &length);
        TEST_ASSERT_EQUAL(status, PSA_ERROR_INVALID_HANDLE, TEST_CHECKPOINT_NUM(3));

        val->print(PRINT_TEST, "[Check %d] Test psa_asymmetric_decrypt - Zero as key handle\n",
                                                                                 g_test_count++);
        /* Encrypt a short message with a public key */
        status = val->crypto_function(VAL_CRYPTO_ASYMMTERIC_DECRYPT, 0,
                    check2[i].key_alg, check2[i].input, check2[i].input_length, salt,
                    check2[i].salt_length, output, check2[i].output_size, &length);
        TEST_ASSERT_EQUAL(status, PSA_ERROR_INVALID_HANDLE, TEST_CHECKPOINT_NUM(4));
    }

    return VAL_STATUS_SUCCESS;
}


