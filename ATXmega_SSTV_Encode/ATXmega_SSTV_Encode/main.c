/*
* ATXmega_SSTV_Encode.c
*
* Created: 29-08-2018 18:50:02
* Author : Puneet Shrivas
*/

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include "USART.h"
#include "avr_compiler.h"
#include "TC_driver.h"
#include "PD90.h"

int main(void)
{
	setUp16MhzExternalOsc();	//Required for setting 16Mhz frequency
	SetUsart();
	SSTVinit();
	SSTVbegin();
	while(1)
	{
		;
	}
}



