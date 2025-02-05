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
#include "test_c015.h"
#include "test_data.h"
#include "val_crypto.h"

client_test_t test_c015_crypto_list[] = {
    NULL,
    psa_hash_abort_test,
    psa_hash_abort_before_operation_finish,
    NULL,
};

static int g_test_count = 1;

int32_t psa_hash_abort_test(security_t caller)
{
    int                     num_checks = sizeof(check1)/sizeof(check1[0]);
    int32_t                 i, status;
    psa_hash_operation_t    operation;

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
        memset(&operation, 0, sizeof(operation));

        /* Setting up the watchdog timer for each check */
        status = val->wd_reprogram_timer(WD_CRYPTO_TIMEOUT);
        TEST_ASSERT_EQUAL(status, VAL_STATUS_SUCCESS, TEST_CHECKPOINT_NUM(2));

        /* Start a multipart hash operation */
        status = val->crypto_function(VAL_CRYPTO_HASH_SETUP, &operation, check1[i].alg);
        TEST_ASSERT_EQUAL(status, PSA_SUCCESS, TEST_CHECKPOINT_NUM(3));

        /* Abort a hash operation */
        status = val->crypto_function(VAL_CRYPTO_HASH_ABORT, &operation);
        TEST_ASSERT_EQUAL(status, check1[i].expected_status, TEST_CHECKPOINT_NUM(4));

        /* Multiple hash abort should succeed */
        status = val->crypto_function(VAL_CRYPTO_HASH_ABORT, &operation);
        TEST_ASSERT_EQUAL(status, check1[i].expected_status, TEST_CHECKPOINT_NUM(5));

    }

    return VAL_STATUS_SUCCESS;
}

int32_t psa_hash_abort_before_operation_finish(security_t caller)
{
    psa_hash_operation_t    operation;
    char                    input = 0xbd;
    size_t                  input_length = 1;
    psa_algorithm_t         alg = PSA_ALG_SHA_256;
    char                    hash[HASH_64B];
    size_t                  hash_length, hash_size = sizeof(hash);
    int32_t                 status;

    /* Initialize the PSA crypto library*/
    status = val->crypto_function(VAL_CRYPTO_INIT);
    TEST_ASSERT_EQUAL(status, PSA_SUCCESS, TEST_CHECKPOINT_NUM(1));

    val->print(PRINT_TEST, "[Check %d] ", g_test_count++);
    val->print(PRINT_TEST, "Test psa_hash_finish after calling psa_hash_abort\n", 0);
    memset(&operation, 0, sizeof(operation));

    /* Setting up the watchdog timer for each check */
    status = val->wd_reprogram_timer(WD_CRYPTO_TIMEOUT);
    TEST_ASSERT_EQUAL(status, VAL_STATUS_SUCCESS, TEST_CHECKPOINT_NUM(2));

    /* Start a multipart hash operation */
    status = val->crypto_function(VAL_CRYPTO_HASH_SETUP, &operation, alg);
    TEST_ASSERT_EQUAL(status, PSA_SUCCESS, TEST_CHECKPOINT_NUM(3));

    /* Add a message fragment to a multipart hash operation */
    status = val->crypto_function(VAL_CRYPTO_HASH_UPDATE, &operation, &input, input_length);
    TEST_ASSERT_EQUAL(status, PSA_SUCCESS, TEST_CHECKPOINT_NUM(4));

    /* Abort a hash operation */
    status = val->crypto_function(VAL_CRYPTO_HASH_ABORT, &operation);
    TEST_ASSERT_EQUAL(status, PSA_SUCCESS, TEST_CHECKPOINT_NUM(5));

    /* Finish the calculation of the hash of a message */
    status = val->crypto_function(VAL_CRYPTO_HASH_FINISH, &operation, hash, hash_size,
                                  &hash_length);
    TEST_ASSERT_EQUAL(status, PSA_ERROR_BAD_STATE, TEST_CHECKPOINT_NUM(6));

    return VAL_STATUS_SUCCESS;
}
