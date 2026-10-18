/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file platform_device.c
 *
 * @par dependencies
 * - platform_def.h
 * - platform_device.h
 *
 * @author Jack | R&D Dept. | EternalChip
 *
 * @brief Implement the common platform device base object.
 *
 * Processing flow:
 *
 * 1. Build the object identity of a platform device.
 * 2. Record device class and IO capabilities.
 *
 * @version V1.1 2026-05-31
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//

#include "platform_def.h"
#include "platform_device.h"

//******************************** Functions ********************************//

/**
 * @brief Initializes the common management fields of a platform device.
 *
 * Steps:
 *  1. Checks whether the device pointer is valid.
 *  2. Initializes the embedded object identity with lifecycle binding.
 *  3. Records device class and IO capability flags.
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
                                    const platform_lifecycle_ops_t *p_lifecycle)
{
    platform_err_t ret = PLATFORM_ERR_OK;

    if (NULL == p_dev)
    {
        return PLATFORM_ERR_PARAM;
    }

    if (NULL == p_self)
    {
        p_self = p_dev;
    }

    ret = platform_object_init(&p_dev->object,
                               p_name,
                               PLATFORM_OBJECT_DEVICE,
                               p_self,
                               NULL,
                               p_lifecycle);
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    p_dev->dev_class = dev_class;
    p_dev->caps = caps;

    return PLATFORM_ERR_OK;
}
