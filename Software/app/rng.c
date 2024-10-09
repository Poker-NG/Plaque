#include "rng.h"
#include "ch32v20x_rcc.h"
#include "clock.h"
#include <stdlib.h>

void rng_init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC2 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef pin_init;
    pin_init.GPIO_Pin = GPIO_Pin_0;
    pin_init.GPIO_Mode = GPIO_Mode_AIN;

    GPIO_Init(GPIOA, &pin_init); // PA0
    GPIO_Init(GPIOB, &pin_init); // PB0

    ADC_InitTypeDef adc_init;
    adc_init.ADC_Mode = ADC_Mode_Independent;
    adc_init.ADC_ScanConvMode = DISABLE;
    adc_init.ADC_ContinuousConvMode = ENABLE;
    adc_init.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    adc_init.ADC_DataAlign = ADC_DataAlign_Right;
    adc_init.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &adc_init);
    ADC_Init(ADC2, &adc_init);

    ADC_Cmd(ADC1, ENABLE);
    ADC_Cmd(ADC2, ENABLE);

    ADC_ResetCalibration(ADC1);
    ADC_ResetCalibration(ADC2);

    while (ADC_GetResetCalibrationStatus(ADC1) || ADC_GetResetCalibrationStatus(ADC2))
    {
    }

    ADC_StartCalibration(ADC1);
    ADC_StartCalibration(ADC2);

    while (ADC_GetCalibrationStatus(ADC1) || ADC_GetCalibrationStatus(ADC2))
    {
    }

    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    ADC_SoftwareStartConvCmd(ADC2, ENABLE);
}

unsigned int rng_read_raw(void)
{
    while ((!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC)) || (!ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC)))
    {
    }

    return ADC1->RDATAR ^ ((ADC2->RDATAR & 0xF) << 11);
}

void rng_do_seed_quick(void)
{
    srand(rng_read_raw());
}

void rng_do_seed_long(void)
{
    // Let's code up some random shit to make it random somehow.

    for (unsigned int r = 0; r < (rng_read_raw() % 0x1F); r++)
    {
        for (volatile unsigned int n = 0; n < (rng_read_raw() % 0xFFF); n++)
        {
            rand();
            asm("nop");
        }

        rand();
        clock_delay_ms(rng_read_raw() % 0xF);

        srand(rng_read_raw());
    }
}
