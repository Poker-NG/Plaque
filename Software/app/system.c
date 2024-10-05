#include "system.h"
#include "ch32v20x_misc.h"
#include "clock.h"

void system_init(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    USART_Printf_Init(115200);
    SystemCoreClockUpdate();
    Delay_Init();
    clock_systick_init();
}
