/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file service_manager.h
 *
 * @par dependencies
 * - platform_service.h
 *
 * @author Jack | R&D Dept. | EternalChip
 *
 * @brief Provide the platform service manager lifecycle interface.
 *
 * Processing flow:
 *
 * 1. Registers platform service objects.
 * 2. Drives init/start/process/stop/deinit through platform_object_t.
 * 3. Keeps service startup and shutdown order explicit.
 *
 * @version V1.0 2026-06-03
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#ifndef __SERVICE_MANAGER_H__
#define __SERVICE_MANAGER_H__

//******************************** Includes *********************************//

#include "platform_service.h"

//******************************** Defines **********************************//

#ifndef SERVICE_MANAGER_MAX_SERVICES
#define SERVICE_MANAGER_MAX_SERVICES  16u
#endif

//******************************** Declaring ********************************//

typedef struct
{
    uint32_t service_count;
    platform_object_state_t state;
    uint8_t ready;
} service_manager_data_t;

platform_err_t service_manager_register(platform_service_t *p_svc);
platform_err_t service_manager_init(void);
platform_err_t service_manager_start(void);
platform_err_t service_manager_process(void);
platform_err_t service_manager_stop(void);
platform_err_t service_manager_deinit(void);
uint32_t service_manager_count(void);
platform_err_t service_manager_get_data(service_manager_data_t *p_out);

#endif /* __SERVICE_MANAGER_H__ */
