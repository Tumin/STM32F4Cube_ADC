/*
 * file : adc.c
 * author : Ian
 */

#include "adc.h"

ADC_HandleTypeDef g_AdcHandle;

void adc_init(void) {
	GPIO_InitTypeDef gpioInit;

	ADCx_CLK_ENABLE();
	ADCx_PIN_CLK_ENABLE();

	gpioInit.Pin = ADCx_CHANNEL_PIN_NUMBER;
	gpioInit.Mode = GPIO_MODE_ANALOG;
	gpioInit.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(ADCx_CHANNEL_PORT, &gpioInit);

	ADC_ChannelConfTypeDef adcChannel;

	g_AdcHandle.Instance = ADCx;

	g_AdcHandle.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV2;
	g_AdcHandle.Init.Resolution = ADC_RESOLUTION_12B;
	g_AdcHandle.Init.ScanConvMode = DISABLE;
	g_AdcHandle.Init.ContinuousConvMode = ENABLE;
	g_AdcHandle.Init.DiscontinuousConvMode = DISABLE;
	g_AdcHandle.Init.NbrOfDiscConversion = 0;
	g_AdcHandle.Init.ExternalTrigConvEdge = 0; // Ignored since using software start
	g_AdcHandle.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	g_AdcHandle.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	g_AdcHandle.Init.NbrOfConversion = 1;
	g_AdcHandle.Init.DMAContinuousRequests = ENABLE;
	g_AdcHandle.Init.EOCSelection = DISABLE;

	HAL_ADC_Init(&g_AdcHandle);

	adcChannel.Channel = ADCx_CHANNEL;
	adcChannel.Rank = 1;
	adcChannel.SamplingTime = ADC_SAMPLETIME_480CYCLES;
	adcChannel.Offset = 0;

	if (HAL_ADC_ConfigChannel(&g_AdcHandle, &adcChannel) != HAL_OK)
	{
		while(1);
	}
}

// Trying to ill-advisedly implement a 2 channel scan
// on 1 ADC
void adc_sample(xy_val_t * adc) {
	HAL_ADC_Start(&g_AdcHandle);
	HAL_ADC_PollForConversion(&g_AdcHandle,1000); //1s timeout
	adc->x = HAL_ADC_GetValue(&g_AdcHandle);
}
