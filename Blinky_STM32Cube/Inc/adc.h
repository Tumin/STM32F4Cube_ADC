/*
 * adc.h
 *
 *  Created on: Jun 5, 2016
 *      Author: Ian
 */

#ifndef ADC_H_
#define ADC_H_

#include "stm32f4xx_hal.h"

#define ADCx_CHANNEL_PORT			(GPIOC)
#define ADCx_CHANNEL_PIN_NUMBER		(GPIO_PIN_0)
#define ADCx						(ADC1)
#define ADCx_CHANNEL				(ADC_CHANNEL_10)

#define ADCy_CHANNEL_PORT			(GPIOC)
#define ADCy_CHANNEL_PIN_NUMBER		(GPIO_PIN_1)
#define ADCy 						(ADC1)
#define ADCy_CHANNEL				(ADC_CHANNEL_11)

#define ADCx_CLK_ENABLE()			__HAL_RCC_ADC1_CLK_ENABLE()
#define ADCx_PIN_CLK_ENABLE()		__HAL_RCC_GPIOC_CLK_ENABLE()
#define DMAx_CLK_ENABLE()           __HAL_RCC_DMA2_CLK_ENABLE()

/* Definition for ADCx's DMA */
#define ADCx_DMA_CHANNEL                DMA_CHANNEL_0
#define ADCx_DMA_STREAM                 DMA2_Stream0

/* Definition for ADCx's NVIC */
//#define ADCx_DMA_IRQn                   DMA2_Stream0_IRQn
//#define ADCx_DMA_IRQHandler             DMA2_Stream0_IRQHandler

void adc_init(void);

#endif /* ADC_H_ */
