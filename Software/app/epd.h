#ifndef _APP_EPD_H
#define _APP_EPD_H

#include "ch32v20x.h"
#include <stdbool.h>

enum epd_errors
{
    epd_errors_none = 0,
    epd_errors_already_initialized = 1,
    epd_errors_invalid_interface = 2,
    epd_errors_spi_timeout = 3,
    epd_errors_invalid_parameters = 4,
};

enum epd_connectables
{
    epd_connectables_1
};

struct epd_context
{
    SPI_TypeDef* interface;
    SPI_InitTypeDef interface_init;
};

typedef struct epd_context epd_context_t;

unsigned char* epd_get_lut_ws_20_30();

// Control Operations

int epd_context_init(epd_context_t* instance);
bool epd_context_is_busy(epd_context_t* instance);
void epd_context_wait_busy(epd_context_t* instance);
void epd_context_reset(epd_context_t* instance);
void epd_context_spi_set_cs(epd_context_t* instance, bool v);
int epd_context_spi_transfer(epd_context_t* instance, unsigned char data);
int epd_context_send_command(epd_context_t* instance, unsigned char command);
int epd_context_send_data(epd_context_t* instance, unsigned char data);
int epd_context_display_init(epd_context_t* instance);
int epd_context_display_set_memory_area(epd_context_t* instance, int x_start, int y_start, int x_end, int y_end);
int epd_context_display_set_memory_pointer(epd_context_t* instance, int x, int y);
int epd_context_display_set_lut(epd_context_t* instance, unsigned char* lut);
int epd_context_display_set_lut_by_host(epd_context_t* instance, unsigned char* lut);

// Frame Operations

int epd_context_display_clear_frame_memory(epd_context_t* instance, unsigned char color);
int epd_context_display_set_frame_memory(epd_context_t* instance, unsigned char* image_buffer, int x, int y, int image_width, int image_height);
int epd_context_display_frame(epd_context_t* instance);

#endif // !_APP_SPI_H
