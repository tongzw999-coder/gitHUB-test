/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file platform_device.h
 *
 * @par dependencies
 * - platform_object.h
 *
 * @author Jack | R&D Dept. | EternalChip
 *
 * @brief Provide the common platform device base object.
 *
 * Processing flow:
 *
 * 1. Use platform_object_t as the first field of each device object.
 * 2. Classify the device by platform_device_class_t.
 * 3. Declare device IO capabilities by caps.
 *
 * Note: Lifecycle is now stored in platform_object_t, not duplicated here.
 * Power state is tracked by the object lifecycle state machine.
 *
 * @version V1.1 2026-05-31
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#ifndef __PLATFORM_DEVICE_H__
#define __PLATFORM_DEVICE_H__

//******************************** Includes *********************************//

#include "platform_object.h"

//******************************** Declaring ********************************//

/* Smart watch device class. */
typedef enum
{
    PLATFORM_DEVICE_CLASS_DISPLAY,    /* Display panel device.              */
    PLATFORM_DEVICE_CLASS_TOUCH,      /* Touch panel device.                */
    PLATFORM_DEVICE_CLASS_IMU,        /* Motion sensor device.              */
    PLATFORM_DEVICE_CLASS_TEMP_HUMI,  /* Temperature and humidity device.   */
    PLATFORM_DEVICE_CLASS_HEART_RATE, /* Heart-rate sensor device.          */
    PLATFORM_DEVICE_CLASS_BATTERY,    /* Battery and charger monitor.       */
    PLATFORM_DEVICE_CLASS_STORAGE,    /* External or internal storage.      */
    PLATFORM_DEVICE_CLASS_BACKLIGHT,  /* Display backlight device.          */
    PLATFORM_DEVICE_CLASS_MOTOR,      /* Vibration motor device.            */
    PLATFORM_DEVICE_CLASS_CPU,        /* CPU or core clock control object.  */
    PLATFORM_DEVICE_CLASS_CLOCK,      /* System clock tree control object.  */
    PLATFORM_DEVICE_CLASS_POWER,      /* PMIC or board power control object.*/
    PLATFORM_DEVICE_CLASS_RTC,        /* Real-time clock device.            */
    PLATFORM_DEVICE_CLASS_KEY         /* Physical key or button device.     */
} platform_device_class_t;

/* Static device IO capabilities. */
typedef enum
{
    PLATFORM_DEVICE_CAP_READ     = 1u << 0,  /* Supports read operations.      */
    PLATFORM_DEVICE_CAP_WRITE    = 1u << 1,  /* Supports write operations.     */
    PLATFORM_DEVICE_CAP_CONTROL  = 1u << 2,  /* Supports control operations.   */
    PLATFORM_DEVICE_CAP_IRQ      = 1u << 3,  /* Supports interrupt events.     */
    PLATFORM_DEVICE_CAP_DMA      = 1u << 4,  /* Supports DMA transfers.        */
    PLATFORM_DEVICE_CAP_SLEEP    = 1u << 5,  /* Supports low-power sleep.      */
    PLATFORM_DEVICE_CAP_WAKEUP   = 1u << 6,  /* Supports wake-up from sleep.   */
    PLATFORM_DEVICE_CAP_PERIODIC = 1u << 7,  /* Supports periodic processing.   */
} platform_device_cap_t;

/* Common management face of every platform device. */
typedef struct
{
    platform_object_t object;         /* Common identity + lifecycle.      */
    platform_device_class_t dev_class;/* Device class.                     */
    uint32_t caps;                    /* Static IO capability flags.        */
} platform_device_t;

/**
 * @brief Initializes the common management fields of a platform device.
 *
 * Steps:
 *  1. Initializes the device object identity as PLATFORM_OBJECT_DEVICE.
 *  2. Binds lifecycle to the embedded platform_object_t.
 *  3. Records the device class and static capability flags.
 *
 * @param[in] p_dev       : Pointer to the platform device target.
 * @param[in] p_name      : Pointer to the device name string.
 * @param[in] dev_class   : Smart watch device class.
 * @param[in] caps        : Static IO capability flags.
 * @param[in] p_self      : Pointer to the concrete device object.
 * @param[in] p_lifecycle : Pointer to the lifecycle callback table.
 *
 * @return platform_err_t : Status of the function.
 */
platform_err_t platform_device_init(platform_device_t *p_dev,
                                    const char *p_name,
                                    platform_device_class_t dev_class,
                                    uint32_t caps,
                                    void *p_self,
                                    const platform_lifecycle_ops_t *p_lifecycle);

#endif /* __PLATFORM_DEVICE_H__ */
