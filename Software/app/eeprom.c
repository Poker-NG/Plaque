#include "eeprom.h"
#include "ch32v20x_i2c.h"
#include "debug.h"
#include "i2c.h"
#include <string.h>

#define EXTERNAL_EEPROM_MAX_I2C_READ_LENGTH 0x40
#define EXTERNAL_EEPROM_MAX_I2C_WRITE_LENGTH 0x10

int external_eeprom_context_init(external_eeprom_context_t* instance, int i2c_connectable, unsigned char i2c_address)
{
    memset(&instance->i2c_context, 0, sizeof(instance->i2c_context));
    instance->i2c_address = i2c_address;

    if (i2c_context_init(&instance->i2c_context, i2c_connectable) != i2c_errors_none)
    {
        return eeprom_errors_i2c_failed;
    }

    return external_eeprom_context_check_if_is_connected(instance) == true ? eeprom_errors_none : eeprom_errors_failed_to_ping;
}

bool external_eeprom_context_check_if_is_connected(external_eeprom_context_t* instance)
{
    i2c_context_transmission_begin(&instance->i2c_context, instance->i2c_address);
    return i2c_context_transmission_end(&instance->i2c_context, true) == i2c_errors_none;
}

int external_eeprom_context_read_byte(external_eeprom_context_t* instance, unsigned short address, unsigned char* value)
{
    i2c_context_transmission_begin(&instance->i2c_context, instance->i2c_address);
    i2c_context_transmission_write(&instance->i2c_context, (address >> 8) & 0xFF);
    i2c_context_transmission_write(&instance->i2c_context, (address >> 0) & 0xFF);

    int transmission_end_res = i2c_context_transmission_end(&instance->i2c_context, false);

    if (transmission_end_res != i2c_errors_none)
    {
        return eeprom_errors_i2c_failed;
    }

    if (i2c_context_transmission_request_from(&instance->i2c_context, instance->i2c_address, 1) != i2c_errors_none)
    {
        return eeprom_errors_i2c_failed;
    }

    i2c_context_transmission_read(&instance->i2c_context, value);

    return eeprom_errors_none;
}

int external_eeprom_context_read_bytes(external_eeprom_context_t* instance, unsigned short eeprom_address, unsigned char* buffer_data, unsigned short buffer_length)
{
    unsigned short eeprom_current_address = 0;
    unsigned char* buffer_data_current_position = buffer_data;
    unsigned short buffer_length_left = buffer_length;
    unsigned short buffer_split_length = EXTERNAL_EEPROM_MAX_I2C_READ_LENGTH - 1;

    while (buffer_length_left > 0)
    {
        unsigned short current_split_length = buffer_length_left;

        if (current_split_length > buffer_split_length)
        {
            current_split_length = buffer_split_length;
        }

        i2c_context_transmission_begin(&instance->i2c_context, instance->i2c_address);
        i2c_context_transmission_write(&instance->i2c_context, (eeprom_current_address >> 8) & 0xFF);
        i2c_context_transmission_write(&instance->i2c_context, (eeprom_current_address >> 0) & 0xFF);

        int transmission_end_res = i2c_context_transmission_end(&instance->i2c_context, false);

        if (transmission_end_res != i2c_errors_none)
        {
            return eeprom_errors_i2c_failed;
        }

        if (i2c_context_transmission_request_from(&instance->i2c_context, instance->i2c_address, current_split_length) != i2c_errors_none)
        {
            return eeprom_errors_i2c_failed;
        }

        memcpy(buffer_data_current_position, i2c_context_transmission_get_buffer_data(&instance->i2c_context), current_split_length);

        buffer_data_current_position += current_split_length;
        eeprom_current_address += current_split_length;
        buffer_length_left -= current_split_length;
    }

    return eeprom_errors_none;
}

int external_eeprom_context_write_byte(external_eeprom_context_t* instance, unsigned short address, unsigned char value)
{
    i2c_context_transmission_begin(&instance->i2c_context, instance->i2c_address);
    i2c_context_transmission_write(&instance->i2c_context, (address >> 8) & 0xFF);
    i2c_context_transmission_write(&instance->i2c_context, (address >> 0) & 0xFF);
    i2c_context_transmission_write(&instance->i2c_context, value);

    int res = i2c_context_transmission_end(&instance->i2c_context, true);

    if (res != i2c_errors_none)
    {
        return eeprom_errors_i2c_failed;
    }

    return eeprom_errors_none;
}

int external_eeprom_context_write_bytes(external_eeprom_context_t* instance, unsigned short eeprom_address, unsigned char* buffer_data, unsigned short buffer_length)
{
    unsigned short eeprom_current_address = 0;
    unsigned char* buffer_data_current_position = buffer_data;
    unsigned short buffer_length_left = buffer_length;
    unsigned short buffer_split_length = EXTERNAL_EEPROM_MAX_I2C_WRITE_LENGTH;

    while (buffer_length_left > 0)
    {
        unsigned short current_split_length = buffer_length_left;

        if (current_split_length > buffer_split_length)
        {
            current_split_length = buffer_split_length;
        }

        i2c_context_transmission_begin(&instance->i2c_context, instance->i2c_address);
        i2c_context_transmission_write(&instance->i2c_context, (eeprom_current_address >> 8) & 0xFF);
        i2c_context_transmission_write(&instance->i2c_context, (eeprom_current_address >> 0) & 0xFF);

        for (int i = 0; i < current_split_length; i++)
        {
            i2c_context_transmission_write(&instance->i2c_context, buffer_data_current_position[i]);
        }

        int transmission_end_res = i2c_context_transmission_end(&instance->i2c_context, true);

        if (transmission_end_res != i2c_errors_none)
        {
            return eeprom_errors_i2c_failed;
        }

        buffer_data_current_position += current_split_length;
        eeprom_current_address += current_split_length;
        buffer_length_left -= current_split_length;
    }

    return i2c_errors_none;
}
