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
#define RUN_LED				6						// Pin D6 is an OUTPUT
#define RAIN_GAUGE_SWITCH 	5						// Pin D5 is an INPUT

/* miscellanea */
#define T0_TASK_TIME		100						// Task T0 execution time (100 ms)
#define T1_TASK_TIME		1000					// Task T1 execution time (1000 ms)
#define ulong				unsigned long int	
#define uint				unsigned short int

/* structure definitions */
struct ST_Timer
{	/* structure used to define a timer function */
	uint PT = 0;
	uint ET = 0;
};

#endif