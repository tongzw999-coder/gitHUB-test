/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file service_diagnosis.h
 *
 * @par dependencies
 * - platform_service.h
 *
 * @brief Provide the diagnosis service cfg/ctx/data/ops model.
 *
 * Processing flow:
 *
 * 1. Builds a platform service object for service_manager.
 * 2. Tracks a compact boot and health snapshot.
 * 3. Records reset and error information for later troubleshooting.
 *
 * @version V1.0 2026-06-06
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#ifndef __SERVICE_DIAGNOSIS_H__
#define __SERVICE_DIAGNOSIS_H__

//******************************** Includes *********************************//

#include "platform_service.h"

//******************************** Declaring ********************************//

typedef enum
{
    SERVICE_DIAGNOSIS_HEALTH_UNKNOWN,
    SERVICE_DIAGNOSIS_HEALTH_OK,
    SERVICE_DIAGNOSIS_HEALTH_WARNING,
    SERVICE_DIAGNOSIS_HEALTH_ERROR,
    SERVICE_DIAGNOSIS_HEALTH_RESERVED = 0x7FFFFFFF
} service_diagnosis_health_t;

typedef struct
{
    uint8_t reset_reason_enable;
    uint8_t error_record_enable;
    uint8_t log_snapshot_enable;
    uint8_t version_snapshot_enable;
} service_diagnosis_cfg_t;

typedef struct
{
    service_diagnosis_health_t health;
    uint32_t boot_count;
    uint32_t process_count;
    uint32_t last_reset_reason;
    uint32_t last_error_code;
    uint32_t log_write_count;
    uint32_t log_dropped_count;
    uint32_t service_count;
    const char *p_version_string;
    uint8_t running;
    uint8_t valid;
} service_diagnosis_data_t;

typedef struct
{
    platform_err_t (*pf_start)(void);
    platform_err_t (*pf_process)(void);
    platform_err_t (*pf_stop)(void);
    platform_err_t (*pf_get_data)(service_diagnosis_data_t *p_data);
    void (*pf_record_reset_reason)(uint32_t reset_reason,
                                   uint32_t error_code);
    void (*pf_report_error)(uint32_t error_code);
} service_diagnosis_ops_t;

platform_err_t service_diagnosis_init(
                                  const service_diagnosis_cfg_t *p_cfg);
platform_err_t service_diagnosis_start(void);
platform_err_t service_diagnosis_process(void);
platform_err_t service_diagnosis_stop(void);
platform_err_t service_diagnosis_get_data(
                                  service_diagnosis_data_t *p_data);
platform_service_t *service_diagnosis_get_object(void);
void service_diagnosis_record_reset_reason(uint32_t reset_reason,
                                           uint32_t error_code);
void service_diagnosis_report_error(uint32_t error_code);

#endif /* __SERVICE_DIAGNOSIS_H__ */
