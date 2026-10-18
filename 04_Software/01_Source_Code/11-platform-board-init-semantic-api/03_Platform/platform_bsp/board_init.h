/******************************************************************************
 * Copyright (C) 2026 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file board_init.h
 *
 * @par dependencies
 * - platform_error.h
 *
 * @author Codex (AI-assisted update for EternalChip)
 *
 * @brief Declare the board startup and initial-state contract.
 *
 * Processing flow:
 * 1. Complete MCU GPIO and tick setup.
 * 2. Call board_init() and check its return value.
 * 3. Start services after the configured board defaults have been applied.
 *
 * @version V1.0 2026-10-11 Separate board startup from semantic control.
 * @version V1.1 2026-10-15 Clarify local defaults versus remote AHT21 supply.
 *
 * @note Callers must serialize initialization and board control operations.
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#ifndef BOARD_INIT_H
#define BOARD_INIT_H

//******************************** Includes *********************************//

#include "platform_error.h"

//******************************** Declaring ********************************//

/**
 * @brief Switch off backlight and motor, then release display reset.
 * @return platform_err_t : OK after all writes, otherwise the original error.
 * @note Call after MCU GPIO/tick setup. Successful repeated calls do no writes.
 *       Failure stops the sequence and does not roll back earlier writes.
 *       A retry starts all defaults again. This is not fault-safe shutdown.
 *       Core startup levels before this function executes remain unchanged.
 *       Success describes local GPIO defaults. app_boot requests AHT21 supply
 *       separately and reports the pending Nordic protocol integration.
 */
platform_err_t board_init(void);

#endif /* BOARD_INIT_H */
