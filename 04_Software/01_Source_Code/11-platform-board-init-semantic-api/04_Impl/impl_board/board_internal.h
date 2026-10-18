/******************************************************************************
 * Copyright (C) 2026 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file board_internal.h
 *
 * @par dependencies
 * - board_control.h
 * - board_resource_config.h
 * - platform_error.h
 * - platform_type.h
 *
 * @author Codex (AI-assisted update for EternalChip)
 *
 * @brief Share helpers between the board startup and control implementations.
 *
 * Processing flow:
 * 1. Startup applies defaults using the same conversion as later actions.
 * 2. Control checks startup readiness before public operations.
 * 3. A platform GPIO failure invalidates readiness for the next operation.
 *
 * @version V1.0 2026-10-11 Add private declarations for the source-file split.
 * @version V1.1 2026-10-14 Use named signal states and boolean readiness.
 * @version V1.2 2026-10-15 Add the configured UART transport for Nordic control.
 *
 * @note Internal to impl_board; App/Service uses the public board headers.
 * @note Calls are serialized by the caller; helpers provide no internal lock.
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#ifndef BOARD_INTERNAL_H
#define BOARD_INTERNAL_H

//******************************** Includes *********************************//

#include "board_control.h"
#include "board_resource_config.h"
#include "platform_error.h"
#include "platform_type.h"

//******************************** Declaring ********************************//

/**
 * @brief Query completion of the board initial-state sequence.
 * @return bool_t : TRUE after successful initialization; FALSE otherwise.
 */
bool_t board_is_initialized(void);

/**
 * @brief Clear readiness after a platform GPIO failure.
 * @note Performs no hardware writes; a later board_init() can retry defaults.
 */
void board_invalidate(void);

/**
 * @brief Convert a board pin's active state using pinmap, then write GPIO.
 * @param[in] pin_id : Board signal identifier.
 * @param[in] state : BOARD_SIGNAL_ACTIVE asserts the signal;
 *                   BOARD_SIGNAL_INACTIVE releases it.
 * @return platform_err_t : Mapping validation result or platform write result.
 * @note Startup also uses this helper before readiness has been established.
 */
platform_err_t board_pin_set_active(board_pin_id_t pin_id,
                                    board_signal_state_t state);

/**
 * @brief Resolve local backlight control or the pending Nordic AHT21 request.
 * @param[in] domain : BACKLIGHT or AHT21 domain.
 * @param[in] state : BOARD_SIGNAL_ACTIVE asserts the enable signal;
 *                   BOARD_SIGNAL_INACTIVE releases it.
 * @return platform_err_t : Mapping/GPIO result or NOT_SUPPORTED for AHT21 TBD.
 * @note This helper does not check readiness; public control APIs do.
 */
platform_err_t board_power_set_active(board_power_domain_t domain,
                                      board_signal_state_t state);

/**
 * @brief Write a caller-supplied frame through a configured board UART bus.
 * @param[in] bus_id : Board UART bus identifier.
 * @param[in] p_data : Encoded frame buffer, valid for the complete call.
 * @param[in] len : Nonzero frame length in bytes.
 * @param[in] timeout_ms : Platform transmit timeout in milliseconds.
 * @return platform_err_t : Mapping/parameter error or platform write result.
 * @note MCU UART initialization and bus serialization belong to the caller.
 *       This transport does not encode Nordic commands or validate responses.
 */
platform_err_t board_uart_write(board_bus_id_t bus_id,
                                const uint8_t *p_data,
                                uint32_t len,
                                uint32_t timeout_ms);

#endif /* BOARD_INTERNAL_H */
