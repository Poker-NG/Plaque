#include "st25.h"
#include "i2c.h"
#include <string.h>

#define ST25_I2C_ADDRESS_SYSTEM 0x57
#define ST25_I2C_ADDRESS_DATA 0x53
#define ST25_I2C_MAX_RETRIES 12
#define ST25_I2C_MAX_CHUNK_BYTES 32

#define ST25_REG_UID 0x0018
#define ST25_REG_IC_REV 0x0020

#define ST25_DYN_REG_EH_CTRL_DYN 0x2002

#define ST25_BIT_EH_CTRL_DYN_FIELD_ON (1 << 2)

int st25_context_init(st25_context_t* instance)
{
    memset(&instance->i2c_context, 0, sizeof(instance->i2c_context));

    instance->i2c_address_system = ST25_I2C_ADDRESS_SYSTEM;
    instance->i2c_address_data = ST25_I2C_ADDRESS_DATA;

    if (i2c_context_init(&instance->i2c_context, i2c_connectables_1) != i2c_errors_none)
    {
        return st25_errors_i2c_failed;
    }

    return st25_errors_none;
}

bool st25_context_is_connected(st25_context_t* instance)
{
    i2c_context_transmission_begin(&instance->i2c_context, instance->i2c_address_system);
    return i2c_context_transmission_end(&instance->i2c_context, true) == i2c_errors_none;
}

int st25_context_io_read_multiple_bytes(st25_context_t* instance, unsigned char address, unsigned short register_address, unsigned int length, unsigned char* data)
{
    unsigned short current_register_address = register_address;
    unsigned int length_left = length;

    int err = 0;

    while (length_left > 0)
    {
        unsigned int current_chunk_length = (length_left >= ST25_I2C_MAX_CHUNK_BYTES) ? ST25_I2C_MAX_CHUNK_BYTES : length_left;

        for (int currentRetry = 0; currentRetry < ST25_I2C_MAX_RETRIES; currentRetry++)
        {
            i2c_context_transmission_begin(&instance->i2c_context, address);
            i2c_context_transmission_write(&instance->i2c_context, (unsigned char)(current_register_address >> 8) & 0xFF);
            i2c_context_transmission_write(&instance->i2c_context, (unsigned char)(current_register_address >> 0) & 0xFF);

            err = i2c_context_transmission_end(&instance->i2c_context, false);

            if (err != i2c_errors_none)
            {
                continue;
            }

            err = i2c_context_transmission_request_from(&instance->i2c_context, address, current_chunk_length);

            if (err != i2c_errors_none)
            {
                continue;
            }
            else
            {
                break;
            }
        }

        if (err != i2c_errors_none)
        {
            return st25_errors_i2c_failed;
        }

        memcpy(data, i2c_context_transmission_get_buffer_data(&instance->i2c_context), current_chunk_length);

        current_register_address += current_chunk_length;
        length_left -= current_chunk_length;
        data += current_chunk_length;
    }

    return st25_errors_none;
}

int st25_context_io_write_multiple_bytes(st25_context_t* instance, unsigned char address, unsigned short register_address, unsigned char* data, unsigned int length)
{
    unsigned short current_register_address = register_address;
    unsigned int length_left = length;
    unsigned char* current_data = data;

    int err = 0;

    while (length_left > 0)
    {
        unsigned int current_chunk_length = (length_left >= ST25_I2C_MAX_CHUNK_BYTES) ? ST25_I2C_MAX_CHUNK_BYTES : length_left;

        for (int currentRetry = 0; currentRetry < ST25_I2C_MAX_RETRIES; currentRetry++)
        {
            i2c_context_transmission_begin(&instance->i2c_context, address);
            i2c_context_transmission_write(&instance->i2c_context, (unsigned char)(current_register_address >> 8) & 0xFF);
            i2c_context_transmission_write(&instance->i2c_context, (unsigned char)(current_register_address >> 0) & 0xFF);

            for (unsigned int current_chunk_index = 0; current_chunk_index < current_chunk_length; current_chunk_index++)
            {
                i2c_context_transmission_write(&instance->i2c_context, current_data[current_chunk_index]);
            }

            err = i2c_context_transmission_end(&instance->i2c_context, true);

            if (err == i2c_errors_none)
            {
                break;
            }
            else
            {
                // trigger exit and return err
                length_left = 0;
            }
        }

        current_register_address += current_chunk_length;
        length_left -= current_chunk_length;
        current_data += current_chunk_length;
    }

    return st25_errors_none + err;
}

unsigned char st25_context_get_device_revision(st25_context_t* instance)
{
    unsigned char reg = 0;

    if (st25_context_io_read_multiple_bytes(instance, instance->i2c_address_system, ST25_REG_IC_REV, 1, &reg) != st25_errors_none)
    {
        return 0;
    }

    return reg;
}

bool st25_context_get_device_uid(st25_context_t* instance, unsigned char* uid)
{
    if (st25_context_io_read_multiple_bytes(instance, instance->i2c_address_system, ST25_REG_UID, 8, uid) != st25_errors_none)
    {
        return 0;
    }

    return true;
}

bool st25_context_is_rf_field_detected(st25_context_t* instance)
{
    unsigned char reg = 0;

    if (st25_context_io_read_multiple_bytes(instance, instance->i2c_address_data, ST25_DYN_REG_EH_CTRL_DYN, 1, &reg) != st25_errors_none)
    {
        return false;
    }

    return (reg & ST25_BIT_EH_CTRL_DYN_FIELD_ON) > 0;
}
