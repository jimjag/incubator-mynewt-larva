/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifndef H_BLE_LL_HCI_
#define H_BLE_LL_HCI_

/* Define the number of data packets that the controller can store */
#define BLE_LL_CFG_NUM_ACL_DATA_PKTS    (4)
#define BLE_LL_CFG_ACL_DATA_PKT_LEN     (251)

/* 
 * This determines the number of outstanding commands allowed from the
 * host to the controller.
 */
#define BLE_LL_CFG_NUM_HCI_CMD_PKTS     (1)

/* 
 * Determines the maximum rate at which the controller will send the
 * number of completed packets event to the host. This number is in
 * milliseconds.
 */
#define BLE_LL_CFG_NUM_COMP_PKT_RATE    (2000)  /* msecs */

/* Initialize LL HCI */
void ble_ll_hci_init(void);

/* HCI command processing function */
void ble_ll_hci_cmd_proc(struct os_event *ev);

/* Used to determine if the LE event is enabled/disabled */
uint8_t ble_ll_hci_is_le_event_enabled(int subev);

/* Used to determine if event is enabled/disabled */
uint8_t ble_ll_hci_is_event_enabled(int bitpos);

/* Send event from controller to host */
int ble_ll_hci_event_send(uint8_t *evbuf);

/* Sends a command complete with a no-op opcode to host */
int ble_ll_hci_send_noop(void);


#endif /* H_BLE_LL_HCI_ */
