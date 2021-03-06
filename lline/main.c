/*
 * ATtiny861
 	general initialization and main loop
 */

#include "defines.h"
#include "common.h"
#include "debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <avr/io.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "spi_io.h"
#include "text_cmd.h"

#include "adc.h"
#include "motor.h"

static inline void init(void)
{
	power_all_disable();
	debug_led_init();
	clock_prescale_set(clock_div_1);
	spi_io_init();
	adc_init();
	motor_init();
	sei();
}

__attribute__((noreturn))
void main(void)
{
	init();
	uint16_t adc_val[ADC_CHANNEL_CT];	
	for(;;) {
		if (adc_new_data) {
			adc_new_data = false;
			adc_val_cpy(adc_val);
			for (uint8_t i = 0; i < ADC_CHANNEL_CT; i++) {			
				spi_putchar((char) (i+'0'));
				spi_putchar(':');
				spi_putchar(' ');
				spi_puth2(adc_val[i]);
				spi_putchar('\t');
			}
			spi_putchar('\n');
		}
		process_rx();		
		_delay_ms(200);
	}
}

