/******************************************************************************
 * Copyright (C) 2026 EternalChip, Inc.(Gmbh) or its affiliates.
 * All Rights Reserved.
 * @file fake_uart.c
 * @par dependencies
 * - fake_uart.h
 * @author Codex (AI-assisted update for EternalChip)
 * @brief Record host UART requests without communicating with hardware.
 * @version V1.0 2026-10-15 Add UART routing and no-transmission checks.
 * @note 1 tab == 4 spaces!
 *****************************************************************************/
#include "fake_uart.h"

unsigned int g_test_uart_writes = 0u;
plat_uart_id_t g_test_uart_id = PLAT_UART_ID_RESERVED;
const uint8_t *g_test_uart_data = NULL;
uint32_t g_test_uart_length = 0u;
uint32_t g_test_uart_timeout = 0u;
platform_err_t g_test_uart_result = PLATFORM_ERR_OK;

/** @brief Record arguments and return the selected platform result. */
platform_err_t plat_uart_write(plat_uart_id_t uart_id,
                               const uint8_t *p_data,
                               uint32_t len,
                               uint32_t timeout_ms)
{
    ++g_test_uart_writes;
    g_test_uart_id = uart_id;
    g_test_uart_data = p_data;
    g_test_uart_length = len;
    g_test_uart_timeout = timeout_ms;
    return g_test_uart_result;
}
