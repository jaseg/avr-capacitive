/* 	AVR capacitive softbutton test host implementation
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

#include <unistd.h>
#include <sys/time.h>

#include "opendevice.h"
#include "../usbdefines.h" //TODO relative path: not so nice.

int main(int argc, char** argv)
{
	usb_dev_handle* penta;
	usb_init();
	int ec = 0;
	if((ec = usbOpenDevice(&penta, 0x16c0, "s@jaseg.de", 0x05dc, "Softbutton Test", "*", 0, stdout))){ //Ugh. Double reference.
		printf("Cannot open device handle (error code %d).\n", ec);
		return 2;
	}
	while(1){
		int transmitted = 0;
		char data[8];
		if((transmitted = usb_interrupt_read(penta, 1, data, sizeof(data), 0)) < 0){
			printf("Error receiving interrupt data (error code %d).\n", ec);
			//return 1;
		}
		//printf("Received interrupt data: %x\n", data[0]);
		if(data[0] == VALUE_ACQUIRED){
			int value = data[1]<<8|data[2]; //The sample is transmitted big-endian
			printf("%d\n", value);
		}
	}
	usb_close(penta);
	return 0;
}


