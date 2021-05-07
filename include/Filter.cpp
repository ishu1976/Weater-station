/*
 Name:		Filter.cpp
 Created:	05/05/2021
 Author:	Andrea Santinelli
*/

// dependencies
#include "Filter.h"

// constructor
Filter::Filter(void)
{
	// clear buffer
	delete[] _movingAverageBuffer;

	// reset all internal variables
	_bufferSize		= 0;
	_samplesNr		= 0;
	_averageValue	= 0;
	_outputValue	= 0;
}

void Filter::Begin(uint8_t size)
{
	// size must be greater than 0 to create an array
	if (size > 0)
	{
		_bufferSize = size;
		_movingAverageBuffer = new word[_bufferSize + 1];
	}
}

float Filter::MovingAverage(float pv)
{
	// buffer has been to be defined!
	if (_movingAverageBuffer)
	{
		// load the first cell of the buffer
		_movingAverageBuffer[0] = pv;

		/* 
		 count nr.of elements loaded (_bufferSize is the maximum)
		 N.B. - this count is useful if you have large buffers. 
		 It allows to have a realistic average even if not all elements 
		 of the buffer are populated with valid data.
		*/
		if (_samplesNr < _bufferSize)
		{
			_samplesNr++;
		}

		// move buffer forward and load cell [0]
		for (_index = _samplesNr; _index >= 1; _index--)
		{
			_movingAverageBuffer[_index] = _movingAverageBuffer[_index - 1];
		}

		// average value calculation
		if (_samplesNr > 0)
		{
			// init value before calculation
			_averageValue = 0;
			// sum of samples loaded
			for (_index = 1; _index <= _samplesNr; _index++)
			{
				_averageValue += (float)_movingAverageBuffer[_index];
			}
			// average calculation
			_averageValue = _averageValue / (float)_samplesNr;
		}
	}
	// return value!
	return _averageValue;
}

float Filter::LowPassIIR(float pv, float kv)
{
	// apply IIR LowPass filter
	_outputValue += (constrain(0.0f, kv, 1.0f)) * (pv - _outputValue);

	// return value!
	return _outputValue;
}