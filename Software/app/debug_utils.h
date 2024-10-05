#ifndef _APP_DEBUG_UTILS_H
#define _APP_DEBUG_UTILS_H

#include "config.h"

#define LOG_ERROR(fmt, ...) printf("[!%s:%i!] " fmt "\r\n", __FUNCTION__, __LINE__ __VA_OPT__(, ) __VA_ARGS__)
#define LOG_DEBUG(fmt, ...) printf("[%s:%i] " fmt "\r\n", __FUNCTION__, __LINE__ __VA_OPT__(, ) __VA_ARGS__)
#define LOG_FLUSH() printf("\r\n");

#if CONFIG_ENABLE_I2C_DEBUG
#define LOG_I2C_DEBUG(fmt, ...) printf("[%s:%i] " fmt "\r\n", __FUNCTION__, __LINE__ __VA_OPT__(, ) __VA_ARGS__)
#else
#define LOG_I2C_DEBUG(...)
#endif

#endif // !_APP_DEBUG_UTILS_H
