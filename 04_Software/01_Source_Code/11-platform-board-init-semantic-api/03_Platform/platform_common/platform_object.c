/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file platform_object.c
 *
 * @par dependencies
 * - platform_def.h
 * - platform_object.h
 *
 * @author Jack | R&D Dept. | EternalChip
 *
 * @brief Implement the common platform object identity and lifecycle model.
 *
 * Processing flow:
 *
 * 1. Create the object identity fields with lifecycle binding.
 * 2. Maintain the object lifecycle state record.
 * 3. Provide basic magic and type validation.
 *
 * @version V1.1 2026-05-31
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//

#include "platform_def.h"
#include "platform_object.h"

//******************************** Declaring ********************************//

static platform_err_t platform_object_run_hook(
                                    platform_object_t *p_obj,
                                    platform_err_t (*pf_hook)(void *p_self),
                                    platform_object_state_t success_state);

//******************************** Functions ********************************//

/**
 * @brief Initializes the common identity and lifecycle binding of a platform object.
 *
 * Steps:
 *  1. Checks whether the object pointer is valid.
 *  2. Writes the object magic, name, type and created state.
 *  3. Binds p_self, p_parent and lifecycle callback table.
 *  4. Clears extension flags and user data.
 *
 * @param[in] p_obj        : Pointer to the platform object target.
 * @param[in] p_name       : Pointer to the object name string.
 * @param[in] type         : Platform object high-level type.
 * @param[in] p_self       : Pointer to the concrete owner object.
 * @param[in] p_parent     : Pointer to the parent or manager object.
 * @param[in] p_lifecycle  : Pointer to the lifecycle callback table.
 *
 * @return platform_err_t : Status of the function.
 */
platform_err_t platform_object_init(platform_object_t *p_obj,
                                    const char *p_name,
                                    platform_object_type_t type,
                                    void *p_self,
                                    void *p_parent,
                                    const platform_lifecycle_ops_t *p_lifecycle)
{
    if (NULL == p_obj)
    {
        return PLATFORM_ERR_PARAM;
    }
    if (NULL == p_self)
    {
        p_self = p_obj;
    }

    p_obj->magic = PLATFORM_OBJECT_MAGIC;
    p_obj->name = p_name;
    p_obj->type = type;
    p_obj->state = PLATFORM_OBJECT_CREATED;
    p_obj->flags = 0u;
    p_obj->p_self = p_self;
    p_obj->p_parent = p_parent;
    p_obj->p_lifecycle = p_lifecycle;
    p_obj->user_data = NULL;

    return PLATFORM_ERR_OK;
}

/**
 * @brief Updates the lifecycle state record of a platform object.
 *
 * @param[in] p_obj : Pointer to the platform object target.
 * @param[in] state : New object lifecycle state.
 *
 * @return platform_err_t : Status of the function.
 */
platform_err_t platform_object_set_state(platform_object_t *p_obj,
                                         platform_object_state_t state)
{
    if (NULL == p_obj)
    {
        return PLATFORM_ERR_PARAM;
    }

    p_obj->state = state;

    return PLATFORM_ERR_OK;
}

/**
 * @brief Updates the parent pointer of a platform object.
 *
 * @param[in] p_obj    : Pointer to the platform object target.
 * @param[in] p_parent : Pointer to the parent or manager object.
 *
 * @return platform_err_t : Status of the function.
 */
platform_err_t platform_object_set_parent(platform_object_t *p_obj,
                                          void *p_parent)
{
    if (NULL == p_obj)
    {
        return PLATFORM_ERR_PARAM;
    }

    p_obj->p_parent = p_parent;

    return PLATFORM_ERR_OK;
}

/**
 * @brief Checks whether a platform object pointer matches the expected type.
 *
 * @param[in] p_obj : Pointer to the platform object target.
 * @param[in] type  : Expected platform object high-level type.
 *
 * @return bool_t : TRUE if the object pointer is valid.
 */
bool_t platform_object_is_valid(const platform_object_t *p_obj,
                                platform_object_type_t type)
{
    return (NULL != p_obj) &&
           (PLATFORM_OBJECT_MAGIC == p_obj->magic) &&
           (type == p_obj->type);
}

/**
 * @brief Moves the object into registered state.
 *
 * @param[in] p_obj    : Pointer to the platform object target.
 * @param[in] p_parent : Pointer to the parent or manager object.
 *
 * @return platform_err_t : Status of the function.
 */
platform_err_t platform_object_lifecycle_register(platform_object_t *p_obj,
                                                  void *p_parent)
{
    platform_err_t ret = PLATFORM_ERR_OK;

    if (NULL == p_obj)
    {
        return PLATFORM_ERR_PARAM;
    }

    ret = platform_object_set_parent(p_obj, p_parent);
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    return platform_object_set_state(p_obj, PLATFORM_OBJECT_REGISTERED);
}

/**
 * @brief Executes one lifecycle stage through the object's lifecycle table.
 *
 * @param[in] p_obj : Pointer to the platform object target.
 * @param[in] stage : Lifecycle stage to execute.
 *
 * @return platform_err_t : Status of the function.
 */
platform_err_t platform_object_lifecycle_run(platform_object_t *p_obj,
                                             platform_lifecycle_stage_t stage)
{
    if (NULL == p_obj)
    {
        return PLATFORM_ERR_PARAM;
    }

    switch (stage)
    {
        case PLATFORM_LIFECYCLE_STAGE_INIT:
            return platform_object_lifecycle_init(p_obj);

        case PLATFORM_LIFECYCLE_STAGE_START:
            return platform_object_lifecycle_start(p_obj);

        case PLATFORM_LIFECYCLE_STAGE_PROCESS:
            return platform_object_lifecycle_process(p_obj);

        case PLATFORM_LIFECYCLE_STAGE_STOP:
            return platform_object_lifecycle_stop(p_obj);

        case PLATFORM_LIFECYCLE_STAGE_SLEEP:
            return platform_object_lifecycle_sleep(p_obj);

        case PLATFORM_LIFECYCLE_STAGE_WAKEUP:
            return platform_object_lifecycle_wakeup(p_obj);

        case PLATFORM_LIFECYCLE_STAGE_DEINIT:
            return platform_object_lifecycle_deinit(p_obj);

        default:
            break;
    }

    return PLATFORM_ERR_PARAM;
}

platform_err_t platform_object_lifecycle_init(platform_object_t *p_obj)
{
    platform_err_t (*pf_hook)(void *p_self) = NULL;

    if (NULL == p_obj)
    {
        return PLATFORM_ERR_PARAM;
    }
    if ((PLATFORM_OBJECT_INITIALIZED == p_obj->state) ||
        (PLATFORM_OBJECT_STARTED == p_obj->state))
    {
        return PLATFORM_ERR_OK;
    }
    if (NULL != p_obj->p_lifecycle)
    {
        pf_hook = p_obj->p_lifecycle->init;
    }

    return platform_object_run_hook(p_obj,
                                    pf_hook,
                                    PLATFORM_OBJECT_INITIALIZED);
}

platform_err_t platform_object_lifecycle_start(platform_object_t *p_obj)
{
    platform_err_t (*pf_hook)(void *p_self) = NULL;

    if (NULL == p_obj)
    {
        return PLATFORM_ERR_PARAM;
    }
    if (PLATFORM_OBJECT_STARTED == p_obj->state)
    {
        return PLATFORM_ERR_OK;
    }
    if (NULL != p_obj->p_lifecycle)
    {
        pf_hook = p_obj->p_lifecycle->start;
    }

    return platform_object_run_hook(p_obj,
                                    pf_hook,
                                    PLATFORM_OBJECT_STARTED);
}

platform_err_t platform_object_lifecycle_process(platform_object_t *p_obj)
{
    platform_err_t ret = PLATFORM_ERR_OK;

    if (NULL == p_obj)
    {
        return PLATFORM_ERR_PARAM;
    }
    if (PLATFORM_OBJECT_STARTED != p_obj->state)
    {
        return PLATFORM_ERR_OK;
    }
    if ((NULL == p_obj->p_lifecycle) ||
        (NULL == p_obj->p_lifecycle->process))
    {
        return PLATFORM_ERR_OK;
    }

    ret = p_obj->p_lifecycle->process(p_obj->p_self);
    if (PLATFORM_ERR_OK != ret)
    {
        p_obj->state = PLATFORM_OBJECT_ERROR;
        return ret;
    }

    return PLATFORM_ERR_OK;
}

platform_err_t platform_object_lifecycle_stop(platform_object_t *p_obj)
{
    platform_err_t (*pf_hook)(void *p_self) = NULL;

    if (NULL == p_obj)
    {
        return PLATFORM_ERR_PARAM;
    }
    if (PLATFORM_OBJECT_STARTED != p_obj->state)
    {
        p_obj->state = PLATFORM_OBJECT_STOPPED;
        return PLATFORM_ERR_OK;
    }
    if (NULL != p_obj->p_lifecycle)
    {
        pf_hook = p_obj->p_lifecycle->stop;
    }

    return platform_object_run_hook(p_obj,
                                    pf_hook,
                                    PLATFORM_OBJECT_STOPPED);
}

platform_err_t platform_object_lifecycle_sleep(platform_object_t *p_obj)
{
    platform_err_t (*pf_hook)(void *p_self) = NULL;

    if (NULL == p_obj)
    {
        return PLATFORM_ERR_PARAM;
    }
    if (PLATFORM_OBJECT_STARTED != p_obj->state)
    {
        return PLATFORM_ERR_OK;
    }
    if ((NULL == p_obj->p_lifecycle) ||
        (NULL == p_obj->p_lifecycle->sleep))
    {
        return PLATFORM_ERR_OK;
    }

    pf_hook = p_obj->p_lifecycle->sleep;

    return platform_object_run_hook(p_obj,
                                    pf_hook,
                                    PLATFORM_OBJECT_STARTED);
}

platform_err_t platform_object_lifecycle_wakeup(platform_object_t *p_obj)
{
    platform_err_t (*pf_hook)(void *p_self) = NULL;

    if (NULL == p_obj)
    {
        return PLATFORM_ERR_PARAM;
    }
    if (PLATFORM_OBJECT_STARTED != p_obj->state)
    {
        return PLATFORM_ERR_OK;
    }
    if ((NULL == p_obj->p_lifecycle) ||
        (NULL == p_obj->p_lifecycle->wakeup))
    {
        return PLATFORM_ERR_OK;
    }

    pf_hook = p_obj->p_lifecycle->wakeup;

    return platform_object_run_hook(p_obj,
                                    pf_hook,
                                    PLATFORM_OBJECT_STARTED);
}

platform_err_t platform_object_lifecycle_deinit(platform_object_t *p_obj)
{
    platform_err_t (*pf_hook)(void *p_self) = NULL;

    if (NULL == p_obj)
    {
        return PLATFORM_ERR_PARAM;
    }
    if (NULL != p_obj->p_lifecycle)
    {
        pf_hook = p_obj->p_lifecycle->deinit;
    }

    return platform_object_run_hook(p_obj,
                                    pf_hook,
                                    PLATFORM_OBJECT_DEINITIALIZED);
}

/**
 * @brief Runs an optional lifecycle hook and updates object state.
 *
 * @param[in] p_obj         : Pointer to the platform object target.
 * @param[in] pf_hook       : Pointer to optional lifecycle hook.
 * @param[in] success_state : State written after successful execution.
 *
 * @return platform_err_t : Status of the function.
 */
static platform_err_t platform_object_run_hook(
                                    platform_object_t *p_obj,
                                    platform_err_t (*pf_hook)(void *p_self),
                                    platform_object_state_t success_state)
{
    platform_err_t ret = PLATFORM_ERR_OK;

    if (NULL == p_obj)
    {
        return PLATFORM_ERR_PARAM;
    }

    if (NULL != pf_hook)
    {
        ret = pf_hook(p_obj->p_self);
        if (PLATFORM_ERR_OK != ret)
        {
            p_obj->state = PLATFORM_OBJECT_ERROR;
            return ret;
        }
    }

    p_obj->state = success_state;

    return PLATFORM_ERR_OK;
}
