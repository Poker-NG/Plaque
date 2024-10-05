#include "ch32v20x_gpio.h"
#include "clock.h"
#include "debug.h"
#include "debug_utils.h"
#include "eeprom.h"
#include "epd.h"
#include "i2c.h"
#include "st25.h"
#include "tests.h"
#include "system.h"

#include <malloc.h>
#include <string.h>
#include <sys/reent.h>

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
        LOG_DEBUG("st25_context_get_device_revision: %u\r\n", st25_context_get_device_revision(&st25_instance));
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

    test_run_external_eeprom(&eeprom_instance);
    test_run_st25(&st25_instance);
    LOG_FLUSH();

    LOG_DEBUG("Entering loop.");
    while (true)
    {
        if (st25_context_is_rf_field_detected(&st25_instance) == true)
        {
            LOG_DEBUG("RF field detected.");
            clock_delay_ms(2000);
        }
    }
}
