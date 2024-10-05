#include "epd.h"
#include "ch32v20x.h"
#include "ch32v20x_exti.h"
#include "ch32v20x_gpio.h"
#include "ch32v20x_rcc.h"
#include "ch32v20x_spi.h"
#include "clock.h"
#include "debug.h"
#include "i2c.h"
#include "spi.h"
#include "system_ch32v20x.h"

static int epd_display_width = 128;
static int epd_display_height = 296;

int epd_context_init(epd_context_t* instance)
{
    if (instance->interface != NULL)
    {
        return epd_errors_already_initialized;
    }

    instance->interface = SPI1;

    if (instance->interface == NULL)
    {
        return epd_errors_invalid_interface;
    }

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef pins_init = {0};

    pins_init.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    pins_init.GPIO_Speed = GPIO_Speed_50MHz;
    pins_init.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &pins_init);
    // BUSY (PB9)
    pins_init.GPIO_Pin = GPIO_Pin_9;
    pins_init.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    pins_init.GPIO_Speed = 0;
    GPIO_Init(GPIOB, &pins_init);
    // RST (PB8)
    pins_init.GPIO_Pin = GPIO_Pin_8;
    pins_init.GPIO_Mode = GPIO_Mode_Out_PP;
    pins_init.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &pins_init);

    return epd_errors_none;
}

bool epd_context_is_busy(epd_context_t* instance)
{
    return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9) == Bit_SET;
}

void epd_context_wait_busy(epd_context_t* instance)
{
    while (true)
    {
        if (epd_context_is_busy(instance) == false)
        {
            break;
        }

        clock_delay_ms(5);
    }

    clock_delay_ms(5);
}

void epd_context_reset(epd_context_t* instance)
{
    GPIO_WriteBit(GPIOB, GPIO_Pin_8, Bit_SET);
    clock_delay_ms(20);
    GPIO_WriteBit(GPIOB, GPIO_Pin_8, Bit_RESET);
    clock_delay_ms(5);
    GPIO_WriteBit(GPIOB, GPIO_Pin_8, Bit_SET);
    clock_delay_ms(20);
}

void epd_context_spi_set_cs(epd_context_t* instance, bool v)
{
    if (v == 1)
    {
        GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);
    }
    else
    {
        GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_RESET);
    }
}

void epd_context_spi_set_clk(epd_context_t* instance, bool v)
{
    if (v == 1)
    {
        GPIO_WriteBit(GPIOA, GPIO_Pin_5, Bit_SET);
    }
    else
    {
        GPIO_WriteBit(GPIOA, GPIO_Pin_5, Bit_RESET);
    }
}

void epd_context_spi_set_mosi(epd_context_t* instance, bool v)
{
    if (v == 1)
    {
        GPIO_WriteBit(GPIOA, GPIO_Pin_7, Bit_SET);
    }
    else
    {
        GPIO_WriteBit(GPIOA, GPIO_Pin_7, Bit_RESET);
    }
}

int epd_context_spi_transfer(epd_context_t* instance, unsigned char data)
{
    u8 i;
    epd_context_spi_set_cs(instance, false);
    for (i = 0; i < 8; i++)
    {
        epd_context_spi_set_clk(instance, false);
        if (data & 0x80)
        {
            epd_context_spi_set_mosi(instance, true);
        }
        else
        {
            epd_context_spi_set_mosi(instance, false);
        }
        epd_context_spi_set_clk(instance, true);
        data <<= 1;
    }
    epd_context_spi_set_cs(instance, true);

    return i2c_errors_none;
}

int epd_context_send_command(epd_context_t* instance, unsigned char command)
{
    GPIO_WriteBit(GPIOA, GPIO_Pin_6, Bit_RESET);
    GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_RESET);
    int result = epd_context_spi_transfer(instance, command);
    GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);

    return result;
}

int epd_context_send_data(epd_context_t* instance, unsigned char data)
{
    GPIO_WriteBit(GPIOA, GPIO_Pin_6, Bit_SET);
    GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_RESET);
    int result = epd_context_spi_transfer(instance, data);
    GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);

    return result;
}

int epd_context_display_init(epd_context_t* instance)
{
    int res = 0;

    epd_context_reset(instance);

    epd_context_wait_busy(instance);
    res += epd_context_send_command(instance, 0x12); // SWRESET
    epd_context_wait_busy(instance);

    res += epd_context_send_command(instance, 0x01);
    res += epd_context_send_data(instance, 0x27);
    res += epd_context_send_data(instance, 0x01);
    res += epd_context_send_data(instance, 0x00);

    res += epd_context_send_command(instance, 0x11);
    res += epd_context_send_data(instance, 0x03);

    res += epd_context_display_set_memory_area(instance, 0, 0, epd_display_width - 1, epd_display_height - 1);

    res += epd_context_send_command(instance, 0x21); //  Display update control
    res += epd_context_send_data(instance, 0x00);
    res += epd_context_send_data(instance, 0x80);

    res += epd_context_display_set_memory_pointer(instance, 0, 0);

    epd_context_wait_busy(instance);

    res += epd_context_display_set_lut_by_host(instance, epd_get_lut_ws_20_30());

    return res;
}

int epd_context_display_set_memory_area(epd_context_t* instance, int x_start, int y_start, int x_end, int y_end)
{
    int res = 0;

    res += epd_context_send_command(instance, 0x44);
    res += epd_context_send_data(instance, (x_start >> 3) & 0xFF);
    res += epd_context_send_data(instance, (x_end >> 3) & 0xFF);
    res += epd_context_send_command(instance, 0x45);
    res += epd_context_send_data(instance, y_start & 0xFF);
    res += epd_context_send_data(instance, (y_start >> 8) & 0xFF);
    res += epd_context_send_data(instance, y_end & 0xFF);
    res += epd_context_send_data(instance, (y_end >> 8) & 0xFF);

    return res;
}

int epd_context_display_set_memory_pointer(epd_context_t* instance, int x, int y)
{
    int res = 0;

    res += epd_context_send_command(instance, 0x4E);
    /* x point must be the multiple of 8 or the last 3 bits will be ignored */
    res += epd_context_send_data(instance, (x >> 3) & 0xFF);
    res += epd_context_send_command(instance, 0x4F);
    res += epd_context_send_data(instance, y & 0xFF);
    res += epd_context_send_data(instance, (y >> 8) & 0xFF);

    epd_context_wait_busy(instance);

    return res;
}

int epd_context_display_set_lut(epd_context_t* instance, unsigned char* lut)
{
    int res = 0;

    res += epd_context_send_command(instance, 0x32);

    for (unsigned char count = 0; count < 153; count++)
    {
        res += epd_context_send_data(instance, lut[count]);
    }

    epd_context_wait_busy(instance);

    return res;
}

int epd_context_display_set_lut_by_host(epd_context_t* instance, unsigned char* lut)
{
    int res = 0;

    res += epd_context_display_set_lut(instance, lut);
    res += epd_context_send_command(instance, 0x3f);
    res += epd_context_send_data(instance, *(lut + 153));
    res += epd_context_send_command(instance, 0x03); // gate voltage
    res += epd_context_send_data(instance, *(lut + 154));
    res += epd_context_send_command(instance, 0x04);      // source voltage
    res += epd_context_send_data(instance, *(lut + 155)); // VSH
    res += epd_context_send_data(instance, *(lut + 156)); // VSH2
    res += epd_context_send_data(instance, *(lut + 157)); // VSL
    res += epd_context_send_command(instance, 0x2c);      // VCOM
    res += epd_context_send_data(instance, *(lut + 158));

    return res;
}

int epd_context_display_clear_frame_memory(epd_context_t* instance, unsigned char color)
{
    int res = 0;

    res += epd_context_display_set_memory_area(instance, 0, 0, epd_display_width - 1, epd_display_height - 1);
    res += epd_context_display_set_memory_pointer(instance, 0, 0);

    res += epd_context_send_command(instance, 0x24);

    for (int i = 0; i < epd_display_width / 8 * epd_display_height; i++)
    {
        res += epd_context_send_data(instance, color);
    }

    res += epd_context_send_command(instance, 0x26);

    for (int i = 0; i < epd_display_width / 8 * epd_display_height; i++)
    {
        res += epd_context_send_data(instance, color);
    }

    return res;
}

int epd_context_display_set_frame_memory(epd_context_t* instance, unsigned char* image_buffer, int x, int y, int image_width, int image_height)
{
    int x_end = 0;
    int y_end = 0;

    x &= 0xF8;
    image_width &= 0xF8;
    if (x + image_width >= epd_display_width)
    {
        x_end = epd_display_width - 1;
    }
    else
    {
        x_end = x + image_width - 1;
    }
    if (y + image_height >= epd_display_height)
    {
        y_end = epd_display_height - 1;
    }
    else
    {
        y_end = y + image_height - 1;
    }

    epd_context_display_set_memory_area(instance, x, y, x_end, y_end);
    epd_context_display_set_memory_pointer(instance, x, y);

    epd_context_send_command(instance, 0x24);

    for (int j = 0; j < y_end - y + 1; j++)
    {
        for (int i = 0; i < (x_end - x + 1) / 8; i++)
        {
            epd_context_send_data(instance, image_buffer[i + j * (image_width / 8)]);
        }
    }

    return epd_errors_none;
}

int epd_context_display_frame(epd_context_t* instance)
{
    int res = 0;

    res += epd_context_send_command(instance, 0x22); // Display Update Control 2
    res += epd_context_send_data(instance, 0xc7);    // Enable clock signal
    res += epd_context_send_command(instance, 0x20); // Master Activation
    epd_context_wait_busy(instance);

    return res;
}
