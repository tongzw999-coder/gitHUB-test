/******************************************************************************
 * Copyright (C) 2026 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file test_board_validation.c
 *
 * @par dependencies
 * - board_resource_config.h
 * - plat_gpio.h
 * - plat_tick.h
 * - stdio.h
 * - stdlib.h
 * - board_control.h
 * - board_init.h
 * - board_internal.h
 *
 * @author Codex (AI-assisted update for EternalChip)
 *
 * @brief Exercise private board validation with immutable synthetic mappings.
 *
 * Processing flow:
 * 1. Supply intentionally malformed and remapped resource fixtures.
 * 2. Link the separate board initialization and control implementations.
 * 3. Verify validation results, polarity conversion, and initialization.
 *
 * @version V1.1 2026-09-13
 *
 * @note 1 tab == 4 spaces!
 *****************************************************************************/

//******************************** Includes *********************************//

#include "board_resource_config.h"
#include "board_control.h"
#include "board_init.h"
#include "board_internal.h"
#include "plat_gpio.h"
#include "plat_tick.h"
#include "fake_uart.h"

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

#ifndef BAD_POWER
#define BAD_POWER 0
#endif

#ifndef BAD_UART
#define BAD_UART 0
#endif

//******************************** Variables ********************************//

const board_pin_desc_t g_board_pinmap[BOARD_PIN_ID_COUNT] = {
    [BOARD_PIN_ID_LED_EN] = {
        BOARD_PIN_ID_LED_EN, "gpio", PLAT_GPIO_CH_3,
        BOARD_SIGNAL_GPIO, BOARD_ACTIVE_LEVEL_HIGH
    },
    [BOARD_PIN_ID_USER_KEY3] = {
        BOARD_PIN_ID_LED_EN, "bad-id", PLAT_GPIO_CH_1,
        BOARD_SIGNAL_GPIO, BOARD_ACTIVE_LEVEL_LOW
    },
    [BOARD_PIN_ID_LCD_BACKLIGHT] = {
        BOARD_PIN_ID_LCD_BACKLIGHT, "remapped-motor", PLAT_GPIO_CH_6,
        BOARD_SIGNAL_POWER, BOARD_ACTIVE_LEVEL_LOW
    },
    [BOARD_PIN_ID_LIGHT_ADC] = {
        BOARD_PIN_ID_LIGHT_ADC, "reserved", PLAT_GPIO_ID_RESERVED,
        BOARD_SIGNAL_GPIO, BOARD_ACTIVE_LEVEL_HIGH
    },
    [BOARD_PIN_ID_HEART_INT] = {
        BOARD_PIN_ID_HEART_INT, "count", PLAT_GPIO_NUM,
        BOARD_SIGNAL_GPIO, BOARD_ACTIVE_LEVEL_HIGH
    },
    [BOARD_PIN_ID_LCD_CS] = {
        BOARD_PIN_ID_LCD_CS, "negative", (plat_gpio_id_t)-1,
        BOARD_SIGNAL_GPIO, BOARD_ACTIVE_LEVEL_LOW
    },
    [BOARD_PIN_ID_LCD_CLK] = {
        BOARD_PIN_ID_LCD_CLK, "spi", PLAT_GPIO_CH_4,
        BOARD_SIGNAL_SPI, BOARD_ACTIVE_LEVEL_LOW
    },
    [BOARD_PIN_ID_LCD_DC] = {
        BOARD_PIN_ID_LCD_DC, "none", PLAT_GPIO_CH_3,
        BOARD_SIGNAL_GPIO, BOARD_ACTIVE_LEVEL_NONE
    },
    [BOARD_PIN_ID_LCD_MOSI] = {
        BOARD_PIN_ID_LCD_MOSI, "reserved-level", PLAT_GPIO_CH_3,
        BOARD_SIGNAL_GPIO, BOARD_ACTIVE_LEVEL_RESERVED
    },
    [BOARD_PIN_ID_BAT_ADC] = {
        BOARD_PIN_ID_BAT_ADC, "negative-level", PLAT_GPIO_CH_3,
        BOARD_SIGNAL_GPIO, (board_active_level_t)-1
    },
    [BOARD_PIN_ID_USER_KEY1] = {
        BOARD_PIN_ID_USER_KEY1, "signal", PLAT_GPIO_CH_2,
        BOARD_SIGNAL_RESERVED, BOARD_ACTIVE_LEVEL_LOW
    },
    [BOARD_PIN_ID_LCD_RESET] = {
        BOARD_PIN_ID_LCD_RESET, "reset", PLAT_GPIO_CH_4,
        BOARD_SIGNAL_GPIO, BOARD_ACTIVE_LEVEL_LOW
    },
    [BOARD_PIN_ID_MOTOR_EN] = {
        BOARD_PIN_ID_MOTOR_EN, "remapped-light", PLAT_GPIO_CH_5,
        BOARD_SIGNAL_POWER, BOARD_ACTIVE_LEVEL_HIGH
    }
};

/* Remap backlight and contradict its descriptive power-map polarity.
 * Motor action must still follow MOTOR_EN in pinmap directly. */
const board_power_desc_t g_board_powermap[BOARD_POWER_DOMAIN_COUNT] = {
#if 1 == BAD_POWER
    [BOARD_POWER_DOMAIN_BACKLIGHT] = {
        BOARD_POWER_DOMAIN_AHT21, "bad-domain", BOARD_PIN_ID_MOTOR_EN,
        BOARD_ACTIVE_LEVEL_LOW, BOARD_POWER_STATE_OFF, BOARD_BUS_ID_INVALID
    },
#elif 2 == BAD_POWER
    [BOARD_POWER_DOMAIN_BACKLIGHT] = {
        BOARD_POWER_DOMAIN_BACKLIGHT, "bad-pin", BOARD_PIN_ID_INVALID,
        BOARD_ACTIVE_LEVEL_LOW, BOARD_POWER_STATE_OFF, BOARD_BUS_ID_INVALID
    },
#elif 3 == BAD_POWER
    [BOARD_POWER_DOMAIN_BACKLIGHT] = {
        BOARD_POWER_DOMAIN_BACKLIGHT, "reserved-pin", BOARD_PIN_ID_RESERVED,
        BOARD_ACTIVE_LEVEL_LOW, BOARD_POWER_STATE_OFF, BOARD_BUS_ID_INVALID
    },
#else
    [BOARD_POWER_DOMAIN_BACKLIGHT] = {
        BOARD_POWER_DOMAIN_BACKLIGHT, "light", BOARD_PIN_ID_MOTOR_EN,
        BOARD_ACTIVE_LEVEL_LOW, BOARD_POWER_STATE_OFF, BOARD_BUS_ID_INVALID
    },
#endif
    [BOARD_POWER_DOMAIN_AHT21] = {
#if 5 == BAD_UART
        BOARD_POWER_DOMAIN_SYSTEM,
#else
        BOARD_POWER_DOMAIN_AHT21,
#endif
        "AHT21",
#if 6 == BAD_UART
        BOARD_PIN_ID_MOTOR_EN,
#else
        BOARD_PIN_ID_INVALID,
#endif
        BOARD_ACTIVE_LEVEL_HIGH, BOARD_POWER_STATE_ON,
#if 4 == BAD_UART
        BOARD_BUS_ID_INVALID
#else
        BOARD_BUS_ID_NORDIC_UART
#endif
    }
};

const board_bus_desc_t g_board_busmap[BOARD_BUS_ID_COUNT] = {
    [BOARD_BUS_ID_NORDIC_UART] = {
#if 1 == BAD_UART
        BOARD_BUS_ID_COMMON_I2C,
#else
        BOARD_BUS_ID_NORDIC_UART,
#endif
        "NORDIC_UART",
#if 2 == BAD_UART
        BOARD_BUS_TYPE_I2C,
#else
        BOARD_BUS_TYPE_UART,
#endif
#if 3 == BAD_UART
        PLAT_UART_NUM,
#else
        PLAT_UART_CH_1,
#endif
        BOARD_PIN_ID_INVALID, BOARD_PIN_ID_INVALID, BOARD_PIN_ID_INVALID,
        BOARD_PIN_ID_BT_UART_TX, BOARD_PIN_ID_BT_UART_RX
    }
};

static unsigned int g_writes = 0u;
static plat_gpio_id_t g_ids[8] = {PLAT_GPIO_CH_1};
static plat_gpio_level_t g_levels[8] = {PLAT_GPIO_LEVEL_LOW};

//******************************** Functions ********************************//

/**
 * @brief Record a GPIO write produced by a synthetic board mapping.
 *
 * @param[in] id Platform GPIO channel selected by the board.
 * @param[in] level Requested electrical level.
 *
 * @return PLATFORM_ERR_OK after recording the write.
 */
platform_err_t plat_gpio_write(plat_gpio_id_t id, plat_gpio_level_t level)
{
    CHECK(g_writes < 8u);
    g_ids[g_writes] = id;
    g_levels[g_writes++] = level;
    return PLATFORM_ERR_OK;
}

/**
 * @brief Reject delays because mapping validation must not pulse reset.
 *
 * @param[in] ms Unexpected requested delay in milliseconds.
 *
 * @return None.
 */
void plat_delay_ms(uint32_t ms)
{
    (void)ms;
    CHECK(0);
}

/**
 * @brief Check malformed resources and the selected power-map fixture.
 *
 * @par Parameters
 * None.
 *
 * @return 0 after all checks and output succeed; 1 if result output fails.
 *         Failed checks terminate the process with status 1.
 */
int main(void)
{
    const board_signal_state_t invalid_states[] = {
        (board_signal_state_t)-1, (board_signal_state_t)2,
        (board_signal_state_t)256, BOARD_SIGNAL_STATE_RESERVED
    };
    unsigned int index = 0u;

    for (index = 0u;
         index < (sizeof(invalid_states) / sizeof(invalid_states[0]));
         ++index)
    {
        CHECK(PLATFORM_ERR_PARAM ==
              board_pin_set_active(BOARD_PIN_ID_LED_EN,
                                   invalid_states[index]));
        CHECK(PLATFORM_ERR_PARAM ==
              board_power_set_active(BOARD_POWER_DOMAIN_BACKLIGHT,
                                     invalid_states[index]));
    }
    CHECK(FALSE == board_is_initialized());
    CHECK(PLATFORM_ERR_PARAM ==
          board_pin_set_active((board_pin_id_t)-1, BOARD_SIGNAL_INACTIVE));
    CHECK(PLATFORM_ERR_PARAM ==
          board_pin_set_active(BOARD_PIN_ID_COUNT, BOARD_SIGNAL_INACTIVE));
    CHECK(PLATFORM_ERR_PARAM ==
          board_pin_set_active(BOARD_PIN_ID_INVALID, BOARD_SIGNAL_INACTIVE));
    CHECK(PLATFORM_ERR_PARAM ==
          board_pin_set_active(BOARD_PIN_ID_RESERVED, BOARD_SIGNAL_INACTIVE));
    CHECK(PLATFORM_ERR_PARAM ==
          board_pin_set_active(BOARD_PIN_ID_LED_EN, (board_signal_state_t)2));
    CHECK(PLATFORM_ERR_PARAM ==
          board_pin_set_active(BOARD_PIN_ID_USER_KEY3, BOARD_SIGNAL_INACTIVE));
    CHECK(PLATFORM_ERR_NOT_SUPPORTED ==
          board_pin_set_active(BOARD_PIN_ID_LIGHT_ADC, BOARD_SIGNAL_INACTIVE));
    CHECK(PLATFORM_ERR_PARAM ==
          board_pin_set_active(BOARD_PIN_ID_HEART_INT, BOARD_SIGNAL_INACTIVE));
    CHECK(PLATFORM_ERR_PARAM ==
          board_pin_set_active(BOARD_PIN_ID_LCD_CS, BOARD_SIGNAL_INACTIVE));
    CHECK(PLATFORM_ERR_NOT_SUPPORTED ==
          board_pin_set_active(BOARD_PIN_ID_LCD_CLK, BOARD_SIGNAL_INACTIVE));
    CHECK(PLATFORM_ERR_PARAM ==
          board_pin_set_active(BOARD_PIN_ID_LCD_DC, BOARD_SIGNAL_INACTIVE));
    CHECK(PLATFORM_ERR_PARAM ==
          board_pin_set_active(BOARD_PIN_ID_LCD_MOSI, BOARD_SIGNAL_INACTIVE));
    CHECK(PLATFORM_ERR_PARAM ==
          board_pin_set_active(BOARD_PIN_ID_BAT_ADC, BOARD_SIGNAL_INACTIVE));
    CHECK(PLATFORM_ERR_NOT_SUPPORTED ==
          board_pin_set_active(BOARD_PIN_ID_USER_KEY1, BOARD_SIGNAL_INACTIVE));
    CHECK(PLATFORM_ERR_PARAM ==
          board_power_set_active((board_power_domain_t)-1,
                                 BOARD_SIGNAL_INACTIVE));
    CHECK(PLATFORM_ERR_PARAM ==
          board_power_set_active(BOARD_POWER_DOMAIN_COUNT,
                                 BOARD_SIGNAL_INACTIVE));
    CHECK(PLATFORM_ERR_PARAM ==
          board_power_set_active(BOARD_POWER_DOMAIN_RESERVED,
                                 BOARD_SIGNAL_INACTIVE));
    CHECK(PLATFORM_ERR_PARAM ==
          board_power_set_active(BOARD_POWER_DOMAIN_BACKLIGHT,
                                 (board_signal_state_t)2));
    CHECK(PLATFORM_ERR_NOT_SUPPORTED ==
          board_power_set_active(BOARD_POWER_DOMAIN_DISPLAY,
                                 BOARD_SIGNAL_ACTIVE));
    CHECK(PLATFORM_ERR_NOT_SUPPORTED ==
          board_power_set_active(BOARD_POWER_DOMAIN_SYSTEM,
                                 BOARD_SIGNAL_ACTIVE));
    CHECK(0u == g_writes);
#if 0 == BAD_UART
    CHECK(PLATFORM_ERR_NOT_SUPPORTED ==
          board_power_set_active(BOARD_POWER_DOMAIN_AHT21, BOARD_SIGNAL_ACTIVE));
#else
    CHECK(PLATFORM_ERR_PARAM ==
          board_power_set_active(BOARD_POWER_DOMAIN_AHT21, BOARD_SIGNAL_ACTIVE));
#endif
    CHECK(0u == g_test_uart_writes);
    CHECK(0u == g_writes);
#if 0 != BAD_POWER
    CHECK(PLATFORM_ERR_PARAM == board_init());
    CHECK(0u == g_writes);
    CHECK(PLATFORM_ERR_NOT_INITIALIZED == board_backlight_on());
    CHECK(PLATFORM_ERR_NOT_INITIALIZED ==
          board_motor_enable(BOARD_SIGNAL_ACTIVE));
    CHECK(PLATFORM_ERR_NOT_INITIALIZED == board_display_reset());
#else
    CHECK(PLATFORM_ERR_OK == board_init());
    CHECK(TRUE == board_is_initialized());
    CHECK(3u == g_writes);
    CHECK((PLAT_GPIO_CH_5 == g_ids[0]) &&
          (PLAT_GPIO_LEVEL_LOW == g_levels[0]));
    CHECK((PLAT_GPIO_CH_5 == g_ids[1]) &&
          (PLAT_GPIO_LEVEL_LOW == g_levels[1]));
    CHECK((PLAT_GPIO_CH_4 == g_ids[2]) &&
          (PLAT_GPIO_LEVEL_HIGH == g_levels[2]));
    CHECK(PLATFORM_ERR_OK == board_backlight_on());
    CHECK(PLATFORM_ERR_OK == board_motor_enable(BOARD_SIGNAL_ACTIVE));
    CHECK((PLAT_GPIO_CH_5 == g_ids[3]) &&
          (PLAT_GPIO_LEVEL_HIGH == g_levels[3]));
    CHECK((PLAT_GPIO_CH_5 == g_ids[4]) &&
          (PLAT_GPIO_LEVEL_HIGH == g_levels[4]));
#endif
    if (0 > printf("PASS private mapping validation BAD_POWER=%d BAD_UART=%d\n", BAD_POWER, BAD_UART))
    {
        return 1;
    }
    return 0;
}
