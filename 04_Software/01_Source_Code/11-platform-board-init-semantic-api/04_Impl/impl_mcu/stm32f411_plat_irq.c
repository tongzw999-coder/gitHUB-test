/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file stm32f411_plat_irq.c
 *
 * @brief Implement platform IRQ APIs on STM32F411 HAL.
 *
 *****************************************************************************/

//******************************** Includes *********************************//

#include "plat_irq.h"

#include "stm32f4xx_hal.h"

//******************************** Defines **********************************//

#define STM32F411_IRQ_PRIORITY_MAX    ((uint8_t)((1u << __NVIC_PRIO_BITS) - 1u))

//******************************** Declaring ********************************//

static platform_err_t stm32f411_irq_get_num(plat_irq_id_t irq_id,
                                            IRQn_Type *p_irqn);

//******************************** Functions ********************************//

platform_err_t plat_irq_enable(plat_irq_id_t irq_id)
{
    IRQn_Type irqn = NonMaskableInt_IRQn;
    platform_err_t ret = stm32f411_irq_get_num(irq_id, &irqn);

    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    HAL_NVIC_EnableIRQ(irqn);

    return PLATFORM_ERR_OK;
}

platform_err_t plat_irq_disable(plat_irq_id_t irq_id)
{
    IRQn_Type irqn = NonMaskableInt_IRQn;
    platform_err_t ret = stm32f411_irq_get_num(irq_id, &irqn);

    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    HAL_NVIC_DisableIRQ(irqn);

    return PLATFORM_ERR_OK;
}

platform_err_t plat_irq_clear_pending(plat_irq_id_t irq_id)
{
    IRQn_Type irqn = NonMaskableInt_IRQn;
    platform_err_t ret = stm32f411_irq_get_num(irq_id, &irqn);

    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    HAL_NVIC_ClearPendingIRQ(irqn);

    return PLATFORM_ERR_OK;
}

platform_err_t plat_irq_set_priority(plat_irq_id_t irq_id,
                                     uint8_t priority)
{
    IRQn_Type irqn = NonMaskableInt_IRQn;
    platform_err_t ret = stm32f411_irq_get_num(irq_id, &irqn);

    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    if (STM32F411_IRQ_PRIORITY_MAX < priority)
    {
        return PLATFORM_ERR_PARAM;
    }

    HAL_NVIC_SetPriority(irqn, priority, 0u);

    return PLATFORM_ERR_OK;
}

platform_err_t plat_irq_is_enabled(plat_irq_id_t irq_id,
                                   platform_bool_t *p_enabled)
{
    IRQn_Type irqn = NonMaskableInt_IRQn;
    platform_err_t ret = PLATFORM_ERR_OK;

    if (NULL == p_enabled)
    {
        return PLATFORM_ERR_PARAM;
    }

    ret = stm32f411_irq_get_num(irq_id, &irqn);
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    *p_enabled = (0u != NVIC_GetEnableIRQ(irqn)) ?
                 PLATFORM_TRUE :
                 PLATFORM_FALSE;

    return PLATFORM_ERR_OK;
}

static platform_err_t stm32f411_irq_get_num(plat_irq_id_t irq_id,
                                            IRQn_Type *p_irqn)
{
    if (NULL == p_irqn)
    {
        return PLATFORM_ERR_PARAM;
    }

    switch (irq_id)
    {
    case PLAT_IRQ_ID_PA0_USER_KEY3_ISR:
        *p_irqn = EXTI0_IRQn;
        break;

    case PLAT_IRQ_ID_TIMER1_ISR:
        *p_irqn = TIM1_TRG_COM_TIM11_IRQn;
        break;

    case PLAT_IRQ_ID_USART1:
        *p_irqn = USART1_IRQn;
        break;

    case PLAT_IRQ_ID_ADC_CONVERSION:
        *p_irqn = ADC_IRQn;
        break;

    case PLAT_IRQ_ID_DMA_UART_DEBUG_RX:
        *p_irqn = DMA2_Stream2_IRQn;
        break;

    default:
        return PLATFORM_ERR_PARAM;
    }

    return PLATFORM_ERR_OK;
}
