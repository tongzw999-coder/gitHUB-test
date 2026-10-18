/******************************************************************************
 * Copyright (C) 2026 EternalChip, Inc.(Gmbh) or its affiliates.
 * All Rights Reserved.
 * @file test_aht21_power.c
 * @par dependencies
 * - board_init.h
 * - board_internal.h
 * - fake_uart.h
 * - plat_gpio.h
 * - assert.h
 * - stdio.h
 * @author Codex (AI-assisted update for EternalChip)
 * @brief Verify the real AHT power mapping and protocol-pending behavior.
 * @version V1.0 2026-10-15 Add host-only UART and AHT tests.
 * @note Transport test bytes have no Nordic protocol meaning.
 * @note 1 tab == 4 spaces!
 *****************************************************************************/
#include "board_init.h"
#include "board_internal.h"
#include "fake_uart.h"
#include "plat_gpio.h"
#include <assert.h>
#include <stdio.h>

static unsigned int g_gpio_writes = 0u;

/** @brief Count writes without changing hardware. */
platform_err_t plat_gpio_write(plat_gpio_id_t id, plat_gpio_level_t level)
{
    (void)id;
    (void)level;
    ++g_gpio_writes;
    return PLATFORM_ERR_OK;
}

/** @brief Reject unexpected reset delays. */
void plat_delay_ms(uint32_t ms)
{
    (void)ms;
    assert(0);
}

/** @brief Check mapping, states, no-TX stub and real transport delegation. */
int main(void)
{
    const uint8_t test_bytes[] = {'T', 'E', 'S', 'T'};
    const board_signal_state_t invalid_states[] = {
        (board_signal_state_t)-1, (board_signal_state_t)2,
        (board_signal_state_t)256, BOARD_SIGNAL_STATE_RESERVED
    };
    const platform_err_t errors[] = {
        PLATFORM_ERR_TIMEOUT, PLATFORM_ERR_BUSY, PLATFORM_ERR_FAIL
    };
    unsigned int index = 0u;

    assert(PLATFORM_ERR_NOT_INITIALIZED ==
           board_aht21_power_enable(BOARD_SIGNAL_ACTIVE));
    assert(PLATFORM_ERR_OK == board_init());
    assert(3u == g_gpio_writes);
    assert(BOARD_SIGNAL_GPIO == g_board_pinmap[BOARD_PIN_ID_MOTOR_EN].signal_type);
    assert(BOARD_PIN_ID_INVALID == g_board_powermap[BOARD_POWER_DOMAIN_AHT21].enable_pin);
    assert(BOARD_BUS_ID_NORDIC_UART == g_board_powermap[BOARD_POWER_DOMAIN_AHT21].control_bus);
    assert(BOARD_POWER_STATE_ON == g_board_powermap[BOARD_POWER_DOMAIN_AHT21].default_state);
    assert(BOARD_BUS_ID_COMMON_I2C == g_board_bus_device_map[BOARD_BUS_DEVICE_AHT21].bus_id);
    assert(PLATFORM_ERR_NOT_SUPPORTED == board_aht21_power_enable(BOARD_SIGNAL_ACTIVE));
    assert(PLATFORM_ERR_NOT_SUPPORTED == board_aht21_power_enable(BOARD_SIGNAL_INACTIVE));
    for (index = 0u; index < sizeof(invalid_states) / sizeof(invalid_states[0]); ++index)
    {
        assert(PLATFORM_ERR_PARAM == board_aht21_power_enable(invalid_states[index]));
    }
    assert(3u == g_gpio_writes);
    assert(0u == g_test_uart_writes);
    assert(TRUE == board_is_initialized());

    assert(PLATFORM_ERR_PARAM == board_uart_write(BOARD_BUS_ID_INVALID, test_bytes, sizeof(test_bytes), 25u));
    assert(PLATFORM_ERR_PARAM == board_uart_write((board_bus_id_t)-1, test_bytes, sizeof(test_bytes), 25u));
    assert(PLATFORM_ERR_PARAM == board_uart_write(BOARD_BUS_ID_NORDIC_UART, NULL, sizeof(test_bytes), 25u));
    assert(PLATFORM_ERR_PARAM == board_uart_write(BOARD_BUS_ID_NORDIC_UART, test_bytes, 0u, 25u));
    assert(PLATFORM_ERR_PARAM == board_uart_write(BOARD_BUS_ID_COMMON_I2C, test_bytes, sizeof(test_bytes), 25u));
    assert(0u == g_test_uart_writes);
    assert(PLATFORM_ERR_OK == board_uart_write(BOARD_BUS_ID_NORDIC_UART, test_bytes, sizeof(test_bytes), 25u));
    assert(PLAT_UART_CH_1 == g_test_uart_id);
    assert(test_bytes == g_test_uart_data);
    assert(sizeof(test_bytes) == g_test_uart_length);
    assert(25u == g_test_uart_timeout);
    for (index = 0u; index < sizeof(errors) / sizeof(errors[0]); ++index)
    {
        g_test_uart_result = errors[index];
        assert(errors[index] == board_uart_write(BOARD_BUS_ID_NORDIC_UART, test_bytes, sizeof(test_bytes), 25u));
    }
    assert(4u == g_test_uart_writes);
    assert(3u == g_gpio_writes);
    puts("PASS AHT21 mapping, named states, zero-TX TBD and UART routing/errors");
    return 0;
}
