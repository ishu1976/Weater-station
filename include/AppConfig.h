/*
 Name:		AppConfig.h
 Created:	20/04/2021
 Author:	Andrea Santinelli
*/

#ifndef _AppConfig_h
#define _AppConfig_h

/* dependencies */
#include "ModbusCfg.h"

/* application pins */
#define RUN_LED					6			// Pin D6 is an OUTPUT
#define RAIN_GAUGE_SWITCH 		5			// Pin D5 is an INPUT

/* scheduler defines */
#define LOOP_DURATION_MS		20			// Main loop execution time (20 ms)
#define TASK_DURATION_SEC(n)	(n * 1000)	// Task execution time
/* rain gauge constant */
#define RAIN_CONSTANT			0.3			// represents the mm of rain fallen for each tipping of the buckets

#pragma region ENUMERATED
	/* task index list */
	enum taskIndex
	{
		T1_TASK,
		T2_TASK,
		T3_TASK,
		TOTAL_NR_OF_TASKS // leave this last entry!!
	};
	/* enumerated used to specify the wind direction returned by the  wind vane sensor */
	enum windDirection
	{
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
#pragma endregion

/* data define */
#define ulong	unsigned long int	
#define uint	unsigned short int

/* global var declarations */
ulong millisAtLoopBegin = 0;			// millis value at the begin of loop
ulong taskCounter[TOTAL_NR_OF_TASKS] = { 0, 0, 0 };

/* structured var */
BME280ModbusData BME280Modbus;			// it contains the data read by the BME280 modbus sensor
AnemometerData Anemometer;				// it contains the data read by the anemometer together with other data about wind velocity
WindVaneData WindVane;					// it contains the data read from the wind vane together with other data about wind direction

#endif