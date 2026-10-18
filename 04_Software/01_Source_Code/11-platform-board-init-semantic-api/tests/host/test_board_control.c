/******************************************************************************
 * Copyright (C) 2026 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file test_board_control.c
 *
 * @par dependencies
 * - board_control.h
 * - board_init.h
 * - board_resource_config.h
 * - plat_gpio.h
 * - plat_tick.h
 * - stdio.h
 * - stdlib.h
 * - string.h
 *
 * @author Codex (AI-assisted update for EternalChip)
 *
 * @brief Check real board actions and resource tables using host event records.
 *
 * Processing flow:
 * 1. Run each named scenario in a new process with production static state.
 * 2. Record GPIO writes and delays, optionally injecting a write failure.
 * 3. Check initialization, polarity, ordering, and error propagation.
 *
 * @version V1.1 2026-09-13
 *
 * @note 1 tab == 4 spaces!
 *****************************************************************************/

//******************************** Includes *********************************//

#include "board_control.h"
#include "board_init.h"
#include "board_resource_config.h"
#include "plat_gpio.h"
#include "plat_tick.h"
#include "fake_uart.h"

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

typedef struct
{
    char kind;
    unsigned int id;
    unsigned int value;
} event_t;

//******************************** Variables ********************************//

static event_t g_events[32] = {{0, 0u, 0u}};
static unsigned int g_event_count = 0u;
static unsigned int g_writes = 0u;
static unsigned int g_fail_write = 0u;
static platform_err_t g_injected_error = PLATFORM_ERR_TIMEOUT;

//******************************** Functions ********************************//

/**
 * @brief Record a platform GPIO write and inject a selected failure.
 *
 * @param[in] id Platform GPIO channel requested by the board.
 * @param[in] level Requested electrical level.
 *
 * @return The injected error at the selected write; PLATFORM_ERR_OK otherwise.
 */
platform_err_t plat_gpio_write(plat_gpio_id_t id, plat_gpio_level_t level)
{
    CHECK(g_event_count < 32u);
    g_events[g_event_count++] = (event_t){
        'W', (unsigned int)id, (unsigned int)level
    };
    ++g_writes;
    return (g_writes == g_fail_write) ? g_injected_error : PLATFORM_ERR_OK;
}

/**
 * @brief Record a requested delay without waiting on hardware.
 *
 * @param[in] ms Requested delay in milliseconds.
 *
 * @return None.
 */
void plat_delay_ms(uint32_t ms)
{
    CHECK(g_event_count < 32u);
    g_events[g_event_count++] = (event_t){'D', 0u, ms};
}

/**
 * @brief Check one recorded event against its expected fields.
 *
 * @param[in] index Event position in the recorded sequence.
 * @param[in] kind Expected write or delay event kind.
 * @param[in] id Expected platform channel identifier.
 * @param[in] value Expected electrical level or delay duration.
 *
 * @return None.
 */
static void expect_event(unsigned int index, char kind,
                         unsigned int id, unsigned int value)
{
    CHECK(index < g_event_count);
    CHECK(g_events[index].kind == kind);
    CHECK(g_events[index].id == id);
    CHECK(g_events[index].value == value);
}

/**
 * @brief Clear recorded events and failure selection without resetting Board.
 *
 * @par Parameters
 * None.
 *
 * @return None.
 */
static void clear_events(void)
{
    g_event_count = 0u;
    g_writes = 0u;
    g_fail_write = 0u;
}

/**
 * @brief Check that every semantic action rejects a board that is not ready.
 *
 * @par Parameters
 * None.
 *
 * @return None.
 */
static void expect_not_ready(void)
{
    unsigned int before = g_event_count;

    CHECK(PLATFORM_ERR_NOT_INITIALIZED == board_backlight_on());
    CHECK(PLATFORM_ERR_NOT_INITIALIZED == board_backlight_off());
    CHECK(PLATFORM_ERR_NOT_INITIALIZED == board_display_reset());
    CHECK(PLATFORM_ERR_NOT_INITIALIZED ==
          board_motor_enable(BOARD_SIGNAL_INACTIVE));
    CHECK(PLATFORM_ERR_NOT_INITIALIZED ==
          board_motor_enable(BOARD_SIGNAL_ACTIVE));
    CHECK(PLATFORM_ERR_NOT_INITIALIZED ==
          board_motor_enable(BOARD_SIGNAL_STATE_RESERVED));
    CHECK(PLATFORM_ERR_NOT_INITIALIZED ==
          board_aht21_power_enable(BOARD_SIGNAL_ACTIVE));
    CHECK(PLATFORM_ERR_NOT_INITIALIZED ==
          board_aht21_power_enable(BOARD_SIGNAL_INACTIVE));
    CHECK(g_event_count == before);
}

/**
 * @brief Check the three supported default writes and their order.
 *
 * @par Parameters
 * None.
 *
 * @return None.
 */
static void expect_safe_init(void)
{
    CHECK(PLATFORM_ERR_OK == board_init());
    CHECK(3u == g_event_count);
    /* Channels are zero based: CH6=5, CH5=4, CH4=3. */
    expect_event(0u, 'W', 5u, PLAT_GPIO_LEVEL_LOW);
    expect_event(1u, 'W', 4u, PLAT_GPIO_LEVEL_LOW);
    expect_event(2u, 'W', 3u, PLAT_GPIO_LEVEL_HIGH);
}

/**
 * @brief Execute one named board scenario in an isolated process.
 *
 * @param[in] argc Argument count, including the executable name.
 * @param[in] p_argv Argument vector containing the scenario name.
 *
 * @return 0 after all checks and output succeed; 1 for an invalid action
 *         selector or output failure. Failed checks exit with status 1.
 */
int main(int argc, char **p_argv)
{
    unsigned int index = 0u;

    CHECK(2 == argc);
    CHECK(NULL != p_argv);
    CHECK(NULL != p_argv[1]);
    if (0 == strcmp(p_argv[1], "before_init"))
    {
        expect_not_ready();
    }
    else if (0 == strcmp(p_argv[1], "init_order_idempotent"))
    {
        expect_safe_init();
        CHECK(PLATFORM_ERR_OK == board_init());
        CHECK(3u == g_event_count);
        CHECK(PLATFORM_ERR_OK == board_motor_enable(BOARD_SIGNAL_ACTIVE));
        CHECK(PLATFORM_ERR_OK == board_backlight_on());
        index = g_event_count;
        CHECK(PLATFORM_ERR_OK == board_init());
        /* Init is not a second off command. */
        CHECK(g_event_count == index);
    }
    else if (0 == strcmp(p_argv[1], "polarity_and_delay"))
    {
        expect_safe_init();
        clear_events();
        CHECK(PLATFORM_ERR_OK == board_backlight_on());
        CHECK(PLATFORM_ERR_OK == board_backlight_off());
        CHECK(PLATFORM_ERR_OK == board_motor_enable(BOARD_SIGNAL_ACTIVE));
        CHECK(PLATFORM_ERR_OK == board_motor_enable(BOARD_SIGNAL_INACTIVE));
        CHECK(PLATFORM_ERR_OK == board_display_reset());
        CHECK(8u == g_event_count);
        expect_event(0u, 'W', 5u, PLAT_GPIO_LEVEL_HIGH);
        expect_event(1u, 'W', 5u, PLAT_GPIO_LEVEL_LOW);
        expect_event(2u, 'W', 4u, PLAT_GPIO_LEVEL_HIGH);
        expect_event(3u, 'W', 4u, PLAT_GPIO_LEVEL_LOW);
        expect_event(4u, 'W', 3u, PLAT_GPIO_LEVEL_LOW);
        expect_event(5u, 'D', 0u, 10u);
        expect_event(6u, 'W', 3u, PLAT_GPIO_LEVEL_HIGH);
        expect_event(7u, 'D', 0u, 10u);
    }
    else if (0 == strcmp(p_argv[1], "invalid_enable"))
    {
        expect_safe_init();
        clear_events();
        for (index = 2u; index <= 256u; ++index)
        {
            CHECK(PLATFORM_ERR_PARAM ==
                  board_motor_enable((board_signal_state_t)index));
        }
        CHECK(PLATFORM_ERR_PARAM ==
              board_motor_enable((board_signal_state_t)-1));
        CHECK(PLATFORM_ERR_PARAM ==
              board_motor_enable(BOARD_SIGNAL_STATE_RESERVED));
        CHECK(0u == g_event_count);
        CHECK(PLATFORM_ERR_OK == board_motor_enable(BOARD_SIGNAL_INACTIVE));
    }
    else if (0 == strncmp(p_argv[1], "init_failure_", 13u))
    {
        g_fail_write = (unsigned int)(p_argv[1][13] - '0');
        CHECK((g_fail_write >= 1u) && (g_fail_write <= 3u));
        g_injected_error = (1u == g_fail_write) ? PLATFORM_ERR_TIMEOUT :
                           (2u == g_fail_write) ? PLATFORM_ERR_FAIL :
                                                 PLATFORM_ERR_BUSY;
        CHECK(board_init() == g_injected_error);
        /* No writes after the failing stage. */
        CHECK(g_event_count == g_fail_write);
        expect_not_ready();
        clear_events();
        /* Explicit retry replays every default. */
        expect_safe_init();
    }
    else if (0 == strncmp(p_argv[1], "reset_failure_", 14u))
    {
        expect_safe_init();
        clear_events();
        g_fail_write = (unsigned int)(p_argv[1][14] - '0');
        CHECK((1u == g_fail_write) || (2u == g_fail_write));
        CHECK(PLATFORM_ERR_TIMEOUT == board_display_reset());
        CHECK(g_event_count == ((1u == g_fail_write) ? 1u : 3u));
        expect_event(0u, 'W', 3u, PLAT_GPIO_LEVEL_LOW);
        if (2u == g_fail_write)
        {
            expect_event(1u, 'D', 0u, 10u);
            expect_event(2u, 'W', 3u, PLAT_GPIO_LEVEL_HIGH);
        }
        expect_not_ready();
        clear_events();
        expect_safe_init();
    }
    else if (0 == strncmp(p_argv[1], "action_failure_", 15u))
    {
        platform_err_t ret = PLATFORM_ERR_OK;

        expect_safe_init();
        clear_events();
        g_fail_write = 1u;
        switch (p_argv[1][15])
        {
            case '0':
                ret = board_backlight_on();
                break;
            case '1':
                ret = board_backlight_off();
                break;
            case '2':
                ret = board_motor_enable(BOARD_SIGNAL_ACTIVE);
                break;
            case '3':
                ret = board_motor_enable(BOARD_SIGNAL_INACTIVE);
                break;
            default:
                CHECK(0);
                return 1;
        }
        CHECK(PLATFORM_ERR_TIMEOUT == ret);
        CHECK(1u == g_event_count);
        expect_not_ready();
    }
    else
    {
        CHECK(0);
    }
    CHECK(0u == g_test_uart_writes);
    if (0 > printf("PASS board %s\n", p_argv[1]))
    {
        return 1;
    }
    return 0;
}
