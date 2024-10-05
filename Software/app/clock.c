#include "clock.h"
#include "debug.h"

#define TICK_FREQ_1KHz 1

// Header Implementation

__IO clock_tick_t clock_ms_tick = 0;

void clock_systick_init(void)
{
    SysTick->SR = 0;
    SysTick->CTLR = 0;
    SysTick->CNT = 0;
    SysTick->CMP = SystemCoreClock / 1000 - 1;
    SysTick->CTLR = 0xF;
    NVIC_SetPriority(SysTicK_IRQn, 0xFF);
    NVIC_EnableIRQ(SysTicK_IRQn);
}

clock_tick_t clock_get_tick(void)
{
    return clock_ms_tick;
}

bool clock_tick_compare_is_timeout(clock_tick_t start, clock_tick_t timeout)
{
    return (clock_get_tick() - start) > timeout;
}

void clock_delay_ms(unsigned int ms)
{
    if (ms != 0)
    {
        uint32_t start = clock_get_tick();
        do
        {
        } while (clock_get_tick() - start < ms);
    }
}

// Interrupt Handler

void SysTick_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void SysTick_Handler(void)
{
    clock_ms_tick += TICK_FREQ_1KHz;
    SysTick->SR = 0;

    asm("mret");
}
