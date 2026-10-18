/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file app_system.h
 *
 * @par dependencies
 * - platform_error.h
 *
 * @brief Provide the app runtime state interface.
 *
 * Processing flow:
 *
 * 1. App boot updates the coarse runtime state.
 * 2. App system dispatches board and service periodic processing.
 * 3. Diagnosis code can query the app runtime snapshot.
 *
 * @version V1.0 2026-06-06
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#ifndef __APP_SYSTEM_H__
#define __APP_SYSTEM_H__

//******************************** Includes *********************************//

#include "platform_error.h"

//******************************** Declaring ********************************//

typedef enum
{
    APP_SYSTEM_STATE_BOOT,
    APP_SYSTEM_STATE_INIT,
    APP_SYSTEM_STATE_READY,
    APP_SYSTEM_STATE_RUNNING,
    APP_SYSTEM_STATE_LOW_POWER,
    APP_SYSTEM_STATE_ERROR,
    APP_SYSTEM_STATE_MAX_COUNT,
    APP_SYSTEM_STATE_RESERVED = 0x7FFFFFFF
} app_system_state_t;

typedef struct
{
    app_system_state_t state;
    uint32_t process_count;
    platform_err_t last_error;
} app_system_data_t;

platform_err_t app_system_set_state(app_system_state_t state);
platform_err_t app_system_get_state(app_system_state_t *p_state);
platform_err_t app_system_process(void);
platform_err_t app_system_get_data(app_system_data_t *p_out);
void app_system_record_error(platform_err_t error_code);

#endif /* __APP_SYSTEM_H__ */
