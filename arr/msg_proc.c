
#include <stdio.h>
#include <stdbool.h>
#include <avr/pgmspace.h>

#include "version.h"
#include "clock.h"
#include "usart.h"

#ifdef HMC6352_H_
#include "bus/i2c.h"
static void process_hmc6352_cmd(char *msg)
{
	switch(msg[0]) {
	case 'a':
		hmc6352_read_all_mem();
		return;
	case 's':
		i2c_status();
		return;
	case 'r':
		i2c_trans_retry();
		return;
	case ' ': {
		int addr;
		int ret = sscanf(msg+1, "%d", &addr);
		if (ret == 1) {
			hmc6352_read_mem(addr);
			return;
		} else {
			goto help;
		}
	}
	default:
	help:
		puts_P(PSTR("hmc6352:\n"
			    "  ia       -- read all mem\n"
			    "  i <addr> -- read address\n"
			    "  is       -- i2c status\n"
			    "  ir       -- i2c reset"));
	}
}
#endif

#ifdef SERVO_H_
static bool process_servo_cmd(char *msg)
{
	switch(msg[0]) {
	case 's': {
		int pos, num;
		int ret = sscanf(msg+1,"%d %d",&num,&pos);
		if (ret == 2) {
			if (servo_set(num,TICKS_US(pos))) {
				printf(" error.\n");
			}
			return true;
		} else {
			return false;
		}
	}

	case 'S': {
		int pos, num;
		int ret = sscanf(msg+1," %d %d",&num,&pos);
		if (ret == 2) {
			if (servo_set(num,pos)) {
				printf(" error.\n");
			}
			return true;
		} else {
			return false;
		}
	}

	case 'c':
		printf("%d\n",servo_ct());
		return true;

	case 'q': {
		int num;
		int ret = sscanf(msg+1," %d",&num);
		if (ret == 1) {
			printf("%d\n",US_TICKS(servo_get(num)));
			return true;
		} else {
			return false;
		}
	}

	case 'Q': {
		int num;
		int ret = sscanf(msg+1," %d",&num);
		if (ret == 1) {
			printf("%d\n",servo_get(num));
			return true;
		} else {
			return false;
		}
	}

	default:
		puts_P(PSTR("invalid servo command.\n"));
		return true;
	}
}
#endif

void process_msg(void)
{
	// TODO: the size of buf should be the length of the input queue.
	char buf[USART_RX_BUFF_SZ];
	uint8_t i;
	for(i = 0; ; i++) {
		if (i > sizeof(buf)) {
			// exceeded buffer size.
			// means that process_msg was called when
			// the rx buffer does not contain a complete message.
			puts_P(PSTR("process error\n"));
			usart_flush_rx_to('\n');
			return;
		}

		int c = getchar();
		if (c == '\0' || c == EOF || c == '\n') {
			buf[i] = '\0';
			break;
		} else {
			buf[i] = c;
		}
	}

	if(i == 0)
		return;

	switch(buf[0]) {
	case 'u':
		fputs_P(version_str,stdout);
		break;
	case '?':
	case 'h':
		printf_P(PSTR("commands:\n"
		              "  h -- prints this.\n"
#ifdef SERVO_H_
			      "  ss <sn> <val> -- set servos.\n"
			      "  sq <sn> -- query servos (uS).\n"
			      "  s{S,Q} -- \" \" (ticks).\n"
			      "  sc -- get servo count.\n"
#endif
#ifdef HMC6352_H_
			      "  i{a,s, <addr>} -- read hmc6352 memory.\n"
#endif
			      "  c -- clear.\n"
			      "  e{+,-,} -- echo ctrl.\n"
			      "  u -- version.\n"));
		break;
#ifdef SERVO_H_
	case 's':
		if(process_servo_cmd(buf+1))
			break;
		printf_P(PSTR("bad args for \"%s\".\n"), buf);
		break;
#endif
	case 'c':
		printf("\e[H\e[2J");
		break;
#ifdef HMC6352_H_
	case 'i':
		process_hmc6352_cmd(buf+1);
		break;
#endif
	default:
		printf_P(PSTR("unknown command \"%s\".\n"), buf);
		break;
	}
	return;

}


