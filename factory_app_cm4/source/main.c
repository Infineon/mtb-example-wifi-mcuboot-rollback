/******************************************************************************
* File Name: main.c
*
* Description:
*  This is the source code for the CE230815.
*  "PSoC 6 MCU: MCUboot-Based Bootloader with Rollback to Factory App in External Flash"
*  This factory app will be placed into external flash and later the bootloader
*  copies it to the primary slot in internal flash, based on user input.
*  This factory app is designed to receive user input (user button presses) and
*  initiate OTA using AnyClould OTA middleware. When OTA is initiated, the device
*  establishes a connection with the designated MQTT Broker and subscribes to a topic.
*  When an OTA image is published to that topic, the device automatically pulls
*  the OTA image over MQTT and saves it to the secondary slot in internal flash.
*  On next reboot, Bootloader will copy the new image over to the primary slot
*  and run the new application.
*
* Related Document: See README.md
*
*
*******************************************************************************
* Copyright 2021, Cypress Semiconductor Corporation (an Infineon company) or
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

/* Header file includes */
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include "state_mgr.h"

#ifdef DEBUG_PRINT
#include "cy_log.h"
#endif
/* FreeRTOS header file */
#include <FreeRTOS.h>
#include <task.h>

/*******************************************************************************
* Global Variables
********************************************************************************/
/* This enables RTOS aware debugging. */
volatile int uxTopUsedPriority;

/*******************************************************************************
 * Function Name: main
 ********************************************************************************
 * Summary:
 *  System entrance point. This function sets up OTA task and starts
 *  the RTOS scheduler. Asserts if it fails to kick start the RTOS scheduler.
 *
 * Parameters:
 *  None
 *
 * Return:
 *  int
 *
 *******************************************************************************/
int main(void)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    cyhal_wdt_t wdt_obj;

    /* This enables RTOS aware debugging in OpenOCD */
    uxTopUsedPriority = configMAX_PRIORITIES - 1 ;

    /* Initialize the board support package */
    result = cybsp_init() ;
    CY_ASSERT(result == CY_RSLT_SUCCESS);

    /* Initialize retarget-io to use the debug UART port */
    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,
                                 CY_RETARGET_IO_BAUDRATE);
    CY_ASSERT(result == CY_RSLT_SUCCESS);

    /* To avoid compiler warning */
    (void)result;

    /* Enable global interrupts. */
    __enable_irq();

#ifdef DEBUG_PRINT
    /* default for all loggings */
    cy_log_init(CY_LOG_DEBUG, NULL, NULL);
#endif

    printf("===============================================================\n");
    printf("factory_app_cm4 version: %d.%d.%d\n",
            APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_BUILD);
    printf("===============================================================\n\n");

    /* Clear watchdog timer so that it doesn't trigger a reset */
    cyhal_wdt_init(&wdt_obj, cyhal_wdt_get_max_timeout_ms());
    cyhal_wdt_free(&wdt_obj);

    printf("\nWatchdog timer started by the bootloader is now turned off!!!\n\n");

    /* initialize the state manager */
    state_mgr_task_init();

    /* Start the FreeRTOS scheduler. */
    vTaskStartScheduler();

    /* Should never get here. */
    CY_ASSERT(0);

    return 0;
}

/* [] END OF FILE */
