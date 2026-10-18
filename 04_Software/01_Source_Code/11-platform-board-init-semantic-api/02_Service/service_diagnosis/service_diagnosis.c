/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file service_diagnosis.c
 *
 * @par dependencies
 * - platform_def.h
 * - platform_version.h
 * - service_diagnosis.h
 * - service_log.h
 * - service_manager.h
 *
 * @brief Implement the diagnosis service shell.
 *
 * Processing flow:
 *
 * 1. Initializes a platform_service_t with cfg/ctx/data/ops.
 * 2. Starts and processes through service_manager lifecycle.
 * 3. Reads version, log, and service-manager snapshots.
 *
 * @version V1.0 2026-06-06
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//

#include "platform_def.h"
#include "platform_version.h"
#include "service_diagnosis.h"
#include "service_log.h"
#include "service_manager.h"

//******************************** Declaring ********************************//

typedef struct
{
    uint8_t initialized;
    uint8_t running;
} service_diagnosis_ctx_t;

static platform_err_t service_diagnosis_lifecycle_start(void *p_self);
static platform_err_t service_diagnosis_lifecycle_process(void *p_self);
static platform_err_t service_diagnosis_lifecycle_stop(void *p_self);
static void service_diagnosis_refresh_snapshot(void);

//******************************** Variables ********************************//

static service_diagnosis_cfg_t g_diagnosis_cfg;
static service_diagnosis_ctx_t g_diagnosis_ctx;
static service_diagnosis_data_t g_diagnosis_data;
static platform_service_t g_diagnosis_service;

static const service_diagnosis_ops_t g_diagnosis_ops = {
    service_diagnosis_start,
    service_diagnosis_process,
    service_diagnosis_stop,
    service_diagnosis_get_data,
    service_diagnosis_record_reset_reason,
    service_diagnosis_report_error
};

static const platform_lifecycle_ops_t g_diagnosis_lifecycle = {
    NULL,
    service_diagnosis_lifecycle_start,
    service_diagnosis_lifecycle_process,
    service_diagnosis_lifecycle_stop,
    NULL,
    NULL,
    NULL
};

//******************************** Functions ********************************//

platform_err_t service_diagnosis_init(
                                  const service_diagnosis_cfg_t *p_cfg)
{
    platform_err_t ret = PLATFORM_ERR_OK;
    service_diagnosis_data_t empty_data = {0};
    service_diagnosis_ctx_t empty_ctx = {0};

    if (NULL == p_cfg)
    {
        return PLATFORM_ERR_PARAM;
    }

    g_diagnosis_cfg = *p_cfg;
    g_diagnosis_ctx = empty_ctx;
    g_diagnosis_data = empty_data;

    g_diagnosis_ctx.initialized = 1u;
    g_diagnosis_data.health = SERVICE_DIAGNOSIS_HEALTH_UNKNOWN;
    g_diagnosis_data.valid = 1u;

    if (0u != g_diagnosis_cfg.version_snapshot_enable)
    {
        g_diagnosis_data.p_version_string = platform_version_get_string();
    }

    ret = platform_service_model_init(&g_diagnosis_service,
                                      "diagnosis_service",
                                      PLATFORM_SERVICE_CLASS_DIAGNOSIS,
                                      &g_diagnosis_cfg,
                                      &g_diagnosis_ctx,
                                      &g_diagnosis_data,
                                      &g_diagnosis_ops,
                                      &g_diagnosis_lifecycle);
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    return PLATFORM_ERR_OK;
}

platform_err_t service_diagnosis_start(void)
{
    if (0u == g_diagnosis_ctx.initialized)
    {
        return PLATFORM_ERR_NOT_INITIALIZED;
    }

    g_diagnosis_ctx.running = 1u;
    g_diagnosis_data.boot_count++;
    g_diagnosis_data.running = 1u;
    g_diagnosis_data.health = SERVICE_DIAGNOSIS_HEALTH_OK;
    g_diagnosis_data.valid = 1u;
    service_diagnosis_refresh_snapshot();

    return service_log_write(SERVICE_LOG_LEVEL_INFO,
                             "service_diagnosis started");
}

platform_err_t service_diagnosis_process(void)
{
    if (0u == g_diagnosis_ctx.running)
    {
        return PLATFORM_ERR_NOT_INITIALIZED;
    }

    g_diagnosis_data.process_count++;
    service_diagnosis_refresh_snapshot();

    return PLATFORM_ERR_OK;
}

platform_err_t service_diagnosis_stop(void)
{
    g_diagnosis_ctx.running = 0u;
    g_diagnosis_data.running = 0u;

    return PLATFORM_ERR_OK;
}

platform_err_t service_diagnosis_get_data(
                                  service_diagnosis_data_t *p_data)
{
    if (NULL == p_data)
    {
        return PLATFORM_ERR_PARAM;
    }

    *p_data = g_diagnosis_data;

    return PLATFORM_ERR_OK;
}

platform_service_t *service_diagnosis_get_object(void)
{
    return &g_diagnosis_service;
}

void service_diagnosis_record_reset_reason(uint32_t reset_reason,
                                           uint32_t error_code)
{
    if (0u == g_diagnosis_cfg.reset_reason_enable)
    {
        return;
    }

    g_diagnosis_data.last_reset_reason = reset_reason;
    if (0u != error_code)
    {
        g_diagnosis_data.last_error_code = error_code;
    }
    if (0u != reset_reason)
    {
        g_diagnosis_data.health = SERVICE_DIAGNOSIS_HEALTH_WARNING;
    }
    g_diagnosis_data.valid = 1u;
}

void service_diagnosis_report_error(uint32_t error_code)
{
    if (0u == g_diagnosis_cfg.error_record_enable)
    {
        return;
    }

    g_diagnosis_data.last_error_code = error_code;
    g_diagnosis_data.health = SERVICE_DIAGNOSIS_HEALTH_ERROR;
    g_diagnosis_data.valid = 1u;
}

static platform_err_t service_diagnosis_lifecycle_start(void *p_self)
{
    (void)p_self;

    return service_diagnosis_start();
}

static platform_err_t service_diagnosis_lifecycle_process(void *p_self)
{
    (void)p_self;

    return service_diagnosis_process();
}

static platform_err_t service_diagnosis_lifecycle_stop(void *p_self)
{
    (void)p_self;

    return service_diagnosis_stop();
}

static void service_diagnosis_refresh_snapshot(void)
{
    service_log_data_t log_data;

    if (0u != g_diagnosis_cfg.log_snapshot_enable)
    {
        if (PLATFORM_ERR_OK == service_log_get_data(&log_data))
        {
            g_diagnosis_data.log_write_count = log_data.write_count;
            g_diagnosis_data.log_dropped_count = log_data.dropped_count;
            if ((0u != log_data.dropped_count) &&
                (SERVICE_DIAGNOSIS_HEALTH_ERROR != g_diagnosis_data.health))
            {
                g_diagnosis_data.health = SERVICE_DIAGNOSIS_HEALTH_WARNING;
            }
        }
    }

    if ((0u == g_diagnosis_data.last_error_code) &&
        (0u == g_diagnosis_data.last_reset_reason) &&
        (0u == g_diagnosis_data.log_dropped_count))
    {
        g_diagnosis_data.health = SERVICE_DIAGNOSIS_HEALTH_OK;
    }

    if (0u != g_diagnosis_cfg.version_snapshot_enable)
    {
        g_diagnosis_data.p_version_string = platform_version_get_string();
    }

    g_diagnosis_data.service_count = service_manager_count();
    g_diagnosis_data.running = g_diagnosis_ctx.running;
    g_diagnosis_data.valid = 1u;
}
