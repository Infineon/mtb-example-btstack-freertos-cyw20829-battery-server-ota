/*******************************************************************************
* File Name: app_bt_event_handler.h
*
* Description: This file is the public interface of app_bt_event_handler.c
*
* Related Document: See README.md
*
*
********************************************************************************
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
*******************************************************************************/

/*******************************************************************************
*        Header Files
*******************************************************************************/

#include "cyhal.h"
#include "wiced_bt_stack.h"
#include "app_bt_event_handler.h"
#include "app_bt_gatt_handler.h"

/*******************************************************************************
*        Macro Definitions
*******************************************************************************/

/*******************************************************************************
*        Variable Definitions
*******************************************************************************/

/**
 * @brief PWM Handle for controlling advertising LED
 */
cyhal_pwm_t adv_led_pwm;

/**
 * @brief FreeRTOS variable to store handle of task created to update and send
   dummy values of temperature
 */
TaskHandle_t bas_task_handle;

/**
 * @brief Variable for 5 sec timer object
 */
cyhal_timer_t bas_timer_obj;

/**
 * @brief Configure timer for 5 sec
 */
const cyhal_timer_cfg_t bas_timer_cfg =
{
    .compare_value = 0,                    /* Timer compare value, not used */
    .period = BATTERY_LEVEL_UPDATE_MS, /* Defines the timer period */
    .direction = CYHAL_TIMER_DIR_UP,       /* Timer counts up */
    .is_compare = false,                   /* Don't use compare mode */
    .is_continuous = true,                 /* Run timer indefinitely */
    .value = 0                             /* Initial value of counter */
};

app_bt_adv_conn_mode_t app_bt_adv_conn_state = APP_BT_ADV_OFF_CONN_OFF;


/*******************************************************************************
*        Function Prototypes
*******************************************************************************/


/*******************************************************************************
 *       Function Definitions
 ******************************************************************************/

/**
* Function Name: app_bt_management_callback
*
* Function Description:
* @brief
*  This is a Bluetooth stack event handler function to receive management events
*  from the Bluetooth stack and process as per the application.
*
* @param wiced_bt_management_evt_t    Bluetooth LE event code of one byte length
*
* @param wiced_bt_management_evt_data_t Pointer to Bluetooth LE management event
*                                        structures
*
* @return wiced_result_t Error code from WICED_RESULT_LIST or BT_RESULT_LIST
*
*/

wiced_result_t app_bt_management_callback(wiced_bt_management_evt_t event,
                                wiced_bt_management_evt_data_t *p_event_data)
{
    wiced_result_t result  = WICED_BT_ERROR;
    wiced_bt_device_address_t bda = {0};
    wiced_bt_ble_advert_mode_t *p_adv_mode = NULL;
    wiced_bt_dev_encryption_status_t *p_status = NULL;

    switch (event)
    {
    case BTM_ENABLED_EVT:
        /* Bluetooth Controller and Host Stack Enabled */
        if (WICED_BT_SUCCESS == p_event_data->enabled.status)
        {
            /* Initialize the application */
            wiced_bt_set_local_bdaddr((uint8_t *)cy_bt_device_address, BLE_ADDR_PUBLIC);
            /* Bluetooth is enabled */
            wiced_bt_dev_read_local_addr(bda);
            printf("Local Bluetooth Address: ");
            print_bd_address(bda);
            /* Perform application-specific initialization */
            app_bt_init();
            result = WICED_BT_SUCCESS;
        }
        else
        {
            printf("Failed to initialize Bluetooth controller and stack \r\n");
        }
        break;

    case BTM_USER_CONFIRMATION_REQUEST_EVT:
        wiced_bt_dev_confirm_req_reply(WICED_BT_SUCCESS,
                            p_event_data->user_confirmation_request.bd_addr);
        result = WICED_BT_SUCCESS;
        break;

    case BTM_PASSKEY_NOTIFICATION_EVT:
        result = WICED_BT_SUCCESS;
        break;

    case BTM_PAIRING_IO_CAPABILITIES_BLE_REQUEST_EVT:
        p_event_data->pairing_io_capabilities_ble_request.local_io_cap = BTM_IO_CAPABILITIES_NONE;
        p_event_data->pairing_io_capabilities_ble_request.oob_data = BTM_OOB_NONE;
        p_event_data->pairing_io_capabilities_ble_request.auth_req = BTM_LE_AUTH_REQ_BOND | BTM_LE_AUTH_REQ_MITM;
        p_event_data->pairing_io_capabilities_ble_request.max_key_size = 0x10;
        p_event_data->pairing_io_capabilities_ble_request.init_keys = BTM_LE_KEY_PENC | BTM_LE_KEY_PID;
        p_event_data->pairing_io_capabilities_ble_request.resp_keys = BTM_LE_KEY_PENC | BTM_LE_KEY_PID;
        result = WICED_BT_SUCCESS;
        break;

    case BTM_PAIRING_COMPLETE_EVT:
        result = WICED_BT_SUCCESS;
        break;

    case BTM_LOCAL_IDENTITY_KEYS_UPDATE_EVT:
        /* Local identity Keys Update */
        result = WICED_BT_SUCCESS;
        break;

    case BTM_LOCAL_IDENTITY_KEYS_REQUEST_EVT:
        /* Local identity Keys Request */
        result = WICED_BT_ERROR;
        break;

    case BTM_PAIRED_DEVICE_LINK_KEYS_UPDATE_EVT:
        /* Paired Device Link Keys update */
        result = WICED_BT_SUCCESS;
        break;

    case BTM_PAIRED_DEVICE_LINK_KEYS_REQUEST_EVT:
        /* Paired Device Link Keys Request */
        result = WICED_BT_ERROR;
        break;

    case BTM_ENCRYPTION_STATUS_EVT:
        p_status = &p_event_data->encryption_status;
        printf("Encryption Status Event for : bd ");
        print_bd_address(p_status->bd_addr);
        printf("  res: %d \r\n", p_status->result);
        result = WICED_BT_SUCCESS;
        break;

    case BTM_SECURITY_REQUEST_EVT:
        printf("  BTM_SECURITY_REQUEST_EVT\r\n");
        wiced_bt_ble_security_grant(p_event_data->security_request.bd_addr,
                                    WICED_BT_SUCCESS);
        result = WICED_BT_SUCCESS;
        break;

    case BTM_BLE_CONNECTION_PARAM_UPDATE:
        printf("BTM_BLE_CONNECTION_PARAM_UPDATE \r\n");
        print_bd_address(p_event_data->ble_connection_param_update.bd_addr);
        printf("ble_connection_param_update.conn_interval       : %d\r\n",
                p_event_data->ble_connection_param_update.conn_interval);
        printf("ble_connection_param_update.conn_latency        : %d\r\n",
                p_event_data->ble_connection_param_update.conn_latency);
        printf("ble_connection_param_update.supervision_timeout : %d\r\n",
                p_event_data->ble_connection_param_update.supervision_timeout);
        printf("ble_connection_param_update.status              : %d\r\n\n",
                p_event_data->ble_connection_param_update.status);
        result = WICED_BT_SUCCESS;
        break;

    case BTM_BLE_DATA_LENGTH_UPDATE_EVENT:
        printf("BTM_BLE_DATA_LENGTH_UPDATE_EVENT, \r\n"
                "Max tx octets is :%d ,\r\n"
                "Max rx octets is :%d \r\n",
                p_event_data->ble_data_length_update_event.max_tx_octets,
                p_event_data->ble_data_length_update_event.max_rx_octets);
                break;

    case BTM_BLE_ADVERT_STATE_CHANGED_EVT:
        /* Advertisement State Changed */
        p_adv_mode = &p_event_data->ble_advert_state_changed;
        printf("Advertisement State Change: %s\r\n",
                get_bt_advert_mode_name(*p_adv_mode));
        if (BTM_BLE_ADVERT_OFF == *p_adv_mode)
        {
            /* Advertisement Stopped */
            printf("Advertisement stopped\r\n");
            /* Check connection status after advertisement stops */
            if (ota_app.bt_conn_id == 0)
            {
                app_bt_adv_conn_state = APP_BT_ADV_OFF_CONN_OFF;
            }
            else
            {
                app_bt_adv_conn_state = APP_BT_ADV_OFF_CONN_ON;
            }
        }
        else
        {
            /* Advertisement Started */
            printf("Advertisement started\r\n");
            app_bt_adv_conn_state = APP_BT_ADV_ON_CONN_OFF;
        }
        /* Update Advertisement LED to reflect the updated state */
        app_bt_adv_led_update();
        result = WICED_BT_SUCCESS;
        break;

    default:
        printf("Unhandled Bluetooth Management Event: 0x%x %s\r\n",
                event, get_bt_event_name(event));
        break;
    }

    return result;
}

/**
* Function Name:
* app_bt_init
*
* Function Description:
* @brief This functions handles application level inialization tasks and is
*         called from the BT management callback once the Bluetooth LE stack
*         enabled event (BTM_ENABLED_EVT) is triggered. This function is
*         executed in the BTM_ENABLED_EVT management callback.
*
* @param void: unused
*
* @return void
*/
void app_bt_init(void)
{
    cy_rslt_t cy_result = CY_RSLT_SUCCESS;
    wiced_bt_gatt_status_t status = WICED_BT_GATT_ERROR;
    wiced_result_t result;

    printf("app_bt_INIT() called\r\n");
    /* Initialize the PWM used for Advertising LED */
    cy_result = cyhal_pwm_init(&adv_led_pwm, ADV_LED_GPIO, NULL);

    /* PWM init failed. Stop program execution */
    if (CY_RSLT_SUCCESS != cy_result)
    {
        printf("Advertisement LED PWM Initialization has failed! \r\n");
        CY_ASSERT(0);
    }

    /* Disable pairing for this application */
    wiced_bt_set_pairable_mode(WICED_TRUE, 0);

    /* Set Advertisement Data */
    wiced_bt_ble_set_raw_advertisement_data(CY_BT_ADV_PACKET_DATA_SIZE,
                                            cy_bt_adv_packet_data);

    /* Register with BT stack to receive GATT callback */
    status = wiced_bt_gatt_register(app_bt_gatt_event_callback);
    printf("GATT event Handler registration status: %s \r\n",
            get_bt_gatt_status_name(status));

    /* Initialize GATT Database */
    status = wiced_bt_gatt_db_init(gatt_database, gatt_database_len, NULL);
    printf("GATT database initialization status: %s \r\n",
            get_bt_gatt_status_name(status));

    /* Start Undirected Bluetooth LE Advertisements on device startup.
     * The corresponding parameters are contained in 'app_bt_cfg.c' */
    result = wiced_bt_start_advertisements(BTM_BLE_ADVERT_UNDIRECTED_HIGH, 0, NULL);
    if (WICED_BT_SUCCESS != result)
    {
        printf("Advertisement cannot start because of error: %d \r\n",
                result);
        CY_ASSERT(0);
    }

    printf("***********************************************\r\n");
    printf("**Discover device with \"Battery Server\" name*\r\n");
    printf("***********************************************\r\n\n");

}

/**
* Function Name:
* app_bt_adv_led_update
*
* Function Description :
* @brief This function updates the advertising LED state based on Bluetooth LE
*         advertising connection state.
*
* @return void
*/
void app_bt_adv_led_update(void)
{
    cy_rslt_t cy_result = CY_RSLT_SUCCESS;

    /* Stop the advertising led pwm */
    cy_result = cyhal_pwm_stop(&adv_led_pwm);
    if (CY_RSLT_SUCCESS != cy_result)
    {
        printf("Failed to stop PWM !!\r\n");
    }

    /* Update LED state based on Bluetooth LE advertising/connection state.
     * LED OFF for no advertisement/connection, LED blinking for advertisement
     * state, and LED ON for connected state  */
    switch (app_bt_adv_conn_state)
    {
    case APP_BT_ADV_OFF_CONN_OFF:
        cy_result = cyhal_pwm_set_duty_cycle(&adv_led_pwm, LED_OFF_DUTY_CYCLE,
                                             ADV_LED_PWM_FREQUENCY);
        break;

    case APP_BT_ADV_ON_CONN_OFF:
        cy_result = cyhal_pwm_set_duty_cycle(&adv_led_pwm, LED_BLINKING_DUTY_CYCLE,
                                             ADV_LED_PWM_FREQUENCY);
        break;

    case APP_BT_ADV_OFF_CONN_ON:
        cy_result = cyhal_pwm_set_duty_cycle(&adv_led_pwm, LED_ON_DUTY_CYCLE,
                                             ADV_LED_PWM_FREQUENCY);
        break;

    default:
        /* LED OFF for unexpected states */
        cy_result = cyhal_pwm_set_duty_cycle(&adv_led_pwm, LED_OFF_DUTY_CYCLE,
                                             ADV_LED_PWM_FREQUENCY);
        break;
    }
    /* Check if update to PWM parameters is successful*/
    if (CY_RSLT_SUCCESS != cy_result)
    {
        printf("Failed to set duty cycle parameters!!\r\n");
    }

    /* Start the advertising led pwm */
    cy_result = cyhal_pwm_start(&adv_led_pwm);

    /* Check if PWM started successfully */
    if (CY_RSLT_SUCCESS != cy_result)
    {
        printf("Failed to start PWM !!\r\n");
    }
}

/**
* Function Name:
* app_bt_batt_level_init
*
* Function Description :
* @brief This function Starts the timer for updating Battery Level
*
* @param void
*
* @return void
*/
void app_bt_batt_level_init(void)
{
    /* Start the timer */
    if (CY_RSLT_SUCCESS != cyhal_timer_start(&bas_timer_obj))
    {
        printf("BAS timer start failed !");
        CY_ASSERT(0);
    }
}

/**
* Function Name:
* bas_task
*
* Function Description:
* @brief This task updates dummy battery value every time it is notified
*         and sends a notification to the connected peer
*
* @param pvParam: unused
*
* @return void
*/
void bas_task(void *pvParam)
{

    cy_rslt_t cy_result = CY_RSLT_SUCCESS;
    /* Initialize the HAL timer used to count seconds */
    cy_result = cyhal_timer_init(&bas_timer_obj, NC, NULL);
    if (CY_RSLT_SUCCESS != cy_result)
    {
        printf("BAS timer init failed !\n");
    }
    /* Configure the timer for 5 seconds */
    cyhal_timer_configure(&bas_timer_obj, &bas_timer_cfg);
    cy_result = cyhal_timer_set_frequency(&bas_timer_obj, BATTERY_LEVEL_UPDATE_FREQ);
    if (CY_RSLT_SUCCESS != cy_result)
    {
        printf("BAS timer set freq failed !\n");
    }
    /* Register for a callback whenever timer reaches terminal count */
    cyhal_timer_register_callback(&bas_timer_obj, bas_timer_callb, NULL);
    cyhal_timer_enable_event(&bas_timer_obj, CYHAL_TIMER_IRQ_TERMINAL_COUNT, 3, true);

    /* Start battery level timer */
    app_bt_batt_level_init();

    while(true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        /* Battery level is read from gatt db and is reduced by 2 percent
        * by default and initialized again to 100 once it reaches 0*/
        if (0 == app_bas_battery_level[0])
        {
            app_bas_battery_level[0] = 100;
        }
        else
        {
            app_bas_battery_level[0] = app_bas_battery_level[0] - BATTERY_LEVEL_CHANGE;
        }
        if (ota_app.bt_conn_id)
        {
            if (app_bas_battery_level_client_char_config[0] & GATT_CLIENT_CONFIG_NOTIFICATION)
            {
                wiced_bt_gatt_server_send_notification(ota_app.bt_conn_id,
                                                HDLC_BAS_BATTERY_LEVEL_VALUE,
                                                app_bas_battery_level_len,
                                                app_bas_battery_level,NULL);

                printf("================================================\r\n");
                printf("Sending Notification: Battery level: %u\r\n",
                        app_bas_battery_level[0]);
                printf("================================================\r\n");
            }
        }
    }
}

/**
* Function Name:
* bas_timer_callb
*
* Function Description:
* @brief  This callback function is invoked on timeout of 1 second timer
*
* @param callback_arg: unused
*
* @param event: unused
*
* @return void
*/
void bas_timer_callb(void *callback_arg, cyhal_timer_event_t event)
{
    BaseType_t xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(bas_task_handle, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/* [] END OF FILE */


