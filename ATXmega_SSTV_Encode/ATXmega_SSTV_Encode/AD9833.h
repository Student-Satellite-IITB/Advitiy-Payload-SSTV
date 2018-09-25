/*
 * AD9833.h
 *
 * Created: 25-09-2018 14:04:02
 *  Author: Puneet Shrivas
 */ 


#ifndef AD9833_H_
#define AD9833_H_

#include "avr_compiler.h"

#define FMCLK 25000000			  //Crystal frequency on AD development board
#define PI 3.14159

void Set_AD9833(float frequency, unsigned int phase);

unsigned int getphase(float prevPhase,float nextFreq, float prevTime);

#endif /* AD9833_H_ */