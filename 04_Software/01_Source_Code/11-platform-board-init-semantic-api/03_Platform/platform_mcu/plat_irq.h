/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file plat_irq.h
 *
 * @brief Provide platform IRQ abstraction APIs.
 *
 *****************************************************************************/

#ifndef PLAT_IRQ_H
#define PLAT_IRQ_H

//******************************** Includes *********************************//

#include "platform_def.h"
#include "platform_error.h"
#include "platform_mcu_config.h"

//******************************** Declaring ********************************//

/*
 * These APIs control the NVIC IRQ line mapped by plat_irq_id_t. They do not
 * configure GPIO EXTI, UART RXNE, DMA flags, or other peripheral IRQ sources.
 */
platform_err_t plat_irq_enable(plat_irq_id_t irq_id);
platform_err_t plat_irq_disable(plat_irq_id_t irq_id);

/*
 * Clear NVIC pending bit only. Peripheral pending flags must be cleared by the
 * corresponding GPIO/UART/DMA/Timer implementation.
 */
platform_err_t plat_irq_clear_pending(plat_irq_id_t irq_id);
platform_err_t plat_irq_set_priority(plat_irq_id_t irq_id,
                                     uint8_t priority);
platform_err_t plat_irq_is_enabled(plat_irq_id_t irq_id,
                                   platform_bool_t *p_enabled);

#endif /* PLAT_IRQ_H */
