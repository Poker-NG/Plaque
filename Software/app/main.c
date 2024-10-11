#include "ch32v20x_gpio.h"
#include "clock.h"
#include "debug.h"
#include "debug_utils.h"
#include "eeprom.h"
#include "epd.h"
#include "i2c.h"
#include "rng.h"
#include "st25.h"
#include "system.h"
#include "tests.h"

#include <malloc.h>
#include <stdlib.h>
#include <string.h>

static external_eeprom_context_t eeprom_instance = {0};
static st25_context_t st25_instance = {0};
static epd_context_t epd_instance = {0};

int main(void)
{

    system_init();

    LOG_FLUSH();

    if (external_eeprom_context_init(&eeprom_instance, i2c_connectables_2, 0x50) != eeprom_errors_none)
    {
        LOG_ERROR("Failed to initialize eeprom.");
    }
    else
    {
        LOG_DEBUG("Initialized eeprom.");
    }

    if (st25_context_init(&st25_instance) != st25_errors_none)
    {
        LOG_ERROR("Failed to initialize st25.");
    }
    else
    {
        LOG_DEBUG("Initialized st25.");
        LOG_DEBUG("st25_context_is_connected: %s.", st25_context_is_connected(&st25_instance) == true ? "yes" : "no");
        LOG_DEBUG("st25_context_get_device_revision: %u", st25_context_get_device_revision(&st25_instance));

        unsigned char uid[8];

        if (st25_context_get_device_uid(&st25_instance, uid) == true)
        {
            LOG_DEBUG("st25_context_get_device_uid:");
            printf("   ");
            for (int i = 0; i < sizeof(uid); i++)
            {
                printf("%02x ", uid[i]);
                if (i != 0 && ((i + 1) % 32) == 0)
                {
                    printf("\r\n");
                    printf("   ");
                }
            }
            LOG_FLUSH();
        }
    }

    // epd

    if (epd_context_init(&epd_instance) != epd_errors_none)
    {
        LOG_ERROR("Failed to initialize epd.");
    }
    else
    {
        LOG_DEBUG("Initialized epd.");
        int res = epd_context_display_init(&epd_instance);

#if CONFIG_ENABLE_EPD_TEST
        res += epd_context_display_clear_frame_memory(&epd_instance, 0x00);
        // res += epd_context_display_frame(&epd_instance);

        unsigned char* data = malloc(83 * 83);

        memset(data, 0, 83 * 83);

        for (int x = 0; x < (83 * 83); x++)
        {
            data[x] = (((x + 1) % 3) == 0) ? 0xFF : 0x00;
        }

        int y_add = 0;

        for (int i = 0; i < 3; i++)
        {
            epd_context_display_set_frame_memory(&epd_instance, data, 0, (83 * i) + y_add, 83, 83);
            y_add += ((i + 1) * 15);
        }

        epd_context_display_frame(&epd_instance);

        LOG_DEBUG("Epd routine done, res: %i.", res);
#else
        (void)res;
#endif
    }

    LOG_DEBUG("Fully initialized.", );
    LOG_DEBUG("Current Tick: %u", (unsigned int)(clock_get_tick() & UINT32_MAX));

    LOG_DEBUG("Intitializing rng.");
    rng_init();
    rng_do_seed_long();

    test_run_external_eeprom(&eeprom_instance);
    test_run_st25(&st25_instance);
    test_run_epd(&epd_instance);

    LOG_DEBUG("Turning LED on for 500ms.");
    debug_led_init();
    DEBUG_LED_ON();
    clock_delay_ms(500);
    DEBUG_LED_OFF();

    LOG_DEBUG("Entering loop.");

    clock_tick_t timer_1 = clock_get_tick();
    clock_tick_t timer_2 = clock_get_tick();

    extern char _end[];
    extern char _heap_end[];

    LOG_DEBUG("Heap Left: %u.", (unsigned int)((unsigned long long)_heap_end - (unsigned long long)_end));

    while (true)
    {
        if (st25_context_is_rf_field_detected(&st25_instance) == true)
        {
            LOG_DEBUG("RF field detected.");
            clock_delay_ms(400);
        }

        if (clock_tick_compare_is_timeout(timer_1, 4000))
        {
            timer_1 = clock_get_tick();
            LOG_DEBUG("rng: %x", rand());
        }

        if (clock_tick_compare_is_timeout(timer_2, 9000))
        {
            timer_2 = clock_get_tick();
            rng_do_seed_quick();
        }
    }
}
