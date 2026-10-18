/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file stm32f411_plat_gpio.c
 *
 * @par dependencies
 * - plat_gpio.h
 * - main.h
 * - stm32f4xx_hal.h
 *
 * @author Codex (AI-assisted update for EternalChip)
 *
 * @brief Implement platform GPIO APIs on STM32F411 HAL.
 *
 * Processing flow:
 *
 * 1. Resolves a platform GPIO identifier to its private MCU descriptor.
 * 2. Validates the descriptor and requested read or write capability.
 * 3. Converts logical levels and delegates to the existing HAL GPIO API.
 *
 * @version V1.0 Baseline implementation; date not recorded.
 * @version V1.1 2026-09-13 Coding-standard documentation and formatting.
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//

#include "plat_gpio.h"

#include "main.h"
#include "stm32f4xx_hal.h"

//******************************** Declaring ********************************//

typedef enum
{
    STM32F411_GPIO_CAP_READ  = (1u << 0),
    STM32F411_GPIO_CAP_WRITE = (1u << 1)
} stm32f411_gpio_cap_t;

typedef struct
{
    GPIO_TypeDef *p_port;
    uint16_t pin;
    uint8_t capability;
} stm32f411_gpio_desc_t;

static platform_err_t stm32f411_gpio_get_desc(
    plat_gpio_id_t gpio_id,
    const stm32f411_gpio_desc_t **p_desc_out);

//******************************** Variables ********************************//

static const stm32f411_gpio_desc_t
g_stm32f411_gpio_desc[PLAT_GPIO_NUM] =
{
    /* Keep physical STM32 port/pin mapping private to this implementation. */
    /* PLAT_GPIO_CH_1 -> PA0_USER_KEY3 */
    {
        PA0_USER_KEY3_GPIO_Port,
        PA0_USER_KEY3_Pin,
        STM32F411_GPIO_CAP_READ
    },

    /* PLAT_GPIO_CH_2 -> PB1_USER_KEY1 */
    {
        PB1_USER_KEY1_GPIO_Port,
        PB1_USER_KEY1_Pin,
        STM32F411_GPIO_CAP_READ
    },

    /* PLAT_GPIO_CH_3 -> PC13_LED_EN */
    {
        PC13_LED_EN_GPIO_Port,
        PC13_LED_EN_Pin,
        STM32F411_GPIO_CAP_READ | STM32F411_GPIO_CAP_WRITE
    },

    /* PLAT_GPIO_CH_4 -> PB10_LCD_RESET (configured output by MX_GPIO_Init). */
    {
        PB10_LCD_RESET_GPIO_Port,
        PB10_LCD_RESET_Pin,
        STM32F411_GPIO_CAP_READ | STM32F411_GPIO_CAP_WRITE
    },

    /* PLAT_GPIO_CH_5 -> PB3_MOTOR_EN */
    {
        PB3_MOTOR_EN_GPIO_Port,
        PB3_MOTOR_EN_Pin,
        STM32F411_GPIO_CAP_READ | STM32F411_GPIO_CAP_WRITE
    },

    /*
     * PLAT_GPIO_CH_6 -> PA1_LCD_BLACK: preserve the Core macro spelling.
     * Push-pull GPIO, initially HIGH in Core; Board later switches it off.
     * Binary on/off only: no timer channel or PWM is configured here.
     */
    {
        PA1_LCD_BLACK_GPIO_Port,
        PA1_LCD_BLACK_Pin,
        STM32F411_GPIO_CAP_READ | STM32F411_GPIO_CAP_WRITE
    }
};

//******************************** Functions ********************************//

/**
 * @brief Write a logical level to a configured, writable GPIO channel.
 *
 * @param[in] gpio_id Platform GPIO channel identifier.
 * @param[in] level Logical LOW or HIGH level to write.
 * @retval PLATFORM_ERR_OK The HAL write was issued.
 * @retval PLATFORM_ERR_PARAM Invalid level, channel, or descriptor.
 * @retval PLATFORM_ERR_NOT_SUPPORTED The channel is not writable.
 * @note HAL_GPIO_WritePin returns void, not physical output feedback.
 */
platform_err_t plat_gpio_write(
    plat_gpio_id_t gpio_id,
    plat_gpio_level_t level)
{
    const stm32f411_gpio_desc_t *p_desc = NULL;
    GPIO_PinState hal_state = GPIO_PIN_RESET;
    platform_err_t ret = PLATFORM_ERR_OK;

    if ((PLAT_GPIO_LEVEL_LOW != level) && (PLAT_GPIO_LEVEL_HIGH != level))
    {
        return PLATFORM_ERR_PARAM;
    }

    ret = stm32f411_gpio_get_desc(gpio_id, &p_desc);
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    if (NULL == p_desc)
    {
        return PLATFORM_ERR_PARAM;
    }

    if (0u == (p_desc->capability & STM32F411_GPIO_CAP_WRITE))
    {
        return PLATFORM_ERR_NOT_SUPPORTED;
    }

    hal_state = (PLAT_GPIO_LEVEL_HIGH == level) ? GPIO_PIN_SET : GPIO_PIN_RESET;
    HAL_GPIO_WritePin(p_desc->p_port, p_desc->pin, hal_state);

    return PLATFORM_ERR_OK;
}

/**
 * @brief Read a configured GPIO channel and convert its level.
 *
 * @param[in] gpio_id Platform GPIO channel identifier.
 * @param[out] p_level Receives the level; set LOW before channel validation.
 * @retval PLATFORM_ERR_OK The logical level was read successfully.
 * @retval PLATFORM_ERR_PARAM Invalid pointer, channel, or descriptor.
 * @retval PLATFORM_ERR_NOT_SUPPORTED The channel is not readable.
 */
platform_err_t plat_gpio_read(
    plat_gpio_id_t gpio_id,
    plat_gpio_level_t *p_level)
{
    const stm32f411_gpio_desc_t *p_desc = NULL;
    GPIO_PinState hal_state = GPIO_PIN_RESET;
    platform_err_t ret = PLATFORM_ERR_OK;

    if (NULL == p_level)
    {
        return PLATFORM_ERR_PARAM;
    }

    *p_level = PLAT_GPIO_LEVEL_LOW;

    ret = stm32f411_gpio_get_desc(gpio_id, &p_desc);
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    if (NULL == p_desc)
    {
        return PLATFORM_ERR_PARAM;
    }

    if (0u == (p_desc->capability & STM32F411_GPIO_CAP_READ))
    {
        return PLATFORM_ERR_NOT_SUPPORTED;
    }

    hal_state = HAL_GPIO_ReadPin(p_desc->p_port, p_desc->pin);
    *p_level = (GPIO_PIN_SET == hal_state) ?
        PLAT_GPIO_LEVEL_HIGH :
        PLAT_GPIO_LEVEL_LOW;

    return PLATFORM_ERR_OK;
}

/**
 * @brief Resolve and validate the private descriptor for a GPIO channel.
 *
 * @param[in] gpio_id Platform GPIO channel identifier.
 * @param[out] p_desc_out Receives the descriptor; cleared before lookup.
 * @retval PLATFORM_ERR_OK The descriptor is valid and was returned.
 * @retval PLATFORM_ERR_PARAM Invalid output pointer, channel, or descriptor.
 */
static platform_err_t stm32f411_gpio_get_desc(
    plat_gpio_id_t gpio_id,
    const stm32f411_gpio_desc_t **p_desc_out)
{
    const stm32f411_gpio_desc_t *p_desc = NULL;

    if (NULL == p_desc_out)
    {
        return PLATFORM_ERR_PARAM;
    }

    *p_desc_out = NULL;

    if (gpio_id >= PLAT_GPIO_NUM)
    {
        return PLATFORM_ERR_PARAM;
    }

    p_desc = &g_stm32f411_gpio_desc[gpio_id];
    if (NULL == p_desc)
    {
        return PLATFORM_ERR_PARAM;
    }

    if ((NULL == p_desc->p_port) || (0u == p_desc->pin))
    {
        return PLATFORM_ERR_PARAM;
    }

    *p_desc_out = p_desc;
    return PLATFORM_ERR_OK;
}
