/*
 * AD9833.c
 *
 * Created: 25-09-2018 14:03:29
 *  Author: Puneet Shrivas
 */ 

#include "AD9833.h"
#include "spi_driver.h"

void Set_AD9833(float frequency, unsigned int phase) // Zero amplitude time : 39 miroseconds
{
	long FreqReg = (((float)frequency)*pow(2,28))/(float)FMCLK;	  //Calculate frequency to be sent to AD9833
	int MSB = (int)((FreqReg &  0xFFFC000) >> 14);				  //Extract first 14 bits of FreqReg and place them at last 14 bits of MSB
	int LSB = (int)((FreqReg & 0x3FFF));						  //Extract last 14 bits of FreqReg and place them at last 14 bits of MSB
	MSB|=0x4000;												  //Set D14,D15 = (1,0) for using FREQ0 registers, MSB has all 16 bits set
	LSB|=0x4000;     											  //Set D14,D15 = (1,0) for using FREQ0 registers, LSB has all 16 bits set
	SPI_send16(0x2100);											  //Define waveform and set reset bit
	SPI_send16(LSB);											  //Write LSBs
	SPI_send16(MSB);											  //Write MSBs
	phase&=0x0FFF;
	phase|=0xC000;												  //Set Phase write enable bytes
	//SPI_write16(0xC000);										  //Mode selection for writing to phase register bit, selection of PHASE0 register (Needs to be fixed)
	SPI_send16(phase);											  //Write Phase bytes
	SPI_send16(0x2000);											  //Unset reset bit
}

unsigned int getphase(float prevPhase,float nextFreq, float prevTime)
{
	prevTime/=1000000;																	//Convert time to microseconds
	prevPhase/=2048/PI;																	//Change scaling of Phase as required by AD9833
	float returnPhase=((fmod(prevTime,(1/nextFreq))*2*PI*nextFreq)+prevPhase)*2048/PI;	//Calculate phase completed by previous wave
	return (unsigned int) returnPhase;													//Return in 16 bit format
}

