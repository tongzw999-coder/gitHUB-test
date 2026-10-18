/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file plat_gpio.h
 *
 * @brief Provide platform GPIO abstraction APIs.
 *
 *****************************************************************************/

#ifndef PLAT_GPIO_H
#define PLAT_GPIO_H

//******************************** Includes *********************************//

#include "platform_def.h"
#include "platform_error.h"
#include "platform_mcu_config.h"

//******************************** Declaring ********************************//

typedef enum
{
    PLAT_GPIO_LEVEL_LOW = 0,
    PLAT_GPIO_LEVEL_HIGH,

    PLAT_GPIO_LEVEL_RESERVED = 0x7FFFFFFF
} plat_gpio_level_t;

/* Access a configured platform GPIO resource by logical ID. */
platform_err_t plat_gpio_write(plat_gpio_id_t gpio_id,
                               plat_gpio_level_t level);
platform_err_t plat_gpio_read(plat_gpio_id_t gpio_id,
                              plat_gpio_level_t *p_level);

#endif /* PLAT_GPIO_H */
