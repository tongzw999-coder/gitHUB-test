/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file service_log.c
 *
 * @par dependencies
 * - platform_def.h
 * - service_log.h
 *
 * @brief Implement the log service shell.
 *
 * Processing flow:
 *
 * 1. Initializes a platform_service_t with cfg/ctx/data/ops.
 * 2. Starts, processes, and stops through service_manager lifecycle.
 * 3. Records log write statistics and forwards lines to log_port.
 *
 * @version V1.0 2026-06-06
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//

#include "platform_def.h"
#include "service_log.h"
#include "log_port.h"

//******************************** Declaring ********************************//

typedef struct
{
    uint8_t initialized;
    uint8_t running;
} service_log_ctx_t;

static platform_err_t service_log_lifecycle_start(void *p_self);
static platform_err_t service_log_lifecycle_process(void *p_self);
static platform_err_t service_log_lifecycle_stop(void *p_self);

//******************************** Variables ********************************//

static service_log_cfg_t g_log_cfg;
static service_log_ctx_t g_log_ctx;
static service_log_data_t g_log_data;
static platform_service_t g_log_service;

static const service_log_ops_t g_log_ops = {
    service_log_start,
    service_log_process,
    service_log_stop,
    service_log_set_level,
    service_log_write,
    service_log_get_data
};

static const platform_lifecycle_ops_t g_log_lifecycle = {
    NULL,
    service_log_lifecycle_start,
    service_log_lifecycle_process,
    service_log_lifecycle_stop,
    NULL,
    NULL,
    NULL
};

//******************************** Functions ********************************//

platform_err_t service_log_init(const service_log_cfg_t *p_cfg)
{
    platform_err_t ret = PLATFORM_ERR_OK;
    service_log_data_t empty_data = {0};
    service_log_ctx_t empty_ctx = {0};

    if (NULL == p_cfg)
    {
        return PLATFORM_ERR_PARAM;
    }

    g_log_cfg = *p_cfg;
    g_log_ctx = empty_ctx;
    g_log_data = empty_data;

    g_log_ctx.initialized = 1u;
    g_log_data.current_level = p_cfg->default_level;
    g_log_data.last_level = p_cfg->default_level;
    g_log_data.output_enabled = p_cfg->output_enable;
    g_log_data.async_enabled = p_cfg->async_enable;
    g_log_data.storage_enabled = p_cfg->storage_enable;
    g_log_data.valid = 1u;

    log_port_init();
    log_port_set_output_enabled(g_log_data.output_enabled);
    log_port_set_level((uint8_t)g_log_data.current_level);

    ret = platform_service_model_init(&g_log_service,
                                      "log_service",
                                      PLATFORM_SERVICE_CLASS_LOG,
                                      &g_log_cfg,
                                      &g_log_ctx,
                                      &g_log_data,
                                      &g_log_ops,
                                      &g_log_lifecycle);
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    return PLATFORM_ERR_OK;
}

platform_err_t service_log_start(void)
{
    if (0u == g_log_ctx.initialized)
    {
        return PLATFORM_ERR_NOT_INITIALIZED;
    }

    g_log_ctx.running = 1u;
    g_log_data.running = 1u;
    g_log_data.output_enabled = g_log_cfg.output_enable;
    g_log_data.async_enabled = g_log_cfg.async_enable;
    g_log_data.storage_enabled = g_log_cfg.storage_enable;
    g_log_data.valid = 1u;

    log_port_set_output_enabled(g_log_data.output_enabled);
    log_port_set_level((uint8_t)g_log_data.current_level);

    return service_log_write(SERVICE_LOG_LEVEL_INFO,
                             "service_log started");
}

platform_err_t service_log_process(void)
{
    if (0u == g_log_ctx.running)
    {
        return PLATFORM_ERR_NOT_INITIALIZED;
    }

    g_log_data.process_count++;
    g_log_data.valid = 1u;

    return PLATFORM_ERR_OK;
}

platform_err_t service_log_stop(void)
{
    g_log_ctx.running = 0u;
    g_log_data.running = 0u;

    return PLATFORM_ERR_OK;
}

platform_err_t service_log_set_level(service_log_level_t level)
{
    if (0u == g_log_ctx.initialized)
    {
        return PLATFORM_ERR_NOT_INITIALIZED;
    }
    if (SERVICE_LOG_LEVEL_VERBOSE < level)
    {
        return PLATFORM_ERR_PARAM;
    }

    g_log_data.current_level = level;
    g_log_data.valid = 1u;

    return PLATFORM_ERR_OK;
}

platform_err_t service_log_write(service_log_level_t level,
                                 const char *p_message)
{
    if (0u == g_log_ctx.initialized)
    {
        return PLATFORM_ERR_NOT_INITIALIZED;
    }
    if ((SERVICE_LOG_LEVEL_VERBOSE < level) || (NULL == p_message))
    {
        return PLATFORM_ERR_PARAM;
    }

    if (level > g_log_data.current_level)
    {
        g_log_data.dropped_count++;
        return PLATFORM_ERR_OK;
    }

    g_log_data.write_count++;
    g_log_data.last_level = level;
    g_log_data.p_last_message = p_message;
    g_log_data.valid = 1u;

    log_port_write((uint8_t)level, "PLATFORM", p_message);

    return PLATFORM_ERR_OK;
}

platform_err_t service_log_get_data(service_log_data_t *p_data)
{
    if (NULL == p_data)
    {
        return PLATFORM_ERR_PARAM;
    }

    *p_data = g_log_data;

    return PLATFORM_ERR_OK;
}

platform_service_t *service_log_get_object(void)
{
    return &g_log_service;
}

static platform_err_t service_log_lifecycle_start(void *p_self)
{
    (void)p_self;

    return service_log_start();
}

static platform_err_t service_log_lifecycle_process(void *p_self)
{
    (void)p_self;

    return service_log_process();
}

static platform_err_t service_log_lifecycle_stop(void *p_self)
{
    (void)p_self;

    return service_log_stop();
}
