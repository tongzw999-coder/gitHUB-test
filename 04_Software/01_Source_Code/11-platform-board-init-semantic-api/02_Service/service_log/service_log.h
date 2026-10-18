/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file service_log.h
 *
 * @par dependencies
 * - platform_service.h
 *
 * @brief Provide the log service cfg/ctx/data/ops model.
 *
 * Processing flow:
 *
 * 1. Builds a platform service object for service_manager.
 * 2. Keeps log level and output policy in one service boundary.
 * 3. Records a small runtime data snapshot for diagnosis.
 *
 * @version V1.0 2026-06-06
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#ifndef __SERVICE_LOG_H__
#define __SERVICE_LOG_H__

//******************************** Includes *********************************//

#include "platform_service.h"

//******************************** Declaring ********************************//

typedef enum
{
    SERVICE_LOG_LEVEL_ASSERT,
    SERVICE_LOG_LEVEL_ERROR,
    SERVICE_LOG_LEVEL_WARN,
    SERVICE_LOG_LEVEL_INFO,
    SERVICE_LOG_LEVEL_DEBUG,
    SERVICE_LOG_LEVEL_VERBOSE,
    SERVICE_LOG_LEVEL_RESERVED = 0x7FFFFFFF
} service_log_level_t;

typedef struct
{
    service_log_level_t default_level;
    uint8_t output_enable;
    uint8_t async_enable;
    uint8_t storage_enable;
} service_log_cfg_t;

typedef struct
{
    service_log_level_t current_level;
    service_log_level_t last_level;
    uint32_t write_count;
    uint32_t dropped_count;
    uint32_t process_count;
    const char *p_last_message;
    uint8_t output_enabled;
    uint8_t async_enabled;
    uint8_t storage_enabled;
    uint8_t running;
    uint8_t valid;
} service_log_data_t;

typedef struct
{
    platform_err_t (*pf_start)(void);
    platform_err_t (*pf_process)(void);
    platform_err_t (*pf_stop)(void);
    platform_err_t (*pf_set_level)(service_log_level_t level);
    platform_err_t (*pf_write)(service_log_level_t level,
                               const char *p_message);
    platform_err_t (*pf_get_data)(service_log_data_t *p_data);
} service_log_ops_t;

platform_err_t service_log_init(const service_log_cfg_t *p_cfg);
platform_err_t service_log_start(void);
platform_err_t service_log_process(void);
platform_err_t service_log_stop(void);
platform_err_t service_log_set_level(service_log_level_t level);
platform_err_t service_log_write(service_log_level_t level,
                                 const char *p_message);
platform_err_t service_log_get_data(service_log_data_t *p_data);
platform_service_t *service_log_get_object(void);

#endif /* __SERVICE_LOG_H__ */
