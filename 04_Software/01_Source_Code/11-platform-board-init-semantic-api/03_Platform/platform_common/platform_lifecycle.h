/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file platform_lifecycle.h
 *
 * @par dependencies
 * - platform_error.h
 *
 * @author Jack | R&D Dept. | EternalChip
 *
 * @brief Provide the platform object lifecycle callback table.
 *
 * Processing flow:
 *
 * 1. The object owner fills this callback table.
 * 2. Managers drive the lifecycle uniformly through this interface.
 * 3. These callbacks describe object lifetime including sleep/wakeup.
 *
 * @version V1.1 2026-05-31
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#ifndef __PLATFORM_LIFECYCLE_H__
#define __PLATFORM_LIFECYCLE_H__

//******************************** Includes *********************************//

#include "platform_error.h"

//******************************** Declaring ********************************//

typedef struct
{
    platform_err_t (*init)(void *p_self);    /* Object initialization.       */
    platform_err_t (*start)(void *p_self);   /* Object start entry.          */
    platform_err_t (*process)(void *p_self); /* Optional period processing.  */
    platform_err_t (*stop)(void *p_self);    /* Object stop entry.           */
    platform_err_t (*sleep)(void *p_self);   /* Optional low-power entry.    */
    platform_err_t (*wakeup)(void *p_self);  /* Optional wake-up entry.      */
    platform_err_t (*deinit)(void *p_self);  /* Object resource release.     */
} platform_lifecycle_ops_t;

typedef enum
{
    PLATFORM_LIFECYCLE_STAGE_INIT,
    PLATFORM_LIFECYCLE_STAGE_START,
    PLATFORM_LIFECYCLE_STAGE_PROCESS,
    PLATFORM_LIFECYCLE_STAGE_STOP,
    PLATFORM_LIFECYCLE_STAGE_SLEEP,
    PLATFORM_LIFECYCLE_STAGE_WAKEUP,
    PLATFORM_LIFECYCLE_STAGE_DEINIT,
    PLATFORM_LIFECYCLE_STAGE_RESERVED = 0x7FFFFFFF
} platform_lifecycle_stage_t;

#endif /* __PLATFORM_LIFECYCLE_H__ */
