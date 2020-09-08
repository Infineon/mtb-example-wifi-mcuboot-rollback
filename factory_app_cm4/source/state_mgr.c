/******************************************************************************
* File Name: state_mgr.c
*
* Description: This file contains task and functions related to state manager.
*
*******************************************************************************
* (c) 2020, Cypress Semiconductor Corporation. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* ("Software"), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries ("Cypress") and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software ("EULA").
*
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress's integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
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
* including Cypress's product in a High Risk Product, the manufacturer of such
* system or application assumes all risk of such use and in doing so agrees to
* indemnify Cypress against all liability.
*******************************************************************************/

/* Header file includes */
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include "ota_task.h"
#include "led_task.h"

/* FreeRTOS header file */
#include <FreeRTOS.h>
#include <task.h>
#include <limits.h>

/*******************************************************************************
* Macros
********************************************************************************/
/* State Manager task configurations */
#define STATE_MGR_TASK_STACK_SIZE            (configMINIMAL_STACK_SIZE)
#define STATE_MGR_TASK_PRIORITY              (configMAX_PRIORITIES - 3)

/* Only first bit is used for the interrupt detection */
#define USER_EVENT_DETECT_FLAG               (0x01)

/* interrupt priority configuration */
#define USRBTN_INTERRUPT_PRIORITY             (7u)

/*******************************************************************************
* Global Variables
********************************************************************************/
/* State Mgr handle */
static TaskHandle_t state_mgr_task_handle;

/*******************************************************************************
* Function Prototypes
********************************************************************************/
static void user_button_callback(void *handler_arg, cyhal_gpio_event_t event);
static void state_mgr(void *args);

/*******************************************************************************
 * Function Name: state_mgr_task_init
 *******************************************************************************
 * Summary:
 *  Initialize the state manager task.
 *
 *******************************************************************************/
void state_mgr_task_init(void)
{
    /* Create the tasks */
    xTaskCreate(state_mgr, "STATE MGR", STATE_MGR_TASK_STACK_SIZE, NULL,
                STATE_MGR_TASK_PRIORITY, &state_mgr_task_handle);
}

/*******************************************************************************
* Function Name: user_button_callback
********************************************************************************
* Summary:
*   User button callback, called from the interrupt context. 
*
* Parameters:
*  void *handler_arg (unused)
*  cyhal_gpio_irq_event_t (unused)
*
*******************************************************************************/
static void user_button_callback(void *handler_arg, cyhal_gpio_irq_event_t event)
{
    BaseType_t higher_priority_task_woken = pdFALSE;

    /* notify the task */
      xTaskNotifyFromISR( state_mgr_task_handle,
                          USER_EVENT_DETECT_FLAG,
                          eSetBits,
                          &higher_priority_task_woken );

       portYIELD_FROM_ISR( higher_priority_task_woken );
}

/*******************************************************************************
 * Function Name: state_mgr
 *******************************************************************************
 * Summary:
 *  It starts the LED task immediately on start.
 *  Waits for the user button events and starts the OTA task, when the
 *  user button event is detected and then suspends itself.
 *
 * Parameters:
 *  *args : Task parameter defined during task creation (unused)
 *
 *******************************************************************************/
static void state_mgr(void *args)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    BaseType_t event_result = pdFAIL;
    uint32_t notified_value = 0 ;

    /* Initialize the user button. */
    result = cyhal_gpio_init(CYBSP_USER_BTN, CYHAL_GPIO_DIR_INPUT,
                    CYHAL_GPIO_DRIVE_PULLUP, CYBSP_BTN_OFF);

    /* Configure GPIO interrupt. */
    cyhal_gpio_register_callback(CYBSP_USER_BTN,
                                  user_button_callback, NULL);

    /* enable SW2 event. Global event is already enabled. */
    cyhal_gpio_enable_event(CYBSP_USER_BTN, CYHAL_GPIO_IRQ_FALL,
                             USRBTN_INTERRUPT_PRIORITY, true);

    /* To avoid compiler warning. */
    (void)result;
    (void)args;

    printf("Starting LED task..\n");

    /* kick start the LED task. */
    led_task_init();

    printf("\n****Waiting for user button press event****\n");

    /* wait for the event forever. */
    do
    {
        event_result = xTaskNotifyWait( pdFALSE,    /* Don't clear bits on entry. */
                             ULONG_MAX,             /* Clear all bits on exit. */
                             &notified_value,       /* Stores the notified value. */
                             portMAX_DELAY );       /* wait forever. */

        if( (event_result == pdPASS) && (notified_value && USER_EVENT_DETECT_FLAG) )
        {
            printf("Detected user button event..\n");
            break;
        }

    } while(1);

    printf("Starting OTA task..\n");

    /* start the OTA task */
    ota_task_init();

    printf("%s task entering IDLE state...\n\n",__func__);

    /* suspend the task */
    vTaskSuspend( NULL );
}

/* [] END OF FILE */
