/******************************************************************************
 * Copyright (C) 2026 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file board_control.h
 *
 * @par dependencies
 * - platform_error.h
 *
 * @author Codex (AI-assisted update for EternalChip)
 *
 * @brief Declare board action APIs and the pending AHT21 supply request.
 *
 * Processing flow:
 * 1. Complete MCU GPIO/tick setup and call board_init().
 * 2. Request a backlight, display reset, or motor action.
 * 3. Check the returned status before continuing the caller's operation.
 *
 * @version V1.0 2026-09-13 Initial lesson 11 contract.
 * @version V1.1 2026-09-13 Apply the course coding and comment conventions.
 * @version V1.2 2026-10-11 Separate the startup contract into board_init.h.
 * @version V1.3 2026-10-14 Name the logical signal states in control APIs.
 * @version V1.4 2026-10-15 Add an AHT21 supply request with Nordic protocol TBD.
 *
 * @note No physical pins or mapping tables are exposed by this public header.
 * @note Callers must serialize all calls; this module has no internal lock.
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#ifndef BOARD_CONTROL_H
#define BOARD_CONTROL_H

//******************************** Includes *********************************//

#include "platform_error.h"

//******************************** Declaring ********************************//

/**
 * @brief Requested logical signal state, independent of electrical polarity.
 * @note Pinmap active_level determines the GPIO level for each valid state.
 */
typedef enum
{
    BOARD_SIGNAL_INACTIVE = 0,
    BOARD_SIGNAL_ACTIVE,
    BOARD_SIGNAL_STATE_RESERVED = 0x7FFFFFFF
} board_signal_state_t;

/**
 * @brief Switch the GPIO backlight fully on without PWM dimming.
 * @return platform_err_t : OK or the original mapping/GPIO error.
 * @retval PLATFORM_ERR_NOT_INITIALIZED Initialization has not completed.
 * @note GPIO failure clears readiness and requires explicit reinitialization.
 */
platform_err_t board_backlight_on(void);

/**
 * @brief Switch the GPIO backlight fully off.
 * @return platform_err_t : OK or the original mapping/GPIO error.
 * @retval PLATFORM_ERR_NOT_INITIALIZED Initialization has not completed.
 * @note After a GPIO failure, off commands also require reinitialization.
 */
platform_err_t board_backlight_off(void);

/**
 * @brief Assert reset, wait 10ms, release reset, then wait another 10ms.
 * @return platform_err_t : OK after completion, otherwise the original error.
 * @retval PLATFORM_ERR_NOT_INITIALIZED Initialization has not completed.
 * @note Blocking task-context API with a live tick; never call from an ISR.
 *       The 10ms settings require validation against the actual display manual.
 *       GPIO failure stops the sequence and clears readiness. Reset may remain
 *       asserted, so the caller must handle the failure and reinitialize.
 */
platform_err_t board_display_reset(void);

/**
 * @brief Enable or disable the motor without adding a vibration pattern.
 * @param[in] state : BOARD_SIGNAL_ACTIVE enables the motor;
 *                   BOARD_SIGNAL_INACTIVE disables it.
 * @return platform_err_t : OK or the original mapping/GPIO error.
 * @retval PLATFORM_ERR_NOT_INITIALIZED Takes precedence over parameter checks.
 * @retval PLATFORM_ERR_PARAM The state is neither ACTIVE nor INACTIVE.
 * @note Invalid parameters perform no writes and do not change readiness.
 *       GPIO errors clear readiness. Success is a command result, not feedback
 *       proving that the physical actuator operated.
 */
platform_err_t board_motor_enable(board_signal_state_t state);

/**
 * @brief Request AHT21 supply enable or disable through the Nordic link.
 * @param[in] state : BOARD_SIGNAL_ACTIVE requests supply on;
 *                   BOARD_SIGNAL_INACTIVE requests supply off.
 * @return platform_err_t : Mapping error or protocol integration status.
 * @retval PLATFORM_ERR_NOT_INITIALIZED Local board defaults are not ready.
 * @retval PLATFORM_ERR_PARAM Invalid state or inconsistent resource mapping.
 * @retval PLATFORM_ERR_NOT_SUPPORTED Nordic command protocol is TBD.
 * @note The current implementation sends no bytes and confirms no supply state.
 *       AHT21 measurement remains on its separate I2C bus.
 */
platform_err_t board_aht21_power_enable(board_signal_state_t state);

#endif /* BOARD_CONTROL_H */
