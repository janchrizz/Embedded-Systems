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
	SET_BIT(DDRC, c+4);
	CLR_BIT(PORTC, c+4); //Strong 0
	CLR_BIT(DDRC, r); //Weak 1
	SET_BIT(PORTC, r);
	if (GET_BIT(PINC, r) == 0) {
		return 1;
	}
	return 0;
}

int get_key() {
	for (int r = 0; r < 4; r++) {
		for (int c = 0; c < 4; c++) {
			if (is_pressed(r, c) == 1) {
				return (r + (4*c+1));
			}
		}
	}
	return 0;
}

struct dateTime {
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
	int subsecond;
};

void printDT(struct dateTime *dt) {
	char buffer[17];
	sprintf(buffer,"%02d/%02d/%04d",dt->month,dt->day,dt->year);
	lcd_pos(0,3);
	lcd_puts2(buffer);
	sprintf(buffer,"%02d:%02d:%02d",dt->hour,dt->minute,dt->second);
	lcd_pos(1,3);
	lcd_puts2(buffer);
};

void blink(unsigned int r, unsigned int c) {
	lcd_pos(r,c);
	write(20,1);
	avr_wait(200);
}
enum States { start, setday, setmonth, setyear, sethour, setmin, running } State;

void setdate(struct dateTime *dt) {
	int flag = 0;
	int key = 0;
	int days[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
	State = start;
	while (flag == 0) {
		avr_wait(100);
		switch(State) {   // Transitions
			case start:  // Initial transition
				State = setmonth;
				break;
			case setday:
				key = get_key();
				if (key == 4) {
					State = running;
				} else if (key == 1) {
					State = setyear;
				} else if (key == 2) {
					dt->day++;
					if (dt->day > days[dt->month - 1]) {
						if(dt->year != 0 && dt->year%4 ==0 && dt->month == 2) {							
						} else {
							dt->day = 1;
						}
					}
					printDT(dt);
				} else if (key == 3) {
					dt->day--;
					if (dt->day < 1) {
						if(dt->year != 0 && dt->year%4 ==0 && dt->month == 2) {
							dt->day = days[dt->month - 1] + 1;
						} else {
							dt->day = days[dt->month - 1];
						}
					}
					printDT(dt);
				}
				blink(0,7);
				break;
			case setmonth:
				key = get_key();
				if (key == 4) {
					State = running;
				} else if (key == 1) {
					State = setday;
				} else if (key == 2) {
					dt->month++;
					if (dt->month == 13) {
						dt->month = 1;
					}
					printDT(dt);
				} else if (key == 3) {
					dt->month--;
					if (dt->month < 1) {
						dt->month = 12;
					}
					printDT(dt);
				}
				blink(0,4);
				break;
			case setyear:
				key = get_key();
				if (key == 4) {
					State = running;
				} else if (key == 1) {
					State = sethour;
				} else if (key == 2) {
					dt->year++;
					printDT(dt);
				} else if (key == 3) {
					dt->year--;
					printDT(dt);
				}
				blink(0,12);
				break;
			case sethour:
				key = get_key();
				if (key == 4) {
					State = running;
				} else if (key == 1) {
					State = setmin;
				} else if (key == 2) {
					dt->hour++;
					if (dt->hour == 24) {
						dt->hour = 0;
					}
					printDT(dt);
				} else if (key == 3) {
					dt->hour--;
					if (dt->hour < 0) {
						dt->hour = 23;
					}
					printDT(dt);
				}
				blink(1,4);
				break;
			case setmin:
				key = get_key();
				if (key == 4) {
					State = running;
				} else if (key == 1) {
					State = setmonth;
				} else if (key == 2) {
					dt->minute++;
					if (dt->minute == 60) {
						dt->minute = 0;
					}
					printDT(dt);
				} else if (key == 3) {
					dt->minute--;
					if (dt->minute < 0) {
						dt->minute = 59;
					}
					printDT(dt);
				}
				blink(1,7);
				break;
			case running:
				flag = 1;
				break;
			default:
				State = start;
				break;
		} // Transitions

		if (dt->day > days[dt->month - 1]) {
			if(dt->year != 0 && dt->year%4 ==0 && dt->month == 2 && dt->day <= 29) {
				
			} else {
				dt->day = 1;
			}
		}
		printDT(dt);
		
		switch(State) {   // State actions
			case setday:
				break;
			case setmonth:
				break;
			case setyear:
				break;
			case sethour:
				break;
			case setmin:
				break;
			case running:
				break;
			default:
				break;
		} // State actions
	}
}

void keeptime(struct dateTime *dt) {
	int days[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
	dt->subsecond++;

	if(dt->subsecond == 10) {

		dt->subsecond = 0;
		dt->second++;
		if(dt->second == 60) {

			dt->second = 0;
			dt->minute++;
			if(dt->minute == 60) {

				dt->minute = 0;
				dt->hour++;
				if(dt->hour == 24) {

					dt->hour = 0;
					dt->day++;
					if(dt->day > days[dt->month-1]) {

						if(dt->year != 0 && dt->year%4 ==0 && dt->month == 2 && dt->day < 30) {
							return;
						}//leapyear

						dt->day = 1;
						dt->month++;

						if(dt->month == 13) {
							dt->month = 1;
							dt->year++;
						}//month
					}//day
				}//hour
			}//minute
		}//second
	}//subsecond
}//keeptime

int main(void)
{
    /* Replace with your application code */
	struct dateTime dt;
	dt.day = 1;
	dt.month = 1;
	dt.year = 2019;
	dt.hour = 0;
	dt.minute = 0;
	dt.second = 0;
	dt.subsecond = 0;
	lcd_init();
	printDT(&dt);
    while (1) 
    {
		avr_wait(100);
		keeptime(&dt);
		printDT(&dt); 
		if (get_key() == 4) {
			setdate(&dt);
		}
    }
}
