/*
 * GccApplication1.c
 *
 * Created: 1/24/2019 8:42:37 PM
 * Author : user
 */ 

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

static inline void set_data(unsigned char x)
{
	PORTD = x;
	DDRD = 0xff;
}

static inline unsigned char get_data(void)
{
	DDRD = 0x00;
	return PIND;
}

static inline void sleep_700ns(void)
{
	NOP();
	NOP();
	NOP();
}

static unsigned char input(unsigned char rs)
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

static void output(unsigned char d, unsigned char rs)
{
	if (rs) SET_BIT(PORT, RS_PIN); else CLR_BIT(PORT, RS_PIN);
	CLR_BIT(PORT, RW_PIN);
	set_data(d);
	SET_BIT(PORT, EN_PIN);
	sleep_700ns();
	CLR_BIT(PORT, EN_PIN);
}

static void write(unsigned char c, unsigned char rs)
{
	while (input(0) & 0x80);
	output(c, rs);
}

void lcd_init(void)
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

void lcd_clr(void)
{
	write(0x01, 0);
}

void lcd_pos(unsigned char r, unsigned char c)
{
	unsigned char n = r * 40 + c;
	write(0x02, 0);
	while (n--) {
		write(0x14, 0);
	}
}

void lcd_put(char c)
{
	write(c, 1);
}

void lcd_puts1(const char *s)
{
	char c;
	while ((c = pgm_read_byte(s++)) != 0) {
		write(c, 1);
	}
}

void lcd_puts2(const char *s)
{
	char c;
	while ((c = *(s++)) != 0) {
		write(c, 1);
	}
}

int is_pressed (int r, int c) {
	DDRC = 0;
	PORTC = 0;
	CLR_BIT(DDRC, c+4);
	SET_BIT(PORTC, c+4); //Strong 0
	SET_BIT(DDRC, r); //Weak 1
	CLR_BIT(PORTC, r);
	//avr_wait(1);
	if (GET_BIT(PINC, c+4) == 0) {
		return 1;
	}
	return 0;
}

int get_key() {
	for (int r = 0; r < 4; r++) {
		for (int c = 0; c < 4; c++) {
			if (is_pressed(r, c) == 1) {
				return (r *4+c+1);
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

void printDT(struct dateTime *dt, int count) {
	char buffer[17];
	sprintf(buffer,"%02d/%02d/%04d",dt->month,dt->day,dt->year);
	lcd_pos(0,3);
	lcd_puts2(buffer);
	sprintf(buffer,"%02d:%02d:%02d",dt->hour,dt->minute,dt->second);
	lcd_pos(1,3);
	lcd_puts2(buffer);
	sprintf(buffer,"%02d",count);
	lcd_pos(1,14);
	lcd_puts2(buffer);
};

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

void blink(unsigned int r, unsigned int c) {
	lcd_pos(r,c);
	write(20,1);
	avr_wait(200);
}

enum States { start, setday, setmonth, setyear, sethour, setmin, running } State;

void setdate(struct dateTime *dt, int count) {
	int flag = 0;
	int key = -1;
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
					printDT(dt, count);
				} else if (key == 3) {
					dt->day--;
					if (dt->day < 1) {
						if(dt->year != 0 && dt->year%4 ==0 && dt->month == 2) {
							dt->day = days[dt->month - 1] + 1;
						} else {
							dt->day = days[dt->month - 1];
						}
					}
					printDT(dt, count);
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
					printDT(dt, count);
				} else if (key == 3) {
					dt->month--;
					if (dt->month < 1) {
						dt->month = 12;
					}
					printDT(dt, count);
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
					printDT(dt, count);
				} else if (key == 3) {
					dt->year--;
					printDT(dt, count);
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
					printDT(dt, count);
				} else if (key == 3) {
					dt->hour--;
					if (dt->hour < 0) {
						dt->hour = 23;
					}
					printDT(dt, count);
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
					printDT(dt, count);
				} else if (key == 3) {
					dt->minute--;
					if (dt->minute < 0) {
						dt->minute = 59;
					}
					printDT(dt, count);
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
		printDT(dt, count);
		
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

enum States1 { start1, option, settime, setsong, running1, deletealarm } State1;

void setalarm(struct dateTime *dt, int *hh[], int *mm[], int *count) {
	int flag = 0;
	int key = -1;
	int ahour = 0;
	int amin = 0;
	int position = 0;
	char buffer[17];
	State1 = start1;
	while (flag == 0) {
		avr_wait(100);
		switch(State1) {   // Transitions
			case start1:  // Initial transition
				State1 = option;
				//print options
				lcd_clr();
				sprintf(buffer,"2:Set alarm");
				lcd_pos(0,0);
				lcd_puts2(buffer);
				sprintf(buffer,"3:Del alarm");
				lcd_pos(1,0);
				lcd_puts2(buffer);
				break;
			case option:
				key = get_key();
				if (key == 1) {
					State1 = running1;
				} else if (key == 2) {
					State1 = settime;
					//print(ahour:amin)
					lcd_clr();
					sprintf(buffer,"Set alarm(hh:mm)");
					lcd_pos(0,0);
					lcd_puts2(buffer);
					sprintf(buffer,"%02d:%02d",ahour,amin);
					lcd_pos(1,5);
					lcd_puts2(buffer);
				} else if (key == 3) {
					if (*count == 0) {
						State1 = option;
						//print empty
						sprintf(buffer,"3:Del alarm (E)");
						lcd_pos(1,0);
						lcd_puts2(buffer);
					} else {
						State1 = deletealarm;
					}		
				}
				break;
			case settime:
				key = get_key();
				if (key == 1) {
					State1 = running1;
				} else if (key == 2) {
					ahour++;
					if (ahour == 24) {
						ahour = 0;
					}
					//print(ahour:amin)
					sprintf(buffer,"Set alarm(hh:mm)");
					lcd_pos(0,0);
					lcd_puts2(buffer);
					sprintf(buffer,"%02d:%02d",ahour,amin);
					lcd_pos(1,5);
					lcd_puts2(buffer);
				} else if (key == 6) {
					ahour--;
					if (ahour < 0) {
						ahour = 23;
					}
					//print(ahour:amin)
					sprintf(buffer,"Set alarm(hh:mm)");
					lcd_pos(0,0);
					lcd_puts2(buffer);
					sprintf(buffer,"%02d:%02d",ahour,amin);
					lcd_pos(1,5);
					lcd_puts2(buffer);
				} else if (key == 3) {
					amin++;
					if (amin == 60) {
						amin = 0;
					}
					//print(ahour:amin)
					sprintf(buffer,"Set alarm(hh:mm)");
					lcd_pos(0,0);
					lcd_puts2(buffer);
					sprintf(buffer,"%02d:%02d",ahour,amin);
					lcd_pos(1,5);
					lcd_puts2(buffer);
				} else if (key == 7) {
					amin--;
					if (amin < 0) {
						amin = 59;
					}
					//print(ahour:amin)
					sprintf(buffer,"Set alarm(hh:mm)");
					lcd_pos(0,0);
					lcd_puts2(buffer);
					sprintf(buffer,"%02d:%02d",ahour,amin);
					lcd_pos(1,5);
					lcd_puts2(buffer);
				} else if (key == 4) {
					hh[*count] = ahour; //add hour to array
					mm[*count] = amin; //add min to array
					*count = *count + 1;
					lcd_clr();
					State1 = running1;
				}
				break;
			case deletealarm:
				//print the alarm from array
				lcd_clr();
				sprintf(buffer,"%02d) %02d:%02d",position,hh[position],mm[position]);
				lcd_pos(0,3);
				lcd_puts2(buffer);
				key = get_key();
				if (key == 1) {
					State1 = running1;
				} else if (key == 3) {
					position++;
					if (position >= *count) {
						position = 0;
					}
				} else if (key == 2) {
					position--;
					if (position < 0) {
						position = 0;
					}
				} else if (key == 4) {
					hh[position] = 24;
					mm[position] = 24;
					*count = *count - 1;
					lcd_clr();
					State1 = running1;
				}
				break;
			case running1:
				flag = 1;
				break;
			default:
				State = start1;
				break;
		} // Transitions
		
		switch(State) {   // State actions
			case option:
				break;
			case settime:
				break;
			case setsong:
				break;
			case running1:
				break;
			case deletealarm:
				break;
			default:
				break;
		} // State actions
	}
}

enum States2 { start2, running2, stopping} State2;

void stopwatch_tick(struct dateTime *sw) {
	char record[2][17];
	sprintf(record[0],"%02d:%02d:%02d",0,0,0);
	sprintf(record[1],"%02d:%02d:%02d",0,0,0);
	char buffer[17];
	int flag1 = 0;
	State2 = start2;
	int key = -1;
	int index = 0;
	
	while(flag1==0) {
		avr_wait(100);
		key = get_key();
		switch(State2) {
			case start2:
				State2 = stopping;
				break;
			case running2:
				if(key == 4) {
					State2 = stopping;
				}
				else if(key == 5) {
					sprintf(record[index],"%02d:%02d:%02d",sw->hour,sw->minute,sw->second);
					index = (index==1)? 0: 1;
				}
				else if(key==6) {
					sw->hour = 0;
					sw->minute = 0;
					sw->second = 0;
					sw->subsecond = 0;
					State2 = start2;
				}
				else if(key==7) {
					
					flag1 = 1;
				}
				break;
			case stopping:
				if(key == 4) {
					State2 = running2;
				}
				else if(key == 5) {
					sprintf(record[index],"%02d:%02d:%02d",sw->hour,sw->minute,sw->second);
					index = (index==1)? 0: 1;
				}
				else if(key==6) {
					sw->hour = 0;
					sw->minute = 0;
					sw->second = 0;
					sw->subsecond = 0;
					State2 = start2;
				}
				else if(key==7) {
					
					flag1 = 1;
				}
				break;	
		}
		
		//printStopWatch(&sw, record);
		lcd_clr();
		lcd_pos(0,0);
		lcd_puts2(record[0]);
		lcd_pos(0,8);
		lcd_puts2(record[1]);
		sprintf(buffer,"%02d:%02d:%02d",sw->hour,sw->minute,sw->second);
		
		lcd_pos(1,3);
		lcd_puts2(buffer);
		
		switch(State2) {   // State actions
			case running2:
				keeptime(sw);
				break;
			case stopping:
				break;
			default:
				break;
		} // State actions	
	}
	sw->hour = 0;
	sw->minute = 0;
	sw->second = 0;
	sw->subsecond = 0;
	lcd_clr();
}

enum States3 { start3, running3, stopping1, setHour1, setMinute1, setSecond1} State3;

void timer_tick(struct dateTime *tw) {
	State3 = start3;
	int flag = 0;
	int timer_check = 0;
	int key = -1;
	char buffer[17];
	lcd_clr();
	sprintf(buffer,"%02d:%02d:%02d.%01d",tw->hour,tw->minute,tw->second,tw->subsecond);

	while(flag == 0) {
		avr_wait(100);
		key = 0;
		key = get_key();
		switch(State3) {
			case start3:
				State3 = stopping1;
				break;
			case running3:
				if(timer_check==1) {
					lcd_clr();
					sprintf(buffer,"Time Out");
					lcd_pos(1,0);
					lcd_puts2(buffer);
					State3 = stopping1;
				}
				if(key == 4) { //stop the timer
					lcd_clr();
					State3 = stopping1;
				}
				else if(key==7) { //exit timer mode
					lcd_clr();
					flag = 1;
				}
				break;
			case stopping1:
				if(key == 4) { //start the timer
					timer_check = 0;
					State3 = running3;
				}
				else if(key == 5) { //next set
					State3 = setHour1;
				}
				else if(key == 6) { //reset
					tw->hour = 0;
					tw->minute = 0;
					tw->second = 0;
					tw->subsecond = 0;
				}
				else if(key==7) { //exit timer mode
					lcd_clr();
					flag = 1;
				}
				break;
			case setHour1:
				if(key == 4) { //start
					lcd_clr();
					timer_check = 0;
					State3 = running3;
				}
				else if(key == 5) { //next
					lcd_clr();
					State3 = setMinute1;
				}
				else if(key == 6) { //add
					tw->hour++;
					if(tw->hour > 23) {
						tw->hour = 0;
					}
				}
				else if(key == 8) { //minus
					tw->hour--;
					if(tw->hour < 0) {
						tw->hour = 23;
					}
				}
				else if(key==7) { //exit timer mode
					lcd_clr();
					flag = 1;
				}
				break;
			case setMinute1:
				if(key == 4) { //start
					lcd_clr();
					timer_check = 0;
					State3 = running3;
				}
				else if(key == 5) { //next
					lcd_clr();
					State3 = setSecond1;
				}
				else if(key == 6) {
					tw->minute++;
					if(tw->minute > 59) {
						tw->minute = 0;
					}
				}
				else if(key == 8) { //minus
					tw->minute--;
					if(tw->minute < 0) {
						tw->minute = 59;
					}
				}
				else if(key==7) { //exit timer mode
					lcd_clr();
					flag = 1;
				}
				break;
			case setSecond1:
				if(key == 4) { //start
					lcd_clr();
					timer_check = 0;
					State3 = running3;
				}
				else if(key == 5) { //next
					lcd_clr();
					State3 = setHour1;
				}
				else if(key == 6) {
					tw->second++;
					if(tw->second > 59) {
						tw->second = 0;
					}
				}
				else if(key == 8) { //minus
					tw->second--;
					if(tw->second < 0) {
						tw->second = 59;
					}
				}
				else if(key==7) { //exit timer mode
					lcd_clr();
					flag = 1;
				}
				break;
		}
		
		lcd_pos(0,0);
		sprintf(buffer,"%02d:%02d:%02d.%01d",tw->hour,tw->minute,tw->second,tw->subsecond);
		lcd_puts2(buffer);

		switch(State3) {   // State actions
			case running3:
				sprintf(buffer,"Running");
				lcd_pos(1,0);
				lcd_puts2(buffer);
				tw->subsecond--;
				if(tw->subsecond == -1) {
					tw->subsecond = 9;
					tw->second--;
					if(tw->second == -1) {
						tw->second = 59;
						tw->minute--;
						if(tw->minute == -1) {
							tw->minute = 59;
							tw->hour--;
							if(tw->hour == -1) {
								lcd_clr();
								tw->minute = 0;
								tw->hour=0;
								tw->second = 0;
								tw->subsecond=0;
								timer_check = 1;
							}//hour
						}//min
					}//sec
				}//subsec	
				break;
			case stopping1:
				break;
			case setHour1:
				
				sprintf(buffer,"Setting hour");
				lcd_pos(1,0);
				lcd_puts2(buffer);
				break;
			case setMinute1:
				sprintf(buffer,"Setting minute");
				lcd_pos(1,0);
				lcd_puts2(buffer);
				break;
			case setSecond1:
				sprintf(buffer,"Setting second");
				lcd_pos(1,0);
				lcd_puts2(buffer);
				break;
			default:
				break;
		} // State actions
	}
	tw->hour = 0;
	tw->minute = 0;
	tw->second = 0;
	tw->subsecond = 0;
	lcd_clr();
}

int cmpfunc (const void * a, const void * b) {
	return ( *(int*)a - *(int*)b );
}

int main(void)
{
    /* Replace with your application code */
	struct dateTime dt;
	struct dateTime sw;
	struct dateTime tw;
	
	SET_BIT(DDRA,1);
	dt.day = 1;
	
	dt.month = 1;
	dt.year = 2019;
	dt.hour = 0;
	dt.minute = 0;
	dt.second = 0;
	dt.subsecond = 0;
	
	sw.day = 1;
	sw.month = 1;
	sw.year = 2019;
	sw.hour = 0;
	sw.minute = 0;
	sw.second = 0;
	sw.subsecond = 0;
	
	tw.day = 1;
	tw.month = 1;
	tw.year = 2019;
	tw.hour = 0;
	tw.minute = 0;
	tw.second = 0;
	tw.subsecond = 0;
	
	int hh[100];
	int mm[100];
	for (int i = 0; i < 100; i++) {
		hh[i] = 24;
		mm[i] = 24;
	}
	int count = 0;
	
	lcd_init();
	printDT(&dt, count);
	
    while (1) 
    {
		avr_wait(100);
		keeptime(&dt);
		printDT(&dt, count); 
		if (count > 0) {
			if (dt.hour == hh[0] && dt.minute == mm[0]) {
				playSong();
				hh[0] = 24;
				mm[0] = 24;
				qsort(hh, 100, sizeof(int), cmpfunc);
				qsort(mm, 100, sizeof(int), cmpfunc);
				count--;
				while(hh[0] == dt.hour && mm[0] == dt.minute) {
					hh[0] = 24;
					mm[0] = 24;
					qsort(hh, 100, sizeof(int), cmpfunc);
					qsort(mm, 100, sizeof(int), cmpfunc);
					count--;
				}			
			}
		}
		int key = get_key();
		if (key == 1) {
			setdate(&dt, count);
			continue;
		} else if (key == 2) {
			setalarm(&dt, hh, mm, &count);
			qsort(hh, 100, sizeof(int), cmpfunc);
			qsort(mm, 100, sizeof(int), cmpfunc);
			continue;
		} else if (key == 3) {
			stopwatch_tick(&sw);
			continue;
		} else if (key == 4) {
			timer_tick(&tw);
			continue;
		}
    }
}