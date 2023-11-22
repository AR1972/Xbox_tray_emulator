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
#include <avr/power.h>

#define CLOSED 1
#define OPEN 0
#define NEJECT_PIN PB0
#define CD_RDY_PIN PB1
#define SER_DATA_PIN PB2
#define TRAY_IN_PIN PB3
#define TRAY_OUT_PIN PB4

volatile bool tray_status = CLOSED;

void setup()
{
	// disable intrrupts
	cli();
	// ADC off
	ADCSRA &= ~_BV(ADEN);
	// setup pin 5 as our interrupt pin
	GIMSK |= _BV(PCIE);
	PCMSK |= _BV(NEJECT_PIN);
	pinMode(NEJECT_PIN, INPUT);
	// setup output
	pinMode(TRAY_OUT_PIN, OUTPUT);
	pinMode(TRAY_IN_PIN, OUTPUT);
	pinMode(SER_DATA_PIN, OUTPUT);
	pinMode(CD_RDY_PIN, OUTPUT);
	// setup as tray closed no disc in drive
	digitalWrite(TRAY_OUT_PIN, LOW);
	digitalWrite(TRAY_IN_PIN, HIGH);
	digitalWrite(CD_RDY_PIN, LOW);
	// terminate SER_DATA to avoid possible issues
	digitalWrite(SER_DATA_PIN, LOW);
	// enable interrupts
	sei();
}

void sleep()
{
	ACSR = ADMUX = ADCSRA = 0;
	// Analog comparator off
	ACSR |= _BV(ACD);
	// switch Analog to Digitalconverter off
	ADCSRA &= ~_BV(ADEN);
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	power_all_disable();
	sleep_enable();
	cli();
	// Disable BOD, step 1
	BODCR = _BV(BODSE) | _BV(BODS);
	// Second step
	BODCR = _BV(BODS);
	sei();
	sleep_cpu();
	sleep_disable();
	power_all_enable();
}

ISR(PCINT0_vect)
{
	GIMSK &= ~_BV(PCIE);
	// NEJECT pin is normaly high only respond when pin changes to low
	if (!digitalRead(NEJECT_PIN))
	{
		if (tray_status == CLOSED)
		{
			// if tray is closed emulate the tray opening
			digitalWrite(CD_RDY_PIN, HIGH);
			digitalWrite(TRAY_IN_PIN, LOW);
			delay(1000);
			digitalWrite(TRAY_OUT_PIN, HIGH);
			delay(1000);
			digitalWrite(CD_RDY_PIN, LOW);
			tray_status = OPEN;
		}
		else
		{
			// if tray is open emulate the tray closing
			digitalWrite(CD_RDY_PIN, LOW);
			digitalWrite(TRAY_IN_PIN, HIGH);
			delay(1000);
			digitalWrite(TRAY_OUT_PIN, LOW);
			digitalWrite(TRAY_IN_PIN, LOW);
			delay(2000);
			digitalWrite(TRAY_IN_PIN, HIGH);
			tray_status = CLOSED;
		}
	}
	GIMSK |= _BV(PCIE);
}

void loop()
{
	// power down the cpu when not servicing an interrupt
	sleep();
}
