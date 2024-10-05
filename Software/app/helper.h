#ifndef _APP_HELPER_H
#define _APP_HELPER_H

typedef unsigned short crc16_t;

crc16_t crc16_update(unsigned char data, crc16_t* value);
crc16_t crc16_compute(unsigned char* buffer_data, unsigned int buffer_length, crc16_t seed);

#endif // !_APP_HELPER_H
