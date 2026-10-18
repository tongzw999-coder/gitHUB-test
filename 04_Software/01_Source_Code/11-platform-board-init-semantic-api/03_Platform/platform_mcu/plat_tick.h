/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file plat_tick.h
 *
 * @brief Provide platform tick and blocking delay abstraction APIs.
 *
 *****************************************************************************/

#ifndef PLAT_TICK_H
#define PLAT_TICK_H

//******************************** Includes *********************************//

#include "platform_type.h"

//******************************** Declaring ********************************//

/**
 * @brief Get platform tick in milliseconds.
 *
 * @return Current platform tick in ms.
 *
 * @note The tick value is a 32-bit counter and will wrap around. Use
 *       subtraction to calculate elapsed time instead of comparing absolute
 *       tick values.
 */
uint32_t plat_tick_get_ms(void);

/**
 * @brief Calculate elapsed time from a start tick.
 *
 * @param start_ms Start tick value returned by plat_tick_get_ms().
 *
 * @return Elapsed time in ms.
 *
 * @note This helper is safe when the 32-bit tick counter wraps around.
 */
static inline uint32_t plat_tick_elapsed_ms(uint32_t start_ms)
{
    return (uint32_t)(plat_tick_get_ms() - start_ms);
}

/**
 * @brief Blocking delay in milliseconds.
 *
 * @param ms Delay time in ms.
 *
 * @note This function is a blocking MCU delay. Do not call it from ISR context.
 *       In an RTOS task, prefer OS delay/sleep for long waits.
 */
void plat_delay_ms(uint32_t ms);

#endif /* PLAT_TICK_H */
