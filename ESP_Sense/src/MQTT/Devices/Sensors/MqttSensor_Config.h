#pragma once

#include "../../../Config/config_mqtt.h"
#include "../MqttDevice_Config.h"

#define TOPIC_BASE_MQTT_SENSOR					"/sensors"
//#define TOPIC_BASE_MQTT_SENSOR_AVAILABILITY		TOPIC_BASE_MQTT_DEVICE_AVAILABILITY	
//#define TOPIC_BASE_MQTT_SENSOR_JSON_COMMAND		TOPIC_BASE_MQTT_DEVICE_JSON_COMMAND								
//#define TOPIC_BASE_MQTT_SENSOR_JSON_STATE		TOPIC_BASE_MQTT_DEVICE_JSON_STATE												
//#define TOPIC_BASE_MQTT_SENSOR_COMMAND			TOPIC_BASE_MQTT_DEVICE_COMMAND
//#define TOPIC_BASE_MQTT_SENSOR_STATE			TOPIC_BASE_MQTT_DEVICE_STATE

#define MQTT_SENSOR_MAX_FAILED_READS	5			//Amount of failed reads before it sends not connected payload.

//Payloads
#define SENSOR_DATA_UNKNOWN				"???"		//If a sensor stops being able to read its measurement, it will send this string as the payload.