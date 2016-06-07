/*
 * file : adc.c
 * author : Ian
 */

#include "adc.h"

uint32_t samples[2];
ADC_HandleTypeDef g_AdcHandle;
DMA_HandleTypeDef  hdma_adc;
TIM_HandleTypeDef  htim_adc;

void ErrorHandler(void) {
	while(1){
		;
	}
}

void HAL_ADC_MspInit(ADC_HandleTypeDef * hadc) {

	ADCx_CLK_ENABLE();
	ADCx_PIN_CLK_ENABLE();
	DMAx_CLK_ENABLE();
	__TIM2_CLK_ENABLE();

	// Timer1 runs on APB1
	// SystemClock = 180MHz, APB2 is SYSCLK/4 so 45MHz, max clock for timer block
	// TIMCLK = APB2 x 2, runs at 90MHz
	htim_adc.Instance = TIM2;
	htim_adc.Init.ClockDivision = 0;
	htim_adc.Init.Period = 10000 - 1;
	htim_adc.Init.Prescaler = 9000;
	htim_adc.Init.CounterMode = TIM_COUNTERMODE_UP;
	if(HAL_TIM_OC_Init(&htim_adc) != HAL_OK) {
		ErrorHandler();
	}

	TIM_OC_InitTypeDef tim_oc;
	tim_oc.OCMode = TIM_OCMODE_ACTIVE;
	tim_oc.OCPolarity = TIM_OCPOLARITY_HIGH;
	tim_oc.Pulse = 0;

	HAL_TIM_OC_ConfigChannel(&htim_adc,&tim_oc,TIM_CHANNEL_2);
	HAL_TIM_OC_Start(&htim_adc,TIM_CHANNEL_2);

	GPIO_InitTypeDef gpioInit;
	gpioInit.Pin = ADCx_CHANNEL_PIN_NUMBER | ADCy_CHANNEL_PIN_NUMBER;
	gpioInit.Mode = GPIO_MODE_ANALOG;
	gpioInit.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(ADCx_CHANNEL_PORT, &gpioInit);

	/*##-3- Configure the DMA streams ##########################################*/
	/* Set the parameters to be configured */
	hdma_adc.Instance = ADCx_DMA_STREAM;

	hdma_adc.Init.Channel  = ADCx_DMA_CHANNEL;
	hdma_adc.Init.Direction = DMA_PERIPH_TO_MEMORY;
	hdma_adc.Init.PeriphInc = DMA_PINC_DISABLE;
	hdma_adc.Init.MemInc = DMA_MINC_ENABLE;
	hdma_adc.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
	hdma_adc.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
	hdma_adc.Init.Mode = DMA_CIRCULAR;
	hdma_adc.Init.Priority = DMA_PRIORITY_HIGH;
	hdma_adc.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
	hdma_adc.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
	hdma_adc.Init.MemBurst = DMA_MBURST_SINGLE;
	hdma_adc.Init.PeriphBurst = DMA_PBURST_SINGLE;

	HAL_DMA_Init(&hdma_adc);

	/* Associate the initialized DMA handle to the the ADC handle */
	__HAL_LINKDMA(hadc, DMA_Handle, hdma_adc);

	/*##-4- Configure the NVIC for DMA #########################################*/
	/* NVIC configuration for DMA transfer complete interrupt */
	HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
}

void adc_init(void) {
	g_AdcHandle.Instance = ADCx;

	// Need the ADCCLK to be <36MHz
	g_AdcHandle.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV4;
	g_AdcHandle.Init.Resolution = ADC_RESOLUTION_12B;
	g_AdcHandle.Init.ScanConvMode = ENABLE;
	g_AdcHandle.Init.ContinuousConvMode = ENABLE;
	g_AdcHandle.Init.DiscontinuousConvMode = DISABLE;
	g_AdcHandle.Init.NbrOfDiscConversion = 0;
	g_AdcHandle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
	// Use reload event from TIMER1, running so time is 50ms
	g_AdcHandle.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T2_CC2;
	g_AdcHandle.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	g_AdcHandle.Init.NbrOfConversion = 2;
	g_AdcHandle.Init.DMAContinuousRequests = ENABLE;
	g_AdcHandle.Init.EOCSelection = DISABLE;

	HAL_ADC_Init(&g_AdcHandle);

	ADC_ChannelConfTypeDef adcChannel;
	adcChannel.Channel = ADCx_CHANNEL;
	adcChannel.Rank = 1;
	adcChannel.SamplingTime = ADC_SAMPLETIME_3CYCLES;
	adcChannel.Offset = 0;

	if (HAL_ADC_ConfigChannel(&g_AdcHandle, &adcChannel) != HAL_OK)
	{
		ErrorHandler();
	}

	adcChannel.Channel = ADCy_CHANNEL;
	adcChannel.Rank = 2;
	if (HAL_ADC_ConfigChannel(&g_AdcHandle, &adcChannel) != HAL_OK)
	{
		ErrorHandler();
	}

}

void adc_sample(void) {
	if(HAL_OK != HAL_ADC_Start_DMA(&g_AdcHandle,samples,2)) {
		ErrorHandler();
	}
}
