/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file device_manager.c
 *
 * @par dependencies
 * - device_manager.h
 * - platform_def.h
 *
 * @author Jack | R&D Dept. | EternalChip
 *
 * @brief Implement the platform device manager lifecycle interface.
 *
 * Processing flow:
 *
 * 1. Registers device objects created by board or demo code.
 * 2. Calls every device lifecycle through platform_object_t.
 * 3. Uses forward order for startup and reverse order for shutdown.
 *
 * @version V1.0 2026-06-03
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//

#include "device_manager.h"
#include "platform_def.h"

//******************************** Variables ********************************//

static platform_device_t *g_device_table[DEVICE_MANAGER_MAX_DEVICES];
static uint32_t g_device_count = 0u;
static uint8_t g_device_manager_ready = 0u;
static platform_object_t g_device_manager_object;

//******************************** Declaring ********************************//

static platform_err_t device_manager_ready(void);
static platform_err_t device_manager_check_duplicate(platform_device_t *p_dev);

//******************************** Functions ********************************//

platform_err_t device_manager_register(platform_device_t *p_dev)
{
    platform_err_t ret = PLATFORM_ERR_OK;

    if (NULL == p_dev)
    {
        return PLATFORM_ERR_PARAM;
    }

    ret = device_manager_ready();
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    if (0u == platform_object_is_valid(&p_dev->object,
                                       PLATFORM_OBJECT_DEVICE))
    {
        return PLATFORM_ERR_PARAM;
    }

    ret = device_manager_check_duplicate(p_dev);
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    if (g_device_count >= DEVICE_MANAGER_MAX_DEVICES)
    {
        return PLATFORM_ERR_NO_RESOURCE;
    }

    ret = platform_object_lifecycle_register(&p_dev->object,
                                             &g_device_manager_object);
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    g_device_table[g_device_count] = p_dev;
    g_device_count++;

    return PLATFORM_ERR_OK;
}

platform_err_t device_manager_init(void)
{
    uint32_t index = 0u;
    platform_err_t ret = PLATFORM_ERR_OK;

    ret = device_manager_ready();
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    for (index = 0u; index < g_device_count; index++)
    {
        ret = platform_object_lifecycle_run(&g_device_table[index]->object,
                                            PLATFORM_LIFECYCLE_STAGE_INIT);
        if (PLATFORM_ERR_OK != ret)
        {
            return ret;
        }
    }

    return PLATFORM_ERR_OK;
}

platform_err_t device_manager_start(void)
{
    uint32_t index = 0u;
    platform_err_t ret = PLATFORM_ERR_OK;

    ret = device_manager_ready();
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    for (index = 0u; index < g_device_count; index++)
    {
        ret = platform_object_lifecycle_run(&g_device_table[index]->object,
                                            PLATFORM_LIFECYCLE_STAGE_START);
        if (PLATFORM_ERR_OK != ret)
        {
            return ret;
        }
    }

    return PLATFORM_ERR_OK;
}

platform_err_t device_manager_process(void)
{
    uint32_t index = 0u;
    platform_err_t ret = PLATFORM_ERR_OK;

    ret = device_manager_ready();
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    for (index = 0u; index < g_device_count; index++)
    {
        ret = platform_object_lifecycle_run(&g_device_table[index]->object,
                                            PLATFORM_LIFECYCLE_STAGE_PROCESS);
        if (PLATFORM_ERR_OK != ret)
        {
            return ret;
        }
    }

    return PLATFORM_ERR_OK;
}

platform_err_t device_manager_stop(void)
{
    uint32_t index = 0u;
    platform_err_t ret = PLATFORM_ERR_OK;

    ret = device_manager_ready();
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    for (index = g_device_count; index > 0u; index--)
    {
        ret = platform_object_lifecycle_run(
                                &g_device_table[index - 1u]->object,
                                PLATFORM_LIFECYCLE_STAGE_STOP);
        if (PLATFORM_ERR_OK != ret)
        {
            return ret;
        }
    }

    return PLATFORM_ERR_OK;
}

platform_err_t device_manager_sleep(void)
{
    uint32_t index = 0u;
    platform_err_t ret = PLATFORM_ERR_OK;

    ret = device_manager_ready();
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    for (index = g_device_count; index > 0u; index--)
    {
        platform_device_t *p_dev = g_device_table[index - 1u];

        if (0u == (p_dev->caps & PLATFORM_DEVICE_CAP_SLEEP))
        {
            continue;
        }

        ret = platform_object_lifecycle_run(&p_dev->object,
                                            PLATFORM_LIFECYCLE_STAGE_SLEEP);
        if (PLATFORM_ERR_OK != ret)
        {
            return ret;
        }
    }

    return PLATFORM_ERR_OK;
}

platform_err_t device_manager_wakeup(void)
{
    uint32_t index = 0u;
    platform_err_t ret = PLATFORM_ERR_OK;

    ret = device_manager_ready();
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    for (index = 0u; index < g_device_count; index++)
    {
        platform_device_t *p_dev = g_device_table[index];

        if (0u == (p_dev->caps & PLATFORM_DEVICE_CAP_WAKEUP))
        {
            continue;
        }

        ret = platform_object_lifecycle_run(&p_dev->object,
                                            PLATFORM_LIFECYCLE_STAGE_WAKEUP);
        if (PLATFORM_ERR_OK != ret)
        {
            return ret;
        }
    }

    return PLATFORM_ERR_OK;
}

platform_err_t device_manager_deinit(void)
{
    uint32_t index = 0u;
    platform_err_t ret = PLATFORM_ERR_OK;

    ret = device_manager_ready();
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    for (index = g_device_count; index > 0u; index--)
    {
        ret = platform_object_lifecycle_run(
                                &g_device_table[index - 1u]->object,
                                PLATFORM_LIFECYCLE_STAGE_DEINIT);
        if (PLATFORM_ERR_OK != ret)
        {
            return ret;
        }

        g_device_table[index - 1u] = NULL;
    }

    g_device_count = 0u;

    return PLATFORM_ERR_OK;
}

uint32_t device_manager_count(void)
{
    return g_device_count;
}

platform_err_t device_manager_get_data(device_manager_data_t *p_out)
{
    platform_err_t ret = PLATFORM_ERR_OK;

    if (NULL == p_out)
    {
        return PLATFORM_ERR_PARAM;
    }

    ret = device_manager_ready();
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    p_out->device_count = g_device_count;
    p_out->ready = g_device_manager_ready;
    p_out->state = g_device_manager_object.state;

    return PLATFORM_ERR_OK;
}

static platform_err_t device_manager_ready(void)
{
    platform_err_t ret = PLATFORM_ERR_OK;

    if (0u != g_device_manager_ready)
    {
        return PLATFORM_ERR_OK;
    }

    ret = platform_object_init(&g_device_manager_object,
                               "device_manager",
                               PLATFORM_OBJECT_MANAGER,
                               &g_device_manager_object,
                               NULL,
                               NULL);
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    g_device_manager_ready = 1u;

    return PLATFORM_ERR_OK;
}

static platform_err_t device_manager_check_duplicate(platform_device_t *p_dev)
{
    uint32_t index = 0u;

    if (NULL == p_dev)
    {
        return PLATFORM_ERR_PARAM;
    }

    for (index = 0u; index < g_device_count; index++)
    {
        if (p_dev == g_device_table[index])
        {
            return PLATFORM_ERR_BUSY;
        }
    }

    return PLATFORM_ERR_OK;
}
