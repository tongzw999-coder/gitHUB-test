/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file app_boot.c
 *
 * @par dependencies
 * - app_boot.h
 * - board_init.h
 * - board_control.h
 * - board_resource_config.h
 * - plat_gpio.h
 * - plat_tick.h
 * - platform_def.h
 * - service_diagnosis.h
 * - service_log.h
 * - service_manager.h
 *
 * @author Codex (AI-assisted update for EternalChip)
 *
 * @brief Implement the app boot orchestration entry.
 *
 * Processing flow:
 *
 * 1. Bootstraps log output, then applies supported Board default commands.
 * 2. Builds diagnosis, registers/initializes services, and starts runtime.
 * 3. Probes the MCU platform GPIO abstraction.
 * 4. Moves app system state through INIT, READY, and RUNNING.
 *
 * @version V1.0 2026-06-06
 * @version V1.1 2026-09-13 Coding-standard documentation and formatting.
 * @version V1.2 2026-10-11 Split Board startup and resolve key channels by map.
 * @version V1.3 2026-10-15 Request AHT21 supply and report pending integration.
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//

#include "app_boot.h"
#include "board_init.h"
#include "board_control.h"
#include "board_resource_config.h"
#include "plat_gpio.h"
#include "plat_tick.h"
#include "platform_def.h"
#include "service_diagnosis.h"
#include "service_log.h"
#include "service_manager.h"

//******************************** Variables ********************************//

static const service_log_cfg_t g_app_log_cfg =
{
    SERVICE_LOG_LEVEL_DEBUG,
    1u,
    0u,
    0u
};

static const service_diagnosis_cfg_t g_app_diagnosis_cfg =
{
    1u,
    1u,
    1u,
    1u
};

static app_boot_data_t g_app_boot_data =
{
    0u,
    0u,
    APP_SYSTEM_STATE_BOOT,
    {
        0u,
        0u,
        0u,
        0u,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    }
};

//******************************** Declaring ********************************//

static platform_err_t app_boot_services_init(void);
static platform_err_t app_boot_services_register(void);
static platform_err_t app_boot_runtime_start(void);
static platform_err_t app_boot_mcu_probe(void);
static platform_err_t app_boot_update_data(void);

//******************************** Functions ********************************//

/**
 * @brief Initialize Board and services, then start the application runtime.
 *
 * Steps:
 * 1. Enter INIT and bootstrap logging before Board initialization.
 * 2. Prepare and register services only after Board initialization succeeds.
 * 3. Start services and the MCU probe, then update the boot snapshot.
 *
 * @param None.
 * @retval PLATFORM_ERR_OK Boot completed or was already initialized.
 * @return Original failure from the first unsuccessful boot operation.
 * @note A failure log must not replace the original Board error.
 */
platform_err_t app_boot_init(void)
{
    platform_err_t ret = PLATFORM_ERR_OK;
    platform_err_t log_ret = PLATFORM_ERR_OK;

    if (0u != g_app_boot_data.initialized)
    {
        return PLATFORM_ERR_OK;
    }

    ret = app_system_set_state(APP_SYSTEM_STATE_INIT);
    if (PLATFORM_ERR_OK != ret)
    {
        app_system_record_error(ret);
        return ret;
    }

    /*
     * Log init enables the existing port and pre-start writes. Lifecycle
     * start remains with service_manager; no runtime service starts here.
     */
    ret = service_log_init(&g_app_log_cfg);
    if (PLATFORM_ERR_OK != ret)
    {
        app_system_record_error(ret);
        return ret;
    }

    /*
     * Startup: Core GPIO/tick -> log bootstrap -> Board -> services.
     * A partial Board init is NOT whole-board safety. Preserve its error and
     * stop before diagnosis/registration/start/probe or the READY state.
     */
    ret = board_init();
    if (PLATFORM_ERR_OK != ret)
    {
        app_system_record_error(ret);
        log_ret = service_log_write(SERVICE_LOG_LEVEL_ERROR,
                                    "board init failed");
        if (PLATFORM_ERR_OK != log_ret)
        {
            /* Preserve the original Board error if reporting also fails. */
            return ret;
        }
        return ret;
    }

    /* Early integration: local defaults are ready; Nordic protocol is TBD.
     * Pending supply control permits this local demo, not AHT measurement.
     */
    ret = board_aht21_power_enable(BOARD_SIGNAL_ACTIVE);
    if (PLATFORM_ERR_NOT_SUPPORTED == ret)
    {
        log_ret = service_log_write(SERVICE_LOG_LEVEL_WARN,
                                    "AHT21 power control pending: Nordic protocol TBD");
        if (PLATFORM_ERR_OK != log_ret)
        {
            app_system_record_error(log_ret);
            return log_ret;
        }
    }
    else if (PLATFORM_ERR_OK != ret)
    {
        app_system_record_error(ret);
        log_ret = service_log_write(SERVICE_LOG_LEVEL_ERROR,
                                    "AHT21 power request failed");
        (void)log_ret;
        return ret;
    }

    ret = app_boot_services_init();
    if (PLATFORM_ERR_OK != ret)
    {
        app_system_record_error(ret);
        return ret;
    }

    ret = app_boot_services_register();
    if (PLATFORM_ERR_OK != ret)
    {
        app_system_record_error(ret);
        return ret;
    }

    ret = service_manager_init();
    if (PLATFORM_ERR_OK != ret)
    {
        app_system_record_error(ret);
        return ret;
    }

    ret = app_system_set_state(APP_SYSTEM_STATE_READY);
    if (PLATFORM_ERR_OK != ret)
    {
        app_system_record_error(ret);
        return ret;
    }

    ret = app_boot_runtime_start();
    if (PLATFORM_ERR_OK != ret)
    {
        app_system_record_error(ret);
        return ret;
    }

    g_app_boot_data.initialized = 1u;
    return app_boot_update_data();
}

/**
 * @brief Refresh and copy the current boot snapshot.
 *
 * @param[out] p_out Receives the snapshot when the refresh succeeds.
 * @retval PLATFORM_ERR_OK Snapshot copied successfully.
 * @retval PLATFORM_ERR_PARAM The output pointer is NULL.
 * @return Original error from snapshot refresh on failure.
 */
platform_err_t app_boot_get_data(app_boot_data_t *p_out)
{
    platform_err_t ret = PLATFORM_ERR_OK;

    if (NULL == p_out)
    {
        return PLATFORM_ERR_PARAM;
    }

    ret = app_boot_update_data();
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    *p_out = g_app_boot_data;

    return PLATFORM_ERR_OK;
}

/**
 * @brief Initialize diagnosis after logging and Board initialization.
 *
 * @param None.
 * @retval PLATFORM_ERR_OK Diagnosis initialized and reset reason recorded.
 * @return Original error from diagnosis initialization on failure.
 * @note Logging is already initialized and must not be initialized twice.
 */
static platform_err_t app_boot_services_init(void)
{
    platform_err_t ret = PLATFORM_ERR_OK;

    /* Log was bootstrapped before Board init. Do not initialize it twice. */
    ret = service_diagnosis_init(&g_app_diagnosis_cfg);
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    service_diagnosis_record_reset_reason(0u, 0u);

    return PLATFORM_ERR_OK;
}

/**
 * @brief Register logging first, followed by diagnosis.
 *
 * @param None.
 * @retval PLATFORM_ERR_OK Both service objects registered successfully.
 * @return Original error from the first unsuccessful registration.
 */
static platform_err_t app_boot_services_register(void)
{
    platform_err_t ret = PLATFORM_ERR_OK;

    ret = service_manager_register(service_log_get_object());
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    return service_manager_register(service_diagnosis_get_object());
}

/**
 * @brief Start services, emit boot logs, and enter RUNNING after the probe.
 *
 * @param None.
 * @retval PLATFORM_ERR_OK Runtime started and application state updated.
 * @return Original error from service start, logging, probe, or state update.
 */
static platform_err_t app_boot_runtime_start(void)
{
    platform_err_t ret = PLATFORM_ERR_OK;

    ret = service_manager_start();
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    ret = service_log_write(SERVICE_LOG_LEVEL_INFO,
                            "platform boot start");
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    ret = service_log_write(SERVICE_LOG_LEVEL_INFO,
                            platform_version_get_string());
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    ret = app_boot_mcu_probe();
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    ret = service_log_write(SERVICE_LOG_LEVEL_INFO,
                            "app boot runtime started");
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    return app_system_set_state(APP_SYSTEM_STATE_RUNNING);
}

/**
 * @brief Sample the tick and original GPIO inputs, then log probe completion.
 *
 * @param None.
 * @retval PLATFORM_ERR_OK Both inputs read and the completion log accepted.
 * @return Original GPIO read or log error on failure.
 * @note The probe does not activate Board outputs or run an actuator demo.
 */
static platform_err_t app_boot_mcu_probe(void)
{
    uint32_t now_ms = 0u;
    plat_gpio_level_t wake_key_level = PLAT_GPIO_LEVEL_LOW;
    plat_gpio_level_t user_key1_level = PLAT_GPIO_LEVEL_LOW;
    platform_err_t ret = PLATFORM_ERR_OK;

    now_ms = plat_tick_get_ms();
    (void)now_ms;

    /* Resource-mapping lesson: select a channel by the board signal name. */
    ret = plat_gpio_read(g_board_pinmap[BOARD_PIN_ID_USER_KEY3].gpio_id,
                         &wake_key_level);
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    ret = plat_gpio_read(g_board_pinmap[BOARD_PIN_ID_USER_KEY1].gpio_id,
                         &user_key1_level);
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    (void)wake_key_level;
    (void)user_key1_level;

    return service_log_write(
        SERVICE_LOG_LEVEL_INFO,
        "platform_mcu abstraction ready");
}

/**
 * @brief Refresh the boot snapshot with state, version, and service count.
 *
 * @param None.
 * @retval PLATFORM_ERR_OK Snapshot refreshed successfully.
 * @return Original error from the state or version query on failure.
 */
static platform_err_t app_boot_update_data(void)
{
    platform_err_t ret = PLATFORM_ERR_OK;
    app_system_state_t state = APP_SYSTEM_STATE_BOOT;

    ret = app_system_get_state(&state);
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    ret = platform_version_get(&g_app_boot_data.version);
    if (PLATFORM_ERR_OK != ret)
    {
        return ret;
    }

    g_app_boot_data.app_state = state;
    g_app_boot_data.service_count = service_manager_count();

    return PLATFORM_ERR_OK;
}
