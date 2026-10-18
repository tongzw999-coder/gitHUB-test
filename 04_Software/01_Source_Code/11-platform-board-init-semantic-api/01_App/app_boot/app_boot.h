/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file app_boot.h
 *
 * @par dependencies
 * - app_system.h
 * - platform_version.h
 *
 * @author Codex (AI-assisted update for EternalChip)
 *
 * @brief Provide the app boot orchestration entry.
 *
 * Processing flow:
 *
 * 1. Bootstraps logging, initializes Board directly, then prepares services.
 * 2. Starts runtime services in a stable order.
 * 3. Records a small boot snapshot for debugging and diagnosis.
 *
 * @version V1.0 2026-06-06
 * @version V1.1 2026-09-13 Coding-standard documentation and formatting.
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#ifndef __APP_BOOT_H__
#define __APP_BOOT_H__

//******************************** Includes *********************************//

#include "app_system.h"
#include "platform_version.h"

//******************************** Declaring ********************************//

typedef struct
{
    uint8_t initialized;
    uint32_t service_count;
    app_system_state_t app_state;
    platform_version_t version;
} app_boot_data_t;

//******************************** Functions ********************************//

/**
 * @brief Initialize logging, Board, and services, then start runtime.
 *
 * @param None.
 * @retval PLATFORM_ERR_OK Boot completed or was already initialized.
 * @return Original error from the first unsuccessful boot operation.
 * @note Board failure prevents subsequent service initialization and start.
 */
platform_err_t app_boot_init(void);

/**
 * @brief Refresh and copy the current boot snapshot.
 *
 * @param[out] p_out Receives the snapshot on success; must not be NULL.
 * @retval PLATFORM_ERR_OK Snapshot copied successfully.
 * @retval PLATFORM_ERR_PARAM The output pointer is NULL.
 * @return Original error from the state or version query on failure.
 */
platform_err_t app_boot_get_data(app_boot_data_t *p_out);

#endif /* __APP_BOOT_H__ */
