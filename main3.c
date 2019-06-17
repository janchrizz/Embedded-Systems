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

#define A4 440
#define As 466
#define B 494
#define C 523
#define Cs 554
#define D 587
#define Ds 622
#define E 659
#define F 698
#define Fs 740
#define G 784
#define Gs 831
#define A5 880
#define A5s 932
#define B5 988
#define C5 1046
#define D5 1175

#define W 8
#define H W/2
#define Q H/2
#define R Q/2

void printSong() {
	char buffer[17] = "Playing Song:";
	char buff[17] = "Twinkle Twinkle";
	
	lcd_pos(0,0);
	lcd_puts2(buffer);
	lcd_pos(1,0);
	lcd_puts2(buff);	
	
};

enum States { Start, Playing } State;

void runMusic() {
	avr_wait(200);
	switch(State) {
		case Start:
			if(get_key()) {
				State = Playing;
			}
			else {
				State = Start;
			}
			break;
		case Playing:
			if(get_key() == 0) {
				State = Start;
			}
			break;
		default:
			State = Start;
	}

	switch(State) {
		case Start:
			break;
		case Playing:
			printSong();
			playSong();
			lcd_clr();
			State = Start;
			break;
		
	}
}

struct note {
	int frequency;
	int duration;
};



const struct note NOTES[] = {
	{C, Q},
	{C, Q},
	{G, Q},
	{G, Q},
	{A5, Q},
	{A5, Q},
	{G, H},
	{F, Q},
	{F, Q},
	{E, Q},
	{E, Q},
	{D, Q},
	{D, Q},
	{C, H},
	{G, Q},
	{G, Q},
	{F, Q},
	{F, Q},
	{E, Q},
	{E, Q},
	{D, H},
	{G, Q},
	{G, Q},
	{F, Q},
	{F, Q},
	{E, Q},
	{E, Q},
	{D, H},
	{C, Q},
	{C, Q},
	{G, Q},
	{G, Q},
	{A5, Q},
	{A5, Q},
	{G, H},
	{F, Q},
	{F, Q},
	{E, Q},
	{E, Q},
	{D, R},
	{D, R},
	{D, R},
	{G, R},
	{C, H},
};

void playNote(int frequency, int duration) {
	int dur = frequency * duration/4;
	unsigned short TL = 50000.0 / (frequency);
	unsigned short TH = TL;
	for (int i = 0; i < dur; ++i) {
		SET_BIT(PORTA, 1);
		avr_wait2(TH);
		CLR_BIT(PORTA, 1);
		avr_wait2(TL);
		
	}
}

void playSong () {
	int n = sizeof(NOTES)/sizeof(NOTES[0]);
	for (int i = 0; i < n; ++i) {
		playNote(NOTES[i].frequency, NOTES[i].duration);
		if(get_key()) {
			break;
		}
	}
}

int main(void)
{
    /* Replace with your application code */
	SET_BIT(DDRB,1);
	SET_BIT(DDRC,0);
	SET_BIT(DDRA,1);
	lcd_init();
	
    while (1) 
    {
		runMusic();
		avr_wait(100);
    }
}