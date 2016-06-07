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

/*
 *  Explanation of below functionality
 *  The ADC can sample multiple channels in a single capture in scan mode
 *  We don't really want or need continuous capture because we're making
 *  a HID interface, and we certainly don't need very high accuracy
 *
 *  The ADC supports multiple channels. In the HAL driver, the order of the
 *  reads of each channel is done by "rank". Each channel is mapped to a GPIO pin
 *  with options being listed in Table 10 in the datasheet (not the programmer's guide)
 *
 * 	Since the ADC is so damn fast and isn't really generating all that much data,
 * 	the easiest way to handle it is to set up a DMA stream that will be called
 * 	each time the ADC capture event occurs. The ADC supports either software capture,
 *  which we could support with a simple timer interrupt and triggering it directly, but
 *  it also supports timer output-compare captures. The timer functionality used
 *  is that the timer will count up and check whether the current counter value matches
 *  the "capture" value, at which point it will pulse an output that causes the ADC to trigger
 *
 *  So we have the ADC capturing 2 channels, the data is handled by the DMA, and the timing
 *  granularity is handled by timer2's output compare channel. If the timers were
 *  more heavily in use I would have to play some tricks but...this application is
 *  simple
 */
void HAL_ADC_MspInit(ADC_HandleTypeDef * hadc) {

	ADCx_CLK_ENABLE();
	ADCx_PIN_CLK_ENABLE();
	DMAx_CLK_ENABLE();
	TIMx_CLK_ENABLE();

	// Timer1 runs on APB1
	// SystemClock = 180MHz, APB2 is SYSCLK/4 so 45MHz, max clock for timer block
	// TIMCLK = APB2 x 2, runs at 90MHz
	// I believe this gives me a 1s period so adjust this later
	htim_adc.Instance = TIMx;
	htim_adc.Init.ClockDivision = 0;
	htim_adc.Init.Period = 10000 - 1;
	htim_adc.Init.Prescaler = 9000;
	htim_adc.Init.CounterMode = TIM_COUNTERMODE_UP;
	if(HAL_TIM_OC_Init(&htim_adc) != HAL_OK) {
		ErrorHandler();
	}

	// Configure the timer channel settings
	TIM_OC_InitTypeDef tim_oc;
	tim_oc.OCMode = TIM_OCMODE_ACTIVE;
	tim_oc.OCPolarity = TIM_OCPOLARITY_HIGH;

	// Capture triggered when counter is 0
	tim_oc.Pulse = 0;

	HAL_TIM_OC_ConfigChannel(&htim_adc,&tim_oc,TIMx_CHANNEL);
	HAL_TIM_OC_Start(&htim_adc,TIMx_CHANNEL);

	// COnfigure pins for ADC
	GPIO_InitTypeDef gpioInit;
	gpioInit.Pin = ADCx_CHANNEL_PIN_NUMBER | ADCy_CHANNEL_PIN_NUMBER;
	gpioInit.Mode = GPIO_MODE_ANALOG;
	gpioInit.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(ADCx_CHANNEL_PORT, &gpioInit);

	/*##-3- Configure the DMA streams ##########################################*/
	/* Set the parameters to be configured */
	hdma_adc.Instance = ADCx_DMA_STREAM;

	// Set up the DMA channel settings
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
	HAL_NVIC_SetPriority(ADCx_DMA_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(ADCx_DMA_IRQn);
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

	// This sets up the stream, which is called every time the ADC samples
	// Samples are driven by the timer

	// There is a possibility that there is some issue with starting the timer
	// first but it's probably fine
	if(HAL_OK != HAL_ADC_Start_DMA(&g_AdcHandle,samples,2)) {
		ErrorHandler();
	}
}
