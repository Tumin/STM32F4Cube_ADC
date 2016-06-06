//
// This file is part of the GNU ARM Eclipse distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

#include "BlinkLed.h"

// ----------------------------------------------------------------------------
uint32_t tick = 0;
uint32_t blink = 0;

void
blink_led_init()
{
  // Enable GPIO Peripheral clock
  RCC->AHB1ENR |= BLINK_RCC_MASKx(BLINK_PORT_NUMBER);

  GPIO_InitTypeDef GPIO_InitStructure;

  // Configure pin in output push/pull mode
  GPIO_InitStructure.Pin = BLINK_PIN_MASK(BLINK_PIN_NUMBER);
  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
  GPIO_InitStructure.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(BLINK_GPIOx(BLINK_PORT_NUMBER), &GPIO_InitStructure);

  // Start with led turned on
  blink_led_off();
  blink = 0;
  tick = HAL_GetTick();
}

void check_blink() {
  uint32_t new_tick = HAL_GetTick();
  if(new_tick - tick > 1000) {
	  blink ^= 1;
	  tick = new_tick;

	  if(blink)
		  blink_led_on();
	  else
		  blink_led_off();
  }
}

// ----------------------------------------------------------------------------
