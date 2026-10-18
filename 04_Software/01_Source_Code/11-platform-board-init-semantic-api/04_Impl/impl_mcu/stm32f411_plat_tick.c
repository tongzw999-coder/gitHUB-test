/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file stm32f411_plat_tick.c
 *
 * @brief Implement platform tick APIs on STM32F411 HAL.
 *
 *****************************************************************************/

//******************************** Includes *********************************//

#include "plat_tick.h"

#include "stm32f4xx_hal.h"

//******************************** Functions ********************************//

uint32_t plat_tick_get_ms(void)
{
    return (uint32_t)HAL_GetTick();
}

void plat_delay_ms(uint32_t ms)
{
    if (0u == ms)
    {
        return;
    }

    HAL_Delay(ms);
}
