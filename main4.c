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
#include <stdio.h>

#define XTAL_FRQ 8000000lu

#define SET_BIT(p,i) ((p) |=  (1 << (i)))
#define CLR_BIT(p,i) ((p) &= ~(1 << (i)))
#define GET_BIT(p,i) ((p) &   (1 << (i)))

#define WDR() asm volatile("wdr"::)
#define NOP() asm volatile("nop"::)
#define RST() for(;;);

void avr_init(void);
void avr_wait(unsigned short msec);
void avr_wait2(unsigned short msec);

#endif /* _AVR_H */

void avr_wait(unsigned short msec)
{
	TCCR0 = 3;
	while (msec--) {
		TCNT0 = (unsigned char)(256 - (XTAL_FRQ / 64) * 0.001);
		SET_BIT(TIFR, TOV0);
		while (!GET_BIT(TIFR, TOV0));
	}
	TCCR0 = 0;
}

void avr_wait2(unsigned short msec)
{
	TCCR0 = 3;
	while (msec--) {
		TCNT0 = (unsigned char)(256 - (XTAL_FRQ / 64) * 0.00001);
		SET_BIT(TIFR, TOV0);
		while (!GET_BIT(TIFR, TOV0));
	}
	TCCR0 = 0;
}

#ifndef _LCD_H
#define _LCD_H

void lcd_init(void);

void lcd_clr(void);

void lcd_pos(unsigned char r, unsigned char c);

void lcd_put(char c);

void lcd_puts1(const char *s);

void lcd_puts2(const char *s);

#endif /* _LCD_H */

#define DDR    DDRB
#define PORT   PORTB
#define RS_PIN 0
#define RW_PIN 1
#define EN_PIN 2

static inline void
set_data(unsigned char x)
{
	PORTD = x;
	DDRD = 0xff;
}

static inline unsigned char
get_data(void)
{
	DDRD = 0x00;
	return PIND;
}

static inline void
sleep_700ns(void)
{
	NOP();
	NOP();
	NOP();
}

static unsigned char
input(unsigned char rs)
{
	unsigned char d;
	if (rs) SET_BIT(PORT, RS_PIN); else CLR_BIT(PORT, RS_PIN);
	SET_BIT(PORT, RW_PIN);
	get_data();
	SET_BIT(PORT, EN_PIN);
	sleep_700ns();
	d = get_data();
	CLR_BIT(PORT, EN_PIN);
	return d;
}

static void
output(unsigned char d, unsigned char rs)
{
	if (rs) SET_BIT(PORT, RS_PIN); else CLR_BIT(PORT, RS_PIN);
	CLR_BIT(PORT, RW_PIN);
	set_data(d);
	SET_BIT(PORT, EN_PIN);
	sleep_700ns();
	CLR_BIT(PORT, EN_PIN);
}

static void
write(unsigned char c, unsigned char rs)
{
	while (input(0) & 0x80);
	output(c, rs);
}

void
lcd_init(void)
{
	SET_BIT(DDR, RS_PIN);
	SET_BIT(DDR, RW_PIN);
	SET_BIT(DDR, EN_PIN);
	avr_wait(16);
	output(0x30, 0);
	avr_wait(5);
	output(0x30, 0);
	avr_wait(1);
	write(0x3c, 0);
	write(0x0c, 0);
	write(0x06, 0);
	write(0x01, 0);
}

void
lcd_clr(void)
{
	write(0x01, 0);
}

void
lcd_pos(unsigned char r, unsigned char c)
{
	unsigned char n = r * 40 + c;
	write(0x02, 0);
	while (n--) {
		write(0x14, 0);
	}
}

void
lcd_put(char c)
{
	write(c, 1);
}

void
lcd_puts1(const char *s)
{
	char c;
	while ((c = pgm_read_byte(s++)) != 0) {
		write(c, 1);
	}
}

void
lcd_puts2(const char *s)
{
	char c;
	while ((c = *(s++)) != 0) {
		write(c, 1);
	}
}

int is_pressed (int r, int c) {
	DDRC = 0;
	PORTC = 0;
	CLR_BIT(DDRC, c+4); //Weak 1
	SET_BIT(PORTC, c+4);
	SET_BIT(DDRC, r);
	CLR_BIT(PORTC, r); //Strong 0
	
	
	if (GET_BIT(PINC, c+4) == 0) {
		return 1;
	}
	return 0;
}

int get_key() {
	int r;
	for ( r = 0; r < 4; r++) {
		int c;
		for ( c = 0; c < 4; c++) {
			if (is_pressed(r, c) == 1) {
				return (r * 4 + c + 1);
			}
		}
	}
	return 0;
}

int voltage = 0;
int average = 0;
float sum = 0;
int max = 0;
int min = 1024;
int count = 0;
int maxa,maxb = 0;
int mina,minb = 0;

enum States { Start, Running } State;

void sample() {
	avr_wait(500);
	switch(State) {
		case Start:
			if(get_key()) {
				State = Running;
			}
			else {
				State = Start;
			}
			break;
		case Running:
			if(get_key()) {
				State = Start;
			}
			else {
				State = Running;
			}
			break;
		default:
			State = Start;
	}

	switch(State) {
		case Start:
			lcd_clr();
			voltage = 0;
			average = 0;
			sum = 0;
			max = 0;
			min = 1024;
			count = 0;
			maxa,maxb = 0;
			mina,minb = 0;
			char buff[17];
			lcd_pos(0,0);
			sprintf(buff,"V=---- Mx=----");
			lcd_puts2(buff);
			lcd_pos(1,0);
			sprintf(buff,"Mn=---- Av=----");
			lcd_puts2(buff);
			break;
		case Running:
			read();
			break;	
	}
}

int get_sample() {
	SET_BIT(ADMUX,6);
	SET_BIT(ADCSRA,7);
	SET_BIT(ADCSRA,6);
	while(!(ADCSRA & (1<<ADIF)));
	return ADC;
}

void read() {
	char buffer[17];

	voltage = get_sample();
	count++;
	float f = voltage*5.0/10.23;
	int a = f/100;
	int c = (int)f;
	int b = c%100;
	if(voltage > max) {
		max = voltage;
		maxa = a;
		maxb = b;
	}
	if(voltage < min) {
		min = voltage;
		mina = a;
		minb = b;
	}
	
	float avgg = (voltage*5.0/10.23);
	sum += avgg;
	float avg = sum/count;
	int x = avg/100;
	int n = (int)avg;
	int y = n%100;
	
	lcd_pos(0,0);
	sprintf(buffer,"V=%01d.%02d Mx=%01d.%02d",a,b,maxa,maxb);
	lcd_puts2(buffer);
	lcd_pos(1,0);
	sprintf(buffer,"Mn=%01d.%02d Av=%01d.%02d",mina,minb,x,y);
	lcd_puts2(buffer);
}

int main(void)
{
    /* Replace with your application code */
	SET_BIT(DDRA,0);
	PORTA = 1;
	lcd_init();
	ADCSRA = (1<<ADEN) | (1<< ADPS2) | (1 << ADPS1) | (0 << ADPS0);
    while (1) 
    {
		sample();
    }
}