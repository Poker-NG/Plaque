#ifndef _APP_I2C_H
#define _APP_I2C_H

#include "ch32v20x.h"
#include "ch32v20x_i2c.h"

#include <stdbool.h>

#define I2C_CONNECTION_TRANSMISSION_BUFFER_SIZE_MAX 0x100

enum i2c_errors
{
    i2c_errors_none = 0,
    i2c_errors_already_initialized = 1,
    i2c_errors_invalid_connectable = 2,
    i2c_errors_buffer_eof = 3,
    i2c_errors_timeout = 4,
};

enum i2c_connectables
{
    i2c_connectables_invalid = 0,
    i2c_connectables_1,
    i2c_connectables_2
};

struct i2c_context
{
    int connectable;
    I2C_TypeDef* interface;
    I2C_InitTypeDef interface_init;

    struct
    {
        bool is_transmitting;
        unsigned char address;
        unsigned char buffer_data[I2C_CONNECTION_TRANSMISSION_BUFFER_SIZE_MAX];
        unsigned short buffer_cursor;
    } transmission;
};

typedef struct i2c_context i2c_context_t;

I2C_TypeDef* i2c_context_get_associated_typedef(int connectable);
int i2c_context_init(i2c_context_t* instance, int connectable);
int i2c_context_master_write(i2c_context_t* instance, unsigned char device_address, unsigned char* data, unsigned short size, bool send_stop);
int i2c_context_master_read(i2c_context_t* instance, unsigned char device_address, unsigned char* data, unsigned short size);
void i2c_context_transmission_begin(i2c_context_t* instance, unsigned char address);
int i2c_context_transmission_write(i2c_context_t* instance, unsigned char value);
int i2c_context_transmission_read(i2c_context_t* instance, unsigned char* value);
unsigned char* i2c_context_transmission_get_buffer_data(i2c_context_t* instance);
int i2c_context_transmission_get_buffer_max_length(i2c_context_t* instance);
unsigned short i2c_context_transmission_get_buffer_length(i2c_context_t* instance);
int i2c_context_transmission_set_buffer_length(i2c_context_t* instance, unsigned short buffer_length);
int i2c_context_transmission_end(i2c_context_t* instance, bool send_stop);
int i2c_context_transmission_request_from(i2c_context_t* instance, unsigned char address, unsigned short length);

#endif // !_APP_I2C_H
