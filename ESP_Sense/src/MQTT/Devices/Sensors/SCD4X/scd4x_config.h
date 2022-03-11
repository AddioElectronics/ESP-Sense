#pragma once

#include <Arduino.h>

#include <SensirionCore.h>
#include <SensirionI2CScd4x.h>

#include "../../../../Config/config_master.h"
#include "../../../../Config/config_mqtt.h"
#include "../MqttSensor_Config.h"

enum class SCD4x_PowerMode { PM_STANDARD = 0, PM_LOW = 1 };

#pragma region Config

#define SCD4X_CONFIG_NAME					"scd4x.json"
#define SCD4X_CONFIG_PATH					"/mqtt/devices/sensors/" SCD4X_CONFIG_NAME
#define SCD4X_CONFIG_OBJECT_KEY				"scd4x"
#define SCD4X_WEBSITE_PATH					"/www/mqtt/devices/sensors/scd4x.html"

//Internal
#define SCD4X_ALTITUDE						0					//Elevation from sea level. Required for accurate Co2 measurements.		
#define SCD4X_AMBIENTPRESSURE				0
#define SCD4X_TEMPOFFSET					0
#define SCD4X_RETAININTERNALLY				false
#define SCD4X_SELFCALIBRATE					false

//Program 
#define SCD4X_PERIODICMEASUREMENTS			true
#define SCD4X_SELFTEST_ON_BOOT				false
#define SCD4X_CHECK_DATA_READY				false
#define SCD4X_READRATE_ADDITIONAL			0
#define SCD4X_POWERMODE						SCD4x_PowerMode::PM_STANDARD
#define SCD4X_MAX_FAILED_READS				10					//The maximum read attempts in single-shot mode before a new measurement command is issued. (Singleshot mode only)
#define SCD4X_I2C_PORT						0

//MQTT
#define SCD4X_PUBLISH_CO2					true
#define SCD4X_PUBLISH_TEMPERATURE			true
#define SCD4X_PUBLISH_HUMIDITY				true

////Mqtt
//#define SCD4X_AS_JSON						true
//#define SCD4X_USE_PARENTS					false
//
////Device
//#define SCD4X_DEVICE_USEGLOBALCONFIG		true


#define SCD4X_LAST_11_BITS					0x07FF/*0xFFFE0*/	//Documenation isn't clear what they mean by last 11 bits. That could mean either or depending on CPU. From what I read they mean smallest 11 bits.

#pragma endregion

#pragma region MQTT Topics

#define TOPIC_SENSOR_SCD4X_BASE							"/scd4x"

#define TOPIC_SENSOR_SCD4X_CO2_BASE						"/co2"
#define TOPIC_SENSOR_SCD4X_TEMP_BASE					"/temperature"
#define TOPIC_SENSOR_SCD4X_HUMIDITY_BASE				"/humidity"

//#define TOPIC_SENSOR_SCD4X_AVAILABILITY					TOPIC_BASE TOPIC_BASE_MQTT_SENSOR TOPIC_SENSOR_SCD4X_BASE TOPIC_BASE_AVAILABILITY
//#define TOPIC_SENSOR_SCD4X_JSON_COMMAND					TOPIC_BASE TOPIC_BASE_MQTT_SENSOR TOPIC_SENSOR_SCD4X_BASE TOPIC_BASE_JSON_COMMAND
//#define TOPIC_SENSOR_SCD4X_JSON_STATE					TOPIC_BASE TOPIC_BASE_MQTT_SENSOR TOPIC_SENSOR_SCD4X_BASE TOPIC_BASE_JSON_STATE

//#define TOPIC_SENSOR_SCD41_ENABLED_SET				TOPIC_BASE "/sensor/scd41/enabled/set"						//From HA	
//#define TOPIC_SENSOR_SCD41_ENABLED_GET				TOPIC_BASE "/sensor/scd41/enabled/get"						//To HA	
//#define TOPIC_SENSOR_SCD41_AVAILABILITY				TOPIC_BASE "/sensor/scd41/availability"						//To HA		
//#define TOPIC_SENSOR_SCD41_TEMPERATURE				TOPIC_BASE "/sensor/scd41/temperature" 						//To HA
//#define TOPIC_SENSOR_SCD41_HUMIDITY					TOPIC_BASE "/sensor/scd41/humidity"  						//To HA
//#define TOPIC_SENSOR_SCD41_CO2						TOPIC_BASE "/sensor/scd41/co2"								//To HA
//#define TOPIC_SENSOR_SCD41_ALTITUDE_SET				TOPIC_BASE "/sensor/scd41/settings/altitude/set"			//From HA		
//#define TOPIC_SENSOR_SCD41_ALTITUDE_GET				TOPIC_BASE "/sensor/scd41/settings/altitude/get"			//To HA	
//#define TOPIC_SENSOR_SCD41_TEMPOFFSET_SET			TOPIC_BASE "/sensor/scd41/settings/temperature/offset/set"	//From HA	
//#define TOPIC_SENSOR_SCD41_TEMPOFFSET_GET			TOPIC_BASE "/sensor/scd41/settings/temperature/offset/get"	//To HA	
//#define TOPIC_SENSOR_SCD41_AMBIENTPRESSURE_SET		TOPIC_BASE "/sensor/scd41/settings/ambient_pressure/set"	//From HA		
//#define TOPIC_SENSOR_SCD41_AMBIENTPRESSURE_GET		TOPIC_BASE "/sensor/scd41/settings/ambient_pressure/get"	//To HA	
//#define TOPIC_SENSOR_SCD41_PERIODICMEASURE_SET		TOPIC_BASE "/sensor/scd41/settings/periodMeasure/set"		//From HA	
//#define TOPIC_SENSOR_SCD41_PERIODICMEASURE_GET		TOPIC_BASE "/sensor/scd41/settings/periodMeasure/get"		//To HA	
//#define TOPIC_SENSOR_SCD41_FACTORYRESET				TOPIC_BASE "/sensor/scd41/settings/factoryReset"			//From HA		
//#define TOPIC_SENSOR_SCD41_SELFTEST_START			TOPIC_BASE "/sensor/scd41/selfTest/start"					//From HA	
//#define TOPIC_SENSOR_SCD41_SELFTEST_RESULTS			TOPIC_BASE "/sensor/scd41/selfTest/results"					//To HA	
//#define TOPIC_SENSOR_SCD41_FORCECALIBRATION_START	TOPIC_BASE "/sensor/scd41/calibration/start"				//From HA
//#define TOPIC_SENSOR_SCD41_FORCECALIBRATION_RESULTS	TOPIC_BASE "/sensor/scd41/calibration/results"				//To HA	

#pragma endregion

enum class Scd4x_Command_Type
{
	SCD4X_START_PERIODIC_MEASUREMENT = 0x21b1,
	SCD4X_READ_MEASUREMENT = 0xec05,
	SCD4X_STOP_PERIODIC_MEASUREMENT = 0x3f86,
	SCD4X_SET_TEMPERATURE_OFFSET = 0x241d,
	SCD4X_GET_TEMPERATURE_OFFSET = 0x2318,
	SCD4X_SET_SENSOR_ALTITUDE = 0x2427,
	SCD4X_GET_SENSOR_ALTITUDE = 0x2322,
	SCD4X_SET_AMBIENT_PRESSURE = 0xe000,
	SCD4X_PERFORM_FORCED_CALIBRATION = 0x362f,
	SCD4X_SET_AUTOMATIC_SELF_CALIBRATION_ENABLED = 0x2416,
	SCD4X_GET_AUTOMATIC_SELF_CALIBRATION_ENABLED = 0x2313,
	SCD4X_START_LOW_POWER_PERIODIC_MEASUREMENT = 0x21ac,
	SCD4X_GET_DATA_READY_STATUS = 0xe4b8,
	SCD4X_PERSIST_SETTINGS = 0x3615,
	SCD4X_GET_SERIAL_NUMBER = 0x3682,
	SCD4X_PERFORM_SELF_TEST = 0x3639,
	SCD4X_PERFORM_FACTORY_RESET = 0x3632,
	SCD4X_REINIT = 0x3646,
	SCD4X_MEASURE_SINGLE_SHOT = 0x219d,
	SCD4X_MEASURE_SINGLE_SHOT_RHT_ONLY = 0x2196
};

#pragma region Structures

typedef struct {
	uint16_t serial0;
	uint16_t serial1;
	uint16_t serial2;
}SCD4xSerialNumber_t;

typedef struct {
	struct {
		bool useDefaults : 1;
		bool retainSettings : 1;			//Store settings on the SCD4x's internal EEPROM.
		bool selfCalibrate : 1;
		uint8_t reserved : 5;
		uint16_t altitude;
		uint16_t ambientPressure;
		float tempOffset;
	}internal;
	struct {
		bool useDefaults : 1;
		bool periodicMeasure : 1;
		bool bootSelfTest : 1;
		bool checkDataReady : 1;
		SCD4x_PowerMode powerMode : 2;
		uint8_t i2cPort : 1;			//Which I2C port to use (Wire = 0, Wire1 = 1)
		uint8_t reserved : 1;
		uint8_t readRateAdditional;		//How many extra seconds to add to the minimum read rate.
		uint16_t maxFailedReads;		//The maximum read attempts in single-shot mode before a new measurement command is issued. (Singleshot mode only)
#warning reminder to put maxFailedReads into other sensor or deviceConfig.
	}program;
	struct {
		bool publishCo2 : 1;
		bool publishTemperature : 1;
		bool publishHumdiity : 1;
	}mqtt;
}Scd4xConfig_t;

typedef union {
	struct {
		struct {
			bool useDefaults : 1;
			bool retainSettings : 1;
			bool selfCalibrate : 1;
			bool altitude : 1;
			bool ambientPressure : 1;
			bool tempOffset : 1;
			uint8_t reserved : 2;
		}internal;
		struct {
			bool useDefaults : 1;
			bool periodicMeasure : 1;
			bool bootSelfTest : 1;
			bool powerMode : 1;
			bool readRateAdditional : 1;
			bool maxFailedReads : 1;
			bool i2cPort : 1;
			bool checkDataReady : 1;
		}program;
		struct {
			bool publishCo2 : 1;
			bool publishTemperature : 1;
			bool publishHumdiity : 1;
			uint8_t reserved : 5;
		}mqtt;
	};
	uint32_t bitmap;
}Scd4xConfigMonitor_bm;


typedef struct
{
	bool measurementStarted : 1;
	bool periodicallyMeasuring : 1;
	bool dataReady : 1;
	bool skipNextMeasurement : 1;
	//bool usingGlobalConfig : 1;			//Using settings from config_scd4x.json
	bool performingSelfTest : 1;
	bool performingFactoryReset : 1;
	bool performingCalibration : 1;
	bool reenable : 1;
	bool retainedSettingsInitialized : 1;
	bool standardOperationDisabled : 1;	//Standard operation is disabled like reading and publishing. (Used for when performing a special action)
	SCD4x_PowerMode powerMode : 1;
	//uint8_t failedReads;				//How many reads have failed sequentially.
	uint16_t error;
	uint16_t selfTestResults;
}SCD4xStatus_t;

///// <summary>
///// Status that is Preserved.
///// </summary>
//typedef struct {
//	bool configuredBeforeRetain : 1;
//	uint8_t reserved : 7;
//}SCD4xPreservedStatus_t;

//typedef struct {
//	String co2State;
//	const char* co2Availability;
//	String temperatureState;
//	const char* temperatureAvailability;
//	String humidityState;
//	const char* humidityAvailability;
//}SCD4xSpecialTopics_t;

//typedef struct {
//	String co2;
//	String temperature;
//	String humidity;
//}SCD4xSpecialBaseTopics_t;



#pragma endregion

