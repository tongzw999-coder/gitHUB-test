/******************************************************************************
 * Copyright (C) 2026 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file test_mcu_gpio.c
 *
 * @par dependencies
 * - plat_gpio.h
 * - main.h
 * - stdio.h
 * - stdlib.h
 *
 * @author Codex (AI-assisted update for EternalChip)
 *
 * @brief Verify real MCU GPIO descriptors with host HAL substitutes.
 *
 * Processing flow:
 * 1. Record HAL operations on host GPIO port objects.
 * 2. Check CH1 through CH6 mappings against real main.h macros.
 * 3. Check read/write capabilities and invalid argument handling.
 *
 * @version V1.1 2026-09-13
 *
 * @note 1 tab == 4 spaces!
 *****************************************************************************/

//******************************** Includes *********************************//

#include "plat_gpio.h"
#include "main.h"

#include <stdio.h>
#include <stdlib.h>

//******************************** Defines **********************************//

#define CHECK(expr)                                                         \
    do                                                                      \
    {                                                                       \
        if (!(expr))                                                        \
        {                                                                   \
            if (0 > fprintf(stderr, "FAIL %s:%d: %s\n",                      \
                            __FILE__, __LINE__, #expr))                      \
            {                                                               \
                exit(1);                                                    \
            }                                                               \
            exit(1);                                                        \
        }                                                                   \
    } while (0)

//******************************** Variables ********************************//

GPIO_TypeDef g_host_gpio_a = {1u};
GPIO_TypeDef g_host_gpio_b = {2u};
GPIO_TypeDef g_host_gpio_c = {3u};

static GPIO_TypeDef *g_p_last_port = NULL;
static uint16_t g_last_pin = 0u;
static GPIO_PinState g_last_level = GPIO_PIN_RESET;
static unsigned int g_writes = 0u;

//******************************** Functions ********************************//

/**
 * @brief Record the electrical write requested by the real MCU implementation.
 *
 * @param[in] p_port GPIO port receiving the write.
 * @param[in] pin GPIO pin mask.
 * @param[in] level Requested electrical level.
 *
 * @return None.
 */
void HAL_GPIO_WritePin(GPIO_TypeDef *p_port, uint16_t pin, GPIO_PinState level)
{
    CHECK(NULL != p_port);
    g_p_last_port = p_port;
    g_last_pin = pin;
    g_last_level = level;
    ++g_writes;
}

/**
 * @brief Record the requested GPIO input and return a fixed high level.
 *
 * @param[in] p_port GPIO port being read.
 * @param[in] pin GPIO pin mask.
 *
 * @return GPIO_PIN_SET to exercise the platform high-level conversion.
 */
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *p_port, uint16_t pin)
{
    CHECK(NULL != p_port);
    g_p_last_port = p_port;
    g_last_pin = pin;
    return GPIO_PIN_SET;
}

/**
 * @brief Execute descriptor, capability, and argument validation checks.
 *
 * @par Parameters
 * None.
 *
 * @return 0 after all checks and output succeed; 1 if result output fails.
 *         Failed checks terminate the process with status 1.
 */
int main(void)
{
    unsigned int index = 0u;
    plat_gpio_level_t level = PLAT_GPIO_LEVEL_LOW;
    GPIO_TypeDef *p_ports[] = {GPIOC, GPIOB, GPIOB, GPIOA};
    const uint16_t pins[] = {
        GPIO_PIN_13, GPIO_PIN_10, GPIO_PIN_3, GPIO_PIN_1
    };

    CHECK((0 == PLAT_GPIO_CH_1) && (1 == PLAT_GPIO_CH_2) &&
          (2 == PLAT_GPIO_CH_3));
    CHECK((3 == PLAT_GPIO_CH_4) && (4 == PLAT_GPIO_CH_5) &&
          (5 == PLAT_GPIO_CH_6));
    CHECK(6 == PLAT_GPIO_NUM);
    CHECK(PLATFORM_ERR_NOT_SUPPORTED ==
          plat_gpio_write(PLAT_GPIO_CH_1, PLAT_GPIO_LEVEL_HIGH));
    CHECK(PLATFORM_ERR_NOT_SUPPORTED ==
          plat_gpio_write(PLAT_GPIO_CH_2, PLAT_GPIO_LEVEL_HIGH));
    CHECK(0u == g_writes);
    CHECK(PLATFORM_ERR_OK == plat_gpio_read(PLAT_GPIO_CH_1, &level));
    CHECK((GPIOA == g_p_last_port) && (GPIO_PIN_0 == g_last_pin) &&
          (PLAT_GPIO_LEVEL_HIGH == level));
    CHECK(PLATFORM_ERR_OK == plat_gpio_read(PLAT_GPIO_CH_2, &level));
    CHECK((GPIOB == g_p_last_port) && (GPIO_PIN_1 == g_last_pin));
    for (index = 0u; index < 4u; ++index)
    {
        CHECK(PLATFORM_ERR_OK ==
              plat_gpio_write((plat_gpio_id_t)(index + 2u),
                              PLAT_GPIO_LEVEL_HIGH));
        CHECK((g_p_last_port == p_ports[index]) &&
              (g_last_pin == pins[index]) && (GPIO_PIN_SET == g_last_level));
        CHECK(PLATFORM_ERR_OK ==
              plat_gpio_write((plat_gpio_id_t)(index + 2u),
                              PLAT_GPIO_LEVEL_LOW));
        CHECK((g_p_last_port == p_ports[index]) &&
              (g_last_pin == pins[index]) && (GPIO_PIN_RESET == g_last_level));
        CHECK(PLATFORM_ERR_OK ==
              plat_gpio_read((plat_gpio_id_t)(index + 2u), &level));
        CHECK((g_p_last_port == p_ports[index]) &&
              (g_last_pin == pins[index]) && (PLAT_GPIO_LEVEL_HIGH == level));
    }
    CHECK(8u == g_writes);
    CHECK(PLATFORM_ERR_PARAM ==
          plat_gpio_write(PLAT_GPIO_NUM, PLAT_GPIO_LEVEL_HIGH));
    CHECK(PLATFORM_ERR_PARAM ==
          plat_gpio_write(PLAT_GPIO_ID_RESERVED, PLAT_GPIO_LEVEL_HIGH));
    CHECK(PLATFORM_ERR_PARAM ==
          plat_gpio_write(PLAT_GPIO_CH_6, PLAT_GPIO_LEVEL_RESERVED));
    CHECK(PLATFORM_ERR_PARAM == plat_gpio_read(PLAT_GPIO_CH_6, NULL));
    CHECK(8u == g_writes);
    if (0 > puts("PASS real MCU descriptors CH1..CH6 and GPIO "
                 "capability checks"))
    {
        return 1;
    }
    return 0;
}
