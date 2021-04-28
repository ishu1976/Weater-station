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

/* structure definitions */
struct ST_ModbusRdWrCfg
{	/* structure used to define a modbus read/write function */
	bool enable					= false;
	uint8_t firstElementAdr		= 0x00;
	uint8_t numberOfElements	= 0x00;
};
struct ST_BME280ModbusData
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
struct ST_AnemometerData
{	/* structure used to define all parameters read from anemometer */
	word connectionStatus;
	word actualWindSpeed;
};
struct ST_WindVaneData
{	/* structure used to define all parameters read from wind vane */
	word connectionStatus;
	word actualWindDirection;
};
#pragma region MODBUS MASTER RTU SECTION
	/* modbus configuration */
	#define MODBUS_SPEED			9600
	#define RE_DE_PIN				3			// Arduino pin connected to MAX485 (pin RE/DE)

	/* class definitions */
	ModbusMaster Master_RTU;					// Class for modbus RTU menagement

	/* define IDs of the slave nodes as devices (used as array pointer) */
	#define BME280_TEMP_HUM		0
	#define ANEMOMETER			1
	#define WIND_VANE			2

	/* define slave node address */
	#define BME280_TEMP_HUM_NODE	1
	#define ANEMOMETER_NODE			2
	#define WIND_VANE_NODE			3

	/* synonyms for logic */
	#define	TX_MODE					HIGH
	#define	RX_MODE					LOW

	/* init array with modbus node defines */
	uint8_t modbusNode[] = { BME280_TEMP_HUM_NODE, ANEMOMETER_NODE, WIND_VANE_NODE };

	/* define structured var for read holding register (0x03) function */
	ST_ModbusRdWrCfg arSlaveRdVarCfg[3];

	/* define structured var for write holding register (0x10) function */
	ST_ModbusRdWrCfg arSlaveWrVarCfg[3];

	/* modbus configuration for BME280 modbus */
	#define ACTUAL_TEMPERATURE		0			// register 100, actual dry bulb temperature read by sensor BME280
	#define ACTUAL_PRESSURE			1			// register 101, actual barometric pressure read by sensor BME280
	#define ACTUAL_HUMIDITY			2			// register 102, actual humidity read by sensor BME280
	#define WET_BULB_TEMPERATURE	3			// register 103, wet bulb temperature - calculated
	#define DEW_POINT				4			// register 104, dew point temperature - calculated
	#define HEAT_INDEX				5			// register 105, heat index temperature - calculated
	#define ABS_HUMIDITY			6			// register 106, absolute humidity - calculated
	#define BME280_STATUS			7			// register 107, status of running program on the board
	/* modbus configuration for anemometer */
	#define ACTUAL_WIND_SPEED		0			// register 0, actual wind speed (value 0 to 324 --> 32.4 m/s)
	/* modbus configuration for wind vane */
	#define ACTUAL_WIND_DIRECTION	0			// register 0, actual wind direction (value 0 to 15)
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
