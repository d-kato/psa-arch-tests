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

#ifdef NONSECURE_TEST_BUILD
#include "val_interfaces.h"
#include "val_target.h"
#else
#include "val_client_defs.h"
#include "val_service_defs.h"
#endif

#include "test_i086.h"

#define DATA_VALUE_ORG 0x11
#define DATA_VALUE     0x12
#define BUFFER_SIZE    0x4

client_test_t test_i086_client_tests_list[] = {
    NULL,
    client_test_sp_read_other_sp_heap,
    client_test_sp_write_other_sp_heap,
    NULL,
};

#ifdef SP_HEAP_MEM_SUPP
static int32_t get_secure_partition_address(addr_t *addr)
{
   psa_handle_t            handle = 0;

   handle = psa->connect(SERVER_UNSPECIFED_MINOR_V_SID, 1);
   if (handle < 0)
   {
       val->print(PRINT_ERROR, "\tConnection failed\n", 0);
       return VAL_STATUS_INVALID_HANDLE;
   }

   /* Get App-RoT address */
   psa_outvec outvec[1] = {{addr, BUFFER_SIZE}};
   if (psa->call(handle, NULL, 0, outvec, 1) != PSA_SUCCESS)
   {
       val->print(PRINT_ERROR, "\tmsg request failed\n", 0);
       return VAL_STATUS_CALL_FAILED;
   }

   psa->close(handle);
   return VAL_STATUS_SUCCESS;
}

int32_t client_test_sp_read_other_sp_heap(security_t caller)
{
   addr_t   app_rot_addr;
   uint8_t  data = DATA_VALUE;

   val->print(PRINT_TEST, "[Check 1] Test SP reading other SP heap\n", 0);

   if (VAL_ERROR(get_secure_partition_address(&app_rot_addr)))
       return VAL_STATUS_ERROR;

   /* Setting boot.state before test check */
   if (val->set_boot_flag(BOOT_EXPECTED_REENTER_TEST))
   {
       val->print(PRINT_ERROR, "\tFailed to set boot flag before check\n", 0);
       return VAL_STATUS_ERROR;
   }

   /* Read Application RoT heap address.
    * This should generate internal fault or ignore the read.
    */
   data = *(uint8_t *)app_rot_addr;

   /* Did read ignore? */
   if (data == DATA_VALUE)
        return VAL_STATUS_SUCCESS;

   val->print(PRINT_ERROR, "\tExpected read to fault but it didn't\n", 0);

   /* Resetting boot.state to catch unwanted reboot */
   if (val->set_boot_flag(BOOT_EXPECTED_BUT_FAILED))
   {
       val->print(PRINT_ERROR, "\tFailed to set boot flag after check\n", 0);
       return VAL_STATUS_ERROR;
   }

   return VAL_STATUS_SPM_FAILED;
}

int32_t client_test_sp_write_other_sp_heap(security_t caller)
{
   addr_t   app_rot_addr;
   uint8_t  data = DATA_VALUE;

   val->print(PRINT_TEST, "[Check 2] Test SP writing other SP heap\n", 0);

   if (VAL_ERROR(get_secure_partition_address(&app_rot_addr)))
       return VAL_STATUS_ERROR;

   /* Setting boot.state before test check */
   if (val->set_boot_flag(BOOT_EXPECTED_NS))
   {
       val->print(PRINT_ERROR, "\tFailed to set boot flag before check\n", 0);
       return VAL_STATUS_ERROR;
   }

   /* Write Application RoT heap address.
    * This should generate internal fault or ignore the write.
    */
   *(uint8_t *)app_rot_addr = (uint8_t)data;

   /* Handshake with server to decide write status */
   if ((psa->connect(SERVER_UNSPECIFED_MINOR_V_SID, 1)) > 0)
   {
       val->print(PRINT_ERROR, "\tExpected connection to fail but succeed\n", 0);
       return VAL_STATUS_INVALID_HANDLE;
   }
   return VAL_STATUS_SUCCESS;
}
#else
int32_t client_test_sp_read_other_sp_heap(security_t caller)
{
   val->print(PRINT_TEST, "[Check 1] Test SP reading other SP heap\n", 0);
   val->print(PRINT_ERROR, "\tSkipping test as heap memory not supported\n", 0);
   return RESULT_SKIP(VAL_STATUS_HEAP_NOT_AVAILABLE);
}

int32_t client_test_sp_write_other_sp_heap(security_t caller)
{
   val->print(PRINT_TEST, "[Check 2] Test SP writing other SP heap\n", 0);
   val->print(PRINT_ERROR, "\tSkipping test as heap memory not supported\n", 0);
   return RESULT_SKIP(VAL_STATUS_HEAP_NOT_AVAILABLE);
}
#endif
