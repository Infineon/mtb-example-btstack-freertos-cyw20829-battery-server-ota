/******************************************************************************
* File Name: main.c
*
* Description: This is the source code for the Bluetooth LE Battery Server
* Example for ModusToolbox. The Battery Service exposes the Battery Level
* of the device and comes with support for  OTA update over Bluetooth LE.
* A peer app on windows/Android/iOS can be used to push OTA update to the
* device. The app downloads and writes the image to the secondary slot.
* On the next reboot, MCUBoot copies the new image over to the primary slot
* and runs the application. If the new image is not validated in runtime, on
* the next reboot, MCUboot reverts to the previously validated image.
*
* Related Document: See Readme.md
*
********************************************************************************
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
*******************************************************************************/

/*******************************************************************************
*        Header Files
*******************************************************************************/
#include "cyhal.h"
#include "cybsp.h"
#include "cy_sysclk.h"
#include "cybsp_smif_init.h"
#include <string.h>
#include "cy_retarget_io.h"
#include "cybt_platform_trace.h"
#include "GeneratedSource/cycfg_gatt_db.h"
#include "GeneratedSource/cycfg_bt_settings.h"
#include "app_bt_utils.h"
#include "wiced_bt_ble.h"
#include "wiced_bt_uuid.h"
#include "wiced_memory.h"
#include "wiced_bt_stack.h"
#include "cycfg_bt_settings.h"
#include "cycfg_gap.h"
#include "cyhal_gpio.h"
#include "wiced_bt_l2c.h"
#include "cyabs_rtos.h"
#include "app_bt_event_handler.h"
#include "app_bt_gatt_handler.h"
#include "stdlib.h"
#include <inttypes.h>
#include "cyhal_wdt.h"
/* FreeRTOS header file */
#include <FreeRTOS.h>
#include <task.h>
/* OTA related header files */
#include "cy_log.h"
#include "cy_ota_api.h"
#include "app_ota_context.h"
#include "app_ota_serial_flash.h"
#include "cy_ota_platform.h"
#include "cybsp_bt_config.h"

/*******************************************************************************
*        Macro Definitions
*******************************************************************************/
/* Sufficient Heap size for Bluetooth activities */
#define BT_HEAP_SIZE                        (0x1000)
#define BLE_TASK_NAME                       "BAS Task"
#define BAS_TASK_STACK_SIZE                 (256u)
#define BAS_TASK_PRIORITY                   (configMAX_PRIORITIES - 3)

/*******************************************************************************
*        Variable Definitions
*******************************************************************************/
extern cy_ota_agent_mem_interface_t storage_interfaces;
extern TaskHandle_t bas_task_handle;

/*******************************************************************************
*        Function Prototypes
*******************************************************************************/

/******************************************************************************
*       Function Definitions
 ******************************************************************************/
/**
*  Function name:
*  main
*
*  Function Description:
*  @brief    Entry point to the application. Set device configuration and start
*            BT stack initialization.  The actual application initialization
*            will happen when stack reports that BT device is ready.
*
*  @param    void
*
*  @return   int
*/
int main()
{
    cy_rslt_t cy_result;
    wiced_result_t  result;
    cyhal_wdt_t wdt_obj;
    BaseType_t rtos_result;

    /* Initialize the board support package */
    cy_result = cybsp_init();
    if (CY_RSLT_SUCCESS != cy_result)
    {
        CY_ASSERT(0);
    }

    /* Enable global interrupts */
    __enable_irq();

    /* Initialize retarget-io to use the debug UART port */
    cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,
                        CY_RETARGET_IO_BAUDRATE);

    /* default for all logging to WARNING */
    cy_log_init(CY_LOG_WARNING, NULL, NULL);

    /* default for OTA logging to NOTICE */
    cy_ota_set_log_level(CY_LOG_NOTICE);


    /*Initialize QuadSPI if using external flash*/
    /* Initialize SMIF interface */
    if (cybsp_smif_init() != CY_SMIF_SUCCESS)
    {
        printf("ERROR returned from cybsp_smif_init()!!!\r\n");
    }

    printf("*******BTSTACK FREERTOS EXAMPLE************\r\n");
    printf("*******Battery Server Application Start with OTA************\r\n");
    printf("*********************************************\n");
    printf("Application version: %d.%d.%d\n",APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_BUILD);
    printf("*******************************************\n\n");

    /* Initialising the HCI UART for Host contol */
    cybt_platform_config_init(&cybsp_bt_platform_cfg);

    /* set default values for battery server context */
    ota_initialize_default_values();

    /* Validate the update so we do not revert on reboot */
    cy_ota_storage_validated(&storage_interfaces);

    if (cy_ota_ble_check_build_vs_configurator() != CY_RSLT_SUCCESS)
        {
            printf("Failed configurator check \r \n");
            while(true)
            {
                cy_rtos_delay_milliseconds(1000);
            }
        }

    /* Clear watchdog so it doesn't reboot on us */
    cyhal_wdt_init(&wdt_obj, cyhal_wdt_get_max_timeout_ms());
    cyhal_wdt_free(&wdt_obj);

    /* Register call back and configuration with stack */
    result = wiced_bt_stack_init(app_bt_management_callback, &wiced_bt_cfg_settings);

    /* Check if stack initialization was successful */
    if (WICED_BT_SUCCESS != result)
    {
        printf("Bluetooth Stack Initialization failed!! \r\n");
        CY_ASSERT(0);
    }

    /* Create a buffer heap, make it the default heap.  */
    if ( NULL == wiced_bt_create_heap("app", NULL, BT_HEAP_SIZE, NULL, WICED_TRUE))
    {
        printf("Heap create Failed");
    }

    /*Create battery service task*/
    rtos_result = xTaskCreate(bas_task,BLE_TASK_NAME, BAS_TASK_STACK_SIZE,
                            NULL,BAS_TASK_PRIORITY, &bas_task_handle);
    if(pdPASS != rtos_result)
    {
        printf("BAS task creation failed\n");
        CY_ASSERT(0);
    }

    /* Start the FreeRTOS scheduler */
    vTaskStartScheduler();

    /* Should never get here */
    CY_ASSERT(0);
}
/* [] END OF FILE */
