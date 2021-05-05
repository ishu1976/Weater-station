/*
 Name:		Utility.h
 Created:	05/05/2021
 Author:	Andrea Santinelli
*/

#ifndef _Utility_h
#define _Utility_h

// dependencies
#include "Arduino.h";

class Utility
{
public:
	// class methods
	void Init();
	float MovingAverage(float);
	float Kalman(float, float);

private:

	enum
	{ // enumerated used to define a moving average buffer
		BUFFER_ENTRY	= 0,
		FIRST_ITEM		= 1,
		LAST_ITEM		= 10, 	///> change this value if you want to increase buffer dimension
		TOTAL_NR_OF_ELEMENTS 	///> leave this last entry!!
	};

	// class variables
	word _movingAverageBuffer[TOTAL_NR_OF_ELEMENTS];
	float _outputValue  = 0;
	uint8_t _index		= 0;
	uint8_t _samplesNr	= 0;
};

#endif