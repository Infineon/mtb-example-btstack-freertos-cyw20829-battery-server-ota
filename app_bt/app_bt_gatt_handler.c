/*******************************************************************************
 * File Name: app_bt_gatt_handler.c
 *
 * Description: This file contains the task that handles GATT events.
 *
 * Related Document: See README.md
 *
 *
 *******************************************************************************
 * (c) 2021-2026, Infineon Technologies AG, or an affiliate of Infineon
 * Technologies AG. All rights reserved.
 * This software, associated documentation and materials ("Software") is
 * owned by Infineon Technologies AG or one of its affiliates ("Infineon")
 * and is protected by and subject to worldwide patent protection, worldwide
 * copyright laws, and international treaty provisions. Therefore, you may use
 * this Software only as provided in the license agreement accompanying the
 * software package from which you obtained this Software. If no license
 * agreement applies, then any use, reproduction, modification, translation, or
 * compilation of this Software is prohibited without the express written
 * permission of Infineon.
 *
 * Disclaimer: UNLESS OTHERWISE EXPRESSLY AGREED WITH INFINEON, THIS SOFTWARE
 * IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING, BUT NOT LIMITED TO, ALL WARRANTIES OF NON-INFRINGEMENT OF
 * THIRD-PARTY RIGHTS AND IMPLIED WARRANTIES SUCH AS WARRANTIES OF FITNESS FOR A
 * SPECIFIC USE/PURPOSE OR MERCHANTABILITY.
 * Infineon reserves the right to make changes to the Software without notice.
 * You are responsible for properly designing, programming, and testing the
 * functionality and safety of your intended application of the Software, as
 * well as complying with any legal requirements related to its use. Infineon
 * does not guarantee that the Software will be free from intrusion, data theft
 * or loss, or other breaches ("Security Breaches"), and Infineon shall have
 * no liability arising out of any Security Breaches. Unless otherwise
 * explicitly approved by Infineon, the Software may not be used in any
 * application where a failure of the Product or any consequences of the use
 * thereof can reasonably be expected to result in personal injury.
 ******************************************************************************/

/*******************************************************************************
 * Header Files
 ******************************************************************************/
#include "wiced_bt_stack.h"
#include <FreeRTOS.h>
#include "cyabs_rtos.h"
#include "app_bt_event_handler.h"
#include "app_bt_gatt_handler.h"

/*******************************************************************************
*        Macro Definitions
*******************************************************************************/

/*******************************************************************************
*        Variable Definitions
*******************************************************************************/
extern app_bt_adv_conn_mode_t app_bt_adv_conn_state ;
gatt_write_req_buf_t write_buff;

/* MTU size negotiated between local and peer device */
static uint16_t preferred_mtu_size = CY_BT_MTU_SIZE;

/*******************************************************************************
*        Function Prototypes
*******************************************************************************/

/*******************************************************************************
*       Function Definitions
*******************************************************************************/
/**
* Function Name:
* app_bt_free_buffer
*
* Function Description:
* @brief  This Function frees the buffer requested
*
* @param p_data         pointer to the buffer to be freed
*
* @return void
*/
void app_bt_free_buffer(uint8_t *p_data)
{
    wiced_bt_free_buffer(p_data);
}

/**
* Function Name:
* app_bt_alloc_buffer
*
* Function Description:
* @brief  This Function allocates the buffer of requested length
*
* @param len            Length of the buffer
*
* @return uint8_t*      pointer to allocated buffer
*/
uint8_t *app_bt_alloc_buffer(uint16_t len)
{
    uint8_t *p = (uint8_t *)wiced_bt_get_buffer(len);
    if (!p)
    {
        printf("OOM\r\n");
        CY_ASSERT(0);
    }
    return p;
}

static wiced_bt_gatt_status_t app_bt_ble_send_notification(uint16_t bt_conn_id, uint16_t attr_handle, uint16_t val_len, uint8_t* p_val)
{
    wiced_bt_gatt_status_t status = (wiced_bt_gatt_status_t)WICED_BT_GATT_ERROR;

    status = wiced_bt_gatt_server_send_notification(bt_conn_id, attr_handle, val_len, p_val, NULL);    /* bt_notify_buff is not allocated, no need to keep track of it w/context */
    if (status != WICED_BT_SUCCESS)
    {
        printf("%s() Notification FAILED conn_id:0x%x (%d) handle: %d val_len: %d value:%d\n", __func__, bt_conn_id, bt_conn_id, attr_handle, val_len, *p_val);
    }
    return status;
}

static wiced_bt_gatt_status_t app_bt_ble_send_indication(uint16_t bt_conn_id, uint16_t attr_handle, uint16_t val_len, uint8_t* p_val)
{
    wiced_bt_gatt_status_t status = (wiced_bt_gatt_status_t)WICED_BT_GATT_ERROR;

    status = wiced_bt_gatt_server_send_indication(bt_conn_id, attr_handle, val_len, p_val, NULL);    /* bt_notify_buff is not allocated, no need to keep track of it w/context */
    if (status != WICED_BT_SUCCESS)
    {
        printf("%s() Indication FAILED conn_id:0x%x (%d) handle: %d val_len: %d value:%d\n", __func__, bt_conn_id, bt_conn_id, attr_handle, val_len, *p_val);
    }
    return status;
}

/**
* Function Name:
* app_bt_gatt_event_callback
*
* Function Description:
* @brief  This Function handles the all the GATT events - GATT Event Handler
*
* @param event            Bluetooth LE GATT event type
*
* @param p_event_data     Pointer to Bluetooth LE GATT event data
*
* @return wiced_bt_gatt_status_t  Bluetooth LE GATT status
*/
wiced_bt_gatt_status_t app_bt_gatt_event_callback(wiced_bt_gatt_evt_t event,
                                    wiced_bt_gatt_event_data_t *p_event_data)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;
    uint16_t error_handle = 0;
    wiced_bt_gatt_attribute_request_t *p_attr_req = &p_event_data->attribute_request;
    pfn_free_buffer_t pfn_free;

    /* Call the appropriate callback function based on the GATT event type,
     * and pass the relevant event
     * parameters to the callback function */
    switch (event)
    {
    case GATT_CONNECTION_STATUS_EVT:
        status = app_bt_connect_event_handler (&p_event_data->connection_status);
        break;

    case GATT_ATTRIBUTE_REQUEST_EVT:
        status = app_bt_server_event_handler (p_event_data,
                                              &error_handle);
        if(status != WICED_BT_GATT_SUCCESS)
        {
           wiced_bt_gatt_server_send_error_rsp(p_attr_req->conn_id,
                                               p_attr_req->opcode,
                                               error_handle,
                                               status);
        }
        break;

        /* GATT buffer request, typically sized to max of bearer mtu - 1 */
    case GATT_GET_RESPONSE_BUFFER_EVT:
        p_event_data->buffer_request.buffer.p_app_rsp_buffer = app_bt_alloc_buffer(p_event_data->buffer_request.len_requested);
        p_event_data->buffer_request.buffer.p_app_ctxt = (void *)app_bt_free_buffer;
        status = WICED_BT_GATT_SUCCESS;
        break;

        /* GATT buffer transmitted event,  check \ref wiced_bt_gatt_buffer_transmitted_t*/
    case GATT_APP_BUFFER_TRANSMITTED_EVT:
        pfn_free = (pfn_free_buffer_t)p_event_data->buffer_xmitted.p_app_ctxt;

        /* If the buffer is dynamic, the context will point to a function to free it. */
        if (pfn_free)
        {
            pfn_free(p_event_data->buffer_xmitted.p_app_data);
        }
        break;

    default:
        status = WICED_BT_GATT_ERROR;
        break;
    }

    return status;
}

/**
* Function Name:
* app_bt_server_event_handler
*
* Function Description:
* @brief  The callback function is invoked when GATT_ATTRIBUTE_REQUEST_EVT occurs
*         in GATT Event handler function. GATT Server Event Callback function.
*
* @param p_data   Pointer to Bluetooth LE GATT request data
*
* @param p_error_handle Pointer to the handle
*
* @return wiced_bt_gatt_status_t  Bluetooth LE GATT status
*/
wiced_bt_gatt_status_t app_bt_server_event_handler (wiced_bt_gatt_event_data_t *p_data,
                                                           uint16_t *p_error_handle)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;
    wiced_bt_gatt_attribute_request_t   *p_att_req = &p_data->attribute_request;
    wiced_bt_gatt_status_t gatt_status = WICED_BT_GATT_SUCCESS;
    cy_ota_agent_state_t ota_lib_state;

    switch (p_att_req->opcode)
    {
    /* Attribute read notification (attribute value internally read from GATT database) */
    case GATT_REQ_READ:
    case GATT_REQ_READ_BLOB:
    case GATT_REQ_READ_BY_GRP_TYPE:
        status = app_bt_gatt_req_read_handler(p_att_req->conn_id, p_att_req->opcode,
                            (wiced_bt_gatt_read_t*)&p_att_req->data.read_req,
                            p_att_req->len_requested,
                            p_error_handle);
        break;

    case GATT_REQ_FIND_TYPE_VALUE:
    case GATT_REQ_READ_BY_TYPE:
        status = app_bt_gatt_req_read_by_type_handler(p_att_req->conn_id, p_att_req->opcode,
                (wiced_bt_gatt_read_by_type_t*)&p_att_req->data.read_by_type,
                p_att_req->len_requested,
                p_error_handle);
        break;

    case GATT_REQ_READ_MULTI:
    case GATT_REQ_READ_MULTI_VAR_LENGTH:
        status = app_bt_gatt_req_read_multi_handler(p_att_req->conn_id, p_att_req->opcode,
                                                    &p_att_req->data.read_multiple_req,
                                                    p_att_req->len_requested,
                                                    p_error_handle);
        break;

    case GATT_REQ_WRITE:
    case GATT_CMD_WRITE:
    case GATT_CMD_SIGNED_WRITE:
        status = app_bt_write_handler(p_data, p_error_handle);
        if ((p_att_req->opcode == GATT_REQ_WRITE) && (status == WICED_BT_GATT_SUCCESS))
        {
            wiced_bt_gatt_write_req_t *p_write_request = &p_att_req->data.write_req;
            wiced_bt_gatt_server_send_write_rsp(p_att_req->conn_id,
                                                p_att_req->opcode,
                                                p_write_request->handle);
        }
        break;

    case GATT_REQ_PREPARE_WRITE:
        status = app_bt_prepare_write_handler(p_att_req->conn_id,
                                              p_att_req->opcode,
                                              &p_att_req->data.write_req,
                                              p_error_handle);
        if((p_att_req->opcode == GATT_REQ_PREPARE_WRITE) &&  (status != WICED_BT_GATT_SUCCESS))
        {
         printf("\n\n== Sending Prepare write error response...\n");
        }
        status = WICED_BT_GATT_SUCCESS;
        break;

    case GATT_REQ_EXECUTE_WRITE:
        status = app_bt_execute_write_handler(p_data, p_error_handle);
        if((p_att_req->opcode == GATT_REQ_EXECUTE_WRITE) &&  (gatt_status == WICED_BT_GATT_SUCCESS))
        {
         printf("== Sending execute write success response...\n");
         wiced_bt_gatt_server_send_execute_write_rsp(p_att_req->conn_id, p_att_req->opcode);
         status = WICED_BT_GATT_SUCCESS;
        }
        else
        {
         printf("== Sending execute write error response...\n");
        }
        status = WICED_BT_GATT_SUCCESS;
        break;

    case GATT_REQ_MTU:
        /* Application calls wiced_bt_gatt_server_send_mtu_rsp() with the desired mtu */
        preferred_mtu_size = CY_BT_MTU_SIZE <= (p_att_req->data.remote_mtu) ?
                             CY_BT_MTU_SIZE : (p_att_req->data.remote_mtu);
        status = wiced_bt_gatt_server_send_mtu_rsp(p_att_req->conn_id,
                                                p_att_req->data.remote_mtu,
                        wiced_bt_cfg_settings.p_ble_cfg->ble_max_rx_pdu_size);
        break;

    case GATT_HANDLE_VALUE_CONF: /* Value confirmation */
        cy_ota_get_state(ota_app.ota_context, &ota_lib_state);
        if ((ota_lib_state == CY_OTA_STATE_OTA_COMPLETE) && /* Check if we completed the download before rebooting */
            (ota_app.reboot_at_end != 0))
        {
            cy_rtos_delay_milliseconds(1000);
            Cy_SysPm_TriggerXRes();
        }
        else
        {
            cy_ota_agent_stop(&ota_app.ota_context); /* Stop OTA */
        }
        status = WICED_BT_GATT_SUCCESS;
        break;

    case GATT_HANDLE_VALUE_NOTIF:
        status = WICED_BT_GATT_SUCCESS;
        break;

    default:
        status = WICED_BT_GATT_ERROR;
        break;
    }
    return status;
}

/**
* Function Name
* app_bt_connect_event_handler
*
* Function Description
* @brief   This callback function handles connection status changes.
*
* @param p_conn_status    Pointer to data that has connection details
*
* @return wiced_bt_gatt_status_t See possible status codes in wiced_bt_gatt_status_e
* in wiced_bt_gatt.h
*/

wiced_bt_gatt_status_t app_bt_connect_event_handler (wiced_bt_gatt_connection_status_t *p_conn_status)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_ERROR;
    wiced_result_t result;
    if (NULL != p_conn_status)
    {
        if (p_conn_status->connected)
        {
            /* Device has connected */
            printf("Connected : BDA ");
            print_bd_address(p_conn_status->bd_addr);
            printf("Connection ID '%d'\r\n", p_conn_status->conn_id);
            /* Store the connection ID and peer BD Address */
            ota_app.bt_conn_id = p_conn_status->conn_id;
            memcpy(ota_app.bt_peer_addr, p_conn_status->bd_addr, BD_ADDR_LEN);

            /* Update the adv/conn state */
            app_bt_adv_conn_state = APP_BT_ADV_OFF_CONN_ON;
            /* Save BT connection ID in application data structure */
            ota_app.bt_conn_id = p_conn_status->conn_id;
            /* Save BT peer ADDRESS in application data structure */
            memcpy(ota_app.bt_peer_addr, p_conn_status->bd_addr, BD_ADDR_LEN);
        }
        else
        {
            /* Device has disconnected */
            printf("Disconnected : BDA ");
            print_bd_address(p_conn_status->bd_addr);
            printf("Connection ID '%d', Reason '%s'\r\n", p_conn_status->conn_id,
                    get_bt_gatt_disconn_reason_name(p_conn_status->reason));
            /* Set the connection id to zero to indicate disconnected state */
            ota_app.bt_conn_id = 0;
            /* Restart the advertisements */
            result = wiced_bt_start_advertisements(BTM_BLE_ADVERT_UNDIRECTED_HIGH, 0, NULL);
            if (WICED_BT_SUCCESS != result)
            {
                printf("Advertisement cannot start because of error: %d \r\n",
                        result);
                CY_ASSERT(0);
            }
            /* Update the adv/conn state */
            app_bt_adv_conn_state = APP_BT_ADV_ON_CONN_OFF;
        }
        /* Update Advertisement LED to reflect the updated state */
        app_bt_adv_led_update();
        status = WICED_BT_GATT_SUCCESS;
    }

    return status;
}
/**
* Function Name:
* app_bt_write_handler
*
* Function Description:
* @brief  The function is invoked when GATTS_REQ_TYPE_WRITE is received from the
*         client device and is invoked GATT Server Event Callback function. This
*         handles "Write Requests" received from Client device.
*
* @param p_write_req   Pointer to Bluetooth LE GATT write request
*
* @param p_error_handle Pointer to the handle
*
* @return wiced_bt_gatt_status_t  Bluetooth LE GATT status
*/
wiced_bt_gatt_status_t app_bt_write_handler(wiced_bt_gatt_event_data_t *p_data,
                                                   uint16_t *p_error_handle)
{
    wiced_bt_gatt_write_req_t* p_write_req;
    cy_rslt_t result;
    wiced_bt_gatt_status_t gatt_status = WICED_BT_GATT_SUCCESS;
    uint32_t total_size = 0;
    bool crc_or_sig_verify = true;
    uint32_t final_crc32 = 0;

    CY_ASSERT(NULL != p_data);

    p_write_req = &p_data->attribute_request.data.write_req;

    CY_ASSERT(NULL != p_write_req);

    *p_error_handle = p_write_req->handle;

    switch (p_write_req->handle)
    {
    case HDLD_OTA_FW_UPGRADE_SERVICE_OTA_UPGRADE_CONTROL_POINT_CLIENT_CHAR_CONFIG:
        ota_app.bt_config_descriptor = p_write_req->p_val[0];
        gatt_status = WICED_BT_GATT_SUCCESS;
        break;

    case HDLC_OTA_FW_UPGRADE_SERVICE_OTA_UPGRADE_CONTROL_POINT_VALUE:
        switch (p_write_req->p_val[0])
        {
        case CY_OTA_UPGRADE_COMMAND_PREPARE_DOWNLOAD:
            ota_app.connection_type = CY_OTA_CONNECTION_BLE;
            result = init_ota(&ota_app);
            if (result != CY_RSLT_SUCCESS)
            {
                printf("init_ota() Failed - result: 0x%lx\n", result);
                gatt_status = WICED_BT_GATT_ERROR;
                break;
            }
            printf("Preparing to download the image \r\n");
            result = cy_ota_ble_download_prepare(ota_app.ota_context);
            if (result == CY_RSLT_SUCCESS)
            {
                printf("\ncy_ota_ble_download_prepare completed, Sending notification");
                uint8_t bt_notify_buff = CY_OTA_UPGRADE_STATUS_OK;
                gatt_status = app_bt_ble_send_notification(ota_app.bt_conn_id, HDLC_OTA_FW_UPGRADE_SERVICE_OTA_UPGRADE_CONTROL_POINT_VALUE, 1, &bt_notify_buff);
                if (gatt_status != WICED_BT_GATT_SUCCESS)
                {
                    printf("\nApplication BT Send notification callback failed: 0x%lx\n", result);
                }
                gatt_status = WICED_BT_GATT_SUCCESS;
            }
            else
            {
                printf("cy_ota_ble_prepare_download() Failed - result: 0x%lx\n", result);
                gatt_status = WICED_BT_GATT_ERROR;
            }

            break;

        case CY_OTA_UPGRADE_COMMAND_DOWNLOAD:
            if (p_write_req->val_len < 4)
            {
                printf("CY_OTA_UPGRADE_COMMAND_DOWNLOAD len < 4\n");
                gatt_status = WICED_BT_GATT_ERROR;
                break;
            }

            total_size = (((uint32_t)p_write_req->p_val[4]) << 24) +
                (((uint32_t)p_write_req->p_val[3]) << 16) +
                (((uint32_t)p_write_req->p_val[2]) << 8) +
                (((uint32_t)p_write_req->p_val[1]) << 0);

            result = cy_ota_ble_download(ota_app.ota_context, total_size);
            if (result == CY_RSLT_SUCCESS)
            {
                printf("\ncy_ota_ble_download completed, Sending notification");
                uint8_t bt_notify_buff = CY_OTA_UPGRADE_STATUS_OK;
                gatt_status = app_bt_ble_send_notification(ota_app.bt_conn_id, HDLC_OTA_FW_UPGRADE_SERVICE_OTA_UPGRADE_CONTROL_POINT_VALUE, 1, &bt_notify_buff);
                if (gatt_status != WICED_BT_GATT_SUCCESS)
                {
                    printf("\nApplication BT Send notification callback failed: 0x%lx\n", result);
                    break;
                }
            }
            else
            {
                printf("cy_ota_ble_download() Failed - result: 0x%lx\n", result);
                gatt_status = WICED_BT_GATT_ERROR;
            }

            break;

        case CY_OTA_UPGRADE_COMMAND_VERIFY:
            if (p_write_req->val_len != 5)
            {
                printf("CY_OTA_UPGRADE_COMMAND_VERIFY len != 5\n");
                gatt_status = WICED_BT_GATT_ERROR;
                break;
            }

            final_crc32 = (((uint32_t)p_write_req->p_val[1]) << 0) +
                (((uint32_t)p_write_req->p_val[2]) << 8) +
                (((uint32_t)p_write_req->p_val[3]) << 16) +
                (((uint32_t)p_write_req->p_val[4]) << 24);
            printf("\nFinal CRC from Host : 0x%lx\n", final_crc32);

            result = cy_ota_ble_download_verify(ota_app.ota_context, final_crc32, crc_or_sig_verify);
            if (result == CY_RSLT_SUCCESS)
            {
                printf("\ncy_ota_ble_download_verify completed, Sending notification");
                uint8_t bt_notify_buff = CY_OTA_UPGRADE_STATUS_OK;
                gatt_status = app_bt_ble_send_indication(ota_app.bt_conn_id, HDLC_OTA_FW_UPGRADE_SERVICE_OTA_UPGRADE_CONTROL_POINT_VALUE, 1, &bt_notify_buff);
                if (gatt_status != WICED_BT_GATT_SUCCESS)
                {
                    printf("\nApplication BT Send Indication callback failed: 0x%lx\n", result);
                }
            }
            else
            {
                printf("cy_ota_ble_download_verify() Failed - result: 0x%lx\n", result);
                uint8_t bt_notify_buff = CY_OTA_UPGRADE_STATUS_BAD;
                gatt_status = app_bt_ble_send_indication(ota_app.bt_conn_id, HDLC_OTA_FW_UPGRADE_SERVICE_OTA_UPGRADE_CONTROL_POINT_VALUE, 1, &bt_notify_buff);
                if (gatt_status != WICED_BT_GATT_SUCCESS)
                {
                    printf("\nApplication BT Send Indication callback failed: 0x%lx\n", result);
                }

                gatt_status = WICED_BT_GATT_ERROR;
            }

            break;

        case CY_OTA_UPGRADE_COMMAND_ABORT:
            result = cy_ota_ble_download_abort(ota_app.ota_context);
            gatt_status = WICED_BT_GATT_SUCCESS;
            break;
        }
        break;

    case HDLC_OTA_FW_UPGRADE_SERVICE_OTA_UPGRADE_DATA_VALUE:
        /*Call OTA write handler to handle OTA related writes*/
        printf("application downloading... \r\n");
        result = cy_ota_ble_download_write(ota_app.ota_context, p_write_req->p_val, p_write_req->val_len, p_write_req->offset);
        if (result != CY_RSLT_SUCCESS)
        {
            gatt_status = WICED_BT_GATT_ERROR;
            break;
        }
        gatt_status = WICED_BT_GATT_SUCCESS;
        break;

    default:
        /* Handle normal (non-OTA) indication confirmation requests here */
        /* Attempt to perform the Write Request */
        return app_bt_set_value(p_write_req->handle,
            p_write_req->p_val,
            p_write_req->val_len);
    }
    return (gatt_status);
}

/**
* Function Name:
* app_bt_set_value
*
* Function Description:
* @brief  The function is invoked by app_bt_write_handler to set a value
*         to GATT DB.
*
* @param attr_handle  GATT attribute handle
*
* @param p_val        Pointer to Bluetooth LE GATT write request value
*
* @param len          length of GATT write request
*
* @return wiced_bt_gatt_status_t  Bluetooth LE GATT status
*/
wiced_bt_gatt_status_t app_bt_set_value(uint16_t attr_handle,
                                               uint8_t *p_val,
                                               uint16_t len)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_INVALID_HANDLE;

    for (int i = 0; i < app_gatt_db_ext_attr_tbl_size; i++)
    {
        /* Check for a matching handle entry */
        if (app_gatt_db_ext_attr_tbl[i].handle == attr_handle)
        {
            /* Detected a matching handle in the external lookup table */
            if (app_gatt_db_ext_attr_tbl[i].max_len >= len)
            {
                /* Value fits within the supplied buffer; copy over the value */
                app_gatt_db_ext_attr_tbl[i].cur_len = len;
                memset(app_gatt_db_ext_attr_tbl[i].p_data, 0x00, app_gatt_db_ext_attr_tbl[i].max_len);
                memcpy(app_gatt_db_ext_attr_tbl[i].p_data, p_val, app_gatt_db_ext_attr_tbl[i].cur_len);

                if (memcmp(app_gatt_db_ext_attr_tbl[i].p_data, p_val, app_gatt_db_ext_attr_tbl[i].cur_len) == 0)
                {
                    status = WICED_BT_GATT_SUCCESS;
                }

                if(app_gatt_db_ext_attr_tbl[i].handle == HDLD_BAS_BATTERY_LEVEL_CLIENT_CHAR_CONFIG)
                {
                    if (GATT_CLIENT_CONFIG_NOTIFICATION == app_bas_battery_level_client_char_config[0])
                    {
                        printf("Battery Server Notifications Enabled \r\n");
                    }
                    else
                    {
                        printf("Battery Server Notifications Disabled \r\n");
                    }
                }
            }
            else
            {
                /* Value to write will not fit within the table */
                status = WICED_BT_GATT_INVALID_ATTR_LEN;
                printf("Invalid attribute length\r\n");
            }
            break;
        }
    }
    return status;
}
/**
* Function Name:
* app_bt_find_by_handle
*
* Function Description:
* @brief  Find attribute description by handle
*
* @param handle    handle to look up
*
* @return gatt_db_lookup_table_t   pointer containing handle data
*/
static gatt_db_lookup_table_t *app_bt_find_by_handle(uint16_t handle)
{
    int i;
    for (i = 0; i < app_gatt_db_ext_attr_tbl_size; i++)
    {
        if (app_gatt_db_ext_attr_tbl[i].handle == handle)
        {
            return (&app_gatt_db_ext_attr_tbl[i]);
        }
    }
    return NULL;
}


/**
* Function Name:
* app_bt_gatt_req_read_handler
*
* Function Description:
* @brief  This Function Process read request from peer device
*
* @param conn_id       Connection ID
*
* @param opcode        Bluetooth LE GATT request type opcode
*
* @param p_read_req    Pointer to read request containing the handle to read
*
* @param len_requested length of data requested
*
* @param p_error_handle Pointer to the handle
*
* @return wiced_bt_gatt_status_t  Bluetooth LE GATT status
*/
wiced_bt_gatt_status_t app_bt_gatt_req_read_handler(uint16_t conn_id,
                                                wiced_bt_gatt_opcode_t opcode,
                                                wiced_bt_gatt_read_t *p_read_req,
                                                uint16_t len_requested,
                                                uint16_t *p_error_handle)
{
    gatt_db_lookup_table_t *puAttribute;
    uint16_t attr_len_to_copy, to_send;
    uint8_t *from;

    *p_error_handle = p_read_req->handle;

    if((puAttribute = app_bt_find_by_handle(p_read_req->handle)) == NULL)
    {
        return WICED_BT_GATT_INVALID_HANDLE;
    }

    attr_len_to_copy = puAttribute->cur_len;

    if (p_read_req->offset >= puAttribute->cur_len)
    {
        return WICED_BT_GATT_INVALID_OFFSET;
    }
    to_send = MIN(len_requested, attr_len_to_copy - p_read_req->offset);
    from = puAttribute->p_data + p_read_req->offset;
    /* No need for context, as buff not allocated */
    return wiced_bt_gatt_server_send_read_handle_rsp(conn_id, opcode, to_send, from, NULL);
}

/**
* Function Name:
* app_bt_gatt_req_read_by_type_handler
*
* Function Description:
* @brief  Process read-by-type request from peer device
*
* @param conn_id       Connection ID
*
* @param opcode        Bluetooth LE GATT request type opcode
*
* @param p_read_req    Pointer to read request containing the handle to read
*
* @param len_requested length of data requested
*
* @return wiced_bt_gatt_status_t  Bluetooth LE GATT status
*/
wiced_bt_gatt_status_t app_bt_gatt_req_read_by_type_handler(uint16_t conn_id,
                                                wiced_bt_gatt_opcode_t opcode,
                                    wiced_bt_gatt_read_by_type_t *p_read_req,
                                                    uint16_t len_requested,
                                                    uint16_t *p_error_handle)
{
    gatt_db_lookup_table_t *puAttribute;
    uint16_t attr_handle = p_read_req->s_handle;
    uint8_t *p_rsp = app_bt_alloc_buffer(len_requested);
    uint8_t pair_len = 0;
    int used = 0;

    if (p_rsp == NULL)
    {
        return WICED_BT_GATT_INSUF_RESOURCE;
    }

    /* Read by type returns all attributes of the specified type, between the start and end handles */
    while (WICED_TRUE)
    {
        *p_error_handle = attr_handle;
        attr_handle = wiced_bt_gatt_find_handle_by_type(attr_handle,
                                                        p_read_req->e_handle,
                                                        &p_read_req->uuid);

        if (attr_handle == 0)
            break;

        if ((puAttribute = app_bt_find_by_handle(attr_handle)) == NULL)
        {
            app_bt_free_buffer(p_rsp);
            return WICED_BT_GATT_INVALID_HANDLE;
        }

        {
        int filled = wiced_bt_gatt_put_read_by_type_rsp_in_stream(p_rsp + used,
                                                        len_requested - used,
                                                                    &pair_len,
                                                                    attr_handle,
                                                        puAttribute->cur_len,
                                                        puAttribute->p_data);
        if (filled == 0)
        {
            break;
        }
         used += filled;
        }

        /* Increment starting handle for next search to one past current */
        attr_handle++;
    }

    if (used == 0)
    {
        app_bt_free_buffer(p_rsp);
        return WICED_BT_GATT_INVALID_HANDLE;
    }

    /* Send the response */
    wiced_bt_gatt_server_send_read_by_type_rsp(conn_id, opcode, pair_len, used,
                                            p_rsp, (void *)app_bt_free_buffer);

    return WICED_BT_GATT_SUCCESS;
}


/**
* Function Name:
* app_bt_gatt_req_read_multi_handler
*
* Function Description:
* @brief  Process write read multi request from peer device
*
* @param conn_id       Connection ID
*
* @param opcode        Bluetooth LE GATT request type opcode
*
* @param p_read_req    Pointer to read request containing the handle to read
*
* @param len_requested length of data requested
*
* @param p_error_handle Pointer to the handle
*
* @return wiced_bt_gatt_status_t  Bluetooth LE GATT status
*/
wiced_bt_gatt_status_t app_bt_gatt_req_read_multi_handler(uint16_t conn_id,
                                            wiced_bt_gatt_opcode_t opcode,
                             wiced_bt_gatt_read_multiple_req_t *p_read_req,
                                                    uint16_t len_requested,
                                                uint16_t *p_error_handle)
{
    gatt_db_lookup_table_t *puAttribute;
    uint8_t *p_rsp = app_bt_alloc_buffer(len_requested);
    int used = 0;
    int xx;
    uint16_t handle = wiced_bt_gatt_get_handle_from_stream(p_read_req->p_handle_stream, 0);
    *p_error_handle = handle;

    if (p_rsp == NULL)
    {
        return WICED_BT_GATT_INVALID_HANDLE;
    }

    /* Read by type returns all attributes of the specified type, between the
     * start and end handles */
    for (xx = 0; xx < p_read_req->num_handles; xx++)
    {
        handle = wiced_bt_gatt_get_handle_from_stream(p_read_req->p_handle_stream, xx);
        *p_error_handle = handle;
        if ((puAttribute = app_bt_find_by_handle(handle)) == NULL)
        {
            app_bt_free_buffer(p_rsp);
            return WICED_BT_GATT_ERR_UNLIKELY;
        }
        {
            int filled = wiced_bt_gatt_put_read_multi_rsp_in_stream(opcode, p_rsp + used,
                                                        len_requested - used,
                                                        puAttribute->handle,
                                                        puAttribute->cur_len,
                                                        puAttribute->p_data);
            if (!filled)
            {
                break;
            }
            used += filled;
        }
    }

    if (used == 0)
    {
        return WICED_BT_GATT_INVALID_HANDLE;
    }

    /* Send the response */
    wiced_bt_gatt_server_send_read_multiple_rsp(conn_id, opcode, used, p_rsp,
                                                (void *)app_bt_free_buffer);

    return WICED_BT_GATT_SUCCESS;
}

/**
* Function Name:
* app_bt_prepare_write_handler
*
* Function Description:
* @brief   This function reassemble the fragmented packets
*
* @param   conn_id: Connection ID
*
* @param   opcode: BLE GATT request type opcode
*
* @param   p_req: Pointer to read request containing the handle to read
*
* @param p_error_handle Pointer to the handle
*
* @return wiced_bt_gatt_status_t: GATT result
*
*/
wiced_bt_gatt_status_t app_bt_prepare_write_handler(uint16_t conn_id,
                                        wiced_bt_gatt_opcode_t opcode,
                                        wiced_bt_gatt_write_req_t *p_req,
                                                uint16_t *p_error_handle)
{
    if(write_buff.in_use == false)
    {
        memset(&(write_buff.value[0]), 0x00, CY_BT_MTU_SIZE);
        write_buff.written = 0;
        write_buff.in_use = true;
        write_buff.handle = 0;
    }

    *p_error_handle = p_req->handle;

    /** store the data  */
    if(write_buff.written == p_req->offset)
    {
        int remaining = CY_BT_MTU_SIZE - write_buff.written;
        int to_write = p_req->val_len;

        if (remaining >= to_write)
        {
            memcpy( (void*)((uint32_t)(&(write_buff.value[0]) + write_buff.written)), p_req->p_val, to_write);
            /* send success response */
            printf("== Sending prepare write success response...\n");
            wiced_bt_gatt_server_send_prepare_write_rsp(conn_id, opcode, p_req->handle,
                                                        p_req->offset, to_write,
                                &(write_buff.value[write_buff.written]), NULL);
            write_buff.written += to_write;
            write_buff.handle = p_req->handle;
            return WICED_BT_GATT_SUCCESS;
        }
        else
        {
            printf("remaining >= to_write error...\n");
            return WICED_BT_GATT_ERROR;
        }
    }
    else
    {
        printf("write_buff.written != p_req->offset...\n");
    }
    return WICED_BT_GATT_ERROR;
}

/**
* Function Name:
* app_bt_execute_write_handler
*
* Function Description:
* @brief   This function writes the fragmented packets
*
* @param   p_req: Pointer to read request containing the handle to read
*
* @param p_error_handle Pointer to the handle
*
* @return wiced_bt_gatt_status_t: GATT result
*
*/
wiced_bt_gatt_status_t app_bt_execute_write_handler(wiced_bt_gatt_event_data_t *p_req, uint16_t *p_error_handle)
{
    wiced_bt_gatt_write_req_t       *p_write_req;
    wiced_bt_gatt_status_t          status = WICED_BT_GATT_SUCCESS;

    CY_ASSERT(p_req != NULL);

    p_write_req = &p_req->attribute_request.data.write_req;

    *p_error_handle = p_write_req->handle;

    CY_ASSERT(p_req != NULL);

    if(write_buff.in_use == false)
    {
        return WICED_BT_GATT_ERROR;
    }

    cy_log_msg(CYLF_MIDDLEWARE, CY_LOG_NOTICE, "Execute Write with %d bytes\n", write_buff.written);

    p_write_req->handle = write_buff.handle;
    p_write_req->offset = 0;
    p_write_req->p_val = &(write_buff.value[0]);
    p_write_req->val_len = write_buff.written;

    status = app_bt_write_handler(p_req,p_error_handle);
    if (status != WICED_BT_GATT_SUCCESS)
    {
        printf("app_bt_write_handler() failed....\n");
    }
    write_buff.in_use = false;
    return status;
}
/* [] END OF FILE */

