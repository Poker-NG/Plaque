#include "helper.h"

crc16_t crc16_update(unsigned char data, crc16_t* value)
{
    data = (data ^ (unsigned char)((*value) & 0x00FF));
    data = (data ^ (data << 4));
    *value = (*value >> 8) ^ ((crc16_t)data << 8) ^ ((crc16_t)data << 3) ^ ((crc16_t)data >> 4);

    return (*value);
}

crc16_t crc16_compute(unsigned char* buffer_data, unsigned int buffer_size, crc16_t seed)
{
    crc16_t checksum = seed;

    do
    {
        unsigned char current_data = *buffer_data++;
        crc16_update(current_data, &checksum);
    } while (--buffer_size);

    return checksum;
}
