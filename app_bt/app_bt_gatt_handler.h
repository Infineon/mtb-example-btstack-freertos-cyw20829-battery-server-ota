/*******************************************************************************
 * File Name: app_bt_gatt_handler.h
 *
 * Description: This file is the public interface of app_bt_gatt_handler.c
 *
 * Related Document: See README.md
 *
 *
 *******************************************************************************
 * Copyright 2021-2023, Cypress Semiconductor Corporation (an Infineon company) or
 * an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
 *
 * This software, including source code, documentation and related
 * materials ("Software") is owned by Cypress Semiconductor Corporation
 * or one of its affiliates ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products.  Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 ******************************************************************************/
#ifndef __APP_BT_GATT_HANDLER_H__
#define __APP_BT_GATT_HANDLER_H__

/*******************************************************************************
*        Header Files
*******************************************************************************/

/*******************************************************************************
*        Macro Definitions
*******************************************************************************/

/*******************************************************************************
*        Variable Definitions
*******************************************************************************/
extern cy_ota_agent_mem_interface_t storage_interfaces;

/*******************************************************************************
*        Function Prototypes
*******************************************************************************/
wiced_bt_gatt_status_t app_bt_gatt_req_read_handler(uint16_t conn_id,
                                                    wiced_bt_gatt_opcode_t opcode,
                                                    wiced_bt_gatt_read_t *p_read_req,
                                                    uint16_t len_requested,
                                                    uint16_t *p_error_handle);
wiced_bt_gatt_status_t app_bt_gatt_req_read_multi_handler(uint16_t conn_id,
                                                    wiced_bt_gatt_opcode_t opcode,
                                                    wiced_bt_gatt_read_multiple_req_t *p_read_req,
                                                    uint16_t len_requested,
                                                    uint16_t *p_error_handle);
wiced_bt_gatt_status_t app_bt_gatt_req_read_by_type_handler(uint16_t conn_id,
                                                    wiced_bt_gatt_opcode_t opcode,
                                                    wiced_bt_gatt_read_by_type_t *p_read_req,
                                                    uint16_t len_requested,
                                                    uint16_t *p_error_handle);
wiced_bt_gatt_status_t app_bt_connect_event_handler(wiced_bt_gatt_connection_status_t *p_conn_status);

wiced_bt_gatt_status_t app_bt_server_event_handler(wiced_bt_gatt_event_data_t *p_data,
                                                    uint16_t *p_error_handle);
wiced_bt_gatt_status_t app_bt_set_value(uint16_t attr_handle,uint8_t *p_val,
                                                    uint16_t len);
wiced_bt_gatt_status_t app_bt_write_handler(wiced_bt_gatt_event_data_t *p_data,
                                                    uint16_t *p_error_handle);
wiced_bt_gatt_status_t app_bt_prepare_write_handler(uint16_t conn_id,
                                                    wiced_bt_gatt_opcode_t opcode,
                                                    wiced_bt_gatt_write_req_t *p_req,
                                                    uint16_t *p_error_handle);
wiced_bt_gatt_status_t app_bt_execute_write_handler(wiced_bt_gatt_event_data_t *p_req,
                                                    uint16_t *p_error_handle);
wiced_bt_gatt_status_t app_bt_gatt_event_callback(wiced_bt_gatt_evt_t event,
                                                    wiced_bt_gatt_event_data_t *p_event_data);
/**
 * @brief Typdef for function used to free allocated buffer to stack
 */
typedef void (*pfn_free_buffer_t)(uint8_t *);

#endif
/* [] END OF FILE */
