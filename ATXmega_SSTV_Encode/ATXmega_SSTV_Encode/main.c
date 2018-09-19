/*
 * ATXmega_SSTV_Encode.c
 *
 * Created: 29-08-2018 18:50:02
 * Author : Puneet Shrivas
 */ 


#define F_CPU 2000000UL
#define CPU_PRESCALER 1			  //Prescaler for Timer calculations
#define PI 3.14159
#define NUM_BYTES 4				  //Size of Data Packet Array
#define USART USARTC0			  //Initialization of USART 
#define FMCLK 25000000			  //Crystal frequency on AD development board

#include <avr/io.h>
#include <util/delay.h>
#include "USART.h"
#include "avr_compiler.h"
#include "pmic_driver.h"
#include "spi_driver.h"

SPI_Master_t spiMasterC;											//SPI master on PORT C
SPI_DataPacket_t dataPacket;										//SPI Data packet
uint8_t masterSendData[NUM_BYTES];									//Data packet from Master
uint8_t masterReceivedData[NUM_BYTES];								//Data packet from slave
bool success = true;												//SPI Result of transmission
PORT_t *ssPort = &PORTC;											//instantiation for SPI
volatile int pixelCount=0,timeCount[2]={0,0};
int interruptPeriod = ((((F_CPU)/(CPU_PRESCALER*1000000))*532)-1);	//Counter cycles for 532us
int led =1;
USART_data_t USART_data;											//Instantiation of USART data packet
volatile int frequency=0,phase=0,prevPhase=0,prevFreq=0;
void SetClock0()
{
	//Compare Match A timer													
  /*PMIC_EnableLowLevel();
 	TCC0.CTRLB = TC0_CCAEN_bm | TC_WGMODE_FRQ_gc;
 	TCC0.INTCTRLB = (uint8_t) TC_CCAINTLVL_LO_gc;
 	TCC0.PER =UINT16_MAX;
 	TCC0_CCAH = ((TEMP>>8) & 0x00FF);
 	TCC0_CCAL = (TEMP & 0x00FF);
 	TCC0.CTRLA = TC_CLKSEL_DIV1_gc;*/
  
	//Overflow timer														
	PMIC_EnableHighLevel();				//Enable interrupts : High level for timer
	TCC0.CTRLA = TC_CLKSEL_DIV1_gc;		//Set Prescaler 1(Same as CPU_PRESCALER)
	TCC0.CTRLB= TC_WGMODE_NORMAL_gc;    //Wave generation mode : Normal
	TCC0.INTCTRLA = TC_OVFINTLVL_HI_gc;	//Enable overflow interrupt
	TCC0.PER = interruptPeriod;		    //Initialize Period
}
void SetClock1()
{	
	TCC1.PER =0xFF;					//Set period 
	TCC1.CTRLA = TC_CLKSEL_DIV1_gc;	//Set Prescaler 1
}

void SetUsart()
{
	PORTC.DIRSET   = PIN3_bm;															   		//Set TX as output
	PORTC.DIRCLR   = PIN2_bm;					   												//Set RX as input
	USART_InterruptDriver_Initialize(&USART_data, &USART, USART_DREINTLVL_LO_gc);	
	USART_Format_Set(USART_data.usart, USART_CHSIZE_8BIT_gc,USART_PMODE_DISABLED_gc, false);	//Set 8 bit format, no parity	
	USART_RxdInterruptLevel_Set(USART_data.usart, USART_RXCINTLVL_LO_gc);
	USART_Baudrate_Set(&USART, 12 , 0);															//gives baudrate 9600
	USART_Rx_Enable(USART_data.usart);				
	USART_Tx_Enable(USART_data.usart);
	PMIC.CTRL |= PMIC_LOLVLEX_bm;																//Enable low level interrupt for USART
}
void USARTsend(uint8_t data)
{
	sei();											//Enable global interrupts						
	USART_TXBuffer_PutByte(&USART_data, data);		//Write data and wait for transfer	
	cli();											//Disable global interrupts
}
void USARTsend16(uint16_t data)
{
	uint8_t MSdata = ((data>>8) & 0x00FF);  		//filter out MS
	uint16_t LSdata = (data & 0x00FF);				//filter out LS
	sei();											//Enable global interrupts
	USART_TXBuffer_PutByte(&USART_data, MSdata);	//Write MSdata and wait for transfer
	USART_TXBuffer_PutByte(&USART_data, LSdata);	//Write LSdata and wait for transfer
	cli();											//Disable global interrupts
}

void SPI_Master_init()
{
	 /*  Hardware setup:
	 * - Connect PC4 to PD4 (SS)
	 * - Connect PC5 to PD5 (MOSI)
	 * - Connect PC6 to PD6 (MISO)
	 * - Connect PC7 to PD7 (SCK)
	 */
	PORTC.DIRSET = PIN4_bm;																							//Set SS as output
	PORTC.DIRSET = PIN5_bm;																							//MOSI as output
	PORTC.DIRSET = PIN7_bm;																							//SCK as output
	PORTC.PIN4CTRL = PORT_OPC_WIREDANDPULL_gc;																		//Set PullUp at SS 
	PORTC.OUTSET = PIN4_bm;																							//Set SS high for no Slave
	SPI_MasterInit(&spiMasterC,&SPIC,&PORTC,false,SPI_MODE_2_gc,SPI_INTLVL_OFF_gc,true,SPI_PRESCALER_DIV4_gc);		//Initialize device as master (Mode 2, MSB first, 2X speed, prescaler 4)
}

void SPI_send8(uint8_t data)
{
	SPI_MasterSSLow(ssPort, PIN4_bm);				//Set Slave Select Low	
	SPI_MasterTransceiveByte(&spiMasterC, data);	//Write and wait for transceiving
	SPI_MasterSSHigh(ssPort, PIN4_bm);				//Set Slave Select High
}
uint8_t SPI_receive8()
{
	SPI_MasterCreateDataPacket(&dataPacket,masterSendData,masterReceivedData,NUM_BYTES,&PORTC,PIN4_bm);		
    SPI_MasterTransceivePacket(&spiMasterC, &dataPacket);	//Wait for reception
	return masterReceivedData[0];							
}
void SPI_send16(uint16_t data)
{
	PORTA_DIRSET = PIN0_bm;
	PORTF_DIRSET = PIN0_bm|PIN1_bm|PIN2_bm|PIN3_bm|PIN4_bm|PIN5_bm|PIN6_bm|PIN7_bm;
	PORTA_OUTSET = PIN0_bm;
	uint8_t MSdata = ((data>>8) & 0x00FF);		//filter out MS
	uint8_t LSdata = (data & 0x00FF);			//filter out LS
	SPI_MasterSSLow(ssPort, PIN4_bm);			//Set Slave Select Low
	PORTA_OUTCLR = PIN0_bm;				
	SPI_MasterTransceiveByte(&spiMasterC, MSdata); 
	SPI_MasterTransceiveByte(&spiMasterC, LSdata);
	SPI_MasterSSHigh(ssPort, PIN4_bm);			//Set Slave Select High
	PORTA_OUTSET = PIN0_bm;
}
void Set_AD9833(float frequency, unsigned int phase)
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
int main(void)
{
	SetClock0();	//SetClock1();
	SetUsart();		//sei();
	SPI_Master_init();
	SPI_send16(0x100);	//Reset AD9833
	
//VIS Code 
	_delay_ms(100);
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
	
//Image Data	
	for(int lineSet=0; lineSet<=128; lineSet++)
	{
		Set_AD9833(1200,0); _delay_ms(19); _delay_us(840);	//Sync Pulse
		Set_AD9833(1500,0); _delay_ms(1); _delay_us(919);	//Porch
	//Pixel Interrupt sequence
		pixelCount=0;
		TCC0.CNT=0;
		sei();
		while(pixelCount<=1280);
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

ISR(USARTC0_DRE_vect)
{
	USART_DataRegEmpty(&USART_data);
}

