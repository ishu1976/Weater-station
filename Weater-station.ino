/*
 Name:		Weater_station.ino
 Created:	20/04/2021
 Author:	Andrea Santinelli

 Copyright: 2021 - Andrea Santinelli

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

 Libraries used in the project:
 - https://github.com/4-20ma/ModbusMaster			by Doc Walker
 - https://github.com/andresarmento/modbus-arduino	by André Sarmento Barbosa
*/

/* dependencies */
#include <ModbusMaster.h>
#include <EtherCard.h>
#include <Modbus.h>
#include <ModbusIP_ENC28J60.h>
#include "include/IOMap.h"
#include "include/ModbusCfg.h"

#pragma region DECLARATIONS
	/* local defines */

	/* global var declaration*/
	char softwareVersion[]	= "2104.07";		// software version
	ulong millisAtLoopBegin = 0;				// millis value at the begin of loop
	uint taskCounterT1		= 0;				// counter for schedule call of T1 task

	/* input / output connected var */
	int doRunState			= LOW;				// rapresents the state of GREEN led on sensor board
	int diRainGaugeSwitch	= LOW;				// rapresents the state of rain gouge switch

	/* status var */

#pragma endregion

/* the setup function runs once when you press reset or power the board */
void setup()
{
	/* define i/o mode */
	pinMode(RUN_LED, OUTPUT);
	pinMode(RAIN_GAUGE_SWITCH, INPUT);

	/* init input var at input state */
	diRainGaugeSwitch = digitalRead(RAIN_GAUGE_SWITCH);

	/* modbus read configuration for thermobarometer sensor */
	stBME280ModbusRdVarCfg.slaveId			= BME280_MODBUS_ID;
	stBME280ModbusRdVarCfg.firstElementAdr	= 0x064;
	stBME280ModbusRdVarCfg.numberOfElements	= 8;

	/* modbus read configuration for anemometer sensor */
	stAnemometerRdVarCfg.slaveId			= ANEMOMETER_ID;
	stAnemometerRdVarCfg.firstElementAdr	= 0x000;
	stAnemometerRdVarCfg.numberOfElements	= 1;

	/* modbus read configuration for wind vane sensor */
	stWindVaneRdVarCfg.slaveId				= WIND_VANE_ID;
	stWindVaneRdVarCfg.firstElementAdr		= 0x000;
	stWindVaneRdVarCfg.numberOfElements		= 1;

	/* modbus write configuration for thermobarometer sensor */
	stBME280ModbusRdVarCfg.slaveId			= BME280_MODBUS_ID;
	stBME280ModbusRdVarCfg.firstElementAdr	= 0x0C8;
	stBME280ModbusRdVarCfg.numberOfElements = 6;
}

/* the loop function runs over and over again until power down or reset */
void loop()
{
	/* START LOOP: get millis value at loop begin */
	millisAtLoopBegin = millis();

	/* read digital input */
	digitalWrite(RUN_LED, doRunState);

	/* wait for execution of task T1 */
	if (taskCounterT1 < T1_TASK_TIME)
	{
		/* elapsed time is updated at every loop */
		taskCounterT1 += T0_TASK_TIME;
	}
	else
	{
		/* execute task T1 */
		T1_Task();
	}
	/* write digital output */
	digitalWrite(RUN_LED, doRunState);

	/* END LOOP: wait task time (every loop has a fixed duration) */
	while (abs(millis() - millisAtLoopBegin) <= T0_TASK_TIME);
}

/* task T1 is executed every time the scheduling time has elapsed */
void T1_Task()
{
	/* init task timer T1 for next call */
	taskCounterT1 = 0;
}