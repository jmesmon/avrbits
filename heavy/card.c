#include <stdint.h>
#include <stdio.h>

#include <util/parity.h>
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "ds/glist.h"

/*
 Card Reader:
  Vcc -> Power rail.
  Data -> PA0 (PCINT0)
  Clock-> PA1 (PCINT1)
  Card Motion -> PA2 (..)
  Card Detect -> PA3
  Card End Stop -> PA4
  Gnd -> gnd rail.
*/

#define MAGR_DATA    (1<<0)
#define MAGR_CLK     (1<<1)
#define MAGR_MOTION  (1<<2)
#define MAGR_CDETECT (1<<3)
#define MAGR_ENDSTOP (1<<4)

#define MAGR_BITFIELD  (MAGR_DATA | MAGR_CLK | MAGR_CDETECT | MAGR_ENDSTOP)
//  MAGR_MOTION

#define MAGR_BITS_PACK 5
#define NONE
/*          name, fnattr, dattr, data_t          , index_t*/
LIST_DEFINE(mag, static, NONE, uint8_t, uint8_t);

struct mag_status {
	uint8_t motion_1;
	uint8_t motion_0;

	uint8_t insert_1;
	uint8_t insert_0;

	uint8_t end_stop_1;
	uint8_t end_stop_0;

	uint8_t data;
	uint8_t data_ct;
	uint8_t data_ready;

	uint8_t data_buff[64];
	list_t(mag) data_q;

} static mag = {.data_q = LIST_INITIALIZER(mag.data_buff) };

/* insert_bit()
 * next_byte()
 * need array indexing.
 * forward and bit reversed readback.
 */

static uint8_t bit(uint8_t in, uint8_t pos)
{
	return ((in >> pos) & 1);
}

static uint8_t bit_reverse(uint8_t x)
{
	x = ((x >> 1) & 0x55) | ((x << 1) & 0xaa);
	x = ((x >> 2) & 0x33) | ((x << 2) & 0xcc);
	x = ((x >> 4) & 0x0f) | ((x << 4) & 0xf0);
	return x;
}

/*
	ISO Track 2 table	Hex	  Char Function
	0	0	0	0	1  0x00 0	Data
	1	0	0	0	0  0x01 1	Data
	0	1	0	0	0  0x02 2	Data
	1	1	0	0	1  0x03 3	Data
	0	0	1	0	0  0x04 4	Data
	1	0	1	0	1  0x05 5	Data
	0	1	1	0	1  0x06 6	Data
	1	1	1	0	0  0x07 7	Data
	0	0	0	1	0  0x08 8	Data
	1	0	0	1	1  0x09 9	Data
	0	1	0	1	1  0x0A :	Control
	1	1	0	1	0  0x0B ;	Start Sentinel
	0	0	1	1	1  0x0C <	Control
	1	0	1	1	0  0x0D =	Field Separator
	0	1	1	1	0  0x0E >	Control
	1	1	1	1	1  0x0F ?	End Sentinel
*/

static uint16_t iso_t2(uint8_t in)
{
	uint8_t in_s = in >> 1;	// in stripped of the parity bit.s
	uint8_t out;

	if (in_s < 0xA) {
		out = in_s + '0';
	} else {
		switch (in_s) {
		case 0xA:
			out = ':';
			break;
		case 0xB:
			out = ';';
			break;
		case 0xC:
			out = '<';
			break;
		case 0xD:
			out = '=';
			break;
		case 0xE:
			out = '>';
			break;
		case 0xF:
			out = '?';
			break;
		default:
			out = '_';
			break;
		}
	}



	if (parity_even_bit(in_s) == (in & 1)) {
		return (('!' << 8) | (out));
	} else {
		return ((' ' << 8) | (out));
	}
}

void card_init(void)
{

	PORTA &= (uint8_t) ~ (MAGR_BITFIELD);
	DDRA &= (uint8_t) ~ (MAGR_BITFIELD);

	PCICR |= (1 << PCIE0);
	PCMSK0 |= (MAGR_BITFIELD);
}

enum card_state {
	C_ALL_OUT = 0,
	C_MOVE_IN,
	C_ALL_IN,
	C_MOVE_OUT
} static cstate;

void card_spin(void)
{

	if (mag.end_stop_1) {
		puts_P(PSTR("\nend_stop_1\n"));
		mag.end_stop_1--;
	}

	if (mag.end_stop_0) {
		puts_P(PSTR("end_stop_0\n\n"));
		mag.end_stop_0--;
	}

	if (mag.insert_1) {
		puts_P(PSTR("insert_1\n\n"));
		mag.insert_1--;
	}

	if (mag.insert_0) {
		puts_P(PSTR("\ninsert_0\n"));
		mag.insert_0--;
	}
#if 0   
	if (mag.motion_1) {
		puts_P(PSTR("motion_1\n"));
		mag.motion_1--;
	}

	if (mag.motion_0) {
		puts_P(PSTR("motion_0\n"));
		mag.motion_0--;
	}
#endif

	if (mag.data_ready) {
		uint8_t data = mag.data;
		mag.data_ct = 0;
		mag.data = 0;
		mag.data_ready = 0;

		for (uint8_t i = 0; i < MAGR_BITS_PACK; i++) {
			printf("%d", bit(data, i));
		}
		uint8_t data_i = bit_reverse(data) >> (8 - MAGR_BITS_PACK);
		uint16_t proc = iso_t2(data);
		uint16_t proc_i = iso_t2(data_i);

		putchar('\t');

		putchar(proc >> 8);
		putchar(proc & 0xFF);

		putchar('\t');

		putchar(proc_i >> 8);
		putchar(proc_i & 0xFF);

		putchar('\n');
	}
}

ISR(SIG_PIN_CHANGE0)
{
	static uint8_t old_pin = 0xFF;	// all pins default high
	uint8_t new_pin = PINA;

	uint8_t pin_diff = old_pin ^ new_pin;

	old_pin = new_pin;

	static uint8_t data;

	if (pin_diff & MAGR_CDETECT) {
		if (new_pin & MAGR_CDETECT) {
			//set (line 1, logic 0)
			// state = card all the way out
			mag.insert_0++;
		} else {
			if (cstate == C_ALL_OUT) {
				cstate = C_MOVE_IN;
			}
			//clear (line 0, logic 1)
			// state = card begginning in
			mag.insert_1++;
		}
	}

	if (pin_diff & MAGR_ENDSTOP) {
		if (new_pin & MAGR_ENDSTOP) {
			if (cstate == C_ALL_IN) {
				cstate = C_MOVE_OUT;
			}
			//set (line 1, logic 0)
			// state = card begining to move out
			mag.end_stop_0++;
		} else {
			if (cstate == C_MOVE_IN) {
				cstate = C_ALL_IN;
			}
			//clear (line 0, logic 1)
			// state parity_even_bit(in>>1)= card all the way in
			mag.end_stop_1++;
		}
	}

	if (pin_diff & MAGR_DATA) {
		// data = !MAGR_DATA
		if (new_pin & MAGR_DATA) {
			data = 0;
		} else {
			data = 1;
		}
	}

	if (pin_diff & MAGR_CLK) {
		// read done on falling edge.
		if (new_pin & MAGR_CLK) {
			//set (line 1, logic 0)
			// about to do something?
		} else {
			if (cstate == C_MOVE_IN || cstate == C_MOVE_OUT) {
				//clear (line 0, logic 1)
				// read data
				mag.data |= (data << (mag.data_ct));
				mag.data_ct++;
				if (mag.data_ct >= MAGR_BITS_PACK) {
					if (cstate == C_MOVE_OUT) {
						mag.data = bit_reverse(mag.data);
					}
					mag.data_ready = 1;
					list_push(mag) (&(mag.data_q), mag.data);
					mag.data_ct = 0;
				}
			}
		}
	}

#if 0
	if ( pin_diff & MAGR_MOTION ) {
		if ( new_pin & MAGR_MOTION) {
			//set (line 1, logic 0)
			// state = mag card no longer in motion
			mag.motion_0 ++;
		}
		else {
			//clear (line 0, logic 1)
			// state = mag card in motion
			mag.motion_1 ++;
		}
	}
#endif
}
