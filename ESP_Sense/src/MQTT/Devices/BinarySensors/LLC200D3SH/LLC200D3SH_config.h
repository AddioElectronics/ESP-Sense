#pragma once

#include <Arduino.h>

#include "../../../../Config/config_master.h"
#include "../../../../Config/global_status.h"
#include "../../../../Config/config_mqtt.h"
#include "../MqttBinarySensor_Config.h"

#define LLC200D3SH_CONFIG_NAME					"llc200d3sh.json"
#define LLC200D3SH_CONFIG_PATH					"/mqtt/devices/binarysensors/" LLC200D3SH_CONFIG_NAME
#define LLC200D3SH_CONFIG_OBJECT_KEY			"llc200d3sh"
#define LLC200D3SH_WEBSITE_PATH					"/www/mqtt/devices/binarysensors/llc200d3sh.html"

#if defined(ESP8266)
#define LLC200D3SH_GPIO							1		//Node MCU ESP-12 (D3)
#elif defined(ESP32)
#define LLC200D3SH_GPIO							-1
#endif

#define LLC200D3SH_WATER_PRESENT_VOLTAGE		LOW
#define LLC200D3SH_INVERT_READING				false	//Should the reading be inverted?

#define TOPIC_SENSOR_LLC200D3SH_BASE			"/waterlevel"

//#define TOPIC_SENSOR_LLC200D3SH_AVAILABILITY					TOPIC_BASE TOPIC_BASE_MQTT_SENSOR TOPIC_SENSOR_LLC200D3SH_BASE TOPIC_BASE_AVAILABILITY
//#define TOPIC_SENSOR_LLC200D3SH_JSON_COMMAND					TOPIC_BASE TOPIC_BASE_MQTT_SENSOR TOPIC_SENSOR_LLC200D3SH_BASE TOPIC_BASE_JSON_COMMAND
//#define TOPIC_SENSOR_LLC200D3SH_JSON_STATE						TOPIC_BASE TOPIC_BASE_MQTT_SENSOR TOPIC_SENSOR_LLC200D3SH_BASE TOPIC_BASE_JSON_STATE

typedef struct {
	struct {
		bool useDefaults : 1;
		bool waterPresent : 1;		//Voltage at which the water is present.
		bool invert : 1;			//Invert state before sending via MQTT. *Going to remove either waterPresent or invert.
		uint8_t reserved : 5;
		uint8_t gpio;
	}program;
}Llc200d3sh_Config_t;

typedef struct {
	struct {
		bool useDefaults : 1;
		bool gpio : 1;
		bool waterPresent : 1;
		bool invert : 1;
		uint8_t reserved : 4;
	}program;
}Llc200d3sh_ConfigMonitor_bm;