/*
 * NXH5104.c
 *
 * Created: 27-09-2018 14:04:21
 *  Author: Puneet Shrivas
 */ 
#include "NXH5104.h"
#include "spi_driver.h"
#include "avr_compiler.h"

PORT_t *ssport2 = &PORTD;		//Initialization of Port D as master for EEPROM SPI
SPI_Master_t spiMasterD;		
SPI_DataPacket_t dataPacketD;
uint8_t masterSendDataD[1];
uint8_t masterReceivedDataD[1];
//bool success = true;
int eepromID = 2;				//Downlink 2 set as default memory for testing SSTV

void eepromInit()
{
	PORTD.DIRSET = PIN2_bm;																							//Set SS1 as output
	PORTD.DIRSET = PIN3_bm;																							//Set SS2 as output
	PORTD.DIRSET = PIN4_bm;																							//Set SS3 as output
	PORTD.DIRSET = PIN5_bm;																							//MOSI as output
	PORTD.DIRSET = PIN7_bm;																							//SCK as output
	PORTD.PIN2CTRL = PORT_OPC_WIREDANDPULL_gc;																		//Set PullUp at SS1 
	PORTD.PIN3CTRL = PORT_OPC_WIREDANDPULL_gc;																		//Set PullUp at SS2
	PORTD.PIN4CTRL = PORT_OPC_WIREDANDPULL_gc;																		//Set PullUp at SS3
	PORTD.OUTSET = PIN2_bm;																							//Set SS1 high for no Slave
	PORTD.OUTSET = PIN3_bm;																							//Set SS2 high for no Slave
	PORTD.OUTSET = PIN4_bm;																							//Set SS3 high for no Slave
	SPI_MasterInit(&spiMasterD,&SPID,&PORTD,false,SPI_MODE_2_gc,SPI_INTLVL_OFF_gc,true,SPI_PRESCALER_DIV4_gc);		//Initialize device as master (Mode 2, MSB first, 2X speed, prescaler 4)
	eepromID=DOWNLINK2;
}

void eepromMasterLow(int eepromID)
{
	switch (eepromID)
	{
		case 1 : SPI_MasterSSLow(ssport2, PIN2_bm); break;
		case 2 : SPI_MasterSSLow(ssport2, PIN3_bm); break;
		case 3 : SPI_MasterSSLow(ssport2, PIN4_bm); break;
	}
}

void eepromMasterHigh(int eepromID)
{
	switch (eepromID)
	{
		case 1 : SPI_MasterSSHigh(ssport2, PIN2_bm); break;
		case 2 : SPI_MasterSSHigh(ssport2, PIN3_bm); break;
		case 3 : SPI_MasterSSHigh(ssport2, PIN4_bm); break;
	}
}

void eepromWriteByte(uint8_t* sectorAddress, uint8_t* pageAddress, uint8_t* byteAddress, uint8_t data) //address to be sent for last byte which was written
{
	if(*byteAddress==255)							//Check for end of page
	{
		if(*pageAddress==255)						//Check for end of sector
		{
			*(sectorAddress)=*(sectorAddress)+1;
			*(pageAddress) = 0;
			*byteAddress = 0;
		}
		else 
		{
			*(pageAddress)=*(pageAddress)+1;
			*byteAddress=0;
		}
	}
	else *(byteAddress)=*(byteAddress)+1;
	eepromMasterLow(eepromID);
	SPI_MasterTransceiveByte(&spiMasterD, WRITE);
	SPI_MasterTransceiveByte(&spiMasterD, *sectorAddress);
	SPI_MasterTransceiveByte(&spiMasterD, *pageAddress);
	SPI_MasterTransceiveByte(&spiMasterD, *byteAddress);
	SPI_MasterTransceiveByte(&spiMasterD, data);
	eepromMasterHigh(eepromID);
}

uint8_t eepromReadByte(uint8_t* sectorAddress, uint8_t* pageAddress, uint8_t* byteAddress)
{
	SPI_MasterCreateDataPacket(&dataPacketD,masterSendDataD,masterReceivedDataD,1,&PORTD,PIN4_bm); //sspinmask needs to be set
	eepromMasterLow(eepromID);
	SPI_MasterTransceiveByte(&spiMasterD, READ);
	SPI_MasterTransceiveByte(&spiMasterD, *sectorAddress);
	SPI_MasterTransceiveByte(&spiMasterD, *pageAddress);
	SPI_MasterTransceiveByte(&spiMasterD, *byteAddress);
	SPI_MasterTransceivePacket(&spiMasterD, &dataPacketD);	//Wait for reception
	eepromMasterHigh(eepromID);
	return masterReceivedDataD[0];
}