/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file log_port.h
 *
 * @par dependencies
 * - platform_def.h
 *
 * @brief Provide the log output port.
 *
 * Processing flow:
 *
 * 1. service_log owns log policy and counters.
 * 2. log_port owns the concrete output backend.
 * 3. The RTT backend writes plain text to SEGGER RTT.
 *
 * @version V1.0 2026-06-06
 *
 *****************************************************************************/

#ifndef __LOG_PORT_H__
#define __LOG_PORT_H__

//******************************** Includes *********************************//

#include "platform_def.h"

//******************************** Defines **********************************//

#define LOG_PORT_LEVEL_ASSERT   0u
#define LOG_PORT_LEVEL_ERROR    1u
#define LOG_PORT_LEVEL_WARN     2u
#define LOG_PORT_LEVEL_INFO     3u
#define LOG_PORT_LEVEL_DEBUG    4u
#define LOG_PORT_LEVEL_VERBOSE  5u

//******************************** Declaring ********************************//

void log_port_init(void);
void log_port_deinit(void);
void log_port_set_level(uint8_t level);
void log_port_set_output_enabled(uint8_t enable);
void log_port_write(uint8_t level,
                    const char *p_tag,
                    const char *p_message);

#endif /* __LOG_PORT_H__ */
