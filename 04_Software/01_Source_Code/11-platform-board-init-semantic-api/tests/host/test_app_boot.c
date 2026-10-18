/******************************************************************************
 * Copyright (C) 2026 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file test_app_boot.c
 *
 * @par dependencies
 * - app_boot.h
 * - board_control.h
 * - board_resource_config.h
 * - plat_gpio.h
 * - plat_tick.h
 * - service_log.h
 * - service_diagnosis.h
 * - service_manager.h
 * - log_port.h
 * - stdio.h
 * - stdlib.h
 * - string.h
 *
 * @author Codex (AI-assisted update for EternalChip)
 *
 * @brief Test real boot, services, managers, and Board with host fakes.
 *
 * Processing flow:
 * 1. Run each startup or failure scenario in a new process.
 * 2. Record GPIO, tick, and log operations without replacing the boot logic.
 * 3. Verify startup ordering, failure propagation, and runtime state.
 *
 * @version V1.1 2026-09-13
 *
 * @note 1 tab == 4 spaces!
 *****************************************************************************/

//******************************** Includes *********************************//

#include "app_boot.h"
#include "board_control.h"
#include "board_resource_config.h"
#include "plat_gpio.h"
#include "plat_tick.h"
#include "fake_uart.h"
#include "service_log.h"
#include "service_diagnosis.h"
#include "service_manager.h"
#include "log_port.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//******************************** Defines **********************************//

#define CHECK(expr)                                                         \
    do                                                                      \
    {                                                                       \
        if (!(expr))                                                        \
        {                                                                   \
            if (0 > fprintf(stderr, "FAIL %s:%d: %s\n",                      \
                            __FILE__, __LINE__, #expr))                      \
            {                                                               \
                exit(1);                                                    \
            }                                                               \
            exit(1);                                                        \
        }                                                                   \
    } while (0)

//******************************** Variables ********************************//

static unsigned int g_bootstrap_count = 0u;
static unsigned int g_writes = 0u;
static unsigned int g_reads = 0u;
static unsigned int g_ticks = 0u;
static unsigned int g_fail_write = 0u;
static unsigned int g_log_count = 0u;
static const char *g_p_messages[16] = {NULL};
static uint8_t g_log_levels[16] = {0u};

static const unsigned int g_expected_id[] = {5u, 4u, 3u};
static const plat_gpio_level_t g_expected_level[] = {
    PLAT_GPIO_LEVEL_LOW, PLAT_GPIO_LEVEL_LOW, PLAT_GPIO_LEVEL_HIGH
};

//******************************** Functions ********************************//

/**
 * @brief Record initialization of the host log output port.
 *
 * @par Parameters
 * None.
 *
 * @return None.
 */
void log_port_init(void)
{
    ++g_bootstrap_count;
}

/**
 * @brief Accept the log output setting without changing the host recorder.
 *
 * @param[in] enable Output setting supplied by the real log service.
 *
 * @return None.
 */
void log_port_set_output_enabled(uint8_t enable)
{
    (void)enable;
}

/**
 * @brief Accept the log level while leaving filtering to the real service.
 *
 * @param[in] level Log level supplied by the real log service.
 *
 * @return None.
 */
void log_port_set_level(uint8_t level)
{
    (void)level;
}

/**
 * @brief Record a log message after verifying log port initialization.
 *
 * @param[in] level Message severity retained for warning/error checks.
 * @param[in] p_tag Message tag, unused by this recorder.
 * @param[in] p_message Message text retained for startup-order checks.
 *
 * @return None.
 */
void log_port_write(uint8_t level, const char *p_tag, const char *p_message)
{
    (void)p_tag;
    CHECK(g_bootstrap_count > 0u);
    CHECK(g_log_count < 16u);
    CHECK(NULL != p_message);
    g_log_levels[g_log_count] = level;
    g_p_messages[g_log_count++] = p_message;
}

/**
 * @brief Check each Board default write and optionally fail a startup stage.
 *
 * @param[in] id Platform GPIO channel selected by the real Board code.
 * @param[in] level Requested default electrical level.
 *
 * @return PLATFORM_ERR_TIMEOUT at the selected write;
 *         PLATFORM_ERR_OK otherwise.
 */
platform_err_t plat_gpio_write(plat_gpio_id_t id, plat_gpio_level_t level)
{
    service_log_data_t log_data = {0};
    service_diagnosis_data_t diagnosis = {0};

    CHECK(g_bootstrap_count > 0u);
    CHECK(PLATFORM_ERR_OK == service_log_get_data(&log_data));
    CHECK((1u == log_data.valid) && (0u == log_data.running));
    CHECK(PLATFORM_ERR_OK == service_diagnosis_get_data(&diagnosis));
    /* No diagnosis initialization before Board. */
    CHECK(0u == diagnosis.valid);
    CHECK(0u == service_manager_count());
    CHECK(g_writes < 3u);
    CHECK(g_expected_id[g_writes] == (unsigned int)id);
    CHECK(g_expected_level[g_writes] == level);
    ++g_writes;
    return (g_writes == g_fail_write) ? PLATFORM_ERR_TIMEOUT : PLATFORM_ERR_OK;
}

/**
 * @brief Check the startup probe order and supply a high GPIO input.
 *
 * @param[in] id Platform channel requested by the real MCU probe.
 * @param[out] p_level Receives the fixed high input level.
 *
 * @return PLATFORM_ERR_OK after recording the valid probe.
 */
platform_err_t plat_gpio_read(plat_gpio_id_t id, plat_gpio_level_t *p_level)
{
    CHECK(NULL != p_level);
    CHECK((3u == g_writes) && (2u == service_manager_count()));
#ifdef TEST_REMAPPED_KEYS
    CHECK(0 == strcmp(BOARD_CFG_NAME, "Board-B-Host-Fixture"));
    CHECK(id == ((0u == g_reads) ? PLAT_GPIO_CH_2 : PLAT_GPIO_CH_1));
#else
    CHECK(0 == strcmp(BOARD_CFG_NAME, "EC-S100-Watch"));
    CHECK(id == ((0u == g_reads) ? PLAT_GPIO_CH_1 : PLAT_GPIO_CH_2));
#endif
    *p_level = PLAT_GPIO_LEVEL_HIGH;
    ++g_reads;
    return PLATFORM_ERR_OK;
}

/**
 * @brief Record a tick query and supply a deterministic startup timestamp.
 *
 * @par Parameters
 * None.
 *
 * @return 123 milliseconds for every query.
 */
uint32_t plat_tick_get_ms(void)
{
    ++g_ticks;
    return 123u;
}

/**
 * @brief Reject blocking delays because boot must not demonstrate reset.
 *
 * @param[in] ms Unexpected delay duration in milliseconds.
 *
 * @return None.
 */
void plat_delay_ms(uint32_t ms)
{
    (void)ms;
    CHECK(0);
}

/**
 * @brief Execute one startup scenario with real application and service logic.
 *
 * @param[in] argc Argument count, including the executable name.
 * @param[in] p_argv Argument vector containing the requested failure stage.
 *
 * @return 0 after all checks and output succeed; 1 if result output fails.
 *         Failed checks terminate the process with status 1.
 */
int main(int argc, char **p_argv)
{
    app_boot_data_t boot = {0};
    app_system_data_t system = {0};
    service_log_data_t log_data = {0};
    service_diagnosis_data_t diagnosis = {0};
    platform_err_t ret = PLATFORM_ERR_OK;
    unsigned int before = 0u;

    CHECK(2 == argc);
    CHECK(NULL != p_argv);
    CHECK(NULL != p_argv[1]);
    g_fail_write = (unsigned int)(p_argv[1][0] - '0');
    CHECK(g_fail_write <= 4u);
    ret = app_boot_init();
    CHECK(PLATFORM_ERR_OK == app_boot_get_data(&boot));
    CHECK(PLATFORM_ERR_OK == app_system_get_data(&system));
    CHECK(PLATFORM_ERR_OK == service_log_get_data(&log_data));
    CHECK(PLATFORM_ERR_OK == service_diagnosis_get_data(&diagnosis));
    CHECK(1u == g_bootstrap_count);
    if (4u == g_fail_write)
    {
        CHECK(PLATFORM_ERR_PARAM == ret);
        CHECK(3u == g_writes);
        CHECK((0u == boot.initialized) && (0u == boot.service_count));
        CHECK(APP_SYSTEM_STATE_ERROR == system.state);
        CHECK(PLATFORM_ERR_PARAM == system.last_error);
        CHECK((0u == log_data.running) && (0u == diagnosis.valid));
        CHECK((0u == g_reads) && (0u == g_ticks));
        CHECK(1u == g_log_count);
        CHECK(0 == strcmp(g_p_messages[0], "AHT21 power request failed"));
        CHECK(SERVICE_LOG_LEVEL_ERROR == g_log_levels[0]);
    }
    else if (0u != g_fail_write)
    {
        CHECK(PLATFORM_ERR_TIMEOUT == ret);
        CHECK(g_writes == g_fail_write);
        CHECK((0u == boot.initialized) && (0u == boot.service_count));
        CHECK(APP_SYSTEM_STATE_ERROR == system.state);
        CHECK(PLATFORM_ERR_TIMEOUT == system.last_error);
        CHECK((0u == log_data.running) && (0u == diagnosis.valid));
        CHECK((0u == g_reads) && (0u == g_ticks));
        CHECK(1u == g_log_count);
        CHECK(0 == strcmp(g_p_messages[0], "board init failed"));
        CHECK(SERVICE_LOG_LEVEL_ERROR == g_log_levels[0]);
        CHECK(PLATFORM_ERR_NOT_INITIALIZED == board_backlight_on());
        CHECK(PLATFORM_ERR_OK == app_system_process());
        CHECK(PLATFORM_ERR_OK == app_system_get_data(&system));
        /* ERROR blocks the real task loop. */
        CHECK(0u == system.process_count);
    }
    else
    {
        CHECK(PLATFORM_ERR_OK == ret);
        CHECK((3u == g_writes) && (2u == g_reads) && (1u == g_ticks));
        CHECK((1u == boot.initialized) && (2u == boot.service_count));
        CHECK(APP_SYSTEM_STATE_RUNNING == system.state);
        CHECK((1u == log_data.running) && (1u == diagnosis.running));
        CHECK(7u == g_log_count);
        CHECK(0 == strcmp(g_p_messages[0],
                         "AHT21 power control pending: Nordic protocol TBD"));
        CHECK(SERVICE_LOG_LEVEL_WARN == g_log_levels[0]);
        CHECK(0 == strcmp(g_p_messages[1], "service_log started"));
        CHECK(0 == strcmp(g_p_messages[2], "service_diagnosis started"));
        CHECK(0 == strcmp(g_p_messages[3], "platform boot start"));
        CHECK(0 == strcmp(g_p_messages[4], platform_version_get_string()));
        CHECK(0 == strcmp(g_p_messages[5], "platform_mcu abstraction ready"));
        CHECK(0 == strcmp(g_p_messages[6], "app boot runtime started"));
        before = g_log_count;
        CHECK(PLATFORM_ERR_OK == app_boot_init());
        CHECK((3u == g_writes) && (g_log_count == before) &&
              (1u == g_bootstrap_count));
    }
    CHECK(0u == g_test_uart_writes);
    if (0 > printf("PASS app_boot failure_stage=%u\n", g_fail_write))
    {
        return 1;
    }
    return 0;
}
