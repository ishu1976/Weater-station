/*
 Name:			ModbusCfg.h
 Created:		25/04/2021
 Author:		Andrea Santinelli
 Description:	Header file for modbus configuration

 This header file defines the modbus configuration which is, for design reasons, divided into 2 parts:
 1). Modbus MASTER RTU for data exchange with the sensors of the weather station
 2). Modbus SLAVE TCP / IP for data exchange with the home PLC where the data is processed and sent to a display for visualizations

*/

#ifndef _ModbusCfg_h
#define _ModbusCfg_h

/* dependencies */
#include <ModbusMaster.h>
#include <EtherCard.h>
#include <Modbus.h>
#include <ModbusIP_ENC28J60.h>

#pragma region STRUCTURE DEFINITIONS
struct ModbusSlaveConfig
{	/* structure used to define a modbus read/write function */
	uint8_t slaveId		= 0;
	uint8_t readAddr	= 0;
	uint8_t readQty		= 0;
	uint8_t writeAddr	= 0;
	uint8_t writeQty	= 0;
};
struct BME280ModbusData
{	/* structure used to define all parameters read from BME280 */
	word connectionStatus;
	word actualTemperature;
	word actualPressure;
	word actualHumidity;
	word wetBulbTemperature;
	word dewPoint;
	word heatIndex;
	word absHumidity;
	word statusBME280;
};
struct AnemometerData
{	/* structure used to define all parameters read from anemometer */
	word connectionStatus;
	word actualWindSpeed;
};
struct WindVaneData
{	/* structure used to define all parameters read from wind vane */
	word connectionStatus;
	word actualWindDirection;
};
#pragma endregion

#pragma region MODBUS MASTER RTU SECTION
	/* modbus configuration */
	#define MODBUS_SPEED			9600
	#define RE_DE_PIN				3			// Arduino pin connected to MAX485 (pin RE/DE)

#pragma region ENUMERATED
	/* modbus response enumeration (synonyms in capital letters of ModbusMaster library response) */
	enum modbusResponse
	{
		MB_SUCCESS					= 0x00,
		MB_ILLEGAL_FUNCTION			= 0x01,
		MB_ILLEGAL_DATA_ADDRESS		= 0x02,
		MB_ILLEGAL_DATA_VALUE		= 0x03,
		MB_SLAVE_DEVICE_FAILURE		= 0x04,
		MB_INVALID_SLAVE_ID			= 0xE0,
		MB_INVALID_FUNCTION			= 0xE1,
		MB_RESPONSE_TIMED_OUT		= 0xE2,
		MB_INVALID_CRC				= 0xE3
	};
	/* define slave index (used as array pointer) */
	enum slaveList
	{
		BME280_TEMP_HUM,
		ANEMOMETER,
		WIND_VANE,	
		TOTAL_NR_OF_SLAVES // leave this last entry!!
	};
	/* synonyms for logic */
	enum serialControl
	{
		TX_MODE = HIGH,
		RX_MODE = LOW
	};
	/* modbus configuration read registers for BME280 modbus */
	enum BME280_HR_READ
	{
		ACTUAL_TEMPERATURE,		// register 100: actual dry bulb temperature read by sensor BME280
		ACTUAL_PRESSURE,		// register 101: actual barometric pressure read by sensor BME280
		ACTUAL_HUMIDITY,		// register 102: actual humidity read by sensor BME280
		WET_BULB_TEMPERATURE,	// register 103: wet bulb temperature - calculated
		DEW_POINT,				// register 104: dew point temperature - calculated
		HEAT_INDEX,				// register 105: heat index temperature - calculated
		ABS_HUMIDITY,			// register 106: absolute humidity - calculated
		BME280_STATUS			// register 107: status of running program on the board
	};
	/* modbus configuration read registers for anemometer */
	enum ANEMOMETER_HR_READ
	{
		ACTUAL_WIND_SPEED		// register 0: actual wind speed (value 0 to xyz --> xy.z m/s)
	};
	/* modbus configuration read registers for wind vane */
	enum WIND_VANE_HR_READ
	{
		ACTUAL_WIND_DIRECTION	// register 0: actual wind direction (value 0 to 15)
	};
#pragma endregion

	/* define structured var for slave configuration */
	ModbusSlaveConfig slaveCfg[TOTAL_NR_OF_SLAVES];

	/* class definitions */
	ModbusMaster Master_RTU;					// Class for modbus RTU menagement
#pragma endregion

#pragma region MODBUS SLAVE TCP/IP SECTION
	/* class definitions */
	ModbusIP Slave_TCP_IP;						// Class for modbus TCP/IP menagement

	/* ENC28J60 Ethernet card configuration */
	uint8_t ethMacAddress[] = { 0xDE, 0xAD, 0xBE, 0xEB, 0xDA, 0xED };
	uint8_t ethIpAddress[]	= { 192, 168, 178, 17 };
	uint8_t ethSubnet[]		= { 255, 255, 255, 0 };
	uint8_t ethDns[]		= { 8, 8, 4, 4 };			// set Google DNS
	uint8_t ethGateway[]	= { 192, 168, 178, 254 };	// This is the IP address of gatweay (es. Fritz! router in my home network)
#pragma endregion

#endif