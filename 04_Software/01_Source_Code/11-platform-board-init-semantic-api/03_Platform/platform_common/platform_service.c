/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file platform_service.c
 *
 * @par dependencies
 * - platform_def.h
 * - platform_service.h
 *
 * @author Jack | R&D Dept. | EternalChip
 *
 * @brief Implement the common platform service base object.
 *
 * Processing flow:
 *
 * 1. Build the object identity of a platform service.
 * 2. Record the service class.
 * 3. Bind cfg/ctx/data/ops for the four-tuple model.
 *
 * @version V1.1 2026-05-31
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//

#include "platform_def.h"
#include "platform_service.h"

//******************************** Functions ********************************//

/**
 * @brief Initializes a platform service without data and ops slots.
 *
 * Steps:
 *  1. Delegates to the full four-tuple initializer.
 *  2. Leaves data and ops slots empty (NULL).
 *
 * @param[in] p_svc         : Pointer to the platform service target.
 * @param[in] p_name        : Pointer to the service name string.
 * @param[in] service_class : Smart watch service class.
 * @param[in] p_cfg         : Pointer to the static configuration.
 * @param[in] p_ctx         : Pointer to the runtime context.
 * @param[in] p_lifecycle   : Pointer to the lifecycle callback table.
 *
 * @return platform_err_t : Status of the function.
 */
platform_err_t platform_service_init(platform_service_t *p_svc,
                                     const char *p_name,
                                     platform_service_class_t service_class,
                                     const void *p_cfg,
                                     void *p_ctx,
                                     const platform_lifecycle_ops_t *p_lifecycle)
{
    return platform_service_model_init(p_svc,
                                       p_name,
                                       service_class,
                                       p_cfg,
                                       p_ctx,
                                       NULL,
                                       NULL,
                                       p_lifecycle);
}

/**
 * @brief Initializes a platform service with the full four-tuple model.
 *
 * Steps:
 *  1. Checks whether the service pointer is valid.
 *  2. Initializes the embedded object identity with lifecycle binding.
 *  3. Records service class and all four-tuple pointers.
 *
 * @param[in] p_svc         : Pointer to the platform service target.
 * @param[in] p_name        : Pointer to the service name string.
 * @param[in] service_class : Smart watch service class.
 * @param[in] p_cfg         : Pointer to the static configuration.
 * @param[in] p_ctx         : Pointer to the runtime context.
 * @param[in] p_data        : Pointer to the current service data.
 * @param[in] p_ops         : Pointer to service behavior operations.
 * @param[in] p_lifecycle   : Pointer to the lifecycle callback table.
 *
 * @return platform_err_t : Status of the function.
 */
platform_err_t platform_service_model_init(platform_service_t *p_svc,
                                           const char *p_name,
                                           platform_service_class_t service_class,
                                           const void *p_cfg,
                                           void *p_ctx,
                                           void *p_data,
                                           const void *p_ops,
                                           const platform_lifecycle_ops_t *p_lifecycle)
{
    platform_err_t ret = PLATFORM_ERR_OK;

    if (NULL == p_svc)
    {
        return PLATFORM_ERR_PARAM;
    }

    ret = platform_object_init(&p_svc->object,
                               p_name,
                               PLATFORM_OBJECT_SERVICE,
                               p_svc,
                               NULL,
                               p_lifecycle);
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    p_svc->service_class = service_class;
    p_svc->cfg = p_cfg;
    p_svc->ctx = p_ctx;
    p_svc->data = p_data;
    p_svc->ops = p_ops;

    return PLATFORM_ERR_OK;
}
