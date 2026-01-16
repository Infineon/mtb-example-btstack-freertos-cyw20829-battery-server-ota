/*******************************************************************************
 * File Name: app_bt_event_handler.h
 *
 * Description: This file is the public interface of app_bt_event_handler.c
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

#ifndef APP_BT_EVENT_HANDLER_H__
#define APP_BT_EVENT_HANDLER_H__

/*******************************************************************************
*        Header Files
*******************************************************************************/

#include <string.h>
#include "app_bt_utils.h"
#include "wiced_memory.h"
#include "cycfg_bt_settings.h"
/* FreeRTOS header file */
#include <FreeRTOS.h>
#include <task.h>
/* OTA related header files */
#include "app_ota_context.h"

/*******************************************************************************
*        Macro Definitions
*******************************************************************************/
/**
 * @brief rate of change of battery level
 */
#define BATTERY_LEVEL_CHANGE (2)

/**
 * @brief LED pin assignments for advertising event
 */
#define ADV_LED_GPIO CYBSP_USER_LED1

/**
 * @brief PWM frequency of LED's in Hz when blinking
 */
#define ADV_LED_PWM_FREQUENCY (1)

/**
 * @brief Update rate of Battery level
 */
#define BATTERY_LEVEL_UPDATE_MS   (9999u)
#define BATTERY_LEVEL_UPDATE_FREQ (10000)

/*******************************************************************************
*        Variable Definitions
*******************************************************************************/

/**
 * @brief PWM Duty Cycle of LED's for different states
 */
typedef enum
{
    LED_ON_DUTY_CYCLE = 0,
    LED_BLINKING_DUTY_CYCLE = 50,
    LED_OFF_DUTY_CYCLE = 100
} led_duty_cycles;

/**
 * @brief This enumeration combines the advertising, connection states from two
 *        different callbacks to maintain the status in a single state variable
 */
typedef enum
{
    APP_BT_ADV_OFF_CONN_OFF,
    APP_BT_ADV_ON_CONN_OFF,
    APP_BT_ADV_OFF_CONN_ON
} app_bt_adv_conn_mode_t;

/*******************************************************************************
*        Function Prototypes
*******************************************************************************/

/*******************************************************************************
 *       Function Definitions
 ******************************************************************************/

/* Callback function for Bluetooth stack management type events */
wiced_bt_dev_status_t  app_bt_management_callback(wiced_bt_management_evt_t event,
                                                    wiced_bt_management_evt_data_t *p_event_data);


void                   app_bt_adv_led_update                 (void);
void                   app_bt_init                           (void);
void                   app_bt_batt_level_init                (void);

/* Task to send notifications with dummy battery values */
void bas_task(void *pvParam);
/* HAL timer callback registered when timer reaches terminal count */
void bas_timer_callb(void *callback_arg, cyhal_timer_event_t event);

#endif
/* [] END OF FILE */
