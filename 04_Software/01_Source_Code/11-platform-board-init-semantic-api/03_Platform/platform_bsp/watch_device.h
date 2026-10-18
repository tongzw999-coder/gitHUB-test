/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file watch_device.h
 *
 * @par dependencies
 * - platform_device.h
 *
 * @author Jack | R&D Dept. | EternalChip
 *
 * @brief Define EC-S100 typed device cfg/ctx/data/ops models.
 *
 * Processing flow:
 *
 * 1. Keep platform_device_t as the lightweight base object.
 * 2. Put typed cfg/ctx/data/ops in concrete device objects.
 * 3. Use AHT21, Motion, Battery and Backlight LED as lesson examples.
 *
 * @version V1.0 2026-06-03
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#ifndef __WATCH_DEVICE_H__
#define __WATCH_DEVICE_H__

//******************************** Includes *********************************//

#include "platform_device.h"

//******************************** Declaring ********************************//

typedef struct imu_device imu_device_t;
typedef struct temp_humi_device temp_humi_device_t;
typedef struct battery_device battery_device_t;
typedef struct backlight_device backlight_device_t;

/* Motion / IMU device model. */
typedef struct
{
    uint8_t bus_id;
    uint8_t irq_id;
    uint32_t sample_period_ms;
    uint8_t fifo_enable;
} motion_cfg_t;

typedef struct
{
    uint8_t initialized;
    uint8_t data_ready;
    uint32_t last_sample_ms;
    uint32_t error_count;
} motion_ctx_t;

typedef struct
{
    int16_t x_mg;
    int16_t y_mg;
    int16_t z_mg;
    uint32_t timestamp_ms;
    uint8_t valid;
} motion_data_t;

typedef motion_data_t imu_data_t;

typedef struct
{
    platform_err_t (*read)(imu_device_t *p_dev, motion_data_t *p_out);
    platform_err_t (*read_raw)(imu_device_t *p_dev, uint8_t **p_out);
    platform_err_t (*read_done)(imu_device_t *p_dev);
    platform_err_t (*sleep)(imu_device_t *p_dev);
    platform_err_t (*wakeup)(imu_device_t *p_dev);
} motion_ops_t;

typedef motion_ops_t imu_ops_t;

struct imu_device
{
    platform_device_t base;
    const motion_cfg_t *cfg;
    motion_ctx_t ctx;
    motion_data_t data;
    const motion_ops_t *ops;
};

/* AHT21 temperature and humidity device model. */
typedef struct
{
    uint8_t bus_id;
    uint8_t i2c_read_addr;
    uint8_t i2c_write_addr;
    uint32_t sample_period_ms;
} aht21_cfg_t;

typedef struct
{
    uint8_t initialized;
    uint8_t busy;
    uint32_t last_sample_ms;
    uint32_t error_count;
} aht21_ctx_t;

typedef struct
{
    int32_t temperature_milli_c;
    uint32_t humidity_milli_pct;
    uint32_t timestamp_ms;
    uint8_t valid;
} temp_humi_data_t;

typedef struct
{
    platform_err_t (*read)(temp_humi_device_t *p_dev,
                           temp_humi_data_t *p_out);
    platform_err_t (*sleep)(temp_humi_device_t *p_dev);
    platform_err_t (*wakeup)(temp_humi_device_t *p_dev);
} aht21_ops_t;

typedef aht21_ops_t temp_humi_ops_t;

struct temp_humi_device
{
    platform_device_t base;
    const aht21_cfg_t *cfg;
    aht21_ctx_t ctx;
    temp_humi_data_t data;
    const aht21_ops_t *ops;
};

/* Battery device model. */
typedef struct
{
    uint8_t adc_channel;
    uint8_t charge_status_reg;
    uint8_t low_power_percent;
    uint8_t ota_min_percent;
    uint32_t sample_period_ms;
} battery_cfg_t;

typedef struct
{
    uint8_t initialized;
    uint32_t last_sample_ms;
    uint32_t error_count;
} battery_ctx_t;

typedef struct
{
    uint16_t voltage_mv;
    uint8_t charge_state;
    uint8_t percent;
    uint32_t timestamp_ms;
    uint8_t valid;
} battery_data_t;

typedef struct
{
    platform_err_t (*read)(battery_device_t *p_dev,
                           battery_data_t *p_out);
} battery_ops_t;

struct battery_device
{
    platform_device_t base;
    const battery_cfg_t *cfg;
    battery_ctx_t ctx;
    battery_data_t data;
    const battery_ops_t *ops;
};

/* Backlight LED device model. */
typedef struct
{
    uint16_t default_level;
    uint16_t max_level;
    uint8_t active_level;
    uint32_t pwm_hz;
} led_cfg_t;

typedef struct
{
    uint8_t initialized;
    uint8_t enabled;
    uint32_t last_update_ms;
} led_ctx_t;

typedef struct
{
    uint16_t level;
    uint32_t timestamp_ms;
    uint8_t valid;
} led_data_t;

typedef struct
{
    platform_err_t (*set_level)(backlight_device_t *p_dev,
                                uint16_t level);
    platform_err_t (*sleep)(backlight_device_t *p_dev);
    platform_err_t (*wakeup)(backlight_device_t *p_dev);
} led_ops_t;

typedef led_ops_t backlight_ops_t;

struct backlight_device
{
    platform_device_t base;
    const led_cfg_t *cfg;
    led_ctx_t ctx;
    led_data_t data;
    const led_ops_t *ops;
};

#endif /* __WATCH_DEVICE_H__ */
