/******************************************************************************
 * Copyright (C) 2026 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file board_resource_config.h
 *
 * @par dependencies
 * - platform_type.h
 * - platform_mcu_config.h
 *
 * @author Codex (AI-assisted update for EternalChip)
 *
 * @brief Declare the shared board resource identifiers and table interface.
 *
 * Processing flow:
 * 1. Defines the resource identifiers and descriptor types used by all boards.
 * 2. Declares the resource tables supplied by the selected board source.
 * 3. Exposes the selected board identity through the common interface.
 *
 * @version V1.0 Baseline resource declarations; date not recorded.
 * @version V1.1 2026-10-12 Separate common declarations from board identity.
 * @version V1.2 2026-10-15 Describe AHT21 power control through a UART bus.
 *
 * @note 1 tab == 4 spaces!
 * @note Each firmware target links one board_resource_config_<board>.c.
 *****************************************************************************/

#ifndef __BOARD_RESOURCE_CONFIG_H__
#define __BOARD_RESOURCE_CONFIG_H__

//******************************** Includes *********************************//

#include "platform_type.h"
#include "platform_mcu_config.h"

//******************************** Defines **********************************//

#define BOARD_CFG_NAME              g_board_cfg_name
#define BOARD_CFG_ADDR_INVALID      (0xFFFFU)

//******************************** Declaring ********************************//

typedef enum
{
    BOARD_ACTIVE_LEVEL_LOW = 0,
    BOARD_ACTIVE_LEVEL_HIGH,
    BOARD_ACTIVE_LEVEL_NONE,
    BOARD_ACTIVE_LEVEL_RESERVED = 0x7FFFFFFF
} board_active_level_t;

typedef enum
{
    BOARD_SIGNAL_GPIO = 0,
    BOARD_SIGNAL_I2C,
    BOARD_SIGNAL_SPI,
    BOARD_SIGNAL_UART,
    BOARD_SIGNAL_ADC,
    BOARD_SIGNAL_TIMER,
    BOARD_SIGNAL_EXTI,
    BOARD_SIGNAL_POWER,
    BOARD_SIGNAL_RESERVED = 0x7FFFFFFF
} board_signal_type_t;

/* pinmap: logical board signals mapped to MCU platform GPIO channels. */

typedef enum
{
    BOARD_PIN_ID_LED_EN = 0,
    BOARD_PIN_ID_USER_KEY3,
    BOARD_PIN_ID_LCD_BACKLIGHT,
    BOARD_PIN_ID_LIGHT_ADC,
    BOARD_PIN_ID_HEART_INT,
    BOARD_PIN_ID_LCD_CS,
    BOARD_PIN_ID_LCD_CLK,
    BOARD_PIN_ID_LCD_DC,
    BOARD_PIN_ID_LCD_MOSI,
    BOARD_PIN_ID_BAT_ADC,
    BOARD_PIN_ID_USER_KEY1,
    BOARD_PIN_ID_TOUCH_INT,
    BOARD_PIN_ID_LCD_RESET,
    BOARD_PIN_ID_FLASH_CS,
    BOARD_PIN_ID_FLASH_CLK,
    BOARD_PIN_ID_FLASH_MISO,
    BOARD_PIN_ID_FLASH_MOSI,
    BOARD_PIN_ID_TOUCH_SCL,
    BOARD_PIN_ID_BT_UART_TX,
    BOARD_PIN_ID_BT_UART_RX,
    BOARD_PIN_ID_TOUCH_RESET,
    BOARD_PIN_ID_MOTOR_EN,
    BOARD_PIN_ID_TOUCH_SDA,
    BOARD_PIN_ID_IMU_INT,
    BOARD_PIN_ID_COMMON_SCL,
    BOARD_PIN_ID_COMMON_SDA,
    BOARD_PIN_ID_SPEAKER_SIGNAL,
    BOARD_PIN_ID_SPEAKER_STATUS,
    BOARD_PIN_ID_COUNT,
    BOARD_PIN_ID_RESERVED = 0x7FFFFFFF
} board_pin_id_t;

#define BOARD_PIN_ID_INVALID ((board_pin_id_t)0xFFU)

typedef struct
{
    board_pin_id_t pin_id;
    const char *p_name;
    plat_gpio_id_t gpio_id;
    board_signal_type_t signal_type;
    board_active_level_t active_level;
} board_pin_desc_t;

/* Non-GPIO resources stay reserved until their platform interface is added. */

/* busmap: logical buses and their board-level device attachments. */

typedef enum
{
    BOARD_BUS_TYPE_I2C = 0,
    BOARD_BUS_TYPE_SPI,
    BOARD_BUS_TYPE_UART,
    BOARD_BUS_TYPE_ADC,
    BOARD_BUS_TYPE_TIMER,
    BOARD_BUS_TYPE_RESERVED = 0x7FFFFFFF
} board_bus_type_t;

typedef enum
{
    BOARD_BUS_ID_COMMON_I2C = 0,
    BOARD_BUS_ID_TOUCH_I2C,
    BOARD_BUS_ID_DISPLAY_SPI,
    BOARD_BUS_ID_FLASH_SPI,
    BOARD_BUS_ID_NORDIC_UART,
    BOARD_BUS_ID_BAT_ADC,
    BOARD_BUS_ID_LIGHT_ADC,
    BOARD_BUS_ID_BACKLIGHT_PWM,
    BOARD_BUS_ID_COUNT,
    BOARD_BUS_ID_RESERVED = 0x7FFFFFFF
} board_bus_id_t;

typedef enum
{
    BOARD_BUS_DEVICE_AHT21 = 0,
    BOARD_BUS_DEVICE_IMU,
    BOARD_BUS_DEVICE_POWER_CHIP,
    BOARD_BUS_DEVICE_TOUCH,
    BOARD_BUS_DEVICE_DISPLAY,
    BOARD_BUS_DEVICE_STORAGE,
    BOARD_BUS_DEVICE_BT,
    BOARD_BUS_DEVICE_BATTERY_SAMPLE,
    BOARD_BUS_DEVICE_LIGHT_SAMPLE,
    BOARD_BUS_DEVICE_BACKLIGHT,
    BOARD_BUS_DEVICE_COUNT,
    BOARD_BUS_DEVICE_RESERVED = 0x7FFFFFFF
} board_bus_device_id_t;

#define BOARD_BUS_ID_INVALID ((board_bus_id_t)0xFFU)

typedef struct
{
    board_bus_id_t bus_id;
    const char *p_name;
    board_bus_type_t bus_type;
    /* Platform resource ID; for UART this is a plat_uart_id_t value. */
    uint8_t platform_instance_id;
    board_pin_id_t scl_pin;
    board_pin_id_t sda_pin;
    board_pin_id_t sck_pin;
    board_pin_id_t mosi_pin;
    board_pin_id_t miso_pin;
} board_bus_desc_t;

typedef struct
{
    board_bus_device_id_t device_id;
    const char *p_name;
    board_bus_id_t bus_id;
    uint16_t address_or_channel;
    board_pin_id_t cs_pin;
    board_pin_id_t irq_pin;
} board_bus_device_desc_t;

/* powermap: board power domains and their control signals. */

typedef enum
{
    BOARD_POWER_DOMAIN_SYSTEM = 0,
    BOARD_POWER_DOMAIN_DISPLAY,
    BOARD_POWER_DOMAIN_TOUCH,
    BOARD_POWER_DOMAIN_SENSOR,
    BOARD_POWER_DOMAIN_STORAGE,
    BOARD_POWER_DOMAIN_BACKLIGHT,
    BOARD_POWER_DOMAIN_AHT21,
    BOARD_POWER_DOMAIN_SPEAKER,
    BOARD_POWER_DOMAIN_BATTERY,
    BOARD_POWER_DOMAIN_COUNT,
    BOARD_POWER_DOMAIN_RESERVED = 0x7FFFFFFF
} board_power_domain_t;

typedef enum
{
    BOARD_POWER_STATE_OFF = 0,
    BOARD_POWER_STATE_ON,
    BOARD_POWER_STATE_SLEEP,
    BOARD_POWER_STATE_RESERVED = 0x7FFFFFFF
} board_power_state_t;

typedef struct
{
    board_power_domain_t domain;
    const char *p_name;
    board_pin_id_t enable_pin;
    board_active_level_t active_level;
    board_power_state_t default_state;
    /* Remote supply control bus; INVALID for local or descriptive entries. */
    board_bus_id_t control_bus;
} board_power_desc_t;

/* irqmap: board events mapped to interrupt sources. */

typedef enum
{
    BOARD_IRQ_ID_USER_KEY3 = 0,
    BOARD_IRQ_ID_TOUCH,
    BOARD_IRQ_ID_IMU,
    BOARD_IRQ_ID_HEART_RATE,
    BOARD_IRQ_ID_RTC_WAKEUP,
    BOARD_IRQ_ID_BT_UART,
    BOARD_IRQ_ID_SPI_DISPLAY_DMA,
    BOARD_IRQ_ID_SPI_FLASH_DMA,
    BOARD_IRQ_ID_I2C_COMMON_DMA,
    BOARD_IRQ_ID_COUNT,
    BOARD_IRQ_ID_RESERVED = 0x7FFFFFFF
} board_irq_id_t;

typedef enum
{
    BOARD_IRQ_TRIGGER_RISING = 0,
    BOARD_IRQ_TRIGGER_FALLING,
    BOARD_IRQ_TRIGGER_BOTH,
    BOARD_IRQ_TRIGGER_LEVEL,
    BOARD_IRQ_TRIGGER_RESERVED = 0x7FFFFFFF
} board_irq_trigger_t;

typedef struct
{
    board_irq_id_t irq_id;
    const char *p_name;
    board_pin_id_t pin_id;
    board_irq_trigger_t trigger;
    uint8_t priority;
} board_irq_desc_t;

//******************************** Variables ********************************//

extern const char g_board_cfg_name[];
extern const board_pin_desc_t g_board_pinmap[BOARD_PIN_ID_COUNT];
extern const board_bus_desc_t g_board_busmap[BOARD_BUS_ID_COUNT];
extern const board_bus_device_desc_t g_board_bus_device_map[BOARD_BUS_DEVICE_COUNT];
extern const board_power_desc_t g_board_powermap[BOARD_POWER_DOMAIN_COUNT];
extern const board_irq_desc_t g_board_irqmap[BOARD_IRQ_ID_COUNT];

#endif /* __BOARD_RESOURCE_CONFIG_H__ */
