/******************************************************************************
 * Copyright (C) 2026 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file board_init.c
 *
 * @par dependencies
 * - board_init.h
 * - board_internal.h
 *
 * @author Codex (AI-assisted update for EternalChip)
 *
 * @brief Establish board initial states and maintain startup readiness.
 *
 * Processing flow:
 * 1. Apply backlight off, motor off and display reset released in order.
 * 2. Return the first operation error to the startup caller.
 * 3. Mark readiness after the complete sequence succeeds.
 *
 * @version V1.0 2026-10-11 Move initialization from board_control.c unchanged.
 * @version V1.1 2026-10-14 Name default signal states and boolean readiness.
 * @version V1.2 2026-10-15 Apply motor-off directly through its action signal.
 *
 * @note MCU GPIO/tick setup is a prerequisite supplied by Core/impl_mcu.
 * @note Callers must serialize initialization and semantic control actions.
 * @note Readiness covers local defaults; AHT21 supply is requested by app_boot.
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//

#include "board_init.h"
#include "board_internal.h"

//******************************** Variables ********************************//

static bool_t g_board_initialized = FALSE;

//******************************** Functions ********************************//

/**
 * @brief Apply the supported default states after MCU GPIO setup.
 *
 * Steps:
 * 1. Switch off backlight and motor, then release display reset.
 * 2. Mark initialization complete only after all writes succeed.
 *
 * @return platform_err_t : OK on completion, otherwise the original error.
 * @note Repeated successful initialization performs no further GPIO writes.
 * @note Stop on the first failure; previous writes are not rolled back.
 *       A failed first step can prevent later shutdown commands from running.
 *       This initialization sequence is not a fault-safe shutdown mechanism.
 */
platform_err_t board_init(void)
{
    platform_err_t ret = PLATFORM_ERR_OK;

    if (TRUE == g_board_initialized)
    {
        return PLATFORM_ERR_OK;
    }

    ret = board_power_set_active(BOARD_POWER_DOMAIN_BACKLIGHT,
                                 BOARD_SIGNAL_INACTIVE);
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    ret = board_pin_set_active(BOARD_PIN_ID_MOTOR_EN, BOARD_SIGNAL_INACTIVE);
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    /* Release reset without generating the operational reset pulse. */
    ret = board_pin_set_active(BOARD_PIN_ID_LCD_RESET, BOARD_SIGNAL_INACTIVE);
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    g_board_initialized = TRUE;
    return PLATFORM_ERR_OK;
}

/**
 * @brief Report completion of the supported board default-state sequence.
 * @return bool_t : TRUE when ready; FALSE before startup or after GPIO failure.
 */
bool_t board_is_initialized(void)
{
    return g_board_initialized;
}

/**
 * @brief Require initialization again after a platform GPIO failure.
 * @note The caller handles the original error; no recovery writes run here.
 */
void board_invalidate(void)
{
    g_board_initialized = FALSE;
}
