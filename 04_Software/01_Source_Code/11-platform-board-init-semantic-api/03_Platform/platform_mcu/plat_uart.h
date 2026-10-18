/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file plat_uart.h
 *
 * @brief Provide platform UART abstraction APIs.
 *
 *****************************************************************************/

#ifndef __PLAT_UART_H__
#define __PLAT_UART_H__

//******************************** Includes *********************************//

#include "platform_def.h"
#include "platform_error.h"
#include "platform_mcu_config.h"

//******************************** Declaring ********************************//

/*
 * len uses platform width. Each MCU implementation must check its own driver
 * length limit before converting len to the vendor API argument type.
 */
platform_err_t plat_uart_write(plat_uart_id_t uart_id,
                               const uint8_t *p_data,
                               uint32_t len,
                               uint32_t timeout_ms);

/*
 * Partial read is supported.
 * PLATFORM_ERR_OK means *p_actual_len == len.
 * PLATFORM_ERR_TIMEOUT means *p_actual_len is the bytes received before timeout.
 */
platform_err_t plat_uart_read(plat_uart_id_t uart_id,
                              uint8_t *p_data,
                              uint32_t len,
                              uint32_t *p_actual_len,
                              uint32_t timeout_ms);

#endif /* __PLAT_UART_H__ */
