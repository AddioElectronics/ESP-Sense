/*
	All functions, variables, files and other names are subject to change. 
	Lots of functions are uncommented, and unorganized, they will be fixed in the future.
*/

#define ESP_SENSE_VERSION	0,1,2




#if !defined(ESP8266) && !defined(ESP32)
#error Only currently supporting the ESP8266 and ESP32 WROOM. Newer versions of the ESP32 have not been tested.
#endif

#if defined(ESP8266)
#error ESP8266 is not finished.
#endif

//////////////////////////////////////////////////////
//					Includes
//////////////////////////////////////////////////////

#include <Arduino.h>

#include "src/Config/config_master.h"
#include "src/Config/global_status.h"
#include "src/Config/config_udfs.h"

#include <StreamUtils.hpp>
#include <StreamUtils.h>											
#include <ArduinoJson.hpp>				//Used for deserializing config files, and de/serializing data for/from Home Assistant
#include <ArduinoJson.h>
#include <EEPROM.h>						//For saving a backup of settings. (Improved version of EEPROM.h)
#include <CRC32.h>
#include <PubSubClient.h>				//MQTT Library
#include <ESP32Ping.h>
#include <List.hpp>

#if defined(ESP8266)
#include <Schedule.h>
#include <user_interface.h>
#include <Hash.h>
#include <HardwareSerial.h>
#include <LittleFS.h>					//File system for loading config files.
#include <ESP8266WiFi.h>
#if COMPILE_SERVER
#include <ESPAsyncTCP.h>
#include <ESP8266mDNS.h>
#include <user_interface.h>
#endif	/*COMPILE_SERVER*/
#elif defined(ESP32)
#include <soc/rtc_wdt.h>
#include <esp_task_wdt.h>
#include <esp32-hal-uart.h>
#include <WiFi.h>
#include <FS.h>
#include <LITTLEFS.h>
#if COMPILE_SERVER
#include <WiFiUdp.h>
#include <ESPmDNS.h>
#include <AsyncTCP.h>
#include <Update.h>
#endif	/*COMPILE_SERVER*/
#endif	/*defined(ESP8266) else defined(ESP32)*/

#if COMPILE_OTA
#include <ArduinoOTA.h>
#endif

#if COMPILE_SERVER
#include <ESPAsyncWebServer.h>
#endif	/*COMPILE_SERVER*/

#if COMPILE_FTP
#define DEBUG_ESP_PORT Serial1
#include <FTPServer.h>
#include <FTPCommon.h>
#include "src/Network/Server/FtpManager.h"
#endif	/*COMPILE_FTP*/


#include <Wire.h>						//I2C
#include <Adafruit_Sensor.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_SHT4x.h>
#include <SensirionCore.h>
#include <SensirionI2CScd4x.h>


#include "ESP_Sense.h"
#include "src/FileManager.h"
#include "src/EspSenseValidation.h"
#include "src/Config/ConfigManager.h"
#include "src/Network/WifiManager.h"

#include "src/Config/config_mqtt.h"
#include "src/MQTT/MqttManager.h"
#include "src/MQTT/MqttHelper.h"
#include "src/MQTT/MqttTopicManager.h"
#include "src/MQTT/Devices/MqttDeviceManager.h"
#include "src/MQTT/Devices/MqttDeviceWeb.h"

#include "src/Network/Server/ServerManager.h"
#include "src/Network/Server/Authentication.h"
#include "src/Network/OtaManager.h"
#include "src/Network/Website/WebsiteManager.h"
#include "src/Network/Website/WebConfigBrowser.h"
#include "src/Network/Website/WebConsole.h"
#include "src/Network/Website/WebpageServer.h"
#include "src/Network/Website/WebUpdater.h"
#include "src/Network/Website/Tools/WebFileEditor.h"
#include "src/Network/Website/Tools/WebJsonValidator.h"

#include "src/HelperFunctions.h"
#include "src/JsonHelper.h"
#include "src/macros.h"
#include "src/CrcStream.h"




//////////////////////////////////////////////////////
//					Objects/Variables
//////////////////////////////////////////////////////

Config_t config;
ConfigMonitor_t configMonitor;
GlobalStatus_t status;
ConfigBitmap_bm configBitmap;

extern PubSubClient mqttClient;
extern Mqtt::MqttMessagesSent_t mqttMessagesSent;
extern FTPServer ftpServer;

StatusRetained_t statusRetained;
StatusRetainedMonitor_t statusRetainedMonitor;


HardwareSerial& Serial0 = Serial;			//Reference Serial as Serial0

HardwareSerial* serial = nullptr;			//Message
HardwareSerial* serialDebug = nullptr;		//Debug

unsigned long nextAliveMessage;
void* stackStart;

//#if defined(ESP8266)
//extern void serialEventRun(void) __attribute__((weak));
//#endif




//////////////////////////////////////////////////////
//					Functions
//////////////////////////////////////////////////////

//void BootDelay()
//{
//	DEBUG_LOG_LN("Booting in...");
//	for (int8_t i = BOOT_DELAY; i >= 0; i--)
//	{
//		DEBUG_LOG_LN(i);
//		delay(1000);
//	}
//
//}

void MessageDelay(uint8_t seconds, const char* message)
{
	DEBUG_LOG_LN(message);
	for (int8_t i = seconds; i >= 0; i--)
	{
		DEBUG_LOG_LN(i);
		delay(1000);
	}

}

/// <summary>
/// Device initialization.
/// Called once on boot.
/// </summary>
void setup() 
{
	uint8_t stack;
	stackStart = &stack;

	EspSense::InitializeSerial();

	DEBUG_NEWLINE();
	DEBUG_NEWLINE();
	DEBUG_NEWLINE();

	//BootDelay();	//Delay to give time to open the Serial port.
	MessageDelay(5, "Booting in...");

	//Set Structures to 0 as memory is not guaranteed to blank on reset.
	EspSense::BlankStructures();

	//Set the Version
	status.misc.version = {ESP_SENSE_VERSION};

	DEBUG_LOG_F("Version %d.%d.%d\r\n", status.misc.version.major, status.misc.version.minor, status.misc.version.revision);

	//Initialize settings to default
	Config::Defaults::SetAll();

	EspSense::MountEEPROM();
	FileManager::MountFileSystem();

#if DEVELOPER_MODE

	uint32_t configFileCrc = FileManager::GetFileCRC("/config.json");

	DEBUG_LOG_F("Config File CRC : 0x%0000000X\r\n", configFileCrc);

#endif

	EspSense::LoadRetainedStatus();

	//Load configuration files.
	Config::Documents::LoadBootSettings();

	Config::Documents::LoadConfiguration();
	Config::Documents::SetConfigFromDoc();

	//Apply settings from Config to Serial ports.
	if (configMonitor.device.serial.port0.bitmap)
		EspSense::ApplySerialSettings(Serial0, config.device.serial.port0, configMonitor.device.serial.port0);

	if (configMonitor.device.serial.port1.bitmap)
		EspSense::ApplySerialSettings(Serial1, config.device.serial.port1, configMonitor.device.serial.port1);

	EspSense::ConfigureGpio();

	if (config.device.i2c.enabled)
		EspSense::InitializeI2C();

	WifiManager::Initialize();
	WifiManager::Connect();


	//Serial cuts out after initializing wifi, give enough time to allow serial to be reconnected.
	MessageDelay(5, "Waiting for wifi connections to stabilize...");
	//delay(5000);

	//Start Hosting Website, FTP, OTA and other network modules.
	Network::Server::Initialize();

	//Stop blinking to show that the server is up and running.
	WifiManager::Tasks::StopBlinkTask();

	//Initialize Core 1 loop which handles server requests.
	status.device.tasks.enabled = true;
	WifiManager::Tasks::StartWifiLoopTask();

	if (status.wifi.configMode)
	{
		//goto Label_SkipMqtt;

		Config::Backup::DisableBackups();
		goto Label_SkipBackup;	//Skip MQTT and auto-backup.
	}

	/*Initialize MQTT*/
	Mqtt::Initialize();

	while (!status.mqtt.connected)
	{
		Mqtt::DisplayMessageIntervalMet();
		Mqtt::Connect();

		//EspSense::YieldWatchdog(250);

		//Usually retained status is saved at end of setup, but it may not get there so save now.
		if (status.mqtt.connectAttempts > FORCE_SAVE_RETAINED_AFTER && statusRetainedMonitor.bitmap != 0 && !status.config.saveRetainedLoop)
			Config::Status::SaveRetainedStatus();

		if (status.device.tasks.wifiTaskRunning)
		{
			EspSense::YieldWatchdog(status.mqtt.ipStatus.autoDetectingStation ? 250 : config.mqtt.broker.connectInterval);
		}
		else
		{
			EspSense::YieldWatchdog(500);
			WifiManager::Loop();
		}
	}


	//Configure, connect and enable MQTT devices.
	Mqtt::DeviceManager::ConfigureDevices();

	////Subscribe to ESP-Sense topics
	//Mqtt::Subscribe();

	////Enable all devices that are initially enabled.
	//Mqtt::DeviceManager::EnableAll(true);

	//If wifi and MQTT is connected, we have required data.
	status.config.hasRequiredData = true;

	Label_SkipMqtt:

#if COMPILE_BACKUP
	//Save backup of configuration.
	//Will only save if Auto backup mode is enabled.
	Config::Backup::AutoSaveBackupConfig(false);
#endif

Label_SkipBackup:

	//if (!status.config.testingConfig)
	//	Config::Status::SetRetainedConfigPath();


	//Apply changes to EEPROM.
	Config::Status::SaveRetainedStatus();

	
	//status.mqtt.publishingEnabled = true;

	////Loop task only used for configuration.
	//TaskManager::StopLoopTask();

	//Start MQTT tasks
	if (!status.wifi.configMode)
	{
		//Subscribe to ESP-Sense topics
		Mqtt::Subscribe();

		//Enable all devices that are initially enabled.
		Mqtt::DeviceManager::EnableAll(true);

		Mqtt::Tasks::StartAllTasks();
	}

	status.config.setupComplete = true;

	DEBUG_LOG_LN("Device Initialized\r\n\r\n");

	/*EspSense::Initialize();*/
}

//void EspSense::Initialize()
//{
//	
//}

//void EspSense::Reinitialize()
//{
//	return; //Not implemented yet.
//
//	Mqtt::DeviceManager::UnconfigureDevices();
//	Mqtt::Unsubscribe();
//
//	EspSense::Initialize();
//}

/// <summary>
/// The main program loop.
/// </summary>
void loop() 
{
	if (!status.device.tasks.wifiTaskRunning)
	{
		WifiManager::Loop();
	}

	if (!status.wifi.configMode)
		Mqtt::Loop();

	//if(status.device.tasks.wifiTaskRunning /*&& config.device.taskSettings.enabled*/)
	//	Mqtt::Loop();
	//else
	//	WifiManager::Loop();

	bool longYield = status.device.tasks.wifiTaskRunning && (status.mqtt.connected || status.wifi.configMode);

	EspSense::YieldWatchdog(longYield ? 5000 : 1);

#if COMPILE_ALIVE_MSG
	if (millis() > nextAliveMessage)
	{
		nextAliveMessage = millis() + 10000;
		DEBUG_LOG_LN("Loop Alive");
		EspSense::DisplayMemoryInfo();
		DEBUG_NEWLINE();
	}
#endif
}



void EspSense::BlankStructures()
{
	memset(&status, 0, sizeof(GlobalStatus_t));
	memset(&configMonitor, 0, sizeof(ConfigMonitor_t));
	memset(&statusRetained, 0, sizeof(StatusRetained_t));
	memset(&statusRetainedMonitor, 0, sizeof(StatusRetainedMonitor_t));

	configBitmap.bitmap = 0xFF;
}

void EspSense::InitializeSerial()
{
	/*Initialize Serial Communication*/
//Serial.begin(SERIAL_BAUDRATE);
//Serial1.begin(SERIAL1_BAUDRATE, SERIAL_8N1, GPIO_SERIAL1_RX, GPIO_SERIAL1_TX);

#if defined(ESP8266)

#elif defined(ESP32)
//#if SERIAL0_ENABLED	
	//Serial must always be enabled as libraries and internal debugging functions use it.
	Serial.begin(SERIAL0_BAUDRATE, SERIAL0_CONFIG, -1, -1, false, SERIAL0_TIMEOUT);
	//Serial.begin(SERIAL0_BAUDRATE, SERIAL0_CONFIG, GPIO_SERIAL0_RX, GPIO_SERIAL0_TX, SERIAL0_INVERT, SERIAL0_TIMEOUT);
	//RegisterGpio(GPIO_SERIAL0_RX);
	//RegisterGpio(GPIO_SERIAL0_TX);
//#endif

#if SERIAL1_ENABLED
	Serial1.begin(SERIAL1_BAUDRATE, SERIAL_8N1, GPIO_SERIAL1_RX, GPIO_SERIAL1_TX, SERIAL1_INVERT, SERIAL1_TIMEOUT);
	//RegisterGpio(GPIO_SERIAL1_RX);
	//RegisterGpio(GPIO_SERIAL1_TX);
#endif
#endif

	//Assign a serial port to the pointers.
	serial = &SERIAL_PORT(SERIAL_MESSAGE_PORT);
	serialDebug = &SERIAL_PORT(SERIAL_DEBUG_PORT);

	while (!Serial)
		delay(100);

	#if SERIAL1_ENABLED
		while (!Serial1)
			delay(100);
	#endif
}

void EspSense::ConfigureGpio()
{

	//Wifi Connection LED
	if (IsGpioValidPin(config.wifi.station.ledGpio, GPIOV_OUTPUT))
	{
		pinMode(config.wifi.station.ledGpio, OUTPUT);
		digitalWrite(config.wifi.station.ledGpio, !config.wifi.station.ledOn);
		DEBUG_LOG_F("Wifi LED - GPIO : %d - On Level : %d\r\n", config.wifi.station.ledGpio, config.wifi.station.ledOn);
	}
	else
	{
		config.wifi.station.ledGpio = -1;
		DEBUG_LOG_LN("Wifi Connection LED GPIO Invalid!");
	}

	//MQTT Connection LED
	if (IsGpioValidPin(config.mqtt.ledGpio, GPIOV_OUTPUT))
	{
		pinMode(config.mqtt.ledGpio, OUTPUT);
		digitalWrite(config.mqtt.ledGpio, !config.mqtt.ledOn);
		DEBUG_LOG_F("MQTT LED - GPIO : %d - On Level : %d\r\n", config.mqtt.ledGpio, config.mqtt.ledOn);
	}
	else
	{
		config.mqtt.ledGpio = -1;
		DEBUG_LOG_LN("MQTT Connection LED GPIO Invalid!");
	}

#if COMPILE_ACCESSPOINT

	//Wifi Hotspot Connection LED
	if (config.wifi.accessPoint.ledGpio)
	{
		if (IsGpioValidPin(config.wifi.accessPoint.ledGpio, GpioPull_t::GPIO_NO_PULL, true))
		{
			pinMode(config.wifi.accessPoint.ledGpio, OUTPUT);
			digitalWrite(config.wifi.station.ledGpio, !config.wifi.station.ledOn);
			DEBUG_LOG_F("Wifi AP LED - GPIO : %d - On State : %d\r\n", config.wifi.accessPoint.ledGpio, config.wifi.accessPoint.ledOn);
		}
		else
		{
			config.wifi.accessPoint.ledGpio = -1;
			DEBUG_LOG_LN("Wifi Hotspot LED GPIO Invalid!");
		}
	}

	//Configure Wifi Hotspot button and LED
	// 
	//Button
	if (IsGpioValidPin(config.wifi.accessPoint.buttonGpio, (config.wifi.accessPoint.buttonPullup ? GpioPull_t::GPIO_PULL_UP : GpioPull_t::GPIO_NO_PULL), false))
	{
		pinMode(config.wifi.accessPoint.buttonGpio, (config.wifi.accessPoint.buttonPullup ? INPUT_PULLUP : INPUT));
		DEBUG_LOG_F("Wifi AP Button - GPIO : %d - Pressed : %d - Pull-up : %d\r\n", config.wifi.accessPoint.buttonGpio, config.wifi.accessPoint.buttonPress, config.wifi.accessPoint.buttonPullup);
	}

	
#endif
}

bool EspSense::InitializeI2C()
{
	DEBUG_LOG_LN("Initializing I2C.");

	if (status.device.i2cInitialized)
	{
		DEBUG_LOG_LN("...I2C Already Initialized.");
		return false;
	}

	uint32_t freq = I2C_FREQUENCY;

	if (config.device.i2c.freq != I2C_FREQUENCY)
	{
		if (IsValidI2cFreq(config.device.i2c.freq))
		{
			freq = config.device.i2c.freq;
			DEBUG_LOG_F("...Setting I2C Frequency to %d\r\n", config.device.i2c.freq);
			//Wire.setClock(config.device.i2c.freq);
		}
		else
		{
			DEBUG_LOG_LN("...Cannot set I2C frequency. Frequency Invalid....");
		}
	}

	status.device.i2cInitialized = Wire.begin(config.device.i2c.sdaGpio, config.device.i2c.sclGpio, freq);

	if (!status.device.i2cInitialized)
	{
		DEBUG_LOG_LN("...I2C Failed to Initialize!");
		return false;
	}
	DEBUG_NEWLINE();
}

void EspSense::ApplySerialSettings(HardwareSerial& port, SerialPortConfig_t& portConfig, SerialPortConfigMonitor_t& portConfigMonitor)
{
	bool isSerial0 = &port == &Serial;
	if (!portConfig.useDefaults)
	{
		DEBUG_LOG_F("Applying Serial%d Settings | Baud : %d\r\nRX GPIO : %d\r\nTX GPIO : %d\r\nTimeout : %d\r\nConfig : %000000X\r\n", 0, portConfig.baud, portConfig.rxGpio, portConfig.txGpio, portConfig.timeout, portConfig.config);

		if (IsBaudrateValid(portConfig.baud) && (isSerial0 || IsGpioValidPin(portConfig.rxGpio, GPIOV_UART, !portConfigMonitor.rxGpio)) && (isSerial0 || IsGpioValidPin(portConfig.txGpio, GPIOV_UART, !portConfigMonitor.txGpio)))
		{
			//RegisterGpio(portConfig.rxGpio);
			//RegisterGpio(portConfig.txGpio);

			port.end();

			//Serial0 is only semi-configurable
			if (isSerial0)
				port.begin(portConfig.baud, portConfig.config, -1, -1, false, portConfig.timeout);
			else
				port.begin(portConfig.baud, portConfig.config, portConfig.rxGpio, portConfig.txGpio, portConfig.invert, portConfig.timeout);

			DEBUG_LOG_LN("Serial Settings Applied.");
		}
		else
		{
			DEBUG_LOG_LN("Cannot Apply Serial Settings.");
		}
	}
	else
	{
		//No settings to apply.
	}
}

bool EspSense::LoadRetainedStatus()
{

	if (!status.device.eepromMounted)
	{
		DEBUG_LOG_LN("Cannot load Retained Status : EEPROM not mounted!");
		return status.device.retainedStatusLoaded = false;
	}

	//Get boot settings.
	EEPROM.get(0, statusRetained);

	if (statusRetained.boot.freshBoot || statusRetained.statusRetainedSize != sizeof(StatusRetained_t))
		FreshBoot();


	statusRetainedMonitor.boot.configMode = statusRetained.boot.configMode != false;

//#if COMPILE_BACKUP
//	status.config.backup.eepromBackedUp = status.config.backup.ableToBackupEeprom && statusRetained.crcs.eepromBackupFile != 0;
//	status.config.backup.filesystemBackedUp = statusRetained.crcs.fileSystemBackupFile != 0;
//#endif

	return status.device.retainedStatusLoaded = true;
}


/// <summary>
/// If the internal is booting for the first time we need to do a little configuration.
/// </summary>
void EspSense::FreshBoot()
{
	DEBUG_LOG_LN("Fresh Boot, saving default settings to EEPROM...");

	memset(&statusRetained, 0, sizeof(StatusRetained_t));
	statusRetained.statusRetainedSize = sizeof(StatusRetained_t);

	EEPROM.put(0, statusRetained);
	status.device.freshBoot = true;				//Change retained settings to false, but program still needs to know.
	EEPROM.commit();
}

bool EspSense::MountEEPROM()
{
	DEBUG_LOG_LN("Initializing EEPROM...");

#if DOC_CONFIG_SERIALIZE_SIZE + 64 < 4096
	status.device.eepromMounted = EEPROM.begin(sizeof(StatusRetained_t) + DOC_CONFIG_SERIALIZE_SIZE);
#if COMPILE_BACKUP
	status.config.backup.ableToBackupEeprom = status.device.eepromMounted;
#endif
#else
	status.device.eepromMounted = EEPROM.begin(sizeof(DeviceRetainedStatus_t));
	status.config.backup.ableToBackupEeprom = false;
#endif

	if (!status.device.eepromMounted)
	{
		DEBUG_LOG_LN("EEPROM Failed to Mount!");
	}

	return status.device.eepromMounted;
}



void EspSense::RestartDevice()
{
	DEBUG_LOG_LN("Restarting...");


	if (status.server.updating != UpdateMode::ESP_UPDATE_NULL)
	{
		DEBUG_LOG_LN("Cannot restart, update in progress.");
		return;
	}

//#if defined(ESP8266)
//	ESP.reset();
//#elif defined(ESP32)
//	ESP.restart();
//#endif

	EspSense::StopAllTasks();
	EspSense::EnableWatchdog();
	uint8_t doNothing;
	while (true)
	{
		//Let watchdog fire.
		doNothing++;
	}
}

void EspSense::EnterUpdateMode(UpdateMode mode)
{
	Mqtt::Disconnect();

	Network::FTP::Stop();
	Network::Website::ConfigBrowser::Disable();
	Network::Website::Tools::FileEditor::Disable();
	Network::Website::Tools::JsonValidator::Disable();

	switch (mode)
	{
	case UpdateMode::ESP_UPDATE_WEB:
		Network::OTA::Disable();
		break;

	case UpdateMode::ESP_UPDATE_OTA:
		Network::Website::WebUpdate::Disable();
		break;
	}

}

void EspSense::StopAllTasks()
{
	WifiManager::Tasks::StopWifiLoopTask();
	WifiManager::Tasks::StopBlinkTask();
	Mqtt::Tasks::StopLoopTask();
	Mqtt::Tasks::StopBlinkTask();
	Mqtt::Tasks::StopDeviceManagerLoopTask();
	Mqtt::Tasks::StopPublishAvailabilityTask();
}

void EspSense::EnableWatchdog()
{
#if defined(ESP8266)
	system_soft_wdt_stop();
#elif defined(ESP32)
	rtc_wdt_enable();
#endif
	enableLoopWDT();
	enableCore0WDT();
	//enableCore1WDT();
}

void EspSense::DisableWatchdog()
{
#if defined(ESP8266)
	system_soft_wdt_restart();
#elif defined(ESP32)
	rtc_wdt_disable();
#endif
	disableLoopWDT();
	disableCore0WDT();
	//disableCore1WDT();	
}

void EspSense::FeedWatchdog()
{
#if defined(ESP8266)
	system_soft_wdt_feed();
#elif defined(ESP32)
	esp_task_wdt_reset();
	rtc_wdt_feed();
#endif
	feedLoopWDT();
}

void EspSense::YieldWatchdog(uint32_t _delay)
{
	if(_delay)
		delay(_delay);

#if defined(ESP8266)
	system_soft_wdt_feed();
#elif defined(ESP32)
	esp_task_wdt_reset();
	rtc_wdt_feed();
#endif
	feedLoopWDT();
	yield();
}

void EspSense::DisplayMemoryInfo()
{
	size_t heapFree = ESP.getFreeHeap();
	size_t stackSize = GetStackSize();

	DEBUG_LOG_F("-Displaying Memory Info-\r\n-Free Heap : %d\r\n-Stack Size : %d\r\n", heapFree, stackSize);

}

size_t EspSense::GetStackSize()
{
	return GetStackSize((uint8_t*)stackStart);
}

size_t EspSense::GetStackSize(uint8_t* start)
{
	uint8_t stack;
	return (uint32_t)&stack - (uint32_t)start;
}
