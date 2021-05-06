/*
 Name:		Filter.cpp
 Created:	05/05/2021
 Author:	Andrea Santinelli
*/

// dependencies
#include "Filter.h"

Filter::Filter(void)
{
	// clear buffer
	memset(_movingAverageBuffer, 0, sizeof(_movingAverageBuffer));

	// reset all internal variables
	_samplesNr		= 0;
	_averageValue	= 0;
	_outputValue	= 0;
}

float Filter::MovingAverage(float pv)
{
	// load the first cell of the buffer
	_movingAverageBuffer[BUFFER_ENTRY] = pv;

	// count nr.of elements loaded
	_samplesNr++;
	_samplesNr = constrain(FIRST_ITEM, _samplesNr, LAST_ITEM);

	// move buffer forward and load cell [0]
	for (_index = _samplesNr; _index >= FIRST_ITEM; _index--)
	{
		_movingAverageBuffer[_index] = _movingAverageBuffer[_index - 1];
	}

	// average value calculation
	if (_samplesNr > 0)
	{
		// init value before calculation
		_averageValue = 0;
		// sum of samples loaded
		for (_index = FIRST_ITEM; _index <= _samplesNr; _index++)
		{
			_averageValue += (float)_movingAverageBuffer[_index];
		}
		// average calculation
		_averageValue = _averageValue / (float)_samplesNr;
	}
	
	// return value!
	return _averageValue;
}

float Filter::Kalman(float pv, float kv)
{
	// apply kalman filter
	_outputValue += (constrain(0.0f, kv, 1.0f)) * (pv - _outputValue);

	// return value!
	return _outputValue;
}