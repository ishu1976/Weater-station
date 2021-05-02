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
	char softwareVersion[]		= "2104.07";				// software version
	uint8_t device				= BME280_TEMP_HUM;
	bool serialBusy				= false;

	/* rain gauge global var */
	uint8_t overturning			= LOW;						// it represents the status TRUE of the reed switch inside rain gauge
	uint16_t overturningCnt		= 0;						// is represents the number of overturning of the rain gauges

	/* input / output connected var */
	uint8_t doRunState			= LOW;						// represents the state of GREEN led on sensor board
	uint8_t diRainGaugeSwitch	= LOW;						// represents the state of rain gouge switch

	/* structured var */
	BME280ModbusData BME280Modbus;							// it contains the data read by the BME280 modbus sensor
	AnemometerData Anemometer;								// it contains the data read by the anemometer together with other data about wind velocity
	WindVaneData WindVane;									// it contains the data read from the wind vane together with other data about wind direction

	/* enumerates  */
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

/* the setup function runs once when you press reset or power the board */
void setup()
{
	/* define i/o mode */
	pinMode(RUN_LED, OUTPUT);
	pinMode(RAIN_GAUGE_SWITCH, INPUT);
	pinMode(RE_DE_PIN, OUTPUT);

	/* modbus read configuration for thermobarometer sensor */
	slaveCfg[BME280_TEMP_HUM].slaveId	= 0x01;
	slaveCfg[BME280_TEMP_HUM].readAddr	= 0x64;
	slaveCfg[BME280_TEMP_HUM].readQty	= 8;
	slaveCfg[BME280_TEMP_HUM].writeAddr = 0xCb;
	slaveCfg[BME280_TEMP_HUM].writeQty	= 6;

	/* modbus read configuration for anemometer sensor */
	slaveCfg[ANEMOMETER].slaveId	= 0x02;
	slaveCfg[ANEMOMETER].readAddr	= 0x00;
	slaveCfg[ANEMOMETER].readQty	= 1;
	slaveCfg[ANEMOMETER].writeAddr	= 0x00;
	slaveCfg[ANEMOMETER].writeQty	= 0;

	/* modbus read configuration for wind vane sensor */
	slaveCfg[WIND_VANE].slaveId		= 0x03;
	slaveCfg[WIND_VANE].readAddr	= 0x00;
	slaveCfg[WIND_VANE].readQty		= 1;
	slaveCfg[WIND_VANE].writeAddr	= 0x00;
	slaveCfg[WIND_VANE].writeQty	= 0;

	/* define callback functions for modbus master */
	Master_RTU.preTransmission(setTxMode);
	Master_RTU.postTransmission(setRxMode);
	Master_RTU.idle(overturningCounter);

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
	
	/* counter of the number of overturning */
	overturningCounter();

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
		/* flush serial to avoid unpredictable behavior */
		Serial.flush();
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
	/* if reading function is enabled (qty is greater than 0), call read function for holding registers */
	if (slaveCfg[device].readQty > 0)
	{
		/* start current node */
		Master_RTU.begin(slaveCfg[device].slaveId, Serial);

		/* clear buffer to avoid errors */
		Master_RTU.clearResponseBuffer();
		
		/* declare serial as busy */
		serialBusy = true;

		/* get result of reading request.. */
		uint8_t result = Master_RTU.readHoldingRegisters(slaveCfg[device].readAddr, slaveCfg[device].readQty);

		/* ..and print it on serial monitor */
#ifdef SERIAL_PRINT
		Serial.print(F("Millis time: "));
		Serial.print(millis() / 1000.0F);
		Serial.print(F("sec. | Request made slave Id nr."));
		Serial.print(slaveCfg[device].slaveId);
		Serial.print(F(" | Result is "));
		Serial.print(getModbusErrorInfo(result));
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
				BME280Modbus.connectionStatus = result;
				/* BME280 response is OK */
				if (result == MB_SUCCESS)
				{
					/* compile structured variable.. */
					BME280Modbus.actualTemperature	= Master_RTU.getResponseBuffer(ACTUAL_TEMPERATURE);
					BME280Modbus.actualPressure		= Master_RTU.getResponseBuffer(ACTUAL_PRESSURE);
					BME280Modbus.actualHumidity		= Master_RTU.getResponseBuffer(ACTUAL_HUMIDITY);
					BME280Modbus.wetBulbTemperature	= Master_RTU.getResponseBuffer(WET_BULB_TEMPERATURE);
					BME280Modbus.dewPoint			= Master_RTU.getResponseBuffer(DEW_POINT);
					BME280Modbus.heatIndex			= Master_RTU.getResponseBuffer(HEAT_INDEX);
					BME280Modbus.absHumidity		= Master_RTU.getResponseBuffer(ABS_HUMIDITY);
					BME280Modbus.statusBME280		= Master_RTU.getResponseBuffer(BME280_STATUS);
				}
				break;

			/* get all the values ​​related to the anemometer sensor */
			case ANEMOMETER:
				/* write info about connection status */
				Anemometer.connectionStatus = result;
				/* anemometer response is OK */
				if (result == MB_SUCCESS)
				{
					/* compile structured variable.. */
					Anemometer.actualWindSpeed = Master_RTU.getResponseBuffer(ACTUAL_WIND_SPEED);
				}
				break;

			/* get all the values ​​related to the wind vane sensor */
			case WIND_VANE:
				/* write info about connection status */
				WindVane.connectionStatus = result;
				/* wind vane response is OK */
				if (result == MB_SUCCESS)
				{
					/* compile structured variable.. */
					WindVane.actualWindDirection = Master_RTU.getResponseBuffer(ACTUAL_WIND_DIRECTION);
				}
				else
				{
					/* declare value as invalid */
					WindVane.actualWindDirection = NOT_VALID;
				}
				break;
		}
		/* print data on serial monitor */
#ifdef SERIAL_PRINT
		printData(device);
#endif
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
void printData(uint8_t device)
{
	/* print on serial monitor all the data of the selected device */
	switch (device)
	{
		/* print all the values ​​related to the BME280 sensor */
		case BME280_TEMP_HUM:
			Serial.print(F("Dry temperature.... "));
			Serial.print(BME280Modbus.actualTemperature / 10.0F);
			Serial.println(F(" °C"));

			Serial.print(F("Pressure........... "));
			Serial.print(BME280Modbus.actualPressure / 10.0F);
			Serial.println(F(" hPa"));

			Serial.print(F("Humidity........... "));
			Serial.print(BME280Modbus.actualHumidity / 10.0F);
			Serial.println(F(" %"));

			Serial.print(F("Wet temperature.... "));
			Serial.print(BME280Modbus.wetBulbTemperature / 10.0F);
			Serial.println(F(" °C"));

			Serial.print(F("DewPoint........... "));
			Serial.print(BME280Modbus.dewPoint / 10.0F);
			Serial.println(F(" °C"));

			Serial.print(F("HeatIndex.......... "));
			Serial.print(BME280Modbus.heatIndex / 10.0F);
			Serial.println(F(" °C"));

			Serial.print(F("AbsoluteHumidity... "));
			Serial.print(BME280Modbus.absHumidity / 10.0F);
			Serial.println(F(" %"));
			break;

		/* print all the values ​​related to the anemometer sensor */
		case ANEMOMETER:
			Serial.print(F("Actual wind speed.. "));
			Serial.print(Anemometer.actualWindSpeed / 10.0F);
			Serial.println(F(" m/s"));
			break;

		/* print all the values ​​related to the wind vane sensor */
		case WIND_VANE:
			Serial.print(F("Wind direction..... <"));
			Serial.print(getActualWindDirection(WindVane.actualWindDirection));
			Serial.println(F(">"));
			break;
	}
	/* print empty line */
	Serial.println();
}

/* get a string about modbus error */
char* getModbusErrorInfo(uint8_t result)
{
		 if (result == MB_SUCCESS)					return "0x00: MB_SUCCESS";
	else if (result == MB_ILLEGAL_FUNCTION)			return "0x01: MB_ILLEGAL_FUNCTION";
	else if (result == MB_ILLEGAL_DATA_ADDRESS)		return "0x02: MB_ILLEGAL_DATA_ADDRESS";
	else if (result == MB_ILLEGAL_DATA_VALUE)		return "0x03: MB_ILLEGAL_DATA_VALUE";
	else if (result == MB_SLAVE_DEVICE_FAILURE)		return "0x04: MB_SLAVE_DEVICE_FAILURE";
	else if (result == MB_INVALID_SLAVE_ID)			return "0xE0: MB_INVALID_SLAVE_ID";
	else if (result == MB_INVALID_FUNCTION)			return "0xE1: MB_INVALID_FUNCTION";
	else if (result == MB_RESPONSE_TIMED_OUT)		return "0xE2: MB_RESPONSE_TIMED_OUT";
	else if (result == MB_INVALID_CRC)				return "0xE3: MB_INVALID_CRC";
}

/* get a string about wind direction */
char* getActualWindDirection(uint8_t windDirection)
{
		 if (windDirection == NORD)				return "NORD";
	else if (windDirection == NORD_NORD_EST)	return "NORD_NORD_EST";
	else if (windDirection == NORD_EST)			return "NORD_EST";
	else if (windDirection == EST_NORD_EST)		return "EST_NORD_EST";
	else if (windDirection == EST)				return "EST";
	else if (windDirection == EST_SUD_EST)		return "EST_SUD_EST";
	else if (windDirection == SUD_EST)			return "SUD_EST";
	else if (windDirection == SUD_SUD_EST)		return "SUD_SUD_EST";
	else if (windDirection == SUD)				return "SUD";
	else if (windDirection == SUD_SUD_WEST)		return "SUD_SUD_WEST";
	else if (windDirection == SUD_WEST)			return "SUD_WEST";
	else if (windDirection == WEST_SUD_WEST)	return "WEST_SUD_WEST";
	else if (windDirection == WEST)				return "WEST";
	else if (windDirection == OVEST_NORD_WEST)	return "OVEST_NORD_WEST";
	else if (windDirection == NORD_WEST)		return "NORD_WEST";
	else if (windDirection == NORD_NORD_WEST)	return "NORD_NORD_WEST";
	else if (windDirection == NOT_VALID)		return "NOT_VALID";
}

/* this function counts the number of buckets overturnig */
void overturningCounter()
{
	/* read digital input */
	diRainGaugeSwitch = digitalRead(RAIN_GAUGE_SWITCH);

	/* if overturning is HIGH, the overturning is running */
	if ((overturning == HIGH) && (diRainGaugeSwitch == LOW))
	{
		overturning = LOW;
		overturningCnt++;
#ifdef SERIAL_PRINT
		Serial.print(F("Rain gauge buckets overturning: "));
		Serial.println(overturningCnt);
		Serial.println();
#endif
	}
	else if (diRainGaugeSwitch == HIGH)
	{
		overturning = HIGH;
	}
}