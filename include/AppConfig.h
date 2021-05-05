/*
 Name:		AppConfig.h
 Created:	20/04/2021
 Author:	Andrea Santinelli
*/

#ifndef _AppConfig_h
#define _AppConfig_h

// dependencies
#include "ModbusCfg.h"
#include "Utility.h"

// generic define
#define ulong	unsigned long int	
#define uint	unsigned int
#define usint	unsigned short int

// application pins
#define RUN_LED					6											///< Pin D6 is an OUTPUT
#define RAIN_GAUGE_SWITCH 		5											///< Pin D5 is an INPUT

// rain gauge sensor constant
#define RAIN_BUCKETS_CONSTANT	0.3											///< represents the mm of rain fallen for each tipping of the buckets [mm]
#define END_OF_EVENT_TIME		300000										///< represents the inactivity time of the rain gauge in seconds to cancel the rain rate data [ms]
#define FILTERING_TIME			7500										///< represents the minimum time that must pass between one overturnand the next [ms] (7.5 = 480 overturns/hour = 144mm/h)

// class definitions
Utility filterFunctions[TOTAL_NR_OF_SLAVES];								///> Class utility for all slaves

// structures
struct RainRate
{	// structure used to define a rain rate in various units of measurement
	float mmSec;
	float mmHour;
};

// enumerated
enum windDirection
{	// enumerated used to specify the wind direction
	NORD,
	NORD_NORD_EST,
	NORD_EST,
	EST_NORD_EST,
	EST,
	EST_SUD_EST,
	SUD_EST,
	SUD_SUD_EST,
	SUD,
	SUD_SUD_WEST,
	SUD_WEST,
	WEST_SUD_WEST,
	WEST,
	OVEST_NORD_WEST,
	NORD_WEST,
	NORD_NORD_WEST,
	NOT_VALID // leave this last entry!!
};

// structured var
BME280ModbusData BME280Modbus;												///< it contains the data read by the BME280 modbus sensor
AnemometerData Anemometer;													///< it contains the data read by the anemometer together with other data about wind velocity
WindVaneData WindVane;														///< it contains the data read from the wind vane together with other data about wind direction

#pragma region SCHEDULER CONFIGURATION
	// task index list (used to point elements on arrays)
	enum taskIndex
	{
		T1_TASK,
		T2_TASK,
		T3_TASK,
		TOTAL_NR_OF_TASKS // leave this last entry!!
	};

	// global variables used main loop timing
	static const uint8_t LOOP_DURATION_MS	= 20;							///< Main loop execution time (20 ms)
	ulong millisAtLoopBegin					= 0;							///< millis value at the begin of loop

	// global variables used for scheduler timing
	ulong taskCounter[TOTAL_NR_OF_TASKS]	= { 0,		0,		0 };
	ulong taskPeriod[TOTAL_NR_OF_TASKS]		= { 45000,	15000,	5000 };
#pragma endregion

#endif