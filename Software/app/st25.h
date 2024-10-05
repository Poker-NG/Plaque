#ifndef _APP_ST25_H
#define _APP_ST25_H

#include "i2c.h"
#include <stdbool.h>

enum st25_errors
{
    st25_errors_none = 0,
    st25_errors_i2c_failed = 1
};

struct st25_context
{
    i2c_context_t i2c_context;
    unsigned char i2c_address_system;
    unsigned char i2c_address_data;
};

typedef struct st25_context st25_context_t;

int st25_context_init(st25_context_t* instance);
bool st25_context_is_connected(st25_context_t* instance);
int st25_context_io_read_multiple_bytes(st25_context_t* instance, unsigned char address, unsigned short register_address, unsigned int length, unsigned char* data);
int st25_context_io_write_multiple_bytes(st25_context_t* instance, unsigned char address, unsigned short register_address, unsigned char* data, unsigned int length);
unsigned char st25_context_get_device_revision(st25_context_t* instance);
bool st25_context_is_rf_field_detected(st25_context_t* instance);

#endif // !_APP_ST25_H
