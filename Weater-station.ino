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
	#define SERIAL_PRINT									// comment this define to deactivate print on serial monitor

	/* global var declaration*/
	char softwareVersion[]	= "2104.07";					// software version
	uint8_t device			= BME280_TEMP_HUM;
	bool serialBusy			= false;

	/* input / output connected var */
	int doRunState			= LOW;							// represents the state of GREEN led on sensor board
	int diRainGaugeSwitch	= LOW;							// represents the state of rain gouge switch

	/* structured var */
	ST_BME280ModbusData stBME280Modbus;						// it contains the data read by the BME280 modbus sensor
	ST_AnemometerData stAnemometer;							// it contains the data read by the anemometer together with other data about wind velocity
	ST_WindVaneData stWindVane;								// it contains the data read from the wind vane together with other data about wind direction
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
	arSlaveRdVarCfg[BME280_TEMP_HUM].enable				= true;
	arSlaveRdVarCfg[BME280_TEMP_HUM].firstElementAdr	= 0x64;
	arSlaveRdVarCfg[BME280_TEMP_HUM].numberOfElements	= 8;

	/* modbus read configuration for anemometer sensor */
	arSlaveRdVarCfg[ANEMOMETER].enable				= true;
	arSlaveRdVarCfg[ANEMOMETER].firstElementAdr		= 0x00;
	arSlaveRdVarCfg[ANEMOMETER].numberOfElements	= 1;

	/* modbus read configuration for wind vane sensor */
	arSlaveRdVarCfg[WIND_VANE].enable				= true;
	arSlaveRdVarCfg[WIND_VANE].firstElementAdr		= 0x00;
	arSlaveRdVarCfg[WIND_VANE].numberOfElements		= 1;

	/* modbus write configuration for thermobarometer sensor */
	arSlaveWrVarCfg[BME280_TEMP_HUM].enable				= true;
	arSlaveWrVarCfg[BME280_TEMP_HUM].firstElementAdr	= 0xC8;
	arSlaveWrVarCfg[BME280_TEMP_HUM].numberOfElements	= 6;

	/* define callback functions for modbus master */
	Master_RTU.preTransmission(setTxMode);
	Master_RTU.postTransmission(setRxMode);

	/* init serial at modbus speed */
	Serial.begin(MODBUS_SPEED);
	#ifdef SERIAL_PRINT
	Serial.print(F("Start modbus/serial at "));
	Serial.print(MODBUS_SPEED);
	Serial.println(F(" bit/s"));
	Serial.println();
	#endif

	/* software info */
	#ifdef SERIAL_PRINT
	Serial.println(F("MAIN WEATER STATION BOARD"));
	Serial.print(F("Software version: "));
	Serial.println(softwareVersion);
	Serial.println();
	#endif

	/* wait for the modbus slaves to be operational */
	delay(2500);

	/* board setup is completed */
	doRunState = HIGH;
}

/* the loop function runs over and over again until power down or reset */
void loop()
{
	/* START LOOP: get millis value at loop begin */
	millisAtLoopBegin = millis();

	/* read digital input */

	#pragma region SCHEDULER
	/* execution of scheduled task T1 */
		if (((taskCounter[T1_TASK] == 0) || (abs(millis() - taskCounter[T1_TASK]) >= T1_TASK_TIME)) && !serialBusy)
		{
			/* reset counter and call function for BME280_TEMP_HUM */
			taskCounter[T1_TASK] = millis();
			modbusRequest(BME280_TEMP_HUM);
		}
		/* execution of scheduled task T2 */
		else if (((taskCounter[T2_TASK] == 0) || (abs(millis() - taskCounter[T2_TASK]) >= T2_TASK_TIME)) && !serialBusy)
		{
			/* reset counter and call function for ANEMOMETER */
			taskCounter[T2_TASK] = millis();
			modbusRequest(ANEMOMETER);
		}
		/* execution of scheduled task T3 */
		else if (((taskCounter[T3_TASK] == 0) || (abs(millis() - taskCounter[T3_TASK]) >= T3_TASK_TIME)) && !serialBusy)
		{
			/* reset counter and call function for WIND_VANE */
			taskCounter[T3_TASK] = millis();
			modbusRequest(WIND_VANE);
		}
	#pragma endregion

	/* write digital output */
	digitalWrite(RUN_LED, doRunState);

	/* END LOOP: if current cycle is shorter than task time, wait! */
	if (abs(millis() - millisAtLoopBegin) < LOOP_TIME)
	{
		while (abs(millis() - millisAtLoopBegin) <= LOOP_TIME);
	}
}

/* function modbusRequest is executed every time the scheduling time has elapsed */
void modbusRequest(uint8_t device)
{
	/* if reading function is enabled, call read function for holding registers */
	if (arSlaveRdVarCfg[device].enable)
	{
		/* start current node */
		Master_RTU.begin(modbusNode[device], Serial);

		/* clear buffer to avoid errors */
		Master_RTU.clearResponseBuffer();

		/* declare serial as busy */
		serialBusy = true;

		/* get result of reading request.. */
		uint8_t result = Master_RTU.readHoldingRegisters(arSlaveRdVarCfg[device].firstElementAdr, arSlaveRdVarCfg[device].numberOfElements);

		/* ..and print it on serial monitor */
		#ifdef SERIAL_PRINT
		Serial.print(F("Request made to the node "));
		Serial.print(modbusNode[device]);
		Serial.print(F(". Result is "));
		Serial.println(getModbusErrorInfo(result));
		Serial.print(F("Millis time: "));
		Serial.println(millis() / 1000.0F);
		Serial.println();
		#endif
		
		/* ..and release serial for next com */
		serialBusy = false;

		/* manages the modbus function for the device type required */
		switch (device)
		{
			/* get all the values ​​related to the BME280 sensor */
			case BME280_TEMP_HUM:
				/* write info about connection status */
				stBME280Modbus.connectionStatus = result;
				/* BME280 response is OK */
				if (result == Master_RTU.ku8MBSuccess)
				{
					/* compile structured variable.. */
					stBME280Modbus.actualTemperature	= Master_RTU.getResponseBuffer(ACTUAL_TEMPERATURE);
					stBME280Modbus.actualPressure		= Master_RTU.getResponseBuffer(ACTUAL_PRESSURE);
					stBME280Modbus.actualHumidity		= Master_RTU.getResponseBuffer(ACTUAL_HUMIDITY);
					stBME280Modbus.wetBulbTemperature	= Master_RTU.getResponseBuffer(WET_BULB_TEMPERATURE);
					stBME280Modbus.dewPoint				= Master_RTU.getResponseBuffer(DEW_POINT);
					stBME280Modbus.heatIndex			= Master_RTU.getResponseBuffer(HEAT_INDEX);
					stBME280Modbus.absHumidity			= Master_RTU.getResponseBuffer(ABS_HUMIDITY);
					stBME280Modbus.statusBME280			= Master_RTU.getResponseBuffer(BME280_STATUS);
					/* ..print data on serial monitor.. */
					BME280PrintData();
				}
			break;

			/* get all the values ​​related to the anemometer sensor */
			case ANEMOMETER:
				/* write info about connection status */
				stAnemometer.connectionStatus = result;
				/* anemometer response is OK */
				if (result == Master_RTU.ku8MBSuccess)
				{
					/* compile structured variable.. */
					stAnemometer.actualWindSpeed = Master_RTU.getResponseBuffer(ACTUAL_WIND_SPEED);
					/* ..print data on serial monitor.. */
					anemometerPrintData();
				}
			break;

			/* get all the values ​​related to the wind vane sensor */
			case WIND_VANE:
				/* write info about connection status */
				stWindVane.connectionStatus = result;
				/* wind vane response is OK */
				if (result == Master_RTU.ku8MBSuccess)
				{
					/* compile structured variable.. */
					stWindVane.actualWindDirection = Master_RTU.getResponseBuffer(ACTUAL_WIND_DIRECTION);
					/* ..print data on serial monitor.. */
					windVanePrintData();
				}
			break;
		}
	}
}

/* callback function used to set MAX485 on TX mode */
void setTxMode()
{
	digitalWrite(RE_DE_PIN, TX_MODE);
}

/* callback function used to set MAX485 on RX mode */
void setRxMode()
{
	digitalWrite(RE_DE_PIN, RX_MODE);
}

/* BME280 Modbus print data function */
void BME280PrintData()
{
	/* print data on serial monitor  */
#ifdef SERIAL_PRINT
	Serial.print(F("Dry temperature.... "));
	Serial.print(stBME280Modbus.actualTemperature / 10.0F);
	Serial.println(F(" °C"));

	Serial.print(F("Pressure........... "));
	Serial.print(stBME280Modbus.actualPressure / 10.0F);
	Serial.println(F(" hPa"));

	Serial.print(F("Humidity........... "));
	Serial.print(stBME280Modbus.actualHumidity / 10.0F);
	Serial.println(F(" %"));

	Serial.print(F("Wet temperature.... "));
	Serial.print(stBME280Modbus.wetBulbTemperature / 10.0F);
	Serial.println(F(" °C"));

	Serial.print(F("DewPoint........... "));
	Serial.print(stBME280Modbus.dewPoint / 10.0F);
	Serial.println(F(" °C"));

	Serial.print(F("HeatIndex.......... "));
	Serial.print(stBME280Modbus.heatIndex / 10.0F);
	Serial.println(F(" °C"));

	Serial.print(F("AbsoluteHumidity... "));
	Serial.print(stBME280Modbus.absHumidity / 10.0F);
	Serial.println(F(" %"));

	Serial.println();
#endif
}

/* anemometer print data function */
void anemometerPrintData()
{
	/* print data on serial monitor  */
#ifdef SERIAL_PRINT
	Serial.print(F("Actual wind speed.. "));
	Serial.print(stAnemometer.actualWindSpeed / 10.0F);
	Serial.println(F(" m/s"));

	Serial.println();
#endif
}

/* anemometer print data function */
void windVanePrintData()
{
	/* print data on serial monitor  */
#ifdef SERIAL_PRINT
	Serial.print(F("Wind direction..... "));
	Serial.print(stWindVane.actualWindDirection);
	Serial.println(F(" enum"));

	Serial.println();
#endif
}

/* get a string about modbus error */
char* getModbusErrorInfo(uint8_t result)
{
		 if (result == Master_RTU.ku8MBSuccess)				return "0x00: ku8MBSuccess";
	else if (result == Master_RTU.ku8MBIllegalFunction)		return "0x01: ku8MBIllegalFunction";
	else if (result == Master_RTU.ku8MBIllegalDataAddress)	return "0x02: ku8MBIllegalDataAddress";
	else if (result == Master_RTU.ku8MBIllegalDataValue)	return "0x03: ku8MBIllegalDataValue";
	else if (result == Master_RTU.ku8MBSlaveDeviceFailure)	return "0x04: ku8MBSlaveDeviceFailure";
	else if (result == Master_RTU.ku8MBInvalidSlaveID)		return "0xE0: ku8MBInvalidSlaveID";
	else if (result == Master_RTU.ku8MBInvalidFunction)		return "0xE1: ku8MBInvalidFunction";
	else if (result == Master_RTU.ku8MBResponseTimedOut)	return "0xE2: ku8MBResponseTimedOut";
	else if (result == Master_RTU.ku8MBInvalidCRC)			return "0xE3: ku8MBInvalidCRC";
}