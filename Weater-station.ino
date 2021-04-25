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
	#define SERIAL_PRINT						// comment this define to deactivate print on serial monitor

	/* global var declaration*/
	char softwareVersion[]	= "2104.07";		// software version
	ulong millisAtLoopBegin = 0;				// millis value at the begin of loop
	uint taskCounterT1		= 0;				// counter for schedule call of T1 task
	int slaveId;

	/* input / output connected var */
	int doRunState			= LOW;				// represents the state of GREEN led on sensor board
	int diRainGaugeSwitch	= LOW;				// represents the state of rain gouge switch

	/* status var */

#pragma endregion

/* the setup function runs once when you press reset or power the board */
void setup()
{
	/* define i/o mode */
	pinMode(RUN_LED, OUTPUT);
	pinMode(RAIN_GAUGE_SWITCH, INPUT);
	pinMode(RE_DE_PIN, OUTPUT);

	/* init input var at input state */
	diRainGaugeSwitch = digitalRead(RAIN_GAUGE_SWITCH);

	/* modbus read configuration for thermobarometer sensor */
	arSlaveRdVarCfg[BME280_MODBUS_ID].enable			= true;
	arSlaveRdVarCfg[BME280_MODBUS_ID].firstElementAdr	= 0x064;
	arSlaveRdVarCfg[BME280_MODBUS_ID].numberOfElements	= 8;

	/* modbus read configuration for anemometer sensor */
	arSlaveRdVarCfg[ANEMOMETER_ID].enable				= true;
	arSlaveRdVarCfg[ANEMOMETER_ID].firstElementAdr		= 0x000;
	arSlaveRdVarCfg[ANEMOMETER_ID].numberOfElements		= 1;

	/* modbus read configuration for wind vane sensor */
	arSlaveRdVarCfg[WIND_VANE_ID].enable				= true;
	arSlaveRdVarCfg[WIND_VANE_ID].firstElementAdr		= 0x000;
	arSlaveRdVarCfg[WIND_VANE_ID].numberOfElements		= 1;

	/* modbus write configuration for thermobarometer sensor */
	arSlaveWrVarCfg[BME280_MODBUS_ID].enable			= true;
	arSlaveWrVarCfg[BME280_MODBUS_ID].firstElementAdr	= 0x0C8;
	arSlaveWrVarCfg[BME280_MODBUS_ID].numberOfElements	= 6;

	/* init serial at modbus speed */
	Serial.begin(MODBUS_SPEED);
	Serial.print(F("Start modbus/serial at "));
	Serial.print(MODBUS_SPEED);
	Serial.println(F(" bit/s"));
	Serial.println();

	/* software info */
	#ifdef SERIAL_PRINT
	Serial.println(F("MAIN WEATER STATION BOARD"));
	Serial.print(F("Software version: "));
	Serial.println(softwareVersion);
	Serial.println();
	#endif
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

	

	for (slaveId = BME280_MODBUS_ID; slaveId <= WIND_VANE_ID; slaveId++)
	{
		Master_RTU.begin(slaveId, Serial);
		Master_RTU.preTransmission(setTxMode);
		Master_RTU.postTransmission(setRxMode);
		if (arSlaveRdVarCfg[slaveId].enable)
		{
			Master_RTU.readHoldingRegisters(arSlaveRdVarCfg[slaveId].firstElementAdr, arSlaveRdVarCfg[slaveId].numberOfElements);
		}
	}

}

/* calback function used to set MAX485 on TX mode */
void setTxMode()
{
	digitalWrite(RE_DE_PIN, TX_MODE);
}

/* calback function used to set MAX485 on RX mode */
void setRxMode()
{
	digitalWrite(RE_DE_PIN, RX_MODE);
}
