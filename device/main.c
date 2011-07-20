/* 	AVR capacitive softbutton test firmware
	Copyright (C) 2011 by Sebastian Götte <s@jaseg.de>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "../usbdefines.h" //TODO bad style.
#include "usbdrv.h"

//Main loop state defines
#define CONVERSION_COMPLETE 0x80
#define ADC_STARTUP 0x40 //Used to discard the first sample

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
	//We can safely ignore all requests hitting this callback. The USB stack has already handled the actually important stuff.
	return USB_NO_MSG;
}

int main(void)
{
	//Device initialization
	//The softbutton pin is configured as input, the internal pullup is used to (dis-)charge the sensor plate
	//The ADC is configured in single-shot mode
	ADMUX = 0x4F; //Vref: AVcc with external capacitor at AREF pin
	ADCSRA = 0x87; //Enable ADC, single conversion mode, enable ADC interrupt, ADC clock prescaler 128
	//USB Initialization
	usbInit();
	sei();
	//ADC startup
	ADCSRA |= ADSC;
	//Main loop vars
	static uint8_t intout[3] = {VALUE_ACQUIRED, 0, 0}; //Interrupt data, NOTE: there are two locations below where this is being written, so beware when extending the program.
	uint8_t state = ADC_STARTUP;
	//Infinitely eternal main loop.
	for(;;){
		//Sample acquiration and interrupt request transmission are divided since both need some time, the latter depending on the device and host's config, the former solely depending on the ADC.
		if(ADCSRA & ADIF){
			intout[1] = ADCH;
			intout[2] = ADCL; //Yeah, we do this big-endian
			ADMUX = 0x4F; //Select GND to discharge the s&r cap
			PORTC |= 0x01; //Enable internal pullup on PC0 (ADC0) to charge the sensor plate
			_delay_us(250); //Not sure if that is the right delay. Have yet to figure that out (perhaps it is not even possible)
			PORTC &= 0xFE; //Disable pullup on sensor plate input
			ADMUX = 0x4F; //Select the sensor input (ADC0/PC0) for conversion
			ADCSRA |= ADIF | ADSC; //Clear ADIF and start a new conversion (which takes 1664 cycles)
			//The first sample is discarded (I read this is recommended...)
			//Has the nice bonus that without further initialization the sensor is fully operating from the second cycle (if we would want the first cycle to produce meaningful results, the code above would have to be replicated in front of the loop).
			if(state & ADC_STARTUP)
				state &= ~ADC_STARTUP;
			else
				state |= CONVERSION_COMPLETE; //Signal to send the sample
		}
		if(usbInterruptIsReady() && (state & CONVERSION_COMPLETE)){ //Fresh data for the host!
			usbSetInterrupt(intout, 3);
		}
		usbPoll();
		//_delay_ms(1);
	}
	return 0;
}

