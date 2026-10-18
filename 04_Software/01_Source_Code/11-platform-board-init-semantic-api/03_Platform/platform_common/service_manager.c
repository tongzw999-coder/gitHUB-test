/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file service_manager.c
 *
 * @par dependencies
 * - platform_def.h
 * - service_manager.h
 *
 * @author Jack | R&D Dept. | EternalChip
 *
 * @brief Implement the platform service manager lifecycle interface.
 *
 * Processing flow:
 *
 * 1. Registers service objects created by service modules or demo code.
 * 2. Calls every service lifecycle through platform_object_t.
 * 3. Uses forward order for startup and reverse order for shutdown.
 *
 * @version V1.0 2026-06-03
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//

#include "platform_def.h"
#include "service_manager.h"

//******************************** Variables ********************************//

static platform_service_t *g_service_table[SERVICE_MANAGER_MAX_SERVICES];
static uint32_t g_service_count = 0u;
static uint8_t g_service_manager_ready = 0u;
static platform_object_t g_service_manager_object;

//******************************** Declaring ********************************//

static platform_err_t service_manager_ready(void);
static platform_err_t service_manager_check_duplicate(platform_service_t *p_svc);

//******************************** Functions ********************************//

platform_err_t service_manager_register(platform_service_t *p_svc)
{
    platform_err_t ret = PLATFORM_ERR_OK;

    if (NULL == p_svc)
    {
        return PLATFORM_ERR_PARAM;
    }

    ret = service_manager_ready();
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    if (0u == platform_object_is_valid(&p_svc->object,
                                       PLATFORM_OBJECT_SERVICE))
    {
        return PLATFORM_ERR_PARAM;
    }

    ret = service_manager_check_duplicate(p_svc);
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    if (g_service_count >= SERVICE_MANAGER_MAX_SERVICES)
    {
        return PLATFORM_ERR_NO_RESOURCE;
    }

    ret = platform_object_lifecycle_register(&p_svc->object,
                                             &g_service_manager_object);
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    g_service_table[g_service_count] = p_svc;
    g_service_count++;

    return PLATFORM_ERR_OK;
}

platform_err_t service_manager_init(void)
{
    uint32_t index = 0u;
    platform_err_t ret = PLATFORM_ERR_OK;

    ret = service_manager_ready();
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    for (index = 0u; index < g_service_count; index++)
    {
        ret = platform_object_lifecycle_run(&g_service_table[index]->object,
                                            PLATFORM_LIFECYCLE_STAGE_INIT);
        if (PLATFORM_ERR_OK != ret)
        {
            return ret;
        }
    }

    return PLATFORM_ERR_OK;
}

platform_err_t service_manager_start(void)
{
    uint32_t index = 0u;
    platform_err_t ret = PLATFORM_ERR_OK;

    ret = service_manager_ready();
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    for (index = 0u; index < g_service_count; index++)
    {
        ret = platform_object_lifecycle_run(&g_service_table[index]->object,
                                            PLATFORM_LIFECYCLE_STAGE_START);
        if (PLATFORM_ERR_OK != ret)
        {
            return ret;
        }
    }

    return PLATFORM_ERR_OK;
}

platform_err_t service_manager_process(void)
{
    uint32_t index = 0u;
    platform_err_t ret = PLATFORM_ERR_OK;

    ret = service_manager_ready();
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    for (index = 0u; index < g_service_count; index++)
    {
        ret = platform_object_lifecycle_run(&g_service_table[index]->object,
                                            PLATFORM_LIFECYCLE_STAGE_PROCESS);
        if (PLATFORM_ERR_OK != ret)
        {
            return ret;
        }
    }

    return PLATFORM_ERR_OK;
}

platform_err_t service_manager_stop(void)
{
    uint32_t index = 0u;
    platform_err_t ret = PLATFORM_ERR_OK;

    ret = service_manager_ready();
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    for (index = g_service_count; index > 0u; index--)
    {
        ret = platform_object_lifecycle_run(
                                &g_service_table[index - 1u]->object,
                                PLATFORM_LIFECYCLE_STAGE_STOP);
        if (PLATFORM_ERR_OK != ret)
        {
            return ret;
        }
    }

    return PLATFORM_ERR_OK;
}

platform_err_t service_manager_deinit(void)
{
    uint32_t index = 0u;
    platform_err_t ret = PLATFORM_ERR_OK;

    ret = service_manager_ready();
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    for (index = g_service_count; index > 0u; index--)
    {
        ret = platform_object_lifecycle_run(
                                &g_service_table[index - 1u]->object,
                                PLATFORM_LIFECYCLE_STAGE_DEINIT);
        if (PLATFORM_ERR_OK != ret)
        {
            return ret;
        }

        g_service_table[index - 1u] = NULL;
    }

    g_service_count = 0u;

    return PLATFORM_ERR_OK;
}

uint32_t service_manager_count(void)
{
    return g_service_count;
}

platform_err_t service_manager_get_data(service_manager_data_t *p_out)
{
    platform_err_t ret = PLATFORM_ERR_OK;

    if (NULL == p_out)
    {
        return PLATFORM_ERR_PARAM;
    }

    ret = service_manager_ready();
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    p_out->service_count = g_service_count;
    p_out->ready = g_service_manager_ready;
    p_out->state = g_service_manager_object.state;

    return PLATFORM_ERR_OK;
}

static platform_err_t service_manager_ready(void)
{
    platform_err_t ret = PLATFORM_ERR_OK;

    if (0u != g_service_manager_ready)
    {
        return PLATFORM_ERR_OK;
    }

    ret = platform_object_init(&g_service_manager_object,
                               "service_manager",
                               PLATFORM_OBJECT_MANAGER,
                               &g_service_manager_object,
                               NULL,
                               NULL);
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    g_service_manager_ready = 1u;

    return PLATFORM_ERR_OK;
}

static platform_err_t service_manager_check_duplicate(platform_service_t *p_svc)
{
    uint32_t index = 0u;

    if (NULL == p_svc)
    {
        return PLATFORM_ERR_PARAM;
    }

    for (index = 0u; index < g_service_count; index++)
    {
        if (p_svc == g_service_table[index])
        {
            return PLATFORM_ERR_BUSY;
        }
    }

    return PLATFORM_ERR_OK;
}
