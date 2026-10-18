/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file platform_mcu_config.h
 *
 * @par dependencies
 * - None (no direct includes).
 *
 * @author Codex (AI-assisted update for EternalChip)
 *
 * @brief Provide platform MCU resource configuration.
 *
 * Processing flow:
 *
 * 1. Defines UART, GPIO, and IRQ resource identifiers in their existing order.
 * 2. Retains reserved identifiers for unsupported resources.
 * 3. Provides the configured interrupt priorities.
 *
 * @version V1.0 Baseline implementation; date not recorded.
 * @version V1.1 2026-09-13 Coding-standard documentation and formatting.
 * @version V1.2 2026-10-15 Name the UART channel independently of board roles.
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#ifndef PLATFORM_MCU_CONFIG_H
#define PLATFORM_MCU_CONFIG_H

//******************************** Declaring ********************************//

typedef enum
{
    PLAT_UART_CH_1 = 0,
    PLAT_UART_NUM,
    PLAT_UART_ID_RESERVED = 0x7FFFFFFF
} plat_uart_id_t;

typedef enum
{
    /* Generic GPIO channels provided by the MCU platform. */
    PLAT_GPIO_CH_1 = 0,
    PLAT_GPIO_CH_2,
    PLAT_GPIO_CH_3,
    PLAT_GPIO_CH_4,
    PLAT_GPIO_CH_5,
    PLAT_GPIO_CH_6,
    PLAT_GPIO_NUM,
    PLAT_GPIO_ID_RESERVED = 0x7FFFFFFF
} plat_gpio_id_t;

typedef enum
{
    PLAT_IRQ_ID_PA0_USER_KEY3_ISR = 0,
    PLAT_IRQ_ID_TIMER1_ISR,
    PLAT_IRQ_ID_USART1,
    PLAT_IRQ_ID_ADC_CONVERSION,
    PLAT_IRQ_ID_DMA_UART_DEBUG_RX,
    PLAT_IRQ_NUM,
    PLAT_IRQ_ID_RESERVED = 0x7FFFFFFF
} plat_irq_id_t;

//******************************** Defines **********************************//

#define PLAT_IRQ_PRIORITY_PA0_USER_KEY3_ISR      (5u)
#define PLAT_IRQ_PRIORITY_TIMER1_ISR             (1u)
#define PLAT_IRQ_PRIORITY_USART1                 (2u)
#define PLAT_IRQ_PRIORITY_ADC_CONVERSION         (3u)
#define PLAT_IRQ_PRIORITY_DMA_UART_DEBUG_RX      (5u)

#endif /* PLATFORM_MCU_CONFIG_H */
