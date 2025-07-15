#include <avr/io.h>
#include <avr/interrupt.h>
//#include <avr/signal.h>
#include <util/delay.h>

#ifndef HELPER_H
#define HELPER_H

//MODIFICATIONS:
//Included a function to map joystick values to movement states.


//Functionality - finds the greatest common divisor of two values
//Parameter: Two long int's to find their GCD
//Returns: GCD else 0
unsigned long int findGCD(unsigned long int a, unsigned long int b)
{
	unsigned long int c;
	while(1){
		c = a % b;
		if( c == 0 ) { return b; }
		a = b;
		b = c;
	}
	return 0;
}

unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b) {
   return (b ?  (x | (0x01 << k))  :  (x & ~(0x01 << k)) );
              //   Set bit to 1           Set bit to 0
}

unsigned char GetBit(unsigned char x, unsigned char k) {
   return ((x & (0x01 << k)) != 0);
}

//Return 0 if no movement, 1 if going up, 2 if going down
unsigned int map_value_joystick(unsigned int valueRead){
	if(valueRead >= 0 && valueRead <= 300){
		return 1;
	}else if(valueRead >= 301 && valueRead <= 850){
		return 0;
	}else if(valueRead >= 851 && valueRead <= 1024){
		return 2;
	}else{
		return 0;
	}
}	

#endif /* HEPLER_H */