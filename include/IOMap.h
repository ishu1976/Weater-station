/*
 Name:		IOMap.h
 Created:	20/04/2021
 Author:	Andrea Santinelli
*/

#ifndef _IOMap_h
#define _IOMap_h

/* dependencies */

/* class definitions */

/* application pins */
#define RUN_LED				6		// Pin D6 is an OUTPUT
#define RAIN_GAUGE_SWITCH 	5		// Pin D5 is an INPUT

/* scheduler defines */
#define LOOP_TIME			100		// Main loop execution time (100 ms)

#define T1_TASK_TIME		15000	// Task T1 execution time (15 sec)
#define T2_TASK_TIME		5000	// Task T2 execution time (5 sec)
#define T3_TASK_TIME		5000	// Task T3 execution time (5 sec)

#define T1_TASK				0
#define T2_TASK				1
#define T3_TASK				2

#define ulong				unsigned long int	
#define uint				unsigned short int

/* global var declarations */
ulong millisAtLoopBegin		= 0;	// millis value at the begin of loop
int taskCounter[]			= { T1_TASK_TIME, T2_TASK_TIME, T3_TASK_TIME };

/* structure definitions */
struct ST_Timer
{	/* structure used to define a timer function */
	uint8_t PT = 0;
	uint8_t ET = 0;
};

#endif