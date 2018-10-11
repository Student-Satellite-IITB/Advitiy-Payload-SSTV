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
#include "pmic_driver.h"
#include "spi_driver.h"
#include "TC_driver.h"
#include "AD9833.h"
#include "NXH5104.h"

volatile int frequency=1757,phase=0,prevPhase=0,prevFreq=0,pixelCount=0;
volatile uint8_t sectorAdress=0, pageAddress=0, byteAddress=0;

int main(void)
{
	setUp16MhzExternalOsc();	//Required for setting 16Mhz frequency
	SetClock0();				//Initialize 532 us interrupts
	SPI_Master_init();			//Initialize SPI for AD9833
	eepromInit();				//Initialize SPI for EEPROM
	SetUsart();					
//	SPI_send16(0x100);			//Reset AD9833
/*
// VIS Code 
	{_delay_ms(100);
	Set_AD9833(1900,0);	_delay_ms(300);	//leader tone
	Set_AD9833(1200,0);	_delay_ms(10);	//break
	Set_AD9833(1900,0);	_delay_ms(300);	//leader
	Set_AD9833(1200,0);	_delay_ms(29);	_delay_us(961);	//VIS start bit
	//PD90 VIS code = 99d = 0b1100011
	Set_AD9833(1100,0);	_delay_ms(29);	_delay_us(961);	//bit 0=1
	Set_AD9833(1100,0);	_delay_ms(29);	_delay_us(961);	//bit 1=1
	Set_AD9833(1300,0);	_delay_ms(29);  _delay_us(961);	//bit 2=0
	Set_AD9833(1300,0);	_delay_ms(29);	_delay_us(961);	//bit 3=0
	Set_AD9833(1300,0);	_delay_ms(29);	_delay_us(961);	//bit 4=0
	Set_AD9833(1100,0);	_delay_ms(29);	_delay_us(961);	//bit 5=1
	Set_AD9833(1100,0);	_delay_ms(29);	_delay_us(961);	//bit 6=1
	Set_AD9833(1300,0);	_delay_ms(29);	_delay_us(961);	//Parity bit
	Set_AD9833(1200,0);	_delay_ms(29);	_delay_us(961);	//stop bit
	}
//Image Data	
	for(int lineSet=0; lineSet<=128; lineSet++)
	{	
		Set_AD9833(1200,0); _delay_ms(19); _delay_us(961);	//Sync Pulse
		Set_AD9833(1500,0); _delay_ms(2); _delay_us(41);	//Porch
	//Pixel Interrupt sequence
		pixelCount=0;
		TCC0.CNT=0;
		sei();
		while(pixelCount<=1280);
		cli();
	}
	Set_AD9833(0,0);
*/



SPI_send16(11365); SPI_send8(0); 
_delay_ms(100);
SPI_send16(12288); SPI_send8(68); 
_delay_ms(100);
SPI_send16(1024); SPI_send8(0); 
_delay_ms(100);
SPI_send16(2564); SPI_send8(63); 
_delay_ms(100);
SPI_send16(10496); SPI_send8(50); 
_delay_ms(100);
SPI_send16(10756); SPI_send8(176); 
_delay_ms(100);
SPI_send16(11035); SPI_send8(0); 
_delay_ms(100);

	while(1)
	{
		;
	}
}

ISR(TCC0_CCA_vect)
{
	Set_AD9833(frequency,phase);											//Set waveform generator with frequency and phase calculated in previous cycle
	prevPhase=phase;														//Save previous wave's ending phase for next phase calculation
	prevFreq=frequency;														//Save previous wave's ending frequency for next phase calculation
	frequency = eepromReadByte(&sectorAdress, &pageAddress, &byteAddress);	//Reads one byte from the given location and increments the address pointer
	frequency = 1500 + (frequency* 3.1372549);								//Converting one byte color data to 2 byte frequency
	/* Code for generating single color image
	if(pixelCount==319) frequency = 2253;
	else if(pixelCount==639) frequency = 1782;
	else if(pixelCount==959) frequency = 1757;
	*/
	phase=getphase(prevPhase,prevFreq,532);									//Retrieve phase for next wave
	pixelCount;
}


