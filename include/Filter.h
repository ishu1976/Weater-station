/*
 Name:		Filter.h
 Created:	05/05/2021
 Author:	Andrea Santinelli
*/

#ifndef _Filter_h
#define _Filter_h

// dependencies
#include "Arduino.h";

class Filter
{
public:
	Filter();
	void Begin(uint8_t);
	float MovingAverage(float);
	float LowPassIIR(float, float);

private:
	// moving average filter vars
	word* _movingAverageBuffer  = 0;
	uint8_t _bufferSize			= 0;
	float _averageValue			= 0;
	uint8_t _index				= 0;
	uint8_t _samplesNr			= 0;

	// IIR filter vars
	float _outputValue			= 0;
};

#endif