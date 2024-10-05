#ifndef _APP_EEPROM_H
#define _APP_EEPROM_H

#include "i2c.h"
#include <stdbool.h>

enum eeprom_errors
{
    eeprom_errors_none = 0,
    eeprom_errors_i2c_failed = 1,
    eeprom_errors_failed_to_ping = 2,
    eeprom_errors_eof = 3,
};

union external_eeprom_device_address
{
    struct
    {
        unsigned char e0 : 1;
        unsigned char e1 : 1;
        unsigned char e2 : 1;
        unsigned char suffix : 4;
    } bits;

    unsigned char raw;
};

struct external_eeprom_context
{
    i2c_context_t i2c_context;
    unsigned char i2c_address;
};

typedef struct external_eeprom_context external_eeprom_context_t;

int external_eeprom_context_init(external_eeprom_context_t* instance, int i2c_connectable, unsigned char i2c_address);
bool external_eeprom_context_check_if_is_connected(external_eeprom_context_t* instance);
int external_eeprom_context_read_byte(external_eeprom_context_t* instance, unsigned short address, unsigned char* value);
int external_eeprom_context_read_bytes(external_eeprom_context_t* instance, unsigned short address, unsigned char* buffer_data, unsigned short buffer_length);
int external_eeprom_context_write_byte(external_eeprom_context_t* instance, unsigned short address, unsigned char value);
int external_eeprom_context_write_bytes(external_eeprom_context_t* instance, unsigned short address, unsigned char* buffer_data, unsigned short buffer_length);

#endif // !_APP_EEPROM_H
