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

volatile int frequency=1800,phase=0,prevPhase=0,prevFreq=0,pixelCount=0;

int main(void)
{
	SetClock0();	//SetClock1();
	SetUsart();		//sei();
	SPI_Master_init();
	setUp16MhzExternalOsc();
	SPI_send16(0x100);	//Reset AD9833
	Set_AD9833(1500,0);
	_delay_ms(3000);
// VIS Code 
	{_delay_ms(100);
	Set_AD9833(1900,0);	_delay_ms(300);	//leader tone
	Set_AD9833(1200,0);	_delay_ms(10);	//break
	Set_AD9833(1900,0);	_delay_ms(300);	//leader
	Set_AD9833(1200,0);	_delay_ms(29);	_delay_us(839);	//VIS start bit
	//PD90 VIS code = 99d = 0b1100011
	Set_AD9833(1100,0);	_delay_ms(29);	_delay_us(839);	//bit 0=1
	Set_AD9833(1100,0);	_delay_ms(29);	_delay_us(839);	//bit 1=1
	Set_AD9833(1300,0);	_delay_ms(29);  _delay_us(839);	//bit 2=0
	Set_AD9833(1300,0);	_delay_ms(29);	_delay_us(839);	//bit 3=0
	Set_AD9833(1300,0);	_delay_ms(29);	_delay_us(839);	//bit 4=0
	Set_AD9833(1100,0);	_delay_ms(29);	_delay_us(839);	//bit 5=1
	Set_AD9833(1100,0);	_delay_ms(29);	_delay_us(839);	//bit 6=1
	Set_AD9833(1300,0);	_delay_ms(29);	_delay_us(839);	//Parity bit
	Set_AD9833(1200,0);	_delay_ms(29);	_delay_us(839);	//stop bit
	}
//Image Data	
	for(int lineSet=0; lineSet<=128; lineSet++)
	{
		Set_AD9833(1200,0); _delay_ms(19); _delay_us(840);	//Sync Pulse
		Set_AD9833(1500,0); _delay_ms(1); _delay_us(919);	//Porch
	//Pixel Interrupt sequence
		pixelCount=0;
		TCC0.CNT=0;
		sei();
		while(pixelCount<=12800);
		cli();
	}
	while(1)
	{
		
	}
}

ISR(TCC0_OVF_vect)
{
	Set_AD9833(frequency,phase);
	prevPhase=phase;
	prevFreq=frequency;
	pixelCount++;
//Frequency Retrieval section
	phase=getphase(prevPhase,prevFreq,532);
	pixelCount++;
}


