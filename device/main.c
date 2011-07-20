/* 	Penta controller firmware v0.0.0.0.0.0.1\alpha0-1
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

static uint8_t sample[2]; //Interrupt data



usbMsgLen_t usbFunctionSetup(uchar data[8])
{
	return USB_NO_MSG; //Is this correct? FIXME
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
	ADSC |= ADSC;
	//Infinitely eternal main loop.
	for(;;){
		if(ADSRA & ADIF){
			ADSRA |= ADIF | ADSC; //Clear ADIF and start a new conversion (which takes 1664 cycles)

		}
		if(usbInterruptIsReady()){
			intout[0] = VALUE_ACQUIRED;
			usbSetInterrupt(intout, 2);
		}
		usbPoll();
		//_delay_ms(1);
	}
	return 0;
}

