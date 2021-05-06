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
	// class methods
	Filter();
	float MovingAverage(float);
	float Kalman(float, float);

private:

	enum
	{ // enumerated used to define a moving average buffer
		BUFFER_ENTRY	= 0,
		FIRST_ITEM		= 1,
		LAST_ITEM		= 40, 	///> change this value if you want to increase buffer dimension
		TOTAL_NR_OF_ELEMENTS 	///> leave this last entry!!
	};

	// moving average filter vars
	word _movingAverageBuffer[TOTAL_NR_OF_ELEMENTS];
	float _averageValue = 0;
	uint8_t _index		= 0;
	uint8_t _samplesNr	= 0;
	// kalman filter vars
	float _outputValue	= 0;
};

#endif