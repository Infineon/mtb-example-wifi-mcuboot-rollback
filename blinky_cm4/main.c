/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the CE230815 -
*              "PSoC 6 MCU: MCUboot-Based Bootloader with Rollback to
*               Factory App in External Flash"
*
* Related Document: See README.md
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

#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
/*******************************************************************************
* Macros
********************************************************************************/
/* LED turn off and on interval based on BOOT or UPGRADE image*/
#ifdef BOOT
    #define LED_TOGGLE_INTERVAL_MS         (5000u)
    #define IMG_TYPE                       "BOOT"
#elif defined(UPGRADE)
    #define LED_TOGGLE_INTERVAL_MS         (250u)
    #define IMG_TYPE                       "UPGRADE"
#else
    #error "[BlinkyApp] Please define the image type: BOOT_IMG or UPGRADE_IMG\n"
#endif

/******************************************************************************
 * Function Name: main
 ******************************************************************************
 * Summary:
 *  System entrance point. This function initializes system resources & 
 *  peripherals, initializes retarget IO, and toggles the user LED at a
 *  configured interval. 
 *
 * Parameters:
 *  void
 *
 * Return:
 *  int
 *
 ******************************************************************************/
int main(void)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    cyhal_wdt_t wdt_obj;

    /* Initialize the device and board peripherals */
    result = cybsp_init();
    CY_ASSERT(result == CY_RSLT_SUCCESS);

    /* Enable global interrupts */
    __enable_irq();
    
    /* Initialize retarget-io to use the debug UART port */
    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, CY_RETARGET_IO_BAUDRATE);
    CY_ASSERT(result == CY_RSLT_SUCCESS);

    /* Initialize the User LED */
    result = cyhal_gpio_init(CYBSP_USER_LED, CYHAL_GPIO_DIR_OUTPUT,
              CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);
    CY_ASSERT(result == CY_RSLT_SUCCESS);

    /* satisfy the compiler */
    (void)result;

    printf("\n=========================================================\n");
    printf("[BlinkyApp] Image Type: %s, Version: %d.%d.%d, CPU: CM4\n", 
           IMG_TYPE, APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_BUILD);
    printf("\n=========================================================\n");

    printf("[BlinkyApp] User LED toggles at %d msec interval\r\n\n", LED_TOGGLE_INTERVAL_MS);

    /* Clear watchdog timer so that it doesn't trigger a reset */
    cyhal_wdt_init(&wdt_obj, cyhal_wdt_get_max_timeout_ms());
    cyhal_wdt_free(&wdt_obj);
    printf("[BlinkyApp] Watchdog timer started by the bootloader is now turned off to mark the successful start of Blinky app.\r\n");

    for (;;)
    {
        /* Toggle the user LED periodically */
        cyhal_system_delay_ms(LED_TOGGLE_INTERVAL_MS);
        cyhal_gpio_toggle(CYBSP_USER_LED);
    }

    return 0;
}


/* [] END OF FILE */
