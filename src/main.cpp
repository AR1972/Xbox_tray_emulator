/*
	   DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
					Version 2, December 2004

 Copyright (C) 2004 Sam Hocevar <sam@hocevar.net>

 Everyone is permitted to copy and distribute verbatim or modified
 copies of this license document, and changing it is allowed as long
 as the name is changed.

			DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

  0. You just DO WHAT THE FUCK YOU WANT TO.
*/

#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

#define CLOSED 1
#define OPEN 0
#define NEJECT_PIN PB0
#define CD_RDY_PIN PB1
#define SER_DATA_PIN PB2
#define TRAY_IN_PIN PB3
#define TRAY_OUT_PIN PB4
#define STATUS_LED PB5

// optional LED, reset must be disabled via fuse
// #define USE_LED

volatile bool tray_status = CLOSED;
volatile bool busy = false;

void setup()
{
	// disable intrrupts
	cli();

	// setup pin 5 as our interrupt pin
	GIMSK |= _BV(PCIE);
	PCMSK |= _BV(NEJECT_PIN);
	pinMode(NEJECT_PIN, INPUT_PULLUP);

	pinMode(TRAY_OUT_PIN, OUTPUT);
	pinMode(TRAY_IN_PIN, OUTPUT);
	pinMode(SER_DATA_PIN, OUTPUT);
	pinMode(CD_RDY_PIN, OUTPUT);

#ifdef USE_LED
	// optional LED, reset must be disabled via fuse
	pinMode(STATUS_LED, OUTPUT);
#endif

	// setup as tray closed no disc in drive
	digitalWrite(TRAY_OUT_PIN, LOW);
	digitalWrite(TRAY_IN_PIN, HIGH);
	digitalWrite(CD_RDY_PIN, LOW);
	// terminate SER_DATA to avoid possible issues
	digitalWrite(SER_DATA_PIN, LOW);
#ifdef USE_LED
	// optional LED, reset must be disabled via fuse
	digitalWrite(STATUS_LED, LOW);
#endif
	// enable interrupts
	sei();
}

void sleep()
{
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_enable();
	sleep_cpu();
}

ISR(PCINT0_vect)
{
	if (!busy)
	{
		// NEJECT pin is normaly high only respond when pin changes to low
		if (!digitalRead(NEJECT_PIN))
		{
			busy = true;
			if (tray_status == CLOSED)
			{
				// if tray is closed emulate the tray opening
				digitalWrite(TRAY_IN_PIN, LOW);
				delay(1000);
				digitalWrite(CD_RDY_PIN, HIGH);
#ifdef USE_LED
				// optional LED, reset must be disabled via fuse
				digitalWrite(STATUS_LED, HIGH);
#endif
				digitalWrite(TRAY_OUT_PIN, HIGH);
				tray_status = OPEN;
			}
			else
			{
				// if tray is open emulate the tray closing
				digitalWrite(TRAY_OUT_PIN, LOW);
				digitalWrite(CD_RDY_PIN, LOW);
				delay(1000);
#ifdef USE_LED
				// optional LED, reset must be disabled via fuse
				digitalWrite(STATUS_LED, LOW);
#endif
				digitalWrite(TRAY_IN_PIN, HIGH);
				tray_status = CLOSED;
			}
		}
		busy = false;
	}
}

void loop()
{
	// power down the cpu when not servicing an interrupt
	sleep();
}
