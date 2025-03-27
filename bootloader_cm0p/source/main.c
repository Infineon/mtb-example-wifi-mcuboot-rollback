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
* Copyright 2021-2025, Cypress Semiconductor Corporation (an Infineon company) or
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

/* standard headers */
#include <stdio.h>

/* Drive header files */
#include "cy_pdl.h"
#include "cyhal.h"
#include "cycfg.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include "cyw_platform_utils.h"
#include "cy_result.h"
/* MCUboot header files */
#include "sysflash/sysflash.h"
#include "flash_map_backend/flash_map_backend.h"
#include "bootutil/image.h"
#include "bootutil/bootutil.h"
#include "bootutil/sign_key.h"
#include "bootutil/bootutil_log.h"
#include "bootutil/fault_injection_hardening.h"
/* External flash interface header files */
#if defined(CY_BOOT_USE_EXTERNAL_FLASH)
#include "flash_qspi.h"
#include "flash_map_backend_platform.h"
#endif /* defined(CY_BOOT_USE_EXTERNAL_FLASH) */


/*******************************************************************************
* Macros
********************************************************************************/
/* Delay for which CM0+ waits before enabling CM4 so that the messages printed
 * by CM0+ do not go unnoticed by the user since these messages may be
 * overwritten by CM4.
 */
#define CM4_BOOT_DELAY_MS       (100UL)

/* Slave Select line to which the external memory is connected.
 * Acceptable values are:
 * 0 - SMIF disabled (no external memory);
 * 1, 2, 3, or 4 - slave select line to which the memory module is connected.
 */
#define QSPI_SLAVE_SELECT_LINE              (1UL)

/* Button status:
 * GPIO will read LOW, if pressed
 */
#define CYBSP_USER_BTN_PRESSED        (0)

/* here, In this code example MCUBOOT_IMAGE_NUMBER=1
 * MCUBOOT_IMAGE_1_INDEX value is always 0.
 * This defines can be useful, when MCUBOOT_IMAGE_NUMBER > 1.
 */
#define MCUBOOT_IMAGE_1_INDEX   (0)

/* WDT time out for reset mode, in milliseconds. */
#define WDT_TIME_OUT_MS                     (4000UL)

#define BOOT_MSG_FINISH "MCUBoot Bootloader finished.\r\n" \
                        "Deinitializing hardware..."

/* user button interrupt configurations  */
static cy_stc_sysint_t user_btn_isr_cfg =
{
    .intrSrc = NvicMux6_IRQn,
    .cm0pSrc = ioss_interrupts_gpio_0_IRQn,
    .intrPriority = 1,
};

/*******************************************************************************
* Global  variables
********************************************************************************/
static volatile bool is_user_button_pressed = false ;

/*******************************************************************************
* Function Prototypes
********************************************************************************/
static cy_rslt_t transfer_factory_image(void);
static bool do_boot(struct boot_rsp *rsp, char *msg);
static void rollback_to_factory_image(void);
static void user_button_isr(void);
static void hw_deinit(void);

/******************************************************************************
 * Function Name: hw_deinit
 ******************************************************************************
 * Summary:
 *  This function performs the necessary hardware de-initialization.
 *
 * Parameters:
 *  None
 *
 ******************************************************************************/
static void hw_deinit(void)
{
#if defined(CY_BOOT_USE_EXTERNAL_FLASH) && !defined(MCUBOOT_ENC_IMAGES_XIP) && !defined(USE_XIP)
    qspi_deinit(QSPI_SLAVE_SELECT_LINE);
#endif /* defined(CY_BOOT_USE_EXTERNAL_FLASH) && !defined(MCUBOOT_ENC_IMAGES_XIP)
        * && !defined(USE_XIP) */

    /* Flush the TX buffer, need to be fixed in retarget-io */
    while(cy_retarget_io_is_tx_active()){}
    /* Deinitializing the retarget-io */
    cy_retarget_io_deinit();
}

/******************************************************************************
 * Function Name: calc_app_addr
 ******************************************************************************
 * Summary:
 *  This function extracts the calculate the application address
 *
 * Parameters:
 *  flash_base - internal flash base address
 *  rsp - Pointer to a structure holding the address to boot from.
 *
 ******************************************************************************/
static inline __attribute__((always_inline))
fih_uint calc_app_addr(uintptr_t flash_base, const struct boot_rsp *rsp)
{
    return fih_uint_encode(flash_base +
                           rsp->br_image_off +
                           rsp->br_hdr->ih_hdr_size);
}

/******************************************************************************
 * Function Name: user_button_isr
 ******************************************************************************
 * Summary:
 * This is the ISR handler, executed on user button press event.
 * Sets event flag when event is detected and clears
 * the interrupts.
 *
 ******************************************************************************/
static void user_button_isr(void)
{
    /* button pressed */
    is_user_button_pressed = true;

    /* clear the interrupt flag */
    Cy_GPIO_ClearInterrupt(CYBSP_USER_BTN_PORT, CYBSP_USER_BTN_PIN);
}

/******************************************************************************
 * Function Name: transfer_factory_image
 ******************************************************************************
 * Summary:
 *  This function configures the QSPI in SFDP mode, does a simple sanity check
 *  on the 'Factory App' and transfers the firmware from external memory to
 *  primary slot. Any critical error will lead to assertion.
 *
 * Parameters:
 *  none
 *
 * Return
 * status of operation cy_rslt_t
 *
 ******************************************************************************/
static cy_rslt_t transfer_factory_image(void)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    struct flash_area fap_extf;
    const struct flash_area *fap_primary = NULL;
    uint8_t ram_buf[CY_FLASH_SIZEOF_ROW] = {0};
    uint32_t index = 0 ;
    uint32_t bytes_to_copy = 0 ;
    uint32_t prim_slot_off = 0 ;
    uint32_t fact_img_off = 0 ;
    uint32_t image_magic = 0 ;
    uint32_t smif_mem_off = 0;
    cy_stc_smif_mem_config_t *cfg;
    cy_en_smif_status_t smif_status;

    /* Factory App area is in external flash.
     * Flash map doesn't have any flash_area entry for the Factory App
     * To be compatible with MCUboot SMIF wrappers, populate a dummy flash_area
     * structure with necessary details.
     * Note: For read operation, only "fa_device_id" is sufficient.
     * Populating only that for now.
     */
    fap_extf.fa_device_id = FLASH_DEVICE_EXTERNAL_FLASH(
            CY_BOOT_EXTERNAL_DEVICE_INDEX);

    /* Open primary slot */
    result = flash_area_open(FLASH_AREA_IMAGE_PRIMARY(MCUBOOT_IMAGE_1_INDEX), &fap_primary);

    if(result != CY_RSLT_SUCCESS)
    {
        BOOT_LOG_ERR("Failed to open primary slot !");

        /* critical error: asserting */
        CY_ASSERT(0);
    }

    cfg = qspi_get_memory_config(FLASH_DEVICE_GET_EXT_INDEX(fap_extf.fa_device_id));
    smif_status = Cy_SMIF_MemRead(qspi_get_device(), cfg, smif_mem_off, (void *)&image_magic, sizeof(image_magic), qspi_get_context());
    if(smif_status != CY_SMIF_SUCCESS)
    {
        BOOT_LOG_ERR("Failed to read 'Factory App' magic from external memory\r\n");

        /* critical error: asserting */
        CY_ASSERT(0);
    }
    else if(image_magic != IMAGE_MAGIC)
    {
        BOOT_LOG_ERR("Invalid image magic 0x%08x !\r\n", (int)image_magic);

        /* critical error: asserting */
        CY_ASSERT(0);
    }
    else
    {
        BOOT_LOG_INF("Valid image magic found");
        BOOT_LOG_INF("Erasing primary slot. Please wait for a while...\r\n");

        /* erase primary slot completely */
        result = flash_area_erase(fap_primary, 0, fap_primary->fa_size);
    }

    if (result != CY_RSLT_SUCCESS)
    {
        BOOT_LOG_ERR("Failed to erase Primary Slot !");

        /* critical error: asserting */
        CY_ASSERT(0);
    }
    else
    {
        /* Initialize the parameters required for copy operation.
         * Partition size of internal flash and that of external flash
         * need not be same always. We select the smaller size out of two
         * to ensure the cleaner boundaries.
         * It is mandatory to keep the size of Factory App firmware to fit within
         * the primary slot for this application to work correctly.
         */
        bytes_to_copy = ((fap_primary->fa_size < CY_FACT_APP_SIZE) ?
                        (fap_primary->fa_size) : (CY_FACT_APP_SIZE));
        fact_img_off = 0;
        prim_slot_off = 0;

        BOOT_LOG_INF("Transferring 'Factory App' to 'primary slot'");
        BOOT_LOG_INF("Please wait for a while...\r\n");

        /* Copy Factory App to primary slot.
         * Read from external memory and then write to primary slot in
         * chunks of "CY_FLASH_SIZEOF_ROW" bytes. status of the transfer will be
         * returned to caller.
         */
        CY_ASSERT((bytes_to_copy % CY_FLASH_SIZEOF_ROW) == 0);
        while (index < bytes_to_copy)
        {
            /* read from QSPI */
            smif_status = Cy_SMIF_MemRead(qspi_get_device(), cfg, fact_img_off, ram_buf, CY_FLASH_SIZEOF_ROW, qspi_get_context());
            if(smif_status != CY_SMIF_SUCCESS)
            {
                BOOT_LOG_ERR("failed to read Factory App @ offset 0x%8x",
                        (int )fact_img_off);
                break;
            }

            /* Write to Internal flash */
            result = flash_area_write(fap_primary, prim_slot_off, ram_buf, CY_FLASH_SIZEOF_ROW);

            if (result != CY_RSLT_SUCCESS)
            {
                BOOT_LOG_ERR("failed to write primary slot @ offset 0x%8x",
                        (int )prim_slot_off);
                break;
            }

            fact_img_off += CY_FLASH_SIZEOF_ROW;
            prim_slot_off += CY_FLASH_SIZEOF_ROW;
            index += CY_FLASH_SIZEOF_ROW;
        }
    }

    /* Cleanup the resources acquired */
    flash_area_close(fap_primary);

    if (result == CY_RSLT_SUCCESS)
    {
        /* control reaches here means, we are good for Factory App boot */
        BOOT_LOG_INF("Factory App copied to primary slot successfully");
    }

    return result;
}


/******************************************************************************
 * Function Name: do_boot
 ******************************************************************************
 * Summary:
 *  This function extracts the image address and enables CM4 to let it boot
 *  from that address.
 *
 * Parameters:
 *  rsp - Pointer to a structure holding the address to boot from.
 *
 ******************************************************************************/
static bool do_boot(struct boot_rsp *rsp, char *msg)
{
    uintptr_t flash_base = 0;

    if ((NULL != rsp) && (NULL != rsp->br_hdr))
    {
        int result = flash_device_base(rsp->br_flash_dev_id, &flash_base);

        if (CY_RSLT_SUCCESS == result)
        {
            fih_uint app_addr = calc_app_addr(flash_base, rsp);

            BOOT_LOG_INF("Starting User Application (wait)...");

            if (IS_ENCRYPTED(rsp->br_hdr))
            {
                BOOT_LOG_DBG(" * User application is encrypted");
            }

            BOOT_LOG_INF("Start slot Address: 0x%08" PRIx32, (uint32_t)fih_uint_decode(app_addr));

            result = flash_device_base(rsp->br_flash_dev_id, &flash_base);

            if (CY_RSLT_SUCCESS != result || fih_uint_eq(calc_app_addr(flash_base, rsp), app_addr) != FIH_TRUE)
            {
                return false;
            }

#ifdef USE_XIP
            BOOT_LOG_DBG("XIP: Switch to SMIF XIP mode");
            qspi_set_mode(CY_SMIF_MEMORY);
#endif /* USE_XIP */

            CY_ASSERT(NULL != msg);
            BOOT_LOG_INF("Launching %s on CM4. Please wait...", msg);
            BOOT_LOG_INF(BOOT_MSG_FINISH);
            hw_deinit();
            /* This function turns on CM4 */
            Cy_SysEnableCM4(fih_uint_decode(app_addr));
            return true;
        }
        else
        {
            BOOT_LOG_ERR("Flash device ID not found");
            return false;
        }
    }

    return false;
}

/******************************************************************************
 * Function Name: rollback_to_factory_image
 ******************************************************************************
 * Summary:
 * This function validates the primary slot and starts CM4 boot, if a valid
 * image is found in primary slot, returns on successful boot of the
 * factory image, and asserts on failure.
 *
 ******************************************************************************/
static void rollback_to_factory_image(void)
{
    struct boot_rsp rsp;
    bool boot_succeeded = false;
    fih_int fih_status = FIH_FAILURE;

    if(transfer_factory_image() != CY_RSLT_SUCCESS)
    {
        BOOT_LOG_ERR("Factory app transfer failed !");
        CY_ASSERT(0);
    }

    /* Image transfer successful. Now verify the image in primary slot.
    * At this point of time, we expect no pending updates.
    * All pending updates are cleared on boot.
    */
    FIH_CALL(boot_go, fih_status, &rsp);

    if (FIH_TRUE == fih_eq(fih_status, FIH_SUCCESS))
    {
        BOOT_LOG_INF("Factory app validated successfully");

        boot_succeeded = do_boot(&rsp, "Factory App");
        if (!boot_succeeded)
        {
            BOOT_LOG_ERR("Booting of Factory App failed");
        }
    }
    else
    {
        /* assert on failure to rollback */
        BOOT_LOG_ERR("Factory app validation failed");
        BOOT_LOG_ERR("Can't Rollback, asserting!!");

        CY_ASSERT(0);
    }
}

/******************************************************************************
 * Function Name: main
 ******************************************************************************
 * Summary:
 *  System entrance point. This function initializes system resources &
 *  peripherals, initializes retarget-IO and user button. Boot to application, if a
 *  valid image is present. Performs the rollback, if requested by the user.
 *
 ******************************************************************************/
int main(void)
{
    struct boot_rsp rsp = {};
    bool boot_succeeded = false;
    fih_int fih_status = FIH_FAILURE;
    cy_rslt_t result;
    cyhal_wdt_t *wdt = NULL;

    result = cybsp_init();
    if (CY_RSLT_SUCCESS != result)
    {
        CY_ASSERT(0);
    }

    /* enable interrupts */
    __enable_irq();

    /* Certain PSOC 6 devices enable CM4 by default at startup. It must be
     * either disabled or enabled & running a valid application for flash write
     * to work from CM0+. Since flash write may happen in boot_go() for updating
     * the image before this bootloader app can enable CM4 in do_boot(), we need
     * to keep CM4 disabled. Note that debugging of CM4 is not supported when it
     * is disabled.
     */
#if defined(CY_DEVICE_PSOC6ABLE2)
    if (CY_SYS_CM4_STATUS_ENABLED == Cy_SysGetCM4Status())
    {
        Cy_SysDisableCM4();
    }
#endif /* defined(CY_DEVICE_PSOC6ABLE2) */

    /* Initialize retarget-io to use the debug UART port */
    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX,
                             CYBSP_DEBUG_UART_RX,
                             CY_RETARGET_IO_BAUDRATE);
    if (CY_RSLT_SUCCESS != result)
    {
        CY_ASSERT(0);
        /* Loop forever... */
        while (true)
        {
            __WFI();
        }
    }

    BOOT_LOG_INF("\x1b[2J\x1b[;H");
    BOOT_LOG_INF("MCUboot Bootloader Started");

#ifdef CY_BOOT_USE_EXTERNAL_FLASH
    /* Initialize QSPI NOR flash using SFDP. */
    cy_en_smif_status_t qspi_status = qspi_init_sfdp(QSPI_SLAVE_SELECT_LINE);

    if (CY_SMIF_SUCCESS == qspi_status)
    {
        result = CY_RSLT_SUCCESS;
        BOOT_LOG_INF("External Memory initialized w/ SFDP.");
    }
    else
    {
        BOOT_LOG_ERR("External Memory initialization w/ SFDP FAILED: 0x%08" PRIx32, (uint32_t)qspi_status);

        /* critical error: asserting */
        CY_ASSERT(0);
    }

    if (CY_RSLT_SUCCESS == result)
#endif /* CY_BOOT_USE_EXTERNAL_FLASH */
    {
        /* perform upgrade if pending and check primary slot is valid or not
        */
        FIH_CALL(boot_go, fih_status, &rsp);

        if (FIH_TRUE == fih_eq(fih_status, FIH_SUCCESS))
        {
            BOOT_LOG_INF("Application validated successfully !");

            /* We have a valid image in primary slot. Check if user wants
            * to initiate rollback. Rollback can be initiated only if user button
            * is held pressed during this stage. Otherwise device will jump to
            * the application directly.
            */

            if(Cy_GPIO_Read(CYBSP_USER_BTN_PORT, CYBSP_USER_BTN_PIN) == CYBSP_USER_BTN_PRESSED)
            {
                BOOT_LOG_INF("Detected user button event");
                BOOT_LOG_INF("Rollback initiated at startup \r\n") ;
                rollback_to_factory_image();
            }
            else
            {
                /* User button is not pressed at this point !
                * Boot to application
                */
                result = cyhal_wdt_init(wdt, WDT_TIME_OUT_MS);
                if (CY_RSLT_SUCCESS != result)
                {
                    BOOT_LOG_ERR("Failed to init WDT");
                    CY_ASSERT(0);
                }

                boot_succeeded = do_boot(&rsp, "User App");
                if (!boot_succeeded)
                {
                    BOOT_LOG_ERR("Booting of User App failed");
                }
            }
        }
        else
        {
            /* No update is pending in secondary slot, primary slot is not valid.
            * Wait for user input for further actions.
            */

            /* Configure GPIO interrupt vector for Port 0 */
            Cy_SysInt_Init(&user_btn_isr_cfg, user_button_isr);
            NVIC_EnableIRQ((IRQn_Type)user_btn_isr_cfg.intrSrc);

            /* console message and inform user that an action is expected */
            BOOT_LOG_INF("No Upgrade available !");
            BOOT_LOG_INF("No valid image found in primary slot !");
            BOOT_LOG_INF("Press and release user button to initiate Rollback \r\n");
            do
            {
                /* Put MCU in WFI and wait for use events.  */
                __WFI();

                if(is_user_button_pressed == true)
                {
                    /* We don't need this interrupt further, disabling it. */
                    NVIC_DisableIRQ((IRQn_Type)user_btn_isr_cfg.intrSrc);

                    is_user_button_pressed = false;

                    BOOT_LOG_INF("Detected user button event ");
                    BOOT_LOG_INF("Initiating the Rollback...\r\n") ;

                    break;
                }
            } while(true);

            /* this function never returns */
            rollback_to_factory_image();
        }
    }
}

/* [] END OF FILE */
