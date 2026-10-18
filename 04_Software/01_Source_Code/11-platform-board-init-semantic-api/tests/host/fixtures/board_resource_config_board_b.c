/******************************************************************************
 * Copyright (C) 2026 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file board_resource_config_board_b.c
 *
 * @par dependencies
 * - board_resource_config.h
 *
 * @author Codex (AI-assisted update for EternalChip)
 *
 * @brief Provide a host-only alternate board fixture for the boot tests.
 *
 * Processing flow:
 * 1. Supplies the pin and power entries exercised by the host boot tests.
 * 2. Swaps the two key channels while retaining the output defaults.
 * 3. Links with the unchanged App instead of the EC-S100 resource source.
 *
 * @version V1.0 2026-10-12 Extract the alternate-board boot fixture.
 *
 * @note 1 tab == 4 spaces!
 * @note Partial host fixture; not a hardware configuration or firmware source.
 *****************************************************************************/

//******************************** Includes *********************************//

#include "board_resource_config.h"

//******************************** Variables ********************************//

const char g_board_cfg_name[] = "Board-B-Host-Fixture";

const board_pin_desc_t g_board_pinmap[BOARD_PIN_ID_COUNT] =
{
    [BOARD_PIN_ID_USER_KEY3] =
    {
        BOARD_PIN_ID_USER_KEY3, "USER_KEY3", PLAT_GPIO_CH_2,
        BOARD_SIGNAL_EXTI, BOARD_ACTIVE_LEVEL_LOW
    },
    [BOARD_PIN_ID_USER_KEY1] =
    {
        BOARD_PIN_ID_USER_KEY1, "USER_KEY1", PLAT_GPIO_CH_1,
        BOARD_SIGNAL_EXTI, BOARD_ACTIVE_LEVEL_LOW
    },
    [BOARD_PIN_ID_LCD_RESET] =
    {
        BOARD_PIN_ID_LCD_RESET, "LCD_RESET", PLAT_GPIO_CH_4,
        BOARD_SIGNAL_GPIO, BOARD_ACTIVE_LEVEL_LOW
    },
    [BOARD_PIN_ID_MOTOR_EN] =
    {
        BOARD_PIN_ID_MOTOR_EN, "MOTOR_EN", PLAT_GPIO_CH_5,
        BOARD_SIGNAL_GPIO, BOARD_ACTIVE_LEVEL_HIGH
    },
    [BOARD_PIN_ID_LCD_BACKLIGHT] =
    {
        BOARD_PIN_ID_LCD_BACKLIGHT, "LCD_BACKLIGHT", PLAT_GPIO_CH_6,
        BOARD_SIGNAL_POWER, BOARD_ACTIVE_LEVEL_HIGH
    }
};

const board_power_desc_t g_board_powermap[BOARD_POWER_DOMAIN_COUNT] =
{
    [BOARD_POWER_DOMAIN_BACKLIGHT] =
    {
        BOARD_POWER_DOMAIN_BACKLIGHT, "BACKLIGHT", BOARD_PIN_ID_LCD_BACKLIGHT,
        BOARD_ACTIVE_LEVEL_HIGH, BOARD_POWER_STATE_OFF, BOARD_BUS_ID_INVALID
    },
    [BOARD_POWER_DOMAIN_AHT21] =
    {
        BOARD_POWER_DOMAIN_AHT21, "AHT21", BOARD_PIN_ID_INVALID,
        BOARD_ACTIVE_LEVEL_HIGH, BOARD_POWER_STATE_ON,
#ifdef TEST_AHT_MAP_INVALID
        BOARD_BUS_ID_INVALID
#else
        BOARD_BUS_ID_NORDIC_UART
#endif
    }
};

const board_bus_desc_t g_board_busmap[BOARD_BUS_ID_COUNT] =
{
    [BOARD_BUS_ID_NORDIC_UART] =
    {
        BOARD_BUS_ID_NORDIC_UART, "NORDIC_UART", BOARD_BUS_TYPE_UART,
        PLAT_UART_CH_1, BOARD_PIN_ID_INVALID, BOARD_PIN_ID_INVALID,
        BOARD_PIN_ID_INVALID, BOARD_PIN_ID_BT_UART_TX, BOARD_PIN_ID_BT_UART_RX
    }
};
