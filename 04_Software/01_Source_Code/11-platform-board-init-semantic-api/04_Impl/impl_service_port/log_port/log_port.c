/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file log_port.c
 *
 * @par dependencies
 * - log_port.h
 * - SEGGER_RTT.h
 *
 * @brief Implement the RTT log output port.
 *
 * Processing flow:
 *
 * 1. Initializes the RTT backend once.
 * 2. Keeps output enable and level policy in a tiny port state.
 * 3. Writes plain text log lines for boot and runtime diagnosis.
 *
 * @version V1.0 2026-06-06
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//

#include "log_port.h"

//******************************** Defines **********************************//

#ifndef LOG_PORT_BACKEND_RTT
#if defined(STM32F411xE) || defined(__CC_ARM) || defined(__ARMCC_VERSION)
#define LOG_PORT_BACKEND_RTT 1u
#else
#define LOG_PORT_BACKEND_RTT 0u
#endif
#endif

#if (0u != LOG_PORT_BACKEND_RTT)
#include "SEGGER_RTT.h"
#endif

#define LOG_PORT_CHANNEL_INDEX 0u

//******************************** Variables ********************************//

static uint8_t g_log_port_initialized;
static uint8_t g_log_port_output_enabled = 1u;
static uint8_t g_log_port_level = LOG_PORT_LEVEL_DEBUG;

static const char *const g_log_port_level_name[] = {
    "A",
    "E",
    "W",
    "I",
    "D",
    "V"
};

//******************************** Declaring ********************************//

static void log_port_backend_write(const char *p_text);
static const char *log_port_level_to_name(uint8_t level);

//******************************** Functions ********************************//

void log_port_init(void)
{
    if (0u != g_log_port_initialized)
    {
        return;
    }

#if (0u != LOG_PORT_BACKEND_RTT)
    SEGGER_RTT_Init();
#endif

    g_log_port_initialized = 1u;
    log_port_write(LOG_PORT_LEVEL_INFO,
                   "LOG",
                   "log_port rtt ready");
}

void log_port_deinit(void)
{
    g_log_port_initialized = 0u;
}

void log_port_set_level(uint8_t level)
{
    if (LOG_PORT_LEVEL_VERBOSE < level)
    {
        return;
    }

    g_log_port_level = level;
}

void log_port_set_output_enabled(uint8_t enable)
{
    g_log_port_output_enabled = (0u != enable) ? 1u : 0u;
}

void log_port_write(uint8_t level,
                    const char *p_tag,
                    const char *p_message)
{
    if ((NULL == p_tag) || (NULL == p_message))
    {
        return;
    }
    if (LOG_PORT_LEVEL_VERBOSE < level)
    {
        return;
    }
    if (level > g_log_port_level)
    {
        return;
    }
    if (0u == g_log_port_output_enabled)
    {
        return;
    }
    if (0u == g_log_port_initialized)
    {
        log_port_init();
    }

    log_port_backend_write(log_port_level_to_name(level));
    log_port_backend_write("/");
    log_port_backend_write(p_tag);
    log_port_backend_write(" ");
    log_port_backend_write(p_message);
    log_port_backend_write("\r\n");
}

static void log_port_backend_write(const char *p_text)
{
    if (NULL == p_text)
    {
        return;
    }

#if (0u != LOG_PORT_BACKEND_RTT)
    (void)SEGGER_RTT_WriteString((unsigned)LOG_PORT_CHANNEL_INDEX, p_text);
#else
    (void)p_text;
#endif
}

static const char *log_port_level_to_name(uint8_t level)
{
    if (LOG_PORT_LEVEL_VERBOSE < level)
    {
        return "?";
    }

    return g_log_port_level_name[level];
}
