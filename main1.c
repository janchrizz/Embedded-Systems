/*
 * GccApplication1.c
 *
 * Created: 1/24/2019 8:42:37 PM
 * Author : user
 */ 

#include <avr/io.h>
#ifndef _AVR_H
#define _AVR_H

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/io.h>

#define XTAL_FRQ 8000000lu

#define SET_BIT(p,i) ((p) |=  (1 << (i)))
#define CLR_BIT(p,i) ((p) &= ~(1 << (i)))
#define GET_BIT(p,i) ((p) &   (1 << (i)))

void avr_wait(unsigned short msec);

#endif /* _AVR_H */

void wait_avr(unsigned short msec)
{
	TCCR0 = 3;
	while (msec--) {
		TCNT0 = (unsigned char)(256 - (XTAL_FRQ / 64) * 0.001);
		SET_BIT(TIFR, TOV0);
		while (!GET_BIT(TIFR, TOV0));
	}
	TCCR0 = 0;
}

int main(void)
{
    /* Replace with your application code */
	SET_BIT(DDRB, 0);
	CLR_BIT(DDRB, 1);
    while (1) 
    {
		if(!GET_BIT(PINB, 1)) {
			PORTB = 1;
			wait_avr(500);
			PORTB = 0;
			wait_avr(500);
		}
    }
}

