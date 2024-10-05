#include "i2c.h"
#include "ch32v20x.h"
#include "ch32v20x_i2c.h"
#include "clock.h"
#include "debug_utils.h"

#include <string.h>

#define I2C_TRANSMISSION_TIMEOUT_TICK 3000

// utility implementation

unsigned int i2c_sanitize_timing(unsigned int frequency)
{
    if (frequency <= 100000)
    {
        return 100000;
    }
    else if (frequency <= 400000)
    {
        return 400000;
    }
    else if (frequency <= 1000000)
    {
        return 1000000;
    }

    // Default fallback
    return 100000;
}

// header implementation

I2C_TypeDef* i2c_context_get_associated_typedef(int connectable)
{
    switch (connectable)
    {
    case i2c_connectables_1:
        return I2C1;
    case i2c_connectables_2:
        return I2C2;
    default:
        return NULL;
    }
}

int i2c_context_init(i2c_context_t* instance, int connectable)
{
    if (instance->interface != NULL)
    {
        return i2c_errors_already_initialized;
    }

    instance->interface = i2c_context_get_associated_typedef(connectable);

    if (instance->interface == NULL)
    {
        return i2c_errors_invalid_connectable;
    }

    instance->connectable = connectable;

    // Enable I2C clock and IRQ's

    switch (instance->connectable)
    {
    case i2c_connectables_1:
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

        GPIO_InitTypeDef pins_init = {0};

        pins_init.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_6;
        pins_init.GPIO_Speed = GPIO_Speed_50MHz;
        pins_init.GPIO_Mode = GPIO_Mode_AF_OD;
        GPIO_Init(GPIOB, &pins_init);
        break;
    }
    case i2c_connectables_2:
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);

        GPIO_InitTypeDef pins_init = {0};

        pins_init.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
        pins_init.GPIO_Speed = GPIO_Speed_50MHz;
        pins_init.GPIO_Mode = GPIO_Mode_AF_OD;
        GPIO_Init(GPIOB, &pins_init);
        break;
    }
    default:
        break;
    }

    instance->interface_init.I2C_ClockSpeed = 100000;
    instance->interface_init.I2C_Mode = I2C_Mode_I2C;

    if (instance->interface_init.I2C_ClockSpeed > 100000)
    {
        instance->interface_init.I2C_DutyCycle = I2C_DutyCycle_16_9;
    }
    else
    {
        instance->interface_init.I2C_DutyCycle = I2C_DutyCycle_2;
    }

    instance->interface_init.I2C_OwnAddress1 = (0x1 << 1);
    instance->interface_init.I2C_Ack = I2C_Ack_Enable;
    instance->interface_init.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;

    I2C_Init(instance->interface, &instance->interface_init);
    I2C_Cmd(instance->interface, ENABLE);
    I2C_AcknowledgeConfig(instance->interface, ENABLE);

    return i2c_errors_none;
}

int i2c_context_master_write(i2c_context_t* instance, unsigned char device_address, unsigned char* data, unsigned short size, bool send_stop)
{
    clock_delay_ms(10);

    int ret = i2c_errors_none;
    clock_tick_t tick = 0;

    {
        I2C_AcknowledgeConfig(instance->interface, ENABLE);

        tick = clock_get_tick();
        while (I2C_GetFlagStatus(instance->interface, I2C_FLAG_BUSY) != RESET)
        {
            if (clock_tick_compare_is_timeout(tick, 10))
            {
                I2C_GenerateSTOP(instance->interface, ENABLE);
                LOG_I2C_DEBUG("I2C Busy timeout, trying to recover.");
                return i2c_errors_timeout;
            }
        }

        I2C_GenerateSTART(instance->interface, ENABLE);
        tick = clock_get_tick();
        while (!I2C_CheckEvent(instance->interface, I2C_EVENT_MASTER_MODE_SELECT))
        {
            if (clock_tick_compare_is_timeout(tick, 10))
            {
                I2C_GenerateSTOP(instance->interface, ENABLE);
                LOG_I2C_DEBUG("Event Master Mode Select timeout, trying to recover.");
                return i2c_errors_timeout;
            }
        }

        I2C_Send7bitAddress(instance->interface, device_address, I2C_Direction_Transmitter);
        tick = clock_get_tick();
        while (!I2C_CheckEvent(instance->interface, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
        {
            if (clock_tick_compare_is_timeout(tick, 10))
            {
                I2C_GenerateSTOP(instance->interface, ENABLE);
                LOG_I2C_DEBUG("Event Master Transmitter timeout, trying to recover.");
                return i2c_errors_timeout;
            }
        }

        // MMOLE: Support for I2C scanning: allow only sending the address (without actual data)
        if (size)
        {
            while (size)
            {
                if (I2C_GetFlagStatus(instance->interface, I2C_FLAG_TXE) != RESET)
                {
                    I2C_SendData(instance->interface, *data++);
                    size--;
                }
            }

            while (!I2C_CheckEvent(instance->interface, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
            {
            }
        } // if(size)

        if (send_stop)
        {
            I2C_GenerateSTOP(instance->interface, ENABLE);
        }
    }
    return ret;
}

int i2c_context_master_read(i2c_context_t* instance, unsigned char device_address, unsigned char* data, unsigned short size)
{
    clock_delay_ms(10);

    int ret = i2c_errors_none;
    uint32_t tickstart = clock_get_tick();

    I2C_GenerateSTART(instance->interface, ENABLE);
    while (!I2C_CheckEvent(instance->interface, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if ((clock_get_tick() - tickstart) > I2C_TRANSMISSION_TIMEOUT_TICK)
        {
            return -1;
        }
    }

    I2C_Send7bitAddress(instance->interface, device_address, I2C_Direction_Receiver);
    while (!I2C_CheckEvent(instance->interface, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
    {
        if ((clock_get_tick() - tickstart) > I2C_TRANSMISSION_TIMEOUT_TICK)
        {
            return -2;
        }
    }

    I2C_AcknowledgeConfig(instance->interface, ENABLE); // MMOLE fix: every byte except last needs to ACK, also on repeated reads
    while (size)
    {
        if (size == 1)
            I2C_AcknowledgeConfig(instance->interface, DISABLE); // last data needn't to ACK
        while (I2C_GetFlagStatus(instance->interface, I2C_FLAG_RXNE) == RESET)
        {
            // MMOLE: at occasion the CH32 would just hang while reading the response. Still unknown why. but adding a timeout may prevent this
            if ((clock_get_tick() - tickstart) > I2C_TRANSMISSION_TIMEOUT_TICK)
            {
                I2C_GenerateSTOP(instance->interface, ENABLE);
                return -3;
            }
        }
        *data++ = I2C_ReceiveData(instance->interface);
        size--;
    }
    I2C_GenerateSTOP(instance->interface, ENABLE);
    return ret;
}

void i2c_context_transmission_begin(i2c_context_t* instance, unsigned char address)
{
    instance->transmission.is_transmitting = true;
    instance->transmission.address = (address << 1);
    instance->transmission.buffer_cursor = 0;

    memset(instance->transmission.buffer_data, 0, sizeof(instance->transmission.buffer_data));
}

int i2c_context_transmission_write(i2c_context_t* instance, unsigned char value)
{
    if (instance->transmission.buffer_cursor > sizeof(instance->transmission.buffer_data))
    {
        return i2c_errors_buffer_eof;
    }

    instance->transmission.buffer_data[instance->transmission.buffer_cursor] = value;
    instance->transmission.buffer_cursor += 1;

    return i2c_errors_none;
}

int i2c_context_transmission_read(i2c_context_t* instance, unsigned char* value)
{
    if (instance->transmission.buffer_cursor > sizeof(instance->transmission.buffer_data))
    {
        return i2c_errors_buffer_eof;
    }

    *value = instance->transmission.buffer_data[instance->transmission.buffer_cursor];
    instance->transmission.buffer_cursor++;
    return i2c_errors_none;
}

unsigned char* i2c_context_transmission_get_buffer_data(i2c_context_t* instance)
{
    return instance->transmission.buffer_data;
}

int i2c_context_transmission_get_buffer_max_length(i2c_context_t* instance)
{
    return sizeof(instance->transmission.buffer_data);
}

unsigned short i2c_context_transmission_get_buffer_length(i2c_context_t* instance)
{
    return instance->transmission.buffer_cursor;
}

int i2c_context_transmission_set_buffer_length(i2c_context_t* instance, unsigned short buffer_length)
{
    if (buffer_length > sizeof(instance->transmission.buffer_data))
    {
        return i2c_errors_buffer_eof;
    }

    instance->transmission.buffer_cursor = buffer_length;

    return i2c_errors_none;
}

int i2c_context_transmission_end(i2c_context_t* instance, bool send_stop)
{
    int res = i2c_context_master_write(instance, instance->transmission.address, instance->transmission.buffer_data, instance->transmission.buffer_cursor, send_stop);

    instance->transmission.is_transmitting = false;
    instance->transmission.address = 0;
    instance->transmission.buffer_cursor = 0;

    return res;
}

int i2c_context_transmission_request_from(i2c_context_t* instance, unsigned char address, unsigned short length)
{
    if (length > I2C_CONNECTION_TRANSMISSION_BUFFER_SIZE_MAX)
    {
        return i2c_errors_buffer_eof;
    }

    instance->transmission.address = (unsigned char)(address << 1);
    instance->transmission.buffer_cursor = 0;

    memset(&instance->transmission.buffer_data, 0, sizeof(instance->transmission.buffer_data));

    return i2c_context_master_read(instance, instance->transmission.address, instance->transmission.buffer_data, length);
}
