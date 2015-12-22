/**
 * Copyright (c) 2015 Runtime Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stddef.h>
#include <errno.h>
#include <string.h>
#include "nimble/hci_common.h"
#include "ble_hs_priv.h"
#include "host/ble_hs_test.h"
#include "host/host_hci.h"
#include "ble_l2cap.h"
#include "ble_att_priv.h"
#include "ble_hs_conn.h"
#include "ble_hs_adv.h"
#include "ble_hci_ack.h"
#include "ble_hci_sched.h"
#include "ble_gap_conn.h"
#include "ble_hs_test_util.h"
#include "testutil/testutil.h"

#define BLE_ADV_TEST_DATA_OFF   4

static void
ble_hs_adv_test_misc_verify_tx_adv_data_hdr(int data_len)
{
    uint16_t opcode;
    uint8_t *sptr;

    TEST_ASSERT(ble_hs_test_util_prev_hci_tx != NULL);

    sptr = ble_hs_test_util_prev_hci_tx;

    opcode = le16toh(sptr + 0);
    TEST_ASSERT(BLE_HCI_OGF(opcode) == BLE_HCI_OGF_LE);
    TEST_ASSERT(BLE_HCI_OCF(opcode) == BLE_HCI_OCF_LE_SET_ADV_DATA);

    TEST_ASSERT(sptr[2] == data_len + 1);
    TEST_ASSERT(sptr[3] == data_len);
}

static void
ble_hs_adv_test_misc_verify_tx_field(int off, uint8_t type, uint8_t val_len,
                                     void *val)
{
    uint8_t *sptr;

    TEST_ASSERT_FATAL(ble_hs_test_util_prev_hci_tx != NULL);

    sptr = ble_hs_test_util_prev_hci_tx + off;

    TEST_ASSERT(sptr[0] == val_len + 1);
    TEST_ASSERT(sptr[1] == type);
    TEST_ASSERT(memcmp(sptr + 2, val, val_len) == 0);
}

struct ble_hs_adv_test_field {
    uint8_t type;       /* 0 indicates end of array. */
    uint8_t *val;
    uint8_t val_len;
};

static int
ble_hs_adv_test_misc_calc_data_len(struct ble_hs_adv_test_field *fields)
{
    struct ble_hs_adv_test_field *field;
    int len;

    len = 0;
    for (field = fields; field->type != 0; field++) {
        len += 2 + field->val_len;
    }

    return len;
}

static void
ble_hs_adv_test_misc_verify_tx_fields(int off,
                                      struct ble_hs_adv_test_field *fields)
{
    struct ble_hs_adv_test_field *field;

    for (field = fields; field->type != 0; field++) {
        ble_hs_adv_test_misc_verify_tx_field(off, field->type, field->val_len,
                                          field->val);
        off += 2 + field->val_len;
    }
}

static void
ble_hs_adv_test_misc_verify_tx_data(struct ble_hs_adv_test_field *fields)
{
    int data_len;

    data_len = ble_hs_adv_test_misc_calc_data_len(fields);
    ble_hs_adv_test_misc_verify_tx_adv_data_hdr(data_len);
    ble_hs_adv_test_misc_verify_tx_fields(BLE_ADV_TEST_DATA_OFF, fields);
}

static void
ble_hs_adv_test_misc_tx_and_verify_data(uint8_t disc_mode,
                                        struct ble_hs_adv_test_field *fields)
{
    int rc;

    ble_hs_test_util_init();

    rc = ble_gap_conn_advertise(disc_mode, BLE_GAP_CONN_MODE_UND, NULL, 0);
    TEST_ASSERT_FATAL(rc == 0);

    ble_hs_test_util_rx_und_adv_acks_count(3);
    ble_hs_adv_test_misc_verify_tx_data(fields);
}

TEST_CASE(ble_hs_adv_test_case_flags)
{
    /* No flags. */
    ble_hs_adv_test_misc_tx_and_verify_data(BLE_GAP_DISC_MODE_NON,
        (struct ble_hs_adv_test_field[]) {
            {
                .type = BLE_HS_ADV_TYPE_TX_PWR_LEVEL,
                .val = (uint8_t[]){ 0x00 },
                .val_len = 1,
            },
            { 0 },
        });

    /* Flags = limited discoverable. */
    ble_hs_adv_test_misc_tx_and_verify_data(BLE_GAP_DISC_MODE_LTD,
        (struct ble_hs_adv_test_field[]) {
            {
                .type = BLE_HS_ADV_TYPE_FLAGS,
                .val = (uint8_t[]){ BLE_HS_ADV_F_DISC_LTD },
                .val_len = 1,
            }, {
                .type = BLE_HS_ADV_TYPE_TX_PWR_LEVEL,
                .val = (uint8_t[]){ 0x00 },
                .val_len = 1,
            },
            { 0 },
        });

    /* Flags = general discoverable. */
    ble_hs_adv_test_misc_tx_and_verify_data(BLE_GAP_DISC_MODE_GEN,
        (struct ble_hs_adv_test_field[]) {
            {
                .type = BLE_HS_ADV_TYPE_FLAGS,
                .val = (uint8_t[]){ BLE_HS_ADV_F_DISC_GEN },
                .val_len = 1,
            }, {
                .type = BLE_HS_ADV_TYPE_TX_PWR_LEVEL,
                .val = (uint8_t[]){ 0x00 },
                .val_len = 1,
            },
            { 0 },
        });
}

TEST_CASE(ble_hs_adv_test_case_user)
{
    struct ble_hs_adv_fields fields;

    /*** Complete name. */
    fields.name = (uint8_t *)"myname";
    fields.name_len = 6;
    fields.name_is_complete = 1;
    ble_gap_conn_set_adv_fields(&fields);

    ble_hs_adv_test_misc_tx_and_verify_data(BLE_GAP_DISC_MODE_NON,
        (struct ble_hs_adv_test_field[]) {
            {
                .type = BLE_HS_ADV_TYPE_COMP_NAME,
                .val = (uint8_t*)"myname",
                .val_len = 6,
            },
            {
                .type = BLE_HS_ADV_TYPE_TX_PWR_LEVEL,
                .val = (uint8_t[]){ 0x00 },
                .val_len = 1,
            },
            { 0 },
        });

    /*** Incomplete name. */
    fields.name_is_complete = 0;
    ble_gap_conn_set_adv_fields(&fields);

    ble_hs_adv_test_misc_tx_and_verify_data(BLE_GAP_DISC_MODE_NON,
        (struct ble_hs_adv_test_field[]) {
            {
                .type = BLE_HS_ADV_TYPE_INCOMP_NAME,
                .val = (uint8_t*)"myname",
                .val_len = 6,
            },
            {
                .type = BLE_HS_ADV_TYPE_TX_PWR_LEVEL,
                .val = (uint8_t[]){ 0x00 },
                .val_len = 1,
            },
            { 0 },
        });
}

TEST_SUITE(ble_hs_adv_test_suite)
{
    ble_hs_adv_test_case_flags();
    ble_hs_adv_test_case_user();
}

int
ble_hs_adv_test_all(void)
{
    ble_hs_adv_test_suite();

    return tu_any_failed;
}
