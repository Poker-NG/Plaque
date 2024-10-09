#include "debug_utils.h"
#include "ch32v20x.h"
#include "ch32v20x_gpio.h"

void debug_led_init(void)
{
    GPIO_InitTypeDef led = {.GPIO_Mode = GPIO_Mode_Out_PP, .GPIO_Pin = GPIO_Pin_1, .GPIO_Speed = GPIO_Speed_50MHz};

    GPIO_Init(GPIOB, &led);
}

void debug_led_set(bool value)
{
    GPIO_WriteBit(GPIOB, GPIO_Pin_1, value == true ? Bit_SET : Bit_RESET);
}
