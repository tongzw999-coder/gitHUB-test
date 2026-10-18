/******************************************************************************
 * Copyright (C) 2026 EternalChip, Inc.(Gmbh) or its affiliates.
 * All Rights Reserved.
 * @file fake_uart.h
 * @par dependencies
 * - plat_uart.h
 * @author Codex (AI-assisted update for EternalChip)
 * @brief Expose host UART event records and an injectable result.
 * @version V1.0 2026-10-15 Add the host UART recorder interface.
 * @note 1 tab == 4 spaces!
 *****************************************************************************/
#ifndef TEST_FAKE_UART_H
#define TEST_FAKE_UART_H

#include "plat_uart.h"

extern unsigned int g_test_uart_writes;
extern plat_uart_id_t g_test_uart_id;
extern const uint8_t *g_test_uart_data;
extern uint32_t g_test_uart_length;
extern uint32_t g_test_uart_timeout;
extern platform_err_t g_test_uart_result;

#endif
