/*
 * NXH5104.h
 *
 * Created: 26-09-2018 21:44:01
 *  Author: Puneet Shrivas
 */ 


#ifndef NXH5104_H_
#define NXH5104_H_

#include "avr_compiler.h"

/*EEPROM commands defines. */
#define WREN 6		//Write Enable command
#define WRDI 4		//Write Disable command
#define WRITE 2		//Write Begin command
#define READ 3		//Read Begin command

/*EEPROM array ID. */
#define DOWNLINK1 1 //EEPROM for storing 5 compressed images
#define DOWNLINK2 2 //EEPROM for storing 3 compressed images and runtime memory
#define UPLINK 3	//EEPROM for storing uplinked compressed images

/*  Hardware setup:
 * - PD2 (SS1)
 * - PD3 (SS2)
 * - PD4 (SS3)
 * - PD5 (MOSI)
 * - PD6 (MISO)
 * - PD7 (SCK)
 */
	
/*	Initializes Port D as Master for SPI
 */
void eepromInit();

/*	Pulls Slave Select line low for SPI
 *
 *	Parameters
 *	eepromID	Presently Selected EEPROM 
 */
void eepromMasterLow(int eepromID);

/*	Pulls Slave Select line high for SPI
 *
 *	Parameters
 *	eepromID	Presently Selected EEPROM 
 */
void eepromMasterHigh(int eepromID);

/*	Writes one byte ahead of the provided address
 *
 *	Parameters
 *	sectorAddress	Sector ID (0-7)
 *	pageAddress		Page number (0-255)
 *	byteAddress		Pointer inside the page (0-255)
 *	data			1 byte data to be written
 */
void eepromWriteByte(uint8_t* sectorAddress, uint8_t* pageAddress, uint8_t* byteAddress, uint8_t data); 

/*	Reads one byte from the provided address
 *
 *	Parameters
 *	sectorAddress	Sector ID (0-7)
 *	pageAddress		Page number (0-255)
 *	byteAddress		Pointer inside the page (0-255)
 */
uint8_t eepromReadByte(uint8_t* sectorAddress, uint8_t* pageAddress, uint8_t* byteAddress);

#endif /* NXH5104_H_ */