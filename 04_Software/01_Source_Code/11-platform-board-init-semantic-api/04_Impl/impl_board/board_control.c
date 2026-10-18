/******************************************************************************
 * Copyright (C) 2026 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file board_control.c
 *
 * @par dependencies
 * - board_control.h
 * - board_internal.h
 * - board_resource_config.h
 * - plat_gpio.h
 * - plat_tick.h
 * - plat_uart.h
 * - stddef.h
 *
 * @author Codex (AI-assisted update for EternalChip)
 *
 * @brief Translate board actions into GPIO operations and Nordic requests.
 *
 * Processing flow:
 * 1. Resolve board actions through pinmap, powermap and the UART busmap.
 * 2. Translate local signal state to GPIO level; validate the remote bus.
 * 3. Return platform results or the pending Nordic protocol status.
 *
 * @version V1.0 2026-09-13 Initial lesson 11 implementation.
 * @version V1.1 2026-09-13 Apply the course coding and comment conventions.
 * @version V1.2 2026-10-11 Move startup and readiness to board_init.c.
 * @version V1.3 2026-10-14 Replace numeric action arguments with named states.
 * @version V1.4 2026-10-15 Separate motor action and AHT21 UART power control.
 *
 * @note MCU setup stays in Core/impl_mcu; no physical pins are exposed here.
 * @note Callers must serialize init and actions, including the full reset.
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//

#include "board_control.h"
#include "board_internal.h"
#include "board_resource_config.h"
#include "plat_gpio.h"
#include "plat_tick.h"
#include "plat_uart.h"

#include <stddef.h>

//******************************** Defines **********************************//

/* Teaching values: verify both intervals against the actual display manual. */
#define BOARD_DISPLAY_RESET_ASSERT_MS  (10u)
#define BOARD_DISPLAY_RESET_RELEASE_MS (10u)

//******************************** Declaring ********************************//

static platform_err_t board_uart_get_id(board_bus_id_t bus_id,
                                        plat_uart_id_t *p_uart_id);

//******************************** Functions ********************************//

/**
 * @brief Switch on the GPIO backlight through the configured power domain.
 * @return platform_err_t : OK or the original mapping/GPIO error.
 * @retval PLATFORM_ERR_NOT_INITIALIZED Initialization has not completed.
 * @note Binary control only; no PWM brightness adjustment is performed.
 */
platform_err_t board_backlight_on(void)
{
    platform_err_t ret = PLATFORM_ERR_OK;

    if (FALSE == board_is_initialized())
    {
        return PLATFORM_ERR_NOT_INITIALIZED;
    }

    ret = board_power_set_active(BOARD_POWER_DOMAIN_BACKLIGHT,
                                 BOARD_SIGNAL_ACTIVE);
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    return PLATFORM_ERR_OK;
}

/**
 * @brief Switch off the GPIO backlight through the configured power domain.
 * @return platform_err_t : OK or the original mapping/GPIO error.
 * @retval PLATFORM_ERR_NOT_INITIALIZED Initialization has not completed.
 * @note After a GPIO failure, even off commands require reinitialization.
 */
platform_err_t board_backlight_off(void)
{
    platform_err_t ret = PLATFORM_ERR_OK;

    if (FALSE == board_is_initialized())
    {
        return PLATFORM_ERR_NOT_INITIALIZED;
    }

    ret = board_power_set_active(BOARD_POWER_DOMAIN_BACKLIGHT,
                                 BOARD_SIGNAL_INACTIVE);
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    return PLATFORM_ERR_OK;
}

/**
 * @brief Assert display reset, hold it, release it, and wait for recovery.
 *
 * Steps:
 * 1. Convert active reset through pinmap and wait for the hold interval.
 * 2. Release reset through pinmap and wait for the recovery interval.
 *
 * @return platform_err_t : OK after both writes and delays, or a GPIO error.
 * @retval PLATFORM_ERR_NOT_INITIALIZED Initialization has not completed.
 * @note Blocking task-context operation; the tick must run. Never call in ISR.
 * @note A failed release can leave reset asserted. No recovery is attempted.
 *       The two 10ms settings still require validation for the actual display.
 */
platform_err_t board_display_reset(void)
{
    platform_err_t ret = PLATFORM_ERR_OK;

    if (FALSE == board_is_initialized())
    {
        return PLATFORM_ERR_NOT_INITIALIZED;
    }

    ret = board_pin_set_active(BOARD_PIN_ID_LCD_RESET, BOARD_SIGNAL_ACTIVE);
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    plat_delay_ms(BOARD_DISPLAY_RESET_ASSERT_MS);

    ret = board_pin_set_active(BOARD_PIN_ID_LCD_RESET, BOARD_SIGNAL_INACTIVE);
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    plat_delay_ms(BOARD_DISPLAY_RESET_RELEASE_MS);

    return PLATFORM_ERR_OK;
}

/**
 * @brief Enable or disable motor action through its configured GPIO signal.
 * @param[in] state : BOARD_SIGNAL_ACTIVE enables the motor;
 *                   BOARD_SIGNAL_INACTIVE disables it.
 * @return platform_err_t : OK or the original mapping/GPIO error.
 * @retval PLATFORM_ERR_NOT_INITIALIZED Takes precedence over parameter checks.
 * @retval PLATFORM_ERR_PARAM The state is neither ACTIVE nor INACTIVE.
 * @note This command has no vibration pattern or physical feedback.
 */
platform_err_t board_motor_enable(board_signal_state_t state)
{
    platform_err_t ret = PLATFORM_ERR_OK;

    if (FALSE == board_is_initialized())
    {
        return PLATFORM_ERR_NOT_INITIALIZED;
    }

    if ((BOARD_SIGNAL_INACTIVE != state) && (BOARD_SIGNAL_ACTIVE != state))
    {
        return PLATFORM_ERR_PARAM;
    }

    ret = board_pin_set_active(BOARD_PIN_ID_MOTOR_EN, state);
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    return PLATFORM_ERR_OK;
}

/**
 * @brief Request AHT21 supply through its configured Nordic UART control bus.
 * @param[in] state : ACTIVE requests power on; INACTIVE requests power off.
 * @return platform_err_t : Validation error or NOT_SUPPORTED while protocol TBD.
 * @note No GPIO or UART write is issued by the current pending request.
 */
platform_err_t board_aht21_power_enable(board_signal_state_t state)
{
    if (FALSE == board_is_initialized())
    {
        return PLATFORM_ERR_NOT_INITIALIZED;
    }

    return board_power_set_active(BOARD_POWER_DOMAIN_AHT21, state);
}

/**
 * @brief Resolve local backlight control or a remote AHT21 supply request.
 * @param[in] domain : BACKLIGHT or AHT21 domain in the board powermap.
 * @param[in] state : BOARD_SIGNAL_ACTIVE asserts the enable signal;
 *                   BOARD_SIGNAL_INACTIVE releases it.
 * @return platform_err_t : GPIO result, validation error or protocol status.
 * @retval PLATFORM_ERR_PARAM Invalid argument or inconsistent table entry.
 * @retval PLATFORM_ERR_NOT_SUPPORTED The domain has no action in this lesson.
 * @note Local polarity is resolved in pinmap. Nordic owns the remote EN level.
 *       The AHT21 branch remains pending until its command protocol is defined.
 */
platform_err_t board_power_set_active(board_power_domain_t domain,
                                      board_signal_state_t state)
{
    const board_power_desc_t *p_power = NULL;
    plat_uart_id_t uart_id = PLAT_UART_ID_RESERVED;
    platform_err_t ret = PLATFORM_ERR_OK;

    if (((BOARD_SIGNAL_INACTIVE != state) && (BOARD_SIGNAL_ACTIVE != state)) ||
        ((uint32_t)domain >= (uint32_t)BOARD_POWER_DOMAIN_COUNT))
    {
        return PLATFORM_ERR_PARAM;
    }

    if ((BOARD_POWER_DOMAIN_BACKLIGHT != domain) &&
        (BOARD_POWER_DOMAIN_AHT21 != domain))
    {
        return PLATFORM_ERR_NOT_SUPPORTED;
    }

    p_power = &g_board_powermap[domain];
    if (NULL == p_power)
    {
        return PLATFORM_ERR_PARAM;
    }

    if (p_power->domain != domain)
    {
        return PLATFORM_ERR_PARAM;
    }

    if (BOARD_POWER_DOMAIN_AHT21 == domain)
    {
        if (BOARD_PIN_ID_INVALID != p_power->enable_pin)
        {
            return PLATFORM_ERR_PARAM;
        }

        ret = board_uart_get_id(p_power->control_bus, &uart_id);
        if (PLATFORM_ERR_OK != ret)
        {
            return ret;
        }

        (void)uart_id;
        /* TBD(Nordic): agree command encoding for AHT21 ACTIVE/INACTIVE.
         * Send the encoded frame with:
         * board_uart_write(p_power->control_bus, p_frame, frame_len, timeout_ms).
         * Define acknowledgement/error handling before reporting completion.
         * No placeholder bytes are transmitted while the protocol is pending.
         */
        return PLATFORM_ERR_NOT_SUPPORTED;
    }

    ret = board_pin_set_active(p_power->enable_pin, state);
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    return PLATFORM_ERR_OK;
}

/**
 * @brief Translate a logical active state into a validated platform GPIO write.
 * @param[in] pin_id : Board pin identifier used to index pinmap.
 * @param[in] state : BOARD_SIGNAL_ACTIVE asserts the signal;
 *                   BOARD_SIGNAL_INACTIVE releases it.
 * @return platform_err_t : OK or the original platform GPIO error.
 * @retval PLATFORM_ERR_PARAM Invalid argument, entry, channel or polarity.
 * @retval PLATFORM_ERR_NOT_SUPPORTED Reserved channel or non-GPIO signal.
 * @note GPIO and POWER signal types support binary control in this lesson.
 *       GPIO failures clear readiness; parameter errors do not change it.
 */
platform_err_t board_pin_set_active(board_pin_id_t pin_id,
                                    board_signal_state_t state)
{
    const board_pin_desc_t *p_pin = NULL;
    plat_gpio_level_t level = PLAT_GPIO_LEVEL_LOW;
    platform_err_t ret = PLATFORM_ERR_OK;

    /* The unsigned comparison also rejects negative values cast to the enum. */
    if (((BOARD_SIGNAL_INACTIVE != state) && (BOARD_SIGNAL_ACTIVE != state)) ||
        ((uint32_t)pin_id >= (uint32_t)BOARD_PIN_ID_COUNT))
    {
        return PLATFORM_ERR_PARAM;
    }

    p_pin = &g_board_pinmap[pin_id];
    if (NULL == p_pin)
    {
        return PLATFORM_ERR_PARAM;
    }

    if (p_pin->pin_id != pin_id)
    {
        return PLATFORM_ERR_PARAM;
    }

    if (PLAT_GPIO_ID_RESERVED == p_pin->gpio_id)
    {
        return PLATFORM_ERR_NOT_SUPPORTED;
    }

    if ((uint32_t)p_pin->gpio_id >= (uint32_t)PLAT_GPIO_NUM)
    {
        return PLATFORM_ERR_PARAM;
    }

    if ((BOARD_SIGNAL_GPIO != p_pin->signal_type) &&
        (BOARD_SIGNAL_POWER != p_pin->signal_type))
    {
        return PLATFORM_ERR_NOT_SUPPORTED;
    }

    if ((BOARD_ACTIVE_LEVEL_LOW != p_pin->active_level) &&
        (BOARD_ACTIVE_LEVEL_HIGH != p_pin->active_level))
    {
        return PLATFORM_ERR_PARAM;
    }

    if (BOARD_ACTIVE_LEVEL_HIGH == p_pin->active_level)
    {
        level = (BOARD_SIGNAL_ACTIVE == state) ?
                PLAT_GPIO_LEVEL_HIGH : PLAT_GPIO_LEVEL_LOW;
    }
    else
    {
        level = (BOARD_SIGNAL_ACTIVE == state) ?
                PLAT_GPIO_LEVEL_LOW : PLAT_GPIO_LEVEL_HIGH;
    }

    ret = plat_gpio_write(p_pin->gpio_id, level);
    if (PLATFORM_ERR_OK != ret)
    {
        /* Preserve the original error and require explicit reinitialization. */
        board_invalidate();
    }

    return ret;
}

/**
 * @brief Resolve a board UART bus to a valid platform channel.
 * @param[in] bus_id : Board bus selected by resource configuration.
 * @param[out] p_uart_id : Receives the validated platform UART identifier.
 * @return platform_err_t : OK or PARAM for an invalid or inconsistent mapping.
 */
static platform_err_t board_uart_get_id(board_bus_id_t bus_id,
                                        plat_uart_id_t *p_uart_id)
{
    const board_bus_desc_t *p_bus = NULL;

    if ((NULL == p_uart_id) ||
        ((uint32_t)bus_id >= (uint32_t)BOARD_BUS_ID_COUNT))
    {
        return PLATFORM_ERR_PARAM;
    }

    p_bus = &g_board_busmap[bus_id];
    if ((p_bus->bus_id != bus_id) ||
        (BOARD_BUS_TYPE_UART != p_bus->bus_type) ||
        ((uint32_t)p_bus->platform_instance_id >= (uint32_t)PLAT_UART_NUM))
    {
        return PLATFORM_ERR_PARAM;
    }

    *p_uart_id = (plat_uart_id_t)p_bus->platform_instance_id;
    return PLATFORM_ERR_OK;
}

/**
 * @brief Transmit an encoded frame through a configured board UART bus.
 * @param[in] bus_id : Board UART bus identifier.
 * @param[in] p_data : Encoded frame bytes supplied by the protocol caller.
 * @param[in] len : Nonzero byte count.
 * @param[in] timeout_ms : Platform transmit timeout in milliseconds.
 * @return platform_err_t : Validation error or the original UART result.
 * @note This helper does not construct Nordic commands or wait for an ACK.
 */
platform_err_t board_uart_write(board_bus_id_t bus_id,
                                const uint8_t *p_data,
                                uint32_t len,
                                uint32_t timeout_ms)
{
    plat_uart_id_t uart_id = PLAT_UART_ID_RESERVED;
    platform_err_t ret = PLATFORM_ERR_OK;

    if ((NULL == p_data) || (0u == len))
    {
        return PLATFORM_ERR_PARAM;
    }

    ret = board_uart_get_id(bus_id, &uart_id);
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    return plat_uart_write(uart_id, p_data, len, timeout_ms);
}
