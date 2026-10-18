/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file platform_version.h
 *
 * @par dependencies
 * - platform_error.h
 * - version_config.h
 *
 * @brief Provide a typed platform version information interface.
 *
 * Processing flow:
 *
 * 1. Reads firmware version macros from version_config.h.
 * 2. Exposes a stable platform_version_t snapshot.
 * 3. Lets boot and diagnosis code query version information uniformly.
 *
 * @version V1.0 2026-06-06
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#ifndef __PLATFORM_VERSION_H__
#define __PLATFORM_VERSION_H__

//******************************** Includes *********************************//

#include "platform_error.h"
#include "version_config.h"

//******************************** Declaring ********************************//

typedef struct
{
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
    uint8_t build;
    const char *p_product_name;
    const char *p_version_string;
    const char *p_git_hash;
    const char *p_build_date;
    const char *p_build_time;
} platform_version_t;

platform_err_t platform_version_get(platform_version_t *p_out);
const char *platform_version_get_string(void);

#endif /* __PLATFORM_VERSION_H__ */
