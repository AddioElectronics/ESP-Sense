#pragma once

#include <Arduino.h>

//#include <Arduino.h>
//
//#include <StreamUtils.hpp>
//#include <StreamUtils.h>											
//#include <LittleFS.h>					//File system for loading config files.
//#include <ArduinoJson.hpp>				//Used for deserializing config files, and de/serializing data for/from Home Assistant
//#include <ArduinoJson.h>
//#include <ESP_EEPROM.h>					//For saving a backup of settings. (Improved version of EEPROM.h)
#include <List.hpp>
//
//
//#include <ESP8266WiFi.h>
//#include <ESPAsyncTCP.h>
//#include <ESPAsyncWebServer.h>
//#include <PubSubClient.h>				//MQTT Library
//#include <ArduinoOTA.h>					//Wifi re-flashing
//#include <FTPServer.h>
//#include <FTPCommon.h>
//
//
//#include <Wire.h>						//I2C
//#include <Adafruit_Sensor.h>
//#include <Adafruit_I2CDevice.h>
//#include <Adafruit_SHT4x.h>
//#include <SensirionCore.h>
//#include <SensirionI2CScd4x.h>

#include "src/Config/config_master.h"

//////////////////////////////////////////////////////
//					Function Prototypes
//////////////////////////////////////////////////////

namespace EspSense
{ 
	//void ConfigLoop();			//Config mode loop

	//void Initialize();
	//void Reinitialize();
	void BlankStructures();
	void InitializeSerial();
	void ConfigureGpio();
	bool InitializeI2C();
	void ApplySerialSettings(HardwareSerial& port, SerialPortConfig_t& portConfig, SerialPortConfigMonitor_t& portConfigMonitor);
	bool LoadRetainedStatus();
	void FreshBoot();
	bool MountEEPROM();
	
	void RestartDevice();
	void EnterUpdateMode(UpdateMode mode);
	void StopAllTasks();
	void EnableWatchdog();
	void DisableWatchdog();
	void FeedWatchdog();
	void YieldWatchdog(uint32_t _delay = 0);

	void DisplayMemoryInfo();
	size_t GetStackSize();
	size_t GetStackSize(uint8_t* start);
}

