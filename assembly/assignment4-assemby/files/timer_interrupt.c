/* ========================================================================== */
/*                                                                            */
/*   timer_interrupt.c
/*   based on the following resource:                                                               */
/*   (c) Author: http://efundies.com/avr-timer-interrupts-in-c/                                                                        */
/*   Description                                                              */
/*                                                                            */
/* ========================================================================== */

/********************************************************************************
                                Includes
********************************************************************************/
#include <avr/io.h>
#include <stdbool.h>
#include <avr/interrupt.h>

/********************************************************************************
                                Interrupt Routines
********************************************************************************/
// timer1 overflw
ISR(TIMER1_OVF_vect)
{
	// XOR PORT L Bit 1 to toggle the LED connected to PL1
	PORTL=PORTL ^ 0x02;
}

// timer0 overflow
ISR(TIMER0_OVF_vect)
{
	// XOR PORT B Bit 1 to toggle the LED connected to PB1
	
  
}

/********************************************************************************
                                Main
********************************************************************************/
/*
 * Our 6 LED strip occupies ardruino pins 42, 44, 46, 48, 50, 52
 * and Gnd (ground)
 * Pin 42 Port L: bit 7 (PL7)
 * Pin 44 Port L: bit 5 (PL5)
 * Pin 46 Port L: bit 3 (PL3)
 * Pin 48 Port L: bit 1 (PL1)
 * Pin 50 Port B: bit 3 (PB3)
 * Pin 52 Port B: bit 1 (PB1)
*/
int main( void )
{
	/* set PORTL and PORTB for output*/
  

	// enable timer overflow interrupt for both Timer0 and Timer1
	TIMSK0 |=(1<<TOIE0);
  TIMSK1 |=(1<<TOIE1);
	
	// set timer0 counter initial value to 0
	TCNT0=0x00;
  
  // set timer1 counter initial value to 0
	TCNT1=0x00;

	// start timer0 with /1024 prescaler. Timer clock = system clock/1024
	TCCR0B = (1<<CS02) | (1<<CS00);

	// lets turn on 16 bit timer1 also with /1024
	TCCR1B = (1 << CS10) | (1 << CS12);
	
	// enable interrupts
	sei(); 


	while(true) {
	}
}
