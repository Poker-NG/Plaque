#include "tests.h"
#include "clock.h"
#include "debug_utils.h"
#include "i2c.h"
#include "st25.h"
#include <string.h>

void test_run_external_eeprom(external_eeprom_context_t* external_eeprom)
{
    unsigned char v = 0;
    int r = 0;

    r = external_eeprom_context_write_byte(external_eeprom, 599, 0xEE);
    LOG_DEBUG("result: %i", r);

    r = external_eeprom_context_write_byte(external_eeprom, 0x11, 0xBA);
    r = external_eeprom_context_read_byte(external_eeprom, 0x11, &v);
    LOG_DEBUG("result: %i; value: %x", r, v);

    r = external_eeprom_context_write_byte(external_eeprom, 0x11, v + 1);
    LOG_DEBUG("result: %i", r);

    r = external_eeprom_context_read_byte(external_eeprom, 0x11, &v);
    LOG_DEBUG("result: %i; value: %x", r, v);

    unsigned char data[600] = {};

    memset(data, 0xDD, sizeof(data));

    for (int i = 0; i < sizeof(data); i++)
    {
        data[i] = i % 64;
    }

    external_eeprom_context_write_bytes(external_eeprom, 0, data, sizeof(data));
    external_eeprom_context_write_byte(external_eeprom, 0x23, 0x20);
    external_eeprom_context_write_byte(external_eeprom, 0x52, 0x02);

    memset(data, 0, sizeof(data));

    external_eeprom_context_read_bytes(external_eeprom, 0, data, sizeof(data));

    LOG_DEBUG("Dump of 0x0 - 0x%x", sizeof(data));
    printf("   ");

    for (int i = 0; i < sizeof(data); i++)
    {
        printf("%02x ", data[i]);

        if (i != 0 && ((i + 1) % 64) == 0)
        {
            printf("\r\n");
            printf("   ");
        }
    }

    LOG_FLUSH();
}

void test_run_st25(st25_context_t* st25)
{
    unsigned char buffer[1024] = {};

    int res = 0;
    res = st25_context_io_read_multiple_bytes(st25, st25->i2c_address_data, 0x0, sizeof(buffer), buffer);
    LOG_DEBUG("st25_context_io_read_multiple_bytes: %i", res);

    LOG_DEBUG("Dump of 0x0 - 0x%x", sizeof(buffer));
    printf("   ");
    for (int i = 0; i < sizeof(buffer); i++)
    {
        printf("%02x ", buffer[i]);
        if (i != 0 && ((i + 1) % 32) == 0)
        {
            printf("\r\n");
            printf("   ");
        }
    }
    LOG_FLUSH();

    memset(buffer, 0xFF, sizeof(buffer));

    res = st25_context_io_write_multiple_bytes(st25, st25->i2c_address_data, 0x0, buffer, sizeof(buffer));
    LOG_DEBUG("st25_context_io_write_multiple_bytes: %i", res);

    for (int i = 0; i < sizeof(buffer); i++)
    {
        buffer[i] = (unsigned char)(i % 32);
    }

    clock_tick_t time_taken = clock_get_tick();

    res = st25_context_io_write_multiple_bytes(st25, st25->i2c_address_data, 0x0, buffer, sizeof(buffer));
    LOG_DEBUG("st25_context_io_write_multiple_bytes: %i", res);

    time_taken = clock_get_tick() - time_taken;

    LOG_DEBUG("Took: %i ms", (unsigned int)time_taken);
}
