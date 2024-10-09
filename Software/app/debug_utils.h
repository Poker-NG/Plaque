#ifndef _APP_DEBUG_UTILS_H
#define _APP_DEBUG_UTILS_H

#include "config.h"
#include <stdbool.h>

// Messages

#define LOG_ERROR(fmt, ...) printf("[!%s:%s:%i!] " fmt "\r\n", __FILE__, __FUNCTION__, __LINE__ __VA_OPT__(, ) __VA_ARGS__)
#define LOG_DEBUG(fmt, ...) printf("[%s:%s:%i] " fmt "\r\n", __FILE__, __FUNCTION__, __LINE__ __VA_OPT__(, ) __VA_ARGS__)
#define LOG_FLUSH() printf("\r\n");

#if CONFIG_ENABLE_I2C_DEBUG
#define LOG_I2C_DEBUG(fmt, ...) printf("[%s:%i] " fmt "\r\n", __FUNCTION__, __LINE__ __VA_OPT__(, ) __VA_ARGS__)
#else
#define LOG_I2C_DEBUG(...)
#endif

// Debug Led

void debug_led_init(void);
void debug_led_set(bool value);

#define DEBUG_LED_ON() debug_led_set(true);
#define DEBUG_LED_OFF() debug_led_set(false);

#endif // !_APP_DEBUG_UTILS_H
