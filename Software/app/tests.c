#include "tests.h"
#include "clock.h"
#include "i2c.h"
#include "st25.h"
// #include "m24sr64.h"
#include <string.h>

void test_run_external_eeprom(external_eeprom_context_t* external_eeprom)
{
    unsigned char v = 0;
    int r = 0;

    r = external_eeprom_context_write_byte(external_eeprom, 599, 0xEE);
    printf("[%s:%i] result: %i\r\n", __FUNCTION__, __LINE__, r);

    r = external_eeprom_context_write_byte(external_eeprom, 0x11, 0xBA);
    r = external_eeprom_context_read_byte(external_eeprom, 0x11, &v);
    printf("[%s:%i] result: %i; value: %x\r\n", __FUNCTION__, __LINE__, r, v);

    r = external_eeprom_context_write_byte(external_eeprom, 0x11, v + 1);
    printf("[%s:%i] result: %i\r\n", __FUNCTION__, __LINE__, r);

    r = external_eeprom_context_read_byte(external_eeprom, 0x11, &v);
    printf("[%s:%i] result: %i; value: %x\r\n", __FUNCTION__, __LINE__, r, v);

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

    printf("[%s:%i] Dump of 0x0 - 0x%x\r\n   ", __FUNCTION__, __LINE__, sizeof(data));

    for (int i = 0; i < sizeof(data); i++)
    {
        printf("%02x ", data[i]);

        if (i != 0 && ((i + 1) % 64) == 0)
        {
            printf("\r\n");
            printf("   ");
        }
    }

    printf("\r\n");
}

// void test_run_dynamic_tag(m24sr64_context_t* m24sr64)
// {
//     if (m24sr64_context_check_if_is_connected(m24sr64) == true)
//     {
//         printf("[%s:%i] M24SR64 is connected.\r\n", __FUNCTION__, __LINE__);
//     }
//     else
//     {
//         printf("[!%s:%i!] M24SR64 is not connected.\r\n", __FUNCTION__, __LINE__);
//     }
//
//     int res = 0;
//     res = m24sr64_context_device_init(m24sr64);
//
//     printf("[%s:%i] m24sr64_context_device_init: %i\r\n", __FUNCTION__, __LINE__, res);
//
//     m24sr64_context_device_get_session(m24sr64);
//
//     res = m24sr64_context_command_select_application(m24sr64);
//     printf("[%s:%i] m24sr64_context_command_select_application: %i\r\n", __FUNCTION__, __LINE__, res);
//
//     res = m24sr64_context_command_select_cc_file(m24sr64);
//     printf("[%s:%i] m24sr64_context_command_select_cc_file: %i\r\n", __FUNCTION__, __LINE__, res);
//
//     unsigned char read_binary_buffer_data[15] = {0};
//     res = m24sr64_context_command_read_binary(m24sr64, 0, sizeof(read_binary_buffer_data), read_binary_buffer_data);
//     printf("[%s:%i] m24sr64_context_command_read_binary: %i\r\n", __FUNCTION__, __LINE__, res);
//
//     for (int i = 0; i < sizeof(read_binary_buffer_data); i++)
//     {
//         printf("%02x ", read_binary_buffer_data[i]);
//
//         if (i != 0 && ((i + 1) % 64) == 0)
//         {
//             printf("\r\n");
//             printf("   ");
//         }
//     }
//     printf("\r\n");
//
//     m24sr64_context_device_deselect(m24sr64);
//
//     printf("[%s:%i] Done\r\n", __FUNCTION__, __LINE__);
// }

void test_run_st25(st25_context_t* st25)
{
    unsigned char buffer[1024] = {};

    int res = 0;
    res = st25_context_io_read_multiple_bytes(st25, st25->i2c_address_data, 0x0, sizeof(buffer), buffer);
    printf("[%s:%i] st25_context_io_read_multiple_bytes: %i\r\n", __FUNCTION__, __LINE__, res);

    printf("[%s:%i] Dump of 0x0 - 0x%x\r\n   ", __FUNCTION__, __LINE__, sizeof(buffer));
    for (int i = 0; i < sizeof(buffer); i++)
    {
        printf("%02x ", buffer[i]);
        if (i != 0 && ((i + 1) % 32) == 0)
        {
            printf("\r\n");
            printf("   ");
        }
    }
    printf("\r\n");

    memset(buffer, 0xFF, sizeof(buffer));

    res = st25_context_io_write_multiple_bytes(st25, st25->i2c_address_data, 0x0, buffer, sizeof(buffer));
    printf("[%s:%i] st25_context_io_write_multiple_bytes: %i\r\n", __FUNCTION__, __LINE__, res);

    for (int i = 0; i < sizeof(buffer); i++)
    {
        buffer[i] = (unsigned char)(i % 32);
    }

    clock_tick_t time_taken = clock_get_tick();

    res = st25_context_io_write_multiple_bytes(st25, st25->i2c_address_data, 0x0, buffer, sizeof(buffer));
    printf("[%s:%i] st25_context_io_write_multiple_bytes: %i\r\n", __FUNCTION__, __LINE__, res);

    time_taken = clock_get_tick() - time_taken;

    printf("[%s:%i] Took: %i ms\r\n", __FUNCTION__, __LINE__, (unsigned int)time_taken);
}
