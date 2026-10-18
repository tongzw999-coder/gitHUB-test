/******************************************************************************
 * Copyright (C) 2026 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file stm32f4xx_hal.h
 *
 * @par dependencies
 * - platform_type.h
 *
 * @author Codex (AI-assisted update for EternalChip)
 *
 * @brief Provide the minimal HAL GPIO contract for host MCU tests.
 *
 * Processing flow:
 * 1. Define host GPIO types and pin constants used by real main.h.
 * 2. Bind GPIO port macros to the host test objects.
 * 3. Declare the HAL functions implemented by the host test.
 *
 * @version V1.1 2026-09-13
 *
 * @note 1 tab == 4 spaces!
 *****************************************************************************/

#ifndef HOST_STM32F4XX_HAL_H
#define HOST_STM32F4XX_HAL_H

//******************************** Includes *********************************//

#include "platform_type.h"

//******************************** Defines **********************************//

typedef struct
{
    unsigned int identity;
} GPIO_TypeDef;

typedef enum
{
    GPIO_PIN_RESET = 0,
    GPIO_PIN_SET
} GPIO_PinState;

#define GPIOA (&g_host_gpio_a)
#define GPIOB (&g_host_gpio_b)
#define GPIOC (&g_host_gpio_c)

#define GPIO_PIN_0  ((uint16_t)0x0001u)
#define GPIO_PIN_1  ((uint16_t)0x0002u)
#define GPIO_PIN_3  ((uint16_t)0x0008u)
#define GPIO_PIN_10 ((uint16_t)0x0400u)
#define GPIO_PIN_13 ((uint16_t)0x2000u)

//******************************** Variables ********************************//

extern GPIO_TypeDef g_host_gpio_a;
extern GPIO_TypeDef g_host_gpio_b;
extern GPIO_TypeDef g_host_gpio_c;

//******************************** Declaring ********************************//

/**
 * @brief Record a HAL GPIO write in the host test.
 *
 * @param[in] p_port GPIO port selected by the real MCU implementation.
 * @param[in] pin GPIO pin mask.
 * @param[in] level Requested electrical level.
 *
 * @return None.
 */
void HAL_GPIO_WritePin(GPIO_TypeDef *p_port, uint16_t pin, GPIO_PinState level);

/**
 * @brief Record a HAL GPIO read and supply the host test input level.
 *
 * @param[in] p_port GPIO port selected by the real MCU implementation.
 * @param[in] pin GPIO pin mask.
 *
 * @return GPIO_PIN_SET for the host test's fixed high input.
 */
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *p_port, uint16_t pin);

#endif /* HOST_STM32F4XX_HAL_H */
