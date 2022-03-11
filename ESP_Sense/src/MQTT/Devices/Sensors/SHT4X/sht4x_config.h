#pragma once

#include <Arduino.h>

#include "../../../../Config/config_master.h"
#include "../../../../Config/config_mqtt.h"
#include "../MqttSensor_Config.h"

#define SHT4X_CONFIG_NAME							"sht4x.json"
#define SHT4X_CONFIG_PATH							"/mqtt/devices/sensors/" SHT4X_CONFIG_NAME
#define SHT4X_CONFIG_OBJECT_KEY						"sht4x"
#define SHT4X_WEBSITE_PATH							"/www/mqtt/devices/sensors/sht4x.html"

//Internal
#define SENSOR_SHT40_PRECISION						SHT4X_HIGH_PRECISION
#define SENSOR_SHT40_HEATER							SHT4X_NO_HEATER

//MQTT
#define SHT4X_PUBLISH_TEMPERATURE					true
#define SHT4X_PUBLISH_HUMIDITY						true

////Mqtt
//#define SHT4X_AS_JSON						true
//#define SHT4X_USE_PARENTS					false
//		 
////Device 
//#define SHT4X_DEVICE_USEGLOBALCONFIG		true

#define TOPIC_SENSOR_SHT4X_BASE							"/sht4x"
					  
//#define TOPIC_SENSOR_SHT4X_AVAILABILITY					TOPIC_BASE TOPIC_BASE_MQTT_SENSOR TOPIC_SENSOR_SHT4X_BASE TOPIC_BASE_AVAILABILITY
//#define TOPIC_SENSOR_SHT4X_JSON_COMMAND					TOPIC_BASE TOPIC_BASE_MQTT_SENSOR TOPIC_SENSOR_SHT4X_BASE TOPIC_BASE_JSON_COMMAND
//#define TOPIC_SENSOR_SHT4X_JSON_STATE					TOPIC_BASE TOPIC_BASE_MQTT_SENSOR TOPIC_SENSOR_SHT4X_BASE TOPIC_BASE_JSON_STATE

//#define TOPIC_SENSOR_SHT40_ENABLED_SET				TOPIC_BASE "/sensor/sht40/enabled/set"						//From HA	
//#define TOPIC_SENSOR_SHT40_ENABLED_GET				TOPIC_BASE "/sensor/sht40/enabled/get"						//To HA	
//#define TOPIC_SENSOR_SHT40_AVAILABILITY				TOPIC_BASE "/sensor/sht40/availability"						//To HA
//#define TOPIC_SENSOR_SHT40_TEMPERATURE				TOPIC_BASE "/sensor/sht40/temperature"						//To HA
//#define TOPIC_SENSOR_SHT40_HUMIDITY					TOPIC_BASE "/sensor/sht40/humidity"							//To HA
//#define TOPIC_SENSOR_SHT40_PRECISION_SET			TOPIC_BASE "/sensor/sht40/precision/set"					//From HA
//#define TOPIC_SENSOR_SHT40_PRECISION_GET			TOPIC_BASE "/sensor/sht40/precision/get"					//To HA
//#define TOPIC_SENSOR_SHT40_HEATER_SET				TOPIC_BASE "/sensor/sht40/settings/heater/set"				//From HA
//#define TOPIC_SENSOR_SHT40_HEATER_GET				TOPIC_BASE "/sensor/sht40/settings/heater/get"				//To HA

typedef struct {
	struct {
		bool useDefaults : 1;
		sht4x_heater_t heater : 3;
		sht4x_precision_t precision : 2;
		uint8_t reserved : 2;
	}internal;
	struct {
		bool publishTemperature : 1;
		bool publishHumdiity : 1;
		uint8_t reserved : 6;
	}mqtt;
}Sht4xConfig_t;

typedef union {
	struct {
		bool useDefaults : 1;
		bool heater : 1;
		bool precision : 1;
		uint8_t reserved : 5;
	}internal;
	struct {
		bool publishTemperature : 1;
		bool publishHumdiity : 1;
		uint8_t reserved : 6;
	}mqtt;
	uint16_t bitmap;
}Sht4xConfigMonitor_bm;