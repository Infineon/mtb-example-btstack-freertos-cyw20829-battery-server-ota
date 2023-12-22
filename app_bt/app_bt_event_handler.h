/*******************************************************************************
 * File Name: app_bt_event_handler.h
 *
 * Description: This file is the public interface of app_bt_event_handler.c
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
/* OTA related header files */
#include "app_ota_context.h"
#include "cy_ota_platform.h"

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