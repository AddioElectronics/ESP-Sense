#pragma once

#include <Arduino.h>
#include <PubSubClient.h>
#include "Config/config_master.h"
#include "Config/global_status.h"
#include "Config/config_mqtt.h"

#if defined(ESP8266)
#include <LittleFS.h>
#elif defined(ESP32)
#include <LITTLEFS.h>
#endif

extern Config_t config;
extern ConfigMonitor_t configMonitor;
extern GlobalStatus_t status;
extern ConfigBitmap_bm configBitmap;
extern StatusRetained_t statusRetained;
extern StatusRetainedMonitor_t statusRetainedMonitor;
extern PubSubClient mqttClient;

#define STRING_CONCAT(a, b)		a##b	

//Reset now handled in EspSense::RestartDevice()
#if defined(ESP8266)
#define ESP_FS			LittleFS
//#define ESP_RESTART()	ESP.reset()
#elif defined(ESP32)
#define ESP_FS			LITTLEFS
//#define ESP_RESTART()	ESP.restart()
#endif

#define SERIAL_PORT(n)			STRING_CONCAT(Serial, n)

extern HardwareSerial* serial;
extern HardwareSerial* serialDebug;

//#if ((SERIAL_ENABLED && SERIAL_PORT != 1)  || (SERIAL1_ENABLED))
#define SERIAL_PRINT(s)			(*serial).print(s)
#define SERIAL_PRINTLN(s)		(*serial).println(s)
#define SERIAL_PRINTF(s, ...)	(*serial).printf(s, ##__VA_ARGS__)
#define SERIAL_WRITE(b)			(*serial).write(b)
#define SERIAL_WRITE_L(b, l)	(*serial).write(b, l)
//#else
//#define SERIAL_PRINT(s)			
//#define SERIAL_PRINTLN(s)		
//#define SERIAL_PRINTF(s, ...)	
//#define SERIAL_WRITE(b)			
//#define SERIAL_WRITE_L(b, l)	
//#endif

#if SHOW_DEBUG_MESSAGES/* && ((SERIAL_ENABLED && SERIAL_PORT != 1)  || (SERIAL1_ENABLED))*/
#define DEBUG_LOG(s)			(*serialDebug).print(s)
#define DEBUG_LOG_LN(s)			(*serialDebug).println(s)
#define DEBUG_LOG_F(s, ...)		(*serialDebug).printf(s, ##__VA_ARGS__)
#define DEBUG_WRITE(b)			(*serialDebug).write(b)
#define DEBUG_WRITE_L(b, l)		(*serialDebug).write(b, l)
#define DEBUG_NEWLINE()			(*serialDebug).println()
#define DEBUG_DOTS()			(*serialDebug).println("...")
#else
#define DEBUG_LOG(s)
#define DEBUG_LOG_LN(s)
#define DEBUG_LOG_F(s, ...)
#define DEBUG_WRITE(b)
#define DEBUG_WRITE_L(b, l)
#define DEBUG_NEWLINE()
#define DEBUG_DOTS()
#endif

//Print and send message to MQTT simultaniously
#define MULTI_LOG_LN(s)	DEBUG_LOG_LN(s); \
						if(status.mqtt.connected) mqttClient.publish(TOPIC_DEBUG, s)

#define MULTI_LOG(s)	DEBUG_LOG(s); \
						if(status.mqtt.connected) mqttClient.publish(TOPIC_DEBUG, s)