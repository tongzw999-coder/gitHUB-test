/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file platform_object.h
 *
 * @par dependencies
 * - platform_lifecycle.h
 *
 * @author Jack | R&D Dept. | EternalChip
 *
 * @brief Provide the common platform object identity and lifecycle model.
 *
 * platform_object_t is the shared base identity for platform devices,
 * services, managers and application objects.
 *
 * Processing flow:
 *
 * 1. Concrete objects embed platform_object_t as their first field.
 * 2. Managers register objects and drive the lifecycle uniformly.
 * 3. Lifecycle is a cross-cutting platform ability, not a per-type concern.
 *
 * @version V1.1 2026-05-31
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#ifndef __PLATFORM_OBJECT_H__
#define __PLATFORM_OBJECT_H__

//******************************** Includes *********************************//

#include "platform_lifecycle.h"

//******************************** Defines **********************************//

#define PLATFORM_OBJECT_MAGIC   0x504F424Au

//******************************** Declaring ********************************//

/* Platform object high-level type. */
typedef enum
{
    PLATFORM_OBJECT_DEVICE,     /* Hardware or board capability object.    */
    PLATFORM_OBJECT_SERVICE,    /* Reusable service capability object.      */
    PLATFORM_OBJECT_MANAGER,    /* Manager object for object collection.    */
    PLATFORM_OBJECT_APP         /* Application-level object.                */
} platform_object_type_t;

/* Platform object lifecycle state record. */
typedef enum
{
    PLATFORM_OBJECT_CREATED,      /* Object identity has been created.      */
    PLATFORM_OBJECT_REGISTERED,   /* Object registered with its manager.    */
    PLATFORM_OBJECT_INITIALIZED,  /* Object resource has been initialized.  */
    PLATFORM_OBJECT_STARTED,      /* Object has entered the running phase.  */
    PLATFORM_OBJECT_STOPPED,      /* Object has stopped normal operation.   */
    PLATFORM_OBJECT_DEINITIALIZED,/* Object has released all resources.     */
    PLATFORM_OBJECT_ERROR         /* Object entered error state.            */
} platform_object_state_t;

/* Common identity header of all platform objects. */
typedef struct
{
    uint32_t magic;                          /* Runtime identity check value.   */
    const char *name;                        /* Object name, such as display0. */
    platform_object_type_t type;             /* Object high-level type.        */
    platform_object_state_t state;           /* Object current lifecycle state.*/
    uint32_t flags;                          /* Reserved extension flags.      */
    void *p_self;                            /* Pointer to concrete owner.     */
    void *p_parent;                          /* Pointer to parent or manager.  */
    const platform_lifecycle_ops_t *p_lifecycle; /* Lifecycle callback table.  */
    void *user_data;                         /* User extension pointer.        */
} platform_object_t;

/**
 * @brief Initializes the common identity and lifecycle binding of a platform object.
 *
 * Steps:
 *  1. Checks whether the object pointer is valid.
 *  2. Writes the object magic, name, type and initial state.
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
                                    const platform_lifecycle_ops_t *p_lifecycle);

/**
 * @brief Updates the lifecycle state record of a platform object.
 *
 * @param[in] p_obj : Pointer to the platform object target.
 * @param[in] state : New object lifecycle state.
 *
 * @return platform_err_t : Status of the function.
 */
platform_err_t platform_object_set_state(platform_object_t *p_obj,
                                         platform_object_state_t state);

/**
 * @brief Updates the parent pointer of a platform object.
 *
 * @param[in] p_obj    : Pointer to the platform object target.
 * @param[in] p_parent : Pointer to the parent or manager object.
 *
 * @return platform_err_t : Status of the function.
 */
platform_err_t platform_object_set_parent(platform_object_t *p_obj,
                                          void *p_parent);

/**
 * @brief Checks whether a platform object pointer matches the expected type.
 *
 * @param[in] p_obj : Pointer to the platform object target.
 * @param[in] type  : Expected platform object high-level type.
 *
 * @return bool_t : TRUE if the object pointer is valid.
 */
bool_t platform_object_is_valid(const platform_object_t *p_obj,
                                platform_object_type_t type);

/**
 * @brief Moves the object into registered state.
 *
 * @param[in] p_obj    : Pointer to the platform object target.
 * @param[in] p_parent : Pointer to the parent or manager object.
 *
 * @return platform_err_t : Status of the function.
 */
platform_err_t platform_object_lifecycle_register(platform_object_t *p_obj,
                                                  void *p_parent);

/**
 * @brief Executes one lifecycle stage through the object's lifecycle table.
 *
 * @param[in] p_obj : Pointer to the platform object target.
 * @param[in] stage : Lifecycle stage to execute.
 *
 * @return platform_err_t : Status of the function.
 */
platform_err_t platform_object_lifecycle_run(platform_object_t *p_obj,
                                             platform_lifecycle_stage_t stage);

platform_err_t platform_object_lifecycle_init(platform_object_t *p_obj);
platform_err_t platform_object_lifecycle_start(platform_object_t *p_obj);
platform_err_t platform_object_lifecycle_process(platform_object_t *p_obj);
platform_err_t platform_object_lifecycle_stop(platform_object_t *p_obj);
platform_err_t platform_object_lifecycle_sleep(platform_object_t *p_obj);
platform_err_t platform_object_lifecycle_wakeup(platform_object_t *p_obj);
platform_err_t platform_object_lifecycle_deinit(platform_object_t *p_obj);

#endif /* __PLATFORM_OBJECT_H__ */
