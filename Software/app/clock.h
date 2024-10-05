#ifndef _APP_CLOCK_H
#define _APP_CLOCK_H

#include <stdbool.h>

typedef unsigned long long clock_tick_t;

void clock_systick_init(void);
clock_tick_t clock_get_tick(void);
bool clock_tick_compare_is_timeout(clock_tick_t start, clock_tick_t timeout);
void clock_delay_ms(unsigned int ms);

#endif // !_APP_CLOCK_H
