/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file app_system.c
 *
 * @par dependencies
 * - app_system.h
 * - service_manager.h
 *
 * @brief Implement the app runtime state interface.
 *
 * Processing flow:
 *
 * 1. Maintains the app runtime state.
 * 2. Validates state values at the app boundary.
 * 3. Dispatches service process while RUNNING.
 *
 * @version V1.0 2026-06-06
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//

#include "app_system.h"
#include "platform_def.h"
#include "service_log.h"
#include "service_manager.h"

//******************************** Variables ********************************//

static app_system_data_t g_app_system_data = {
    APP_SYSTEM_STATE_BOOT,
    0u,
    PLATFORM_ERR_OK
};

//******************************** Declaring ********************************//

static uint8_t app_system_state_is_valid(app_system_state_t state);

//******************************** Functions ********************************//

platform_err_t app_system_set_state(app_system_state_t state)
{
    if (0u == app_system_state_is_valid(state))
    {
        return PLATFORM_ERR_PARAM;
    }

    g_app_system_data.state = state;

    return PLATFORM_ERR_OK;
}

platform_err_t app_system_get_state(app_system_state_t *p_state)
{
    if (NULL == p_state)
    {
        return PLATFORM_ERR_PARAM;
    }

    *p_state = g_app_system_data.state;

    return PLATFORM_ERR_OK;
}

platform_err_t app_system_process(void)
{
    platform_err_t ret = PLATFORM_ERR_OK;

    if (APP_SYSTEM_STATE_RUNNING != g_app_system_data.state)
    {
        return PLATFORM_ERR_OK;
    }

    ret = service_manager_process();
    if (PLATFORM_ERR_OK != ret)
    {
        app_system_record_error(ret);
        return ret;
    }

    g_app_system_data.process_count++;
    if (0u == (g_app_system_data.process_count % 100u))
    {
        ret = service_log_write(SERVICE_LOG_LEVEL_DEBUG,
                                "app_system heartbeat");
        if (PLATFORM_ERR_OK != ret)
        {
            app_system_record_error(ret);
            return ret;
        }
    }

    return PLATFORM_ERR_OK;
}

platform_err_t app_system_get_data(app_system_data_t *p_out)
{
    if (NULL == p_out)
    {
        return PLATFORM_ERR_PARAM;
    }

    *p_out = g_app_system_data;

    return PLATFORM_ERR_OK;
}

void app_system_record_error(platform_err_t error_code)
{
    g_app_system_data.last_error = error_code;
    if (PLATFORM_ERR_OK != error_code)
    {
        g_app_system_data.state = APP_SYSTEM_STATE_ERROR;
    }
}

static uint8_t app_system_state_is_valid(app_system_state_t state)
{
    return (APP_SYSTEM_STATE_MAX_COUNT > state) ? 1u : 0u;
}
