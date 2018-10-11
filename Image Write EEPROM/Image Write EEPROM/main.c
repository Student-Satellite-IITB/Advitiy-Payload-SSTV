/*
 * Image Write EEPROM.c
 *
 * Created: 06-10-2018 16:33:28
 * Author : Puneet Shrivas
 */ 

#define F_CPU 16000000

#include <avr/io.h>
#include <util/delay.h>
#include "USART.h"
#include "avr_compiler.h"
#include "pmic_driver.h"
#include "spi_driver.h"
#include "TC_driver.h"
#include "NXH5104.h"

int main(void)
{
	uint8_t sectorAdress=0, pageAddress=0, byteAddress=0, dataByte=0, byteCount=0;
	setUp16MhzExternalOsc();		//Required for setting 16 Mhz frequency
	SetUsart();						
	eepromInit();					//Initialize SPI for EEPROM on Port C
	
	for(byteCount=0; byteCount<163840; byteCount++)								//Loop for writing image bytes to eeprom	
	{
		dataByte=USARTrecieve();												//Transfer of byte from PC to ATxmega
		eepromMasterLow(DOWNLINK1);												//Set slave select low for eeprom
		eepromWriteByte(&sectorAdress, &pageAddress, &byteAddress, dataByte);	//Transcieve byte
		eepromMasterHigh(DOWNLINK1);											//Pull slave select back to high
	}
	
	sectorAdress=0; pageAddress=0; byteAddress=0; dataByte=0;

	for(byteCount=0; byteCount<163840; byteCount++)								//Loop for reading back written data for recheck
	{
		eepromMasterLow(DOWNLINK1);												//Set Slave Select low for eeprom
		dataByte=eepromReadByte(&sectorAdress, &pageAddress, &byteAddress);		//Read byte from location
		eepromMasterHigh(DOWNLINK1);											//Set Slave Select high for eeprom
		USARTsend(dataByte);
	}	
	while (1)
	{
	}
}

