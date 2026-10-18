/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file stm32f411_plat_uart.c
 *
 * @par dependencies
 * - plat_uart.h
 * - usart.h
 *
 * @author Codex (AI-assisted update for EternalChip)
 *
 * @brief Implement platform UART APIs on STM32F411 HAL.
 *
 * Processing flow:
 * 1. Resolve a platform UART channel to the configured HAL handle.
 * 2. Validate transfer arguments and translate the HAL status.
 * 3. Preserve partial receive counts when a read times out.
 *
 * @version V1.0 Baseline implementation; date not recorded.
 * @version V1.1 2026-10-15 Bind generic UART channel 1 to the existing handle.
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//

#include "plat_uart.h"

#include "usart.h"

//******************************** Declaring ********************************//

#define STM32F411_UART_MAX_TRANSFER_SIZE    (0xFFFFu)

static UART_HandleTypeDef *stm32f411_uart_get_handle(plat_uart_id_t uart_id);
static uint32_t stm32f411_uart_get_remaining_timeout(uint32_t start_ms,
                                                     uint32_t timeout_ms);
static platform_err_t stm32f411_uart_status_to_platform(HAL_StatusTypeDef status);

//******************************** Functions ********************************//

platform_err_t plat_uart_write(plat_uart_id_t uart_id,
                               const uint8_t *p_data,
                               uint32_t len,
                               uint32_t timeout_ms)
{
    UART_HandleTypeDef *p_uart = stm32f411_uart_get_handle(uart_id);
    HAL_StatusTypeDef hal_ret = HAL_OK;

    /*
     * STM32 HAL UART uses uint16_t Size. Check before the explicit cast to
     * avoid truncating the platform uint32_t length.
     */
    if ((NULL == p_uart) ||
        (NULL == p_data) ||
        (0u == len) ||
        (STM32F411_UART_MAX_TRANSFER_SIZE < len))
    {
        return PLATFORM_ERR_PARAM;
    }

    hal_ret = HAL_UART_Transmit(p_uart,
                                (uint8_t *)p_data,
                                (uint16_t)len,
                                timeout_ms);

    return stm32f411_uart_status_to_platform(hal_ret);
}

platform_err_t plat_uart_read(plat_uart_id_t uart_id,
                              uint8_t *p_data,
                              uint32_t len,
                              uint32_t *p_actual_len,
                              uint32_t timeout_ms)
{
    UART_HandleTypeDef *p_uart = stm32f411_uart_get_handle(uart_id);
    uint32_t start_ms = HAL_GetTick();
    uint32_t actual_len = 0u;
    HAL_StatusTypeDef hal_ret = HAL_OK;

    if ((NULL == p_uart) || (NULL == p_data) || (NULL == p_actual_len) || (0u == len))
    {
        return PLATFORM_ERR_PARAM;
    }

    *p_actual_len = 0u;

    /*
     * This interface keeps partially received data visible to the caller:
     * timeout returns PLATFORM_ERR_TIMEOUT and p_actual_len reports bytes read.
     */
    while (actual_len < len)
    {
        hal_ret = HAL_UART_Receive(p_uart,
                                   &p_data[actual_len],
                                   1u,
                                   stm32f411_uart_get_remaining_timeout(start_ms, timeout_ms));
        if (HAL_OK != hal_ret)
        {
            *p_actual_len = actual_len;
            return stm32f411_uart_status_to_platform(hal_ret);
        }

        actual_len++;
    }

    *p_actual_len = actual_len;

    return PLATFORM_ERR_OK;
}

static UART_HandleTypeDef *stm32f411_uart_get_handle(plat_uart_id_t uart_id)
{
    if (PLAT_UART_CH_1 == uart_id)
    {
        return &huart1;
    }

    return NULL;
}

static uint32_t stm32f411_uart_get_remaining_timeout(uint32_t start_ms,
                                                     uint32_t timeout_ms)
{
    uint32_t elapsed_ms = 0u;

    if (HAL_MAX_DELAY == timeout_ms)
    {
        return HAL_MAX_DELAY;
    }

    elapsed_ms = HAL_GetTick() - start_ms;
    if (elapsed_ms >= timeout_ms)
    {
        return 0u;
    }

    return timeout_ms - elapsed_ms;
}

static platform_err_t stm32f411_uart_status_to_platform(HAL_StatusTypeDef status)
{
    switch (status)
    {
    case HAL_OK:
        return PLATFORM_ERR_OK;

    case HAL_BUSY:
        return PLATFORM_ERR_BUSY;

    case HAL_TIMEOUT:
        return PLATFORM_ERR_TIMEOUT;

    default:
        break;
    }

    return PLATFORM_ERR_FAIL;
}
