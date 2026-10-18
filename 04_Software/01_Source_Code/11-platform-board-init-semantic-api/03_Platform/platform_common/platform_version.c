/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file platform_version.c
 *
 * @par dependencies
 * - platform_version.h
 *
 * @brief Implement the typed platform version information interface.
 *
 * Processing flow:
 *
 * 1. Builds a static version snapshot from compile-time macros.
 * 2. Copies the snapshot for callers that need structured version fields.
 * 3. Exposes the version string for log and diagnosis service use.
 *
 * @version V1.0 2026-06-06
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//

#include "platform_def.h"
#include "platform_version.h"

//******************************** Variables ********************************//

static const platform_version_t g_platform_version = {
    (uint8_t)PLATFORM_VERSION_MAJOR,
    (uint8_t)PLATFORM_VERSION_MINOR,
    (uint8_t)PLATFORM_VERSION_PATCH,
    (uint8_t)PLATFORM_VERSION_BUILD,
    PLATFORM_PRODUCT_NAME,
    PLATFORM_VERSION_STRING,
    PLATFORM_GIT_HASH,
    PLATFORM_BUILD_DATE,
    PLATFORM_BUILD_TIME
};

//******************************** Functions ********************************//

platform_err_t platform_version_get(platform_version_t *p_out)
{
    if (NULL == p_out)
    {
        return PLATFORM_ERR_PARAM;
    }

    *p_out = g_platform_version;

    return PLATFORM_ERR_OK;
}

const char *platform_version_get_string(void)
{
    return g_platform_version.p_version_string;
}
