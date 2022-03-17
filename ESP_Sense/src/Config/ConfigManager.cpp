#include "ConfigManager.h"


#include <StreamUtils.hpp>
#include <StreamUtils.h>	

#if defined(ESP8266)
#include <LittleFS.h>	
#elif defined(ESP32)
#include <FS.h>
#include <LITTLEFS.h>
#endif

#include <EEPROM.h>
#include <CRC32.h>

#include "../../ESP_Sense.h"
#include "../MQTT/MqttManager.h"
#include "../Macros.h"
#include "../FileManager.h"
#include "../EspSenseValidation.h"
#include "../HelperFunctions.h"
#include "../JsonHelper.h"

extern Config_t config;
extern ConfigMonitor_t configMonitor;
extern GlobalStatus_t status;
extern StatusRetained_t statusRetained;
extern StatusRetainedMonitor_t statusRetainedMonitor;
extern ConfigBitmap_bm configBitmap;

CRC32 crc;


/// <summary>
/// Document used for serializing and deserializing config.json.
/// *API recommends to not use a global document, 
/// but it must be better to create one than to create and free many, right?
/// </summary>
DynamicJsonDocument configDoc(DOC_CONFIG_DESERIALIZE_SIZE);



#pragma region Defaults




///// <summary>
///// Current path to the config.
///// </summary>
//char configPath[CONFIG_PATH_MAX_LENGTH] = FILE_CONFIG_PATH;
//
///// <summary>
///// Current file name of the config.
///// </summary>
//char configName[CONFIG_PATH_MAX_LENGTH / 2] = FILE_CONFIG_NAME;

void Config::Defaults::SetAll(bool flagMonitor)
{
	DEBUG_LOG_LN("Changing settings to default.");

	InitializeStrings();

	if (configBitmap.device)
		RestoreDeviceDefaults(flagMonitor);

	if (configBitmap.wifi)
		RestoreWifiDefaults(flagMonitor);

	if (configBitmap.mqtt)
		RestoreMqttDefaults(flagMonitor);
	//RestoreDocDefaults();
#if COMPILE_FTP
	if (configBitmap.ftp)
		RestoreFtpDefaults(flagMonitor);
#endif

#if COMPILE_SERVER
	if (configBitmap.server)
		RestoreServerDefaults(flagMonitor);
#endif

	//status.config.settingsConfigured = true;
}


/// <summary>
/// Restore Config.Device... settings to default.
/// </summary>
void Config::Defaults::RestoreDeviceDefaults(bool flagMonitor)
{
#warning Potentially remove configMonitor as CRC is now implemented.



	if (flagMonitor)
	{
		configMonitor.device.useDefaults = config.device.useDefaults != false;
		configMonitor.device.autoBackupMode = config.device.autoBackupMode != (ConfigAutobackupMode_t)AUTO_BACKUP_MODE;
		//configMonitor.device.crcOnBackup = config.device.crcOnBackup != CRC_ON_BACKUP;

		configMonitor.device.serial.useDefaults = config.device.serial.useDefaults != false;

		configMonitor.device.serial.debugPort = config.device.serial.debugPort != SERIAL_DEBUG_PORT;
		configMonitor.device.serial.messagePort = config.device.serial.messagePort != SERIAL_MESSAGE_PORT;

		configMonitor.device.serial.port0.useDefaults = config.device.serial.port0.useDefaults != false;
		configMonitor.device.serial.port0.baud = config.device.serial.port0.baud != SERIAL0_BAUDRATE;
		//configMonitor.device.serial.port0.rxGpio = config.device.serial.port0.rxGpio != GPIO_SERIAL0_RX;
		//configMonitor.device.serial.port0.txGpio = config.device.serial.port0.txGpio != GPIO_SERIAL0_TX;
		configMonitor.device.serial.port0.config = config.device.serial.port0.config != SERIAL0_CONFIG /*| UART_CONFIG_CONSTANT*/;

		configMonitor.device.serial.port1.useDefaults = config.device.serial.port1.useDefaults != false;
		configMonitor.device.serial.port1.enabled = config.device.serial.port1.enabled != SERIAL1_ENABLED;
		configMonitor.device.serial.port1.baud = config.device.serial.port1.baud != SERIAL1_BAUDRATE;
		configMonitor.device.serial.port1.rxGpio = config.device.serial.port1.rxGpio != GPIO_SERIAL1_RX;
		configMonitor.device.serial.port1.txGpio = config.device.serial.port1.txGpio != GPIO_SERIAL1_TX;
		configMonitor.device.serial.port1.config = config.device.serial.port1.config != SERIAL1_CONFIG /*| UART_CONFIG_CONSTANT*/;

		configMonitor.device.i2c.useDefaults = config.device.i2c.useDefaults != false;
		configMonitor.device.i2c.enabled = config.device.i2c.enabled != I2C_ENABLED;
		configMonitor.device.i2c.sclGpio = config.device.i2c.sclGpio != GPIO_I2C_SCL;
		configMonitor.device.i2c.sdaGpio = config.device.i2c.sdaGpio != GPIO_I2C_SDA;
		configMonitor.device.i2c.freq = config.device.i2c.freq != I2C_FREQUENCY;
	}

	config.device.useDefaults = false;
	config.device.autoBackupMode = (ConfigAutobackupMode_t)AUTO_BACKUP_MODE;
	//config.device.crcOnBackup =  CRC_ON_BACKUP;

	config.device.serial.useDefaults = false;
	config.device.serial.debugPort = SERIAL_DEBUG_PORT;
	config.device.serial.messagePort = SERIAL_MESSAGE_PORT;

	config.device.serial.port0.useDefaults = false;
	config.device.serial.port0.enabled = true;
	config.device.serial.port0.baud = SERIAL0_BAUDRATE;
	//config.device.serial.port0.rxGpio = GPIO_SERIAL0_RX;
	//config.device.serial.port0.txGpio = GPIO_SERIAL0_TX;
	config.device.serial.port0.config = SERIAL0_CONFIG /*| UART_CONFIG_CONSTANT*/;


	config.device.serial.port1.useDefaults = false;
	config.device.serial.port1.enabled = SERIAL1_ENABLED;
	config.device.serial.port1.baud = SERIAL1_BAUDRATE;
	config.device.serial.port1.rxGpio = GPIO_SERIAL1_RX;
	config.device.serial.port1.txGpio = GPIO_SERIAL1_TX;
	config.device.serial.port1.config = SERIAL0_CONFIG /*| UART_CONFIG_CONSTANT*/;

	//Mark GPIO as in use.
	RegisterGpio(config.device.serial.port1.rxGpio);
	RegisterGpio(config.device.serial.port1.txGpio);

	config.device.i2c.useDefaults = false;
	config.device.i2c.enabled = I2C_ENABLED;
	config.device.i2c.sclGpio = GPIO_I2C_SCL;
	config.device.i2c.sdaGpio = GPIO_I2C_SDA;
	config.device.i2c.freq = I2C_FREQUENCY;

	//Mark GPIO as in use.
	RegisterGpio(config.device.i2c.sclGpio);
	RegisterGpio(config.device.i2c.sdaGpio);
}


/// <summary>
/// Some strings are acting wierd in the SetDefault functions.
/// When being set, they initialize to "", unless if they have been previously set.
/// Until I can figure out what is causing this, I am just going to pre-initialize them.
/// *Might be a problem while JTAG debugging, as when they are combined with another string the text is there.
/// *Something about the memory not being read.
/// </summary>
void Config::Defaults::InitializeStrings()
{
	/*Wifi SSID seems to always set properly. Will initially set to that until the problem is found.*/
	//config.mqtt.publish.unknownPayload = WIFI_SSID;
	config.mqtt.baseTopics.base = WIFI_SSID;
	config.mqtt.baseTopics.command = WIFI_SSID;
	config.mqtt.baseTopics.state = WIFI_SSID;

	config.server.ftp.user = WIFI_SSID;
	config.server.ftp.pass = WIFI_SSID;

	config.server.hostname = WIFI_SSID;
	config.server.user = WIFI_SSID;
	config.server.pass = WIFI_SSID;
}

/// <summary>
/// Restore wifi settings to default.
/// </summary>
void Config::Defaults::RestoreWifiDefaults(bool flagMonitor)
{
	if (flagMonitor)
	{
		configMonitor.wifi.useDefaults = config.wifi.useDefaults != false;
		configMonitor.wifi.channel = config.wifi.channel != WIFI_CHANNEL;
		configMonitor.wifi.powerLevel = config.wifi.powerLevel != WIFI_TX_POWER_LEVEL;

		configMonitor.wifi.station.enabled = config.wifi.station.enabled != WIFI_STATION_ENABLE;
		configMonitor.wifi.station.ssid = config.wifi.station.ssid != WIFI_SSID;
		configMonitor.wifi.station.pass = config.wifi.station.pass != WIFI_PASS;
		//configMonitor.wifi.mode = config.wifi.mode != (WifiMode_t)wifiMode;
		configMonitor.wifi.station.ledGpio = config.wifi.station.ledGpio != WIFI_LED_GPIO;
		configMonitor.wifi.station.ledOn = config.wifi.station.ledOn != WIFI_LED_ON;

#if COMPILE_ACCESSPOINT
		configMonitor.wifi.accessPoint.useDefaults = config.wifi.accessPoint.useDefaults != false;
		configMonitor.wifi.accessPoint.enabled = config.wifi.accessPoint.enabled != WIFI_ACCESSPOINT_ENABLE;
		configMonitor.wifi.accessPoint.configOnly = config.wifi.accessPoint.configOnly != WIFI_ACCESSPOINT_CONFIG_ONLY;
		configMonitor.wifi.accessPoint.ssid = config.wifi.accessPoint.ssid != WIFI_ACCESSPOINT_SSID;
		configMonitor.wifi.accessPoint.pass = config.wifi.accessPoint.pass != WIFI_ACCESSPOINT_PASS;
		configMonitor.wifi.accessPoint.hidden = config.wifi.accessPoint.hidden != WIFI_ACCESSPOINT_HIDDEN;
		configMonitor.wifi.accessPoint.maxConnections = config.wifi.accessPoint.maxConnections != WIFI_ACCESSPOINT_MAX_CONNECTIONS;
		configMonitor.wifi.accessPoint.buttonGpio = config.wifi.accessPoint.buttonGpio != WIFI_ACCESSPOINT_BUTTON_GPIO;
		configMonitor.wifi.accessPoint.buttonPullup = config.wifi.accessPoint.buttonPullup != WIFI_ACCESSPOINT_PULLUP;
		configMonitor.wifi.accessPoint.buttonPress = config.wifi.accessPoint.buttonPress != WIFI_ACCESSPOINT_BUTTON_PRESSED;
		configMonitor.wifi.accessPoint.holdTime = config.wifi.accessPoint.holdTime != WIFI_ACCESSPOINT_HOLDTIME;
		configMonitor.wifi.accessPoint.ledGpio = config.wifi.accessPoint.ledGpio != WIFI_ACCESSPOINT_LED_GPIO;
		configMonitor.wifi.accessPoint.ledOn = config.wifi.accessPoint.ledOn != WIFI_ACCESSPOINT_LED_ON;
#endif

#if DEVELOPER_MODE
		configMonitor.wifi.taskSettings.useDefaults = config.wifi.taskSettings.useDefaults != false;
		configMonitor.wifi.taskSettings.core = config.wifi.taskSettings.core != WIFI_TASK_CORE;
		configMonitor.wifi.taskSettings.priority = config.wifi.taskSettings.priority != WIFI_TASK_PRIORITY;
		configMonitor.wifi.taskSettings.stackSize = config.wifi.taskSettings.stackSize != TASK_STACK_SIZE;
		configMonitor.wifi.taskSettings.recurRate = config.wifi.taskSettings.recurRate != TASK_RECURRANCE_RATE;
#endif
	}

	config.wifi.useDefaults = false;
	config.wifi.channel = WIFI_CHANNEL;
	config.wifi.powerLevel = WIFI_TX_POWER_LEVEL;

	config.wifi.station.ssid = WIFI_SSID;
	config.wifi.station.pass = WIFI_PASS;
	config.wifi.station.ledGpio = WIFI_LED_GPIO;
	config.wifi.station.ledOn = WIFI_LED_ON;

#if COMPILE_ACCESSPOINT
	config.wifi.accessPoint.useDefaults = false;
	config.wifi.accessPoint.enabled = WIFI_ACCESSPOINT_ENABLE;
	config.wifi.accessPoint.configOnly = WIFI_ACCESSPOINT_CONFIG_ONLY;
	config.wifi.accessPoint.ssid = WIFI_ACCESSPOINT_SSID;
	config.wifi.accessPoint.pass = WIFI_ACCESSPOINT_PASS;
	config.wifi.accessPoint.hidden = WIFI_ACCESSPOINT_HIDDEN;
	config.wifi.accessPoint.maxConnections = WIFI_ACCESSPOINT_MAX_CONNECTIONS;
	config.wifi.accessPoint.buttonGpio = WIFI_ACCESSPOINT_BUTTON_GPIO;
	config.wifi.accessPoint.buttonPullup = WIFI_ACCESSPOINT_PULLUP;
	config.wifi.accessPoint.buttonPress = WIFI_ACCESSPOINT_BUTTON_PRESSED;
	config.wifi.accessPoint.holdTime = WIFI_ACCESSPOINT_HOLDTIME;
	config.wifi.accessPoint.ledGpio = WIFI_ACCESSPOINT_LED_GPIO;
	config.wifi.accessPoint.ledOn = WIFI_ACCESSPOINT_LED_ON;
#endif

#if DEVELOPER_MODE
	config.wifi.taskSettings.useDefaults = false;
	config.wifi.taskSettings.core = WIFI_TASK_CORE;
	config.wifi.taskSettings.priority = WIFI_TASK_PRIORITY;
	config.wifi.taskSettings.stackSize = TASK_STACK_SIZE;
	config.wifi.taskSettings.recurRate = TASK_RECURRANCE_RATE;
#endif
}

/// <summary>
/// Restore MQTT settings to default.
/// </summary>
void Config::Defaults::RestoreMqttDefaults(bool flagMonitor)
{
	IPAddress mqttIp = IPAddress(MQTT_IP);
	IPAddress mqttIpAP = IPAddress(MQTT_IP_AP);

	if (flagMonitor)
	{
		configMonitor.mqtt.useDefaults = config.mqtt.useDefaults != false;
		configMonitor.mqtt.ledGpio = config.mqtt.ledGpio != MQTT_LED_GPIO;
		configMonitor.mqtt.ledOn = config.mqtt.ledOn != MQTT_LED_ON;

		configMonitor.mqtt.publish.bufferSize = config.mqtt.publish.bufferSize != MQTT_BUFFER_SIZE;
		configMonitor.mqtt.publish.rate = config.mqtt.publish.rate != MQTT_PUBLISH_RATE * 1000;
		configMonitor.mqtt.publish.errorRate = config.mqtt.publish.errorRate != MQTT_ERROR_PUBLISH_RATE * 1000;
		configMonitor.mqtt.publish.availabilityRate = config.mqtt.publish.availabilityRate != MQTT_AVAILABILITY_PUBLISH_RATE * 1000;
		configMonitor.mqtt.publish.json = config.mqtt.publish.json != true/*MQTT_PUBLISH_AS_JSON*/;
		//configMonitor.mqtt.publish.onIndividualTopics = config.mqtt.publish.onIndividualTopics != MQTT_PUBLISH_ON_SINGLE_TOPICS;
		//configMonitor.mqtt.publish.unknownPayload = config.mqtt.publish.unknownPayload != SENSOR_DATA_UNKNOWN;

		configMonitor.mqtt.broker.ip = config.mqtt.broker.ip != mqttIp;
		configMonitor.mqtt.broker.ipAP = config.mqtt.broker.ipAP != mqttIpAP;
		configMonitor.mqtt.broker.autoDetectIp = config.mqtt.broker.autoDetectIp != MQTT_AUTODETECTIP;
		configMonitor.mqtt.broker.port = config.mqtt.broker.port != MQTT_PORT;
		configMonitor.mqtt.broker.user = config.mqtt.broker.user != MQTT_USER;
		configMonitor.mqtt.broker.pass = config.mqtt.broker.pass != MQTT_PASS;
		configMonitor.mqtt.broker.autoTimeout = config.mqtt.broker.autoTimeout != MQTT_AUTO_IP_TIMEOUT;
		configMonitor.mqtt.broker.timeout = config.mqtt.broker.timeout != MQTT_TIMEOUT;
		configMonitor.mqtt.broker.connectInterval = config.mqtt.broker.connectInterval != MQTT_CONNECTION_INTERVAL;
		configMonitor.mqtt.broker.maxRetries = config.mqtt.broker.maxRetries = MQTT_MAX_RETRIES;
		configMonitor.mqtt.broker.autoMaxRetries = config.mqtt.broker.autoMaxRetries = MQTT_AUTO_MAX_RETRIES;

		configMonitor.mqtt.baseTopics.base = config.mqtt.baseTopics.base != TOPIC_BASE;
		configMonitor.mqtt.baseTopics.availability = config.mqtt.baseTopics.availability != TOPIC_BASE_AVAILABILITY;
		configMonitor.mqtt.baseTopics.jsonCommand = config.mqtt.baseTopics.jsonCommand != TOPIC_BASE_JSON_COMMAND;
		configMonitor.mqtt.baseTopics.jsonState = config.mqtt.baseTopics.jsonState != TOPIC_BASE_JSON_STATE;
		configMonitor.mqtt.baseTopics.command = config.mqtt.baseTopics.command != TOPIC_BASE_COMMAND;
		configMonitor.mqtt.baseTopics.state = config.mqtt.baseTopics.state != TOPIC_BASE_STATE;

		configMonitor.mqtt.topics.availability = config.mqtt.topics.availability != TOPIC_BASE TOPIC_BASE_AVAILABILITY;
		configMonitor.mqtt.topics.jsonState = config.mqtt.topics.jsonState != TOPIC_BASE TOPIC_BASE_JSON_COMMAND;
		configMonitor.mqtt.topics.jsonCommand = config.mqtt.topics.jsonCommand != TOPIC_BASE TOPIC_BASE_JSON_STATE;

#if DEVELOPER_MODE
		configMonitor.mqtt.taskSettings.useDefaults = config.mqtt.taskSettings.useDefaults != false;
		configMonitor.mqtt.taskSettings.core = config.mqtt.taskSettings.core != MQTT_TASK_CORE;
		configMonitor.mqtt.taskSettings.priority = config.mqtt.taskSettings.priority != MQTT_TASK_PRIORITY;
		configMonitor.mqtt.taskSettings.recurRate = config.mqtt.taskSettings.recurRate != MQTT_TASK_RECURRATE;
		configMonitor.mqtt.taskSettings.stackSize = config.mqtt.taskSettings.stackSize != MQTT_TASK_STACKSIZE;
#endif
	}


	config.mqtt.useDefaults = false;
	config.mqtt.ledGpio = MQTT_LED_GPIO;
	config.mqtt.ledOn = MQTT_LED_ON;

	config.mqtt.publish.bufferSize = MQTT_BUFFER_SIZE;
	config.mqtt.publish.rate = MQTT_PUBLISH_RATE * 1000;
	config.mqtt.publish.errorRate = MQTT_ERROR_PUBLISH_RATE * 1000;
	config.mqtt.publish.availabilityRate = MQTT_AVAILABILITY_PUBLISH_RATE * 1000;
	config.mqtt.publish.json = true/*MQTT_PUBLISH_AS_JSON*/;
	//config.mqtt.publish.onIndividualTopics = MQTT_PUBLISH_ON_SINGLE_TOPICS;
	//config.mqtt.publish.unknownPayload = SENSOR_DATA_UNKNOWN;

	config.mqtt.broker.ip = mqttIp;
	config.mqtt.broker.ipAP = mqttIpAP;
	config.mqtt.broker.autoDetectIp = MQTT_AUTODETECTIP;
	config.mqtt.broker.wifiMode = MQTT_WIFI_MODE;
	config.mqtt.broker.port = MQTT_PORT;
	config.mqtt.broker.user = MQTT_USER;
	config.mqtt.broker.pass = MQTT_PASS;
	config.mqtt.broker.autoTimeout = MQTT_AUTO_IP_TIMEOUT;
	config.mqtt.broker.timeout = MQTT_TIMEOUT;
	config.mqtt.broker.connectInterval = MQTT_CONNECTION_INTERVAL;
	config.mqtt.broker.maxRetries = MQTT_MAX_RETRIES;
	config.mqtt.broker.autoMaxRetries = MQTT_AUTO_MAX_RETRIES;

	config.mqtt.baseTopics.base = TOPIC_BASE;
	config.mqtt.baseTopics.availability = TOPIC_BASE_AVAILABILITY;
	config.mqtt.baseTopics.jsonCommand = TOPIC_BASE_JSON_COMMAND;
	config.mqtt.baseTopics.jsonState = TOPIC_BASE_JSON_STATE;
	config.mqtt.baseTopics.command = TOPIC_BASE_COMMAND;
	config.mqtt.baseTopics.state = TOPIC_BASE_STATE;

	config.mqtt.topics.availability = TOPIC_BASE TOPIC_BASE_AVAILABILITY;
	config.mqtt.topics.jsonCommand = TOPIC_BASE TOPIC_BASE_JSON_COMMAND;
	config.mqtt.topics.jsonState = TOPIC_BASE TOPIC_BASE_JSON_STATE;

#if DEVELOPER_MODE
	config.mqtt.taskSettings.useDefaults = false;
	config.mqtt.taskSettings.core = MQTT_TASK_CORE;
	config.mqtt.taskSettings.priority = MQTT_TASK_PRIORITY;
	config.mqtt.taskSettings.recurRate = MQTT_TASK_RECURRATE;
	config.mqtt.taskSettings.stackSize = MQTT_TASK_STACKSIZE;
#endif
}

//void Config::RestoreDocDefaults()
//{
//	configMonitor.docs.devicesJsonSize = config.docs.devicesJsonSize != DOC_DEVICES_SIZE;
//	configMonitor.docs.sensorsJsonSize = config.docs.sensorsJsonSize != DOC_SENSORS_SIZE;
//	configMonitor.docs.binarySensorsJsonSize = config.docs.binarySensorsJsonSize != DOC_BINARYSENSORS_SIZE;
//	configMonitor.docs.lightsJsonSize = config.docs.lightsJsonSize != DOC_LIGHTS_SIZE;
//	configMonitor.docs.buttonsJsonSize = config.docs.buttonsJsonSize != DOC_BUTTONS_SIZE;
//	configMonitor.docs.switchesJsonSize = config.docs.switchesJsonSize != DOC_SWITCHES_SIZE;
//
//	config.docs.devicesJsonSize = DOC_DEVICES_SIZE;
//	config.docs.sensorsJsonSize = DOC_SENSORS_SIZE;
//	config.docs.binarySensorsJsonSize = DOC_BINARYSENSORS_SIZE;
//	config.docs.lightsJsonSize = DOC_LIGHTS_SIZE;
//	config.docs.buttonsJsonSize = DOC_BUTTONS_SIZE;
//	config.docs.switchesJsonSize = DOC_SWITCHES_SIZE;
//}

void Config::Defaults::RestoreFtpDefaults(bool flagMonitor)
{
#if COMPILE_FTP
	if (flagMonitor)
	{
		configMonitor.server.ftp.useDefaults = config.server.ftp.useDefaults != false;
		configMonitor.server.ftp.enabled = config.server.ftp.enabled != FTP_ENABLED;
		configMonitor.server.ftp.anonymous = config.server.ftp.anonymous != FTP_ANONYMOUS;
		configMonitor.server.ftp.user = config.server.ftp.user != FTP_USERNAME;
		configMonitor.server.ftp.pass = config.server.ftp.pass != FTP_PASSWORD;
		configMonitor.server.ftp.timeout = config.server.ftp.timeout != FTP_TIMEOUT;

#if DEVELOPER_MODE
		configMonitor.server.ftp.taskSettings.useDefaults = config.server.ftp.taskSettings.useDefaults != false;
		configMonitor.server.ftp.taskSettings.core = config.server.ftp.taskSettings.core != FTP_TASK_CORE;
		configMonitor.server.ftp.taskSettings.priority = config.server.ftp.taskSettings.priority != FTP_TASK_PRIORITY;
		configMonitor.server.ftp.taskSettings.recurRate = config.server.ftp.taskSettings.recurRate != FTP_TASK_RECURRATE;
		configMonitor.server.ftp.taskSettings.stackSize = config.server.ftp.taskSettings.stackSize != FTP_TASK_STACKSIZE;
#endif
	}

	config.server.ftp.useDefaults = false;
	config.server.ftp.enabled = FTP_ENABLED;
	config.server.ftp.anonymous = FTP_ANONYMOUS;
	config.server.ftp.user = FTP_USERNAME;
	config.server.ftp.pass = FTP_PASSWORD;
	config.server.ftp.timeout = FTP_TIMEOUT;
#if DEVELOPER_MODE
	config.server.ftp.taskSettings.useDefaults = false;
	config.server.ftp.taskSettings.core = FTP_TASK_CORE;
	config.server.ftp.taskSettings.priority = FTP_TASK_PRIORITY;
	config.server.ftp.taskSettings.recurRate = FTP_TASK_RECURRATE;
	config.server.ftp.taskSettings.stackSize = FTP_TASK_STACKSIZE;
#endif
#endif
}

void Config::Defaults::RestoreServerDefaults(bool flagMonitor)
{
#if COMPILE_SERVER
	if (flagMonitor)
	{
		configMonitor.server.useDefaults = config.server.useDefaults != false;
		configMonitor.server.dns = config.server.dns != SERVER_DNS_ENABLED;
		configMonitor.server.hostname = config.server.hostname != SERVER_HOSTNAME;
		configMonitor.server.authenticate = config.server.authenticate != SERVER_AUTHENTICATE;
		configMonitor.server.user = config.server.user != SERVER_USER;
		configMonitor.server.pass = config.server.pass != SERVER_PASS;
		configMonitor.server.sessionTimeout = config.server.sessionTimeout != SERVER_SESSION_TIMEOUT;

		configMonitor.server.browser.useDefaults = config.server.browser.useDefaults != false;
		configMonitor.server.browser.enabled = config.server.browser.enabled != BROWSER_ENABLED;
		configMonitor.server.browser.config = config.server.browser.config != BROWSER_CONFIG_ENABLED;
		configMonitor.server.browser.console = config.server.browser.console != BROWSER_CONSOLE_ENABLED;
		configMonitor.server.browser.updater = config.server.browser.updater != BROWSER_UPDATER_ENABLED;
		configMonitor.server.browser.mqttDevices = config.server.browser.mqttDevices != BROWSER_CONFIG_MQTT_ENABLED;
		configMonitor.server.browser.ssl = config.server.browser.ssl != BROSWER_SSL;

#if COMPILE_BROWSER_TOOLS
		configMonitor.server.browser.tools.fileEditor = config.server.browser.tools.fileEditor != BROWSER_TOOLS_FILE_EDITOR;
		configMonitor.server.browser.tools.jsonVerify = config.server.browser.tools.jsonVerify != BROWSER_TOOLS_JSON_VERIFY;
#endif

#if COMPILE_OTA
		configMonitor.server.ota.useDefaults = config.server.ota.useDefaults != false;
		configMonitor.server.ota.enabled = config.server.ota.enabled != OTA_ENABLED;
#if DEVELOPER_MODE
		configMonitor.server.ota.taskSettings.useDefaults = config.server.ota.taskSettings.useDefaults != false;
		configMonitor.server.ota.taskSettings.core = config.server.ota.taskSettings.core != OTA_TASK_CORE;
		configMonitor.server.ota.taskSettings.priority = config.server.ota.taskSettings.priority != OTA_TASK_PRIORITY;
		configMonitor.server.ota.taskSettings.recurRate = config.server.ota.taskSettings.recurRate != OTA_TASK_RECURRATE;
		configMonitor.server.ota.taskSettings.stackSize = config.server.ota.taskSettings.stackSize != OTA_TASK_STACKSIZE;
#endif
#endif
	}

	config.server.useDefaults = false;
	config.server.dns = SERVER_DNS_ENABLED;
	config.server.hostname = SERVER_HOSTNAME;
	config.server.authenticate = SERVER_AUTHENTICATE;
	config.server.user = SERVER_USER;
	config.server.pass = SERVER_PASS;
	config.server.sessionTimeout = SERVER_SESSION_TIMEOUT;

	config.server.browser.useDefaults = false;
	config.server.browser.enabled = BROWSER_ENABLED;
	config.server.browser.config = BROWSER_CONFIG_ENABLED;
	config.server.browser.console = BROWSER_CONSOLE_ENABLED;
	config.server.browser.updater = BROWSER_UPDATER_ENABLED;
	config.server.browser.mqttDevices = BROWSER_CONFIG_MQTT_ENABLED;
	config.server.browser.ssl = BROSWER_SSL;

#if COMPILE_BROWSER_TOOLS
	config.server.browser.tools.fileEditor = BROWSER_TOOLS_FILE_EDITOR;
	config.server.browser.tools.jsonVerify = BROWSER_TOOLS_JSON_VERIFY;
#endif

#if COMPILE_OTA
	config.server.ota.useDefaults != false;
	config.server.ota.enabled != OTA_ENABLED;
#if DEVELOPER_MODE
	config.server.ota.taskSettings.useDefaults = false;
	config.server.ota.taskSettings.core = OTA_TASK_CORE;
	config.server.ota.taskSettings.priority = OTA_TASK_PRIORITY;
	config.server.ota.taskSettings.recurRate = OTA_TASK_RECURRATE;
	config.server.ota.taskSettings.stackSize = OTA_TASK_STACKSIZE;
#endif
#endif
#endif
}

#pragma endregion

#pragma region Documents

void missing_required_key(const char* key)
{
	DEBUG_LOG_F("Warning!!! Config Missing Required key : %s!\r\n", key);
	status.config.hasRequiredData = false;
}

void missing_key(const char* key)
{
	DEBUG_LOG_F("Config Missing key : %s\r\n", key);
}

void Config::Documents::LoadBootSettings()
{

#if DEVELOPER_MODE
	status.misc.developerMode = DEVELOPER_MODE;
#endif

	DEBUG_LOG_LN("Loading boot settings...");
	StaticJsonDocument<256> doc;

	//Check to see if boot.json has changed since last boot.
	uint32_t crc32 = FileManager::GetFileCRC(FILE_BOOT_PATH);
	statusRetainedMonitor.crcs.bootFile = crc32 != statusRetained.crcs.bootFile;
	statusRetained.crcs.bootFile = crc32;

	if (FileManager::OpenAndParseFile(FILE_BOOT_PATH, doc) == 0)
	{
#if DEVELOPER_MODE
		if (doc.containsKey("disableDevMode"))
			status.misc.developerMode = !doc["disableDevMode"];
#endif

		/*
			You can select the boot source for the next boot.
			Create a key "nextBootSource" and set the value to a number between 0 and 4,
			which is converted to the ConfigSource enum.

			0 = Ignore.
			1 = FS (Unchanged)
			2 = EEPROM
			3 = Firmware
			4 = FS Backup

			It will only work once, until the file has changed again.
			Each time boot.json is loaded, a CRC is calculated to see if it has changed.
			Only if the CRC is different will "nextBootSource" be used.
		*/
		if (doc.containsKey("nextBootSource") && statusRetainedMonitor.crcs.bootFile)
		{
			ConfigSource bootSource = doc["nextBootSource"].as<ConfigSource>();

			if (bootSource > ConfigSource::CFG_DEFAULT)
			{
				//Change boot source without saving.
				//statusRetainedMonitor.boot.bootSource = bootSource != ConfigSource::CFG_DEFAULT; /*statusRetained.boot.bootSource != bootSource;*/
				statusRetained.boot.bootSource = bootSource;
				//status.config.configSource = bootSource;
				//If not booting from File system, skip ConfigPath.

				if (statusRetained.boot.bootSource > ConfigSource::CFG_FILESYSTEM)
				{
					Backup::DisableBackups();
					goto ExitConfigPath;	//Big and scary goto!
				}
			}

			
		}

	ConfigPath: 
		{
			/*
				You can test out a config file for a single boot.
				Just set the testPath value to the file you want to try.

				If you have the browser enabled, you can hit accept and it will overwrite the main config
				with your test file. (If configNum exists, it will use that for the file name)

				It will only work once, until the file has changed again.
				Each time boot.json is loaded, a CRC is calculated to see if it has changed.
				Only if the CRC is different will "nextBootSource" be used.

				If nextBootSource is greater than 1 testPath will be ignored.
			*/
			if (doc.containsKey("testPath") && statusRetainedMonitor.crcs.bootFile)
			{
				JsonVariant testpath = doc["testPath"];

				if (testpath.is<const char*>())
				{
					String path = testpath.as<String>();
					#warning add boot path to EEPROM. or in a file on the FS. Encrpyt with Xor so people dont edit it.
#warning reminder to webpage, or button on Config webpage which overwrites config.
#warning add configNum to status.
					if (ESP_FS.exists(path) && path.length() <= CONFIG_PATH_MAX_LENGTH)
					{
						Backup::DisableBackups();
						status.config.testingConfig = true;
						SetConfigPathAndFileName(path.c_str());
						goto ExitConfigPath;
					}
				}
			}

			/*
				You can have multiple configs stored in the file system.
				It can have any path or name.

				To select a config file, create "boot.json" in the root folder.
				Create a key called "configPath" and set the value to the file's path.
			*/
			if (doc.containsKey("configPath"))
			{
				JsonVariant cpath = doc["configPath"];

				if (cpath.is<const char*>())
				{
					String path = cpath.as<String>();

					if (ESP_FS.exists(path) && path.length() <= CONFIG_PATH_MAX_LENGTH)
					{
						SetConfigPathAndFileName(path.c_str());
						goto ExitConfigPath;
					}
				}
			}

			/*
				You can have multiple configs stored in the file system.
				One way is to number them like "config1.json," "config2.json" and so on.

				To select a config file, create "boot.json" in the root folder.
				Create a key called "configNum" and set the number to the file you wish to select.

				Note: If key "configPath" aims to a valid file, "configNum" will be ignored.
			*/
			if (doc.containsKey("configNum"))
			{
				int index = doc["configNum"];

				char newPath[CONFIG_PATH_MAX_LENGTH];

				sprintf(newPath, "/config%d.json", index);

				if (ESP_FS.exists(newPath))
				{
					SetConfigPathAndFileName(newPath);
				}
			}
		}
	ExitConfigPath:
		delayMicroseconds(1);	//Need something for the ExitConfigPath label to bound to.
	}
	else
	{
		//No boot settings.
	}

}

/// <summary>
/// Checks to see if the config's CRC after configuration matches the previous boot.
/// </summary>
/// <returns>1 if the file has been updated or if it is a different file, 0 if there has been no changes, and -1 if the file does not exist.</returns>
int Config::Documents::CheckConfigCrc(JsonDocument& configDoc)
{
	if (configDoc.size() == 0) return -1;

	status.config.backup.configCRC = FileManager::GetSerializedCRC(configDoc);

	statusRetainedMonitor.crcs.configFile = statusRetained.crcs.configFile != status.config.backup.configCRC;
	statusRetained.crcs.configFile = status.config.backup.configCRC;

	Backup::SetBackupFlags(status.config.backup.configCRC);

	return statusRetainedMonitor.crcs.configFile;
}

/// <summary>
/// Checks to see if the config path is the same path used last boot.
/// </summary>
/// <returns>1 if the path has changed since last boot, 0 if the path is the same, and -1 if configPath has not been set.</returns>
int Config::Documents::CheckConfigPathCrc()
{
	uint8_t length = strlen(status.config.path);

	//configPath has not been set.
	if (length == 0)
		return -1;

	crc.reset();

	for (int i = 0; i < length; i++)
	{
		crc.add(status.config.path[i]);
	}

	uint32_t result = crc.getCRC();

	statusRetainedMonitor.crcs.configPath = result != statusRetained.crcs.configPath;

	//Config path has changed.
	if (statusRetainedMonitor.crcs.configPath)
	{
#if COMPILE_BACKUP
		//Disable backups until manually enabled.
		//Do not want to backup a file that may just be used for testing.
		//May change this in the future.
		if (!status.device.freshBoot)
			Config::Backup::DisableBackups();
#endif
	}


	return statusRetainedMonitor.crcs.configPath;
}


/// <summary>
/// Load the configuration from either the File system, EEPROM, or firmware.
/// First config.json is read, if it fails the backup on the EEPROM will be read,
/// and if the EEPROM fails it will use the firmware defaults.
/// </summary>
/// <returns></returns>
bool Config::Documents::LoadConfiguration()
{
	DEBUG_LOG_LN("Configuring Device...");

#warning function is gross, fix it.

	//if (statusRetainedMonitor.boot.bootSource)
	status.config.configSource = statusRetained.boot.bootSource;


	Backup::SetBackupFlags(statusRetained.crcs.recentBackup);

ReselectBootSource:
	DEBUG_LOG("Boot source : ");
	//If a custom boot source was set last runtime, use the custom boot source.
	switch (status.config.configSource)
	{
	default:
	case ConfigSource::CFG_DEFAULT:
	case ConfigSource::CFG_FILESYSTEM:
		DEBUG_LOG_LN("File System");
		status.config.configSource = ConfigSource::CFG_FILESYSTEM;
		break;

	case ConfigSource::CFG_EEPROM:
		DEBUG_LOG_LN("Backup on EEPROM");
		if (!statusRetained.fileSizes.eepromBackup)
		{
			DEBUG_LOG_LN("...Cannot load from EEPROM! A backup does not exist.\r\n...Using Firmware config source.");
			status.config.configSource = ConfigSource::CFG_FIRMWARE;
			goto ReselectBootSource;
		}
		goto EEPROM_Config;

	case ConfigSource::CFG_FIRMWARE: 
		DEBUG_LOG_LN("Firmware");
		goto SkipLoading;

	case ConfigSource::CFG_BACKUP_FILESYSTEM:
		DEBUG_LOG_LN("Backup on File System");
		if (!status.config.backup.filesystemBackedUp)
		{
			DEBUG_LOG_LN("...Cannot load backup from File System! A backup does not exist or the CRC does not match most recent backup.\r\n...Using EEPROM config source.");
			status.config.configSource = ConfigSource::CFG_EEPROM;
			goto ReselectBootSource;
		}
		break;
	}

	if (!status.config.pathSet)
	{
		if (!SelectConfigPath(status.config.configSource == ConfigSource::CFG_BACKUP_FILESYSTEM))
		{
			//If last attempt was from FS Backup, try EEPROM. Else use file system.
			if (status.config.configSource == ConfigSource::CFG_BACKUP_FILESYSTEM || !status.config.backup.filesystemBackedUp)
			{
				if (status.config.backup.eepromBackedUp)
					goto EEPROM_Config;
				else
					goto Firmware_Config;
			}
			else
				goto FS_Backup_Config;
		}
	}

	//Deserialize the config into a document.
	if (!DeserializeConfig())
	{
		status.config.backup.backupsDisabled = true;

		#if COMPILE_BACKUP
			//Config failed, try backup or EEPROM.
			if (status.config.backup.filesystemBackedUp)
			{
				FS_Backup_Config:

				if (SelectConfigPath(true))
				{
					DEBUG_LOG_LN("...Getting config from FS Backup...");
					status.config.configSource = ConfigSource::CFG_BACKUP_FILESYSTEM;
					if (!DeserializeConfig())
						goto EEPROM_Config;
				}
				else goto EEPROM_Config;
			}
			else 
			{
			EEPROM_Config:		
				if (status.config.backup.eepromBackedUp) {
					DEBUG_LOG_LN("...Getting config from EEPROM...");
					status.config.configSource = ConfigSource::CFG_EEPROM;
					if (!Backup::DeserializeEepromBackupConfig(configDoc))
					{
						goto Firmware_Config;
					}
				}
				else
				{
				Firmware_Config:
					DEBUG_LOG_LN("...Using config from Firmware...");
					status.config.configSource = ConfigSource::CFG_FIRMWARE;
				}
			}
		#endif
	}


SkipLoading:
	ChangeBootSource(ConfigSource::CFG_DEFAULT);	//Reset the boot source to default.
}

void Config::Documents::Reconfigure()
{

}

bool Config::Documents::SelectConfigPath(bool backup)
{
	////If path hasn't already been set, and a path has been stored in the retained status, use it.
	//if (!status.config.pathSet)
	//{
	//	if (statusRetained.lastConfigPath[0] == '/')
	//		if (SetConfigPathAndFileName(statusRetained.lastConfigPath))
	//			return true;
	//}

	//configPath already initialized to default path.
	//If boot.json selected a different path, it will also be set already.
	//Just need to check if backup is selected, or developer mode is enabled.
#if COMPILE_BACKUP
	if (backup && statusRetained.crcs.configPath != 0)
	{
		String path, filename;
		GenerateBackupPath(status.config.path, &path, &filename);

		if (SetConfigPathAndFileName(path.c_str(), filename.c_str()))
			return true;
	}
#endif

#if DEVELOPER_MODE
	if (status.misc.developerMode)
	{
		//If separate config is supplied, use it.
#if defined(ESP8266)
		SetConfigPathAndFileName(FILE_CONFIG_ESP8266_PATH, FILE_CONFIG_ESP8266_NAME);
#elif defined(ESP32)
		SetConfigPathAndFileName(FILE_CONFIG_ESP32_PATH, FILE_CONFIG_ESP32_NAME);
#endif
	}
#endif


	//If false, set back to default.
		if (SetConfigPathAndFileName(FILE_CONFIG_PATH, FILE_CONFIG_NAME))
			return true;

	return false;
}

bool Config::Documents::SetConfigPathAndFileName(const char* path, const char* filename)
{
	if (!ESP_FS.exists(path)) return false;		//File does not exist. Do not change.

	status.config.pathSet = false;

	memset(status.config.path, 0, CONFIG_PATH_MAX_LENGTH);
	memset(status.config.fileName, 0, CONFIG_NAME_MAX_LENGTH);

	uint8_t offset = path[0] != '/';

	//If path does not start with /, add one.
	if (offset)
		status.config.path[0] = '/';

	strlcpy(status.config.path + offset, path, CONFIG_PATH_MAX_LENGTH - offset);

	if (filename == nullptr)
	{
		String name = FileManager::GetFileName(path);
		memset(status.config.fileName, 0, CONFIG_NAME_MAX_LENGTH);
		strlcpy(status.config.fileName, name.c_str(), CONFIG_NAME_MAX_LENGTH);

		//String p = path;
		//char newName[CONFIG_NAME_MAX_LENGTH];

		//int lastFolder = p.lastIndexOf('/');

		////If path contained /, increase offset to not include in name.
		//if (lastFolder >= 0) lastFolder++;			

		//memset(status.config.fileName, 0, CONFIG_NAME_MAX_LENGTH);
		//strlcpy(status.config.fileName, p.c_str() + lastFolder, CONFIG_NAME_MAX_LENGTH);
	}
	else
	{
		strlcpy(status.config.fileName, filename, CONFIG_NAME_MAX_LENGTH);
	}

	return status.config.pathSet = true;
}

bool Config::Documents::GenerateBackupPath(const char* path, String* backupPath, String* backupName)
{
	VALIDATE_ASSERT(backupPath);

	String bpath = path;

	int extIndex = bpath.lastIndexOf('.');
	int dirIndex = bpath.lastIndexOf('/') + 1;

	if (extIndex == -1)
	{
		DEBUG_LOG_LN("Cannot generate backup path! - Invalid Path");
		return false;
	}

	String ext = bpath.substring(extIndex);
	String name = bpath.substring(dirIndex, extIndex);

	if (backupName != nullptr)
		*backupName = name = "_backup";

	bpath = bpath.substring(0, extIndex) + "_backup" + ext;

	*backupPath = bpath;

	return true;
}



bool Config::Documents::DeserializeConfig()
{
	if (!status.config.pathSet)
	{
		DEBUG_LOG_LN("Config cannot be opened as the config path has not been set.");
		return false;
	}

	if (!status.device.fsMounted)
	{
		DEBUG_LOG_LN("Config cannot be opened as the file system is not mounted.");
		return false;
	}

	const char* path = status.config.path;
	const char* filename = status.config.fileName;

	File file;


	if (!FileManager::OpenFile(&file, path, "r"))
	{
		return false;
	}

	CheckConfigPathCrc();
	//CheckConfigCrc();
	
	//SaveRetainedStatus();

	DEBUG_LOG_LN("...Parsing...");

	//DeserializationError derror = deserializeJson(configDoc, file);
	//file.close();

	if (!FileManager::ParseFile(&file, configDoc, filename))
	{
		configDoc.clear();
		return false;
	}
	else
	{
		status.config.configRead = true;
	}

	DEBUG_LOG_LN("...Config read and parsed.");
	DEBUG_NEWLINE();
	return true;
}

bool Config::Documents::SetConfigFromDoc()
{
	DEBUG_LOG_LN("Setting config to values from JSON document...");

	if (!status.config.configRead)
	{
		DEBUG_LOG_LN("Configuration cannot be set. Config Document has not been deserialized.");
		return false;
	}

	JsonVariantConst configVar = configDoc.as<JsonVariantConst>();
	convertFromJson(configVar, config);
	CheckConfigCrc(configDoc);

	configDoc.clear();
	status.config.configRead = false;

	DEBUG_LOG_F("SSID : %s\r\nPASS : %s\r\nMQTT IP : %s\r\nPort : %d\r\nUser : %s\r\nPass : %s\r\n", config.wifi.station.ssid.c_str(), config.wifi.station.pass.c_str(), config.mqtt.broker.ip.toString().c_str(), config.mqtt.broker.port, config.mqtt.broker.user.c_str(), config.mqtt.broker.pass.c_str());

	return status.config.settingsConfigured = true;
}


bool Config::Documents::SerializeConfig(JsonDocument* doc)
{
	if (doc == nullptr) return false;

	doc->clear();

	JsonVariant docVar = doc->as<JsonVariant>();
	return docVar.set(config);
}


size_t Config::Documents::SaveConfig()
{
	//#warning  wip

	DEBUG_LOG_LN("Saving config...");

	if (!status.device.fsMounted)
	{
		DEBUG_LOG_LN("Config cannot be saved as the file system is not mounted.");
		return false;
	}


	DEBUG_LOG_LN("...Creating json document...");
	DynamicJsonDocument serializeConfigDoc(DOC_CONFIG_SERIALIZE_SIZE);
	SerializeConfig(&serializeConfigDoc);

	File file;

	FileManager::OpenFile(&file, status.config.path, "w");

	DEBUG_LOG_LN("...Serializing to file...");
	size_t size = serializeJsonPretty(serializeConfigDoc, file);
	
	file.close();

	if (size)
	{
		DEBUG_LOG_F("Config Saved. Size : %d bytes\r\n", size);
		return size;
	}
	else
	{
		DEBUG_LOG_LN("Saving config failed. Size 0.");
		return false;
	}
}





#pragma endregion

#pragma region Backup

bool Config::Backup::SaveBackupConfig(bool fs, bool eeprom, bool saveRetained, size_t* out_sizeFileSystem, size_t* out_sizeEeprom)
{
#if COMPILE_BACKUP
	DEBUG_NEWLINE();
	DEBUG_LOG_LN("Saving backup config...");

	if (status.config.backup.backupsDisabled)
	{
		DEBUG_LOG_LN("...Backups are disabled.");
		return false;
	}

	if (!status.config.settingsConfigured || !status.config.hasRequiredData)
	{
		DEBUG_LOG_LN("...Backup could not be saved. Settings not configured or required data is missing.");
		return 0;
	}

	if (status.config.backup.filesystemBackedUp /*&& status.config.backup.eepromBackedUp && statusRetained.fileSizes.eepromBackup*/)
	{
		Label_AlreadyBackedup:
		DEBUG_LOG_LN("...Backup already saved.");
		return true;
	}

	DEBUG_LOG_LN("...Creating json document...");
	DynamicJsonDocument serializeConfigDoc(DOC_CONFIG_SERIALIZE_SIZE);
	Documents::SerializeConfig(&serializeConfigDoc);

	uint32_t docCRC = FileManager::GetSerializedCRC(serializeConfigDoc);

	DEBUG_LOG_F("Most Recent Backup CRC : 0x%0000000x - Doc CRC : 0x%0000000x\r\n", statusRetained.crcs.recentBackup, docCRC);

	if (docCRC == statusRetained.crcs.recentBackup)
		goto Label_AlreadyBackedup;

	//status.disableRetainCommits = true;

	size_t sizeFS = 0;
	size_t sizeEeprom = 0;

	if (fs)
	{
		DEBUG_LOG("...Saving to FileSystem...");
		sizeFS = SaveBackupFilesystem(&serializeConfigDoc, docCRC);

		if (sizeFS > 1)
			DEBUG_LOG_F("...Backed up to file system with size of %dbytes...", sizeFS);
	}

	if (out_sizeFileSystem != nullptr)
		*out_sizeFileSystem = sizeFS;

	if (eeprom && status.config.backup.ableToBackupEeprom)
	{
		DEBUG_LOG("...Saving to EEPROM...");
		sizeEeprom = SaveBackupEeprom(&serializeConfigDoc, docCRC);

		if (sizeEeprom > 1)
			DEBUG_LOG_F("...Backed up to EEPROM with size of %dbytes...", sizeEeprom);
	}

	if (out_sizeEeprom != nullptr)
		*out_sizeEeprom = sizeEeprom;

	if (!status.config.backup.ableToBackupEeprom && sizeFS > 1)
	{
		//New backup saved, EEPROM out of date.
		//Could not save to EEPROM, flag that backup does not exist.
		statusRetained.crcs.eepromBackupFile = 0;
		statusRetained.fileSizes.eepromBackup = 0;
	}

	//status.disableRetainCommits = false;
	if (saveRetained)
		Status::SaveRetainedStatus();

	DEBUG_NEWLINE();
	return sizeEeprom || sizeFS;
#endif
}



bool Config::Backup::AutoSaveBackupConfig(bool saveRetained, size_t* out_sizeFileSystem, size_t* out_sizeEeprom)
{
#if COMPILE_BACKUP
	if (config.device.autoBackupMode == (ConfigAutobackupMode_t)ConfigAutobackupMode::AUTOBACKUP_OFF) return 0;

	DEBUG_LOG_LN("Auto-saving backup config...");

	if (status.config.backup.backupsDisabled)
	{
		DEBUG_LOG_LN("...Backups are disabled.");
		return false;
	}

	bool fs = config.device.autoBackupMode == (ConfigAutobackupMode_t)ConfigAutobackupMode::AUTOBACKUP_MODE_FS || config.device.autoBackupMode == (ConfigAutobackupMode_t)ConfigAutobackupMode::AUTOBACKUP_FS_AND_EEPROM;
	bool eeprom = config.device.autoBackupMode == (ConfigAutobackupMode_t)ConfigAutobackupMode::AUTOBACKUP_MODE_EEPROM || config.device.autoBackupMode == (ConfigAutobackupMode_t)ConfigAutobackupMode::AUTOBACKUP_FS_AND_EEPROM;

	return SaveBackupConfig(fs, eeprom, true, out_sizeEeprom, out_sizeFileSystem);
#endif
}

size_t Config::Backup::SaveBackupEeprom(bool saveRetained)
{
#if COMPILE_BACKUP
	DEBUG_LOG_LN("Saving config backup to EEPROM...\r\n...Creating json document...");

	if (status.config.backup.backupsDisabled)
	{
		DEBUG_LOG_LN("...Backups are disabled.");
		return false;
	}

	if (!status.device.eepromMounted)
	{
		DEBUG_LOG_LN("...Backup cannot be saved as the EEPROM is not mounted.");
		return false;
	}

	if (!status.config.backup.eepromBackedUp /*&& statusRetained.fileSizes.eepromBackup*/)
	{
		DEBUG_LOG_LN("...EEPROM backup already saved.");
		return true;
	}

	DEBUG_LOG_LN("...Creating json document...");
	DynamicJsonDocument serializeConfigDoc(DOC_CONFIG_SERIALIZE_SIZE);
	if(Documents::SerializeConfig(&serializeConfigDoc))
		return SaveBackupEeprom(&serializeConfigDoc, saveRetained);

	return 0;
#endif
}

size_t Config::Backup::SaveBackupEeprom(JsonDocument* doc, bool saveRetained, const  uint32_t crc)
{
#if COMPILE_BACKUP
	if (status.config.backup.backupsDisabled)
	{
		DEBUG_LOG_LN("Backups are disabled.\r\n");
		return false;
	}

	if (!status.device.eepromMounted)
	{
		DEBUG_LOG_LN("Backup cannot be saved as the EEPROM is not mounted.\r\n");
		return false;
	}

	if (doc == nullptr)
	{
		DEBUG_LOG_LN("JsonDocument nullptr!\r\n");
		return 0;
	}

	if (status.config.backup.eepromBackedUp /*&& statusRetained.fileSizes.eepromBackup*/)
	{
		DEBUG_LOG_LN("EEPROM backup already saved.\r\n");
		return 1;
	}

	size_t serializedSize = 0;

	if(status.config.backup.filesystemBackedUp)



	DEBUG_LOG("Serializing backup to EEPROM...");
	EepromStream eepromStream(sizeof(StatusRetained_t), DOC_CONFIG_SERIALIZE_SIZE);

	uint32_t docCRC = crc > 0 ? crc : FileManager::GetSerializedCRC(*doc);

	if (docCRC == statusRetained.crcs.eepromBackupFile)
	{
		DEBUG_LOG_LN("CRC Matching, Already backed up.\r\n");

		return 1;	//CRC matches, no need to save.
	}

	size_t size = serializeJson(*doc, eepromStream);

	#warning testing : check if CRC is failing or the backup
#pragma region Test
		DEBUG_LOG_LN("FS Test start");
		doc->clear();
		DeserializeEepromBackupConfig(*doc);

		String test;
		serializeJson(*doc, test);
		DEBUG_LOG_LN(test);

		Config_t fsConfig;		
		convertFromJson(*doc, fsConfig);
		if (memcmp(&config, &fsConfig, sizeof(config)) == 0)
		{
			DEBUG_LOG_LN("Backup matches");
		}
		else
		{
			DEBUG_LOG_LN("Backup Doesn't match");
		}

		doc->clear();
		if (!FileManager::OpenAndParseFile(status.config.path, *doc))
		{
			DEBUG_LOG_LN("Could not open backup file for writing!\r\n");
			return 0;
		}
#pragma endregion

	if (size)
	{
		//Get Saved CRC to validate (starts after StatusRetained)
		eepromStream = EepromStream(sizeof(StatusRetained_t), size);
		uint32_t crc = FileManager::GetEepromCRC(eepromStream, size);

		if (crc != docCRC)
		{
			DEBUG_LOG_F("Failed!\r\nSaved File Corrupted! CRC does not match Document CRC!\r\nFile : %0000000X\r\nDoc : %0000000X\r\n", crc, docCRC);
			size = 0;
			crc = 0;
		}

		DEBUG_LOG_F("...Backup saved to EEPROM. Size : %d bytes\r\n", size);
		status.config.backup.eepromBackedUp = true;
		statusRetainedMonitor.crcs.recentBackup = statusRetained.crcs.recentBackup != crc;
		statusRetainedMonitor.crcs.eepromBackupFile = statusRetained.crcs.eepromBackupFile != crc;
		statusRetainedMonitor.fileSizes.recentBackup = statusRetained.fileSizes.recentBackup != size;
		statusRetainedMonitor.fileSizes.eepromBackup = statusRetained.fileSizes.eepromBackup != size;
		statusRetained.crcs.recentBackup = crc;
		statusRetained.crcs.eepromBackupFile = crc;
		statusRetained.fileSizes.recentBackup = size;
		statusRetained.fileSizes.eepromBackup = size;

		if (saveRetained)
			Status::SaveRetainedStatus();
	}
	else
	{
		DEBUG_LOG_LN("Saving backup to EEPROM failed. Size 0.\r\n");
	}

	return size;
#endif
}

size_t Config::Backup::SaveBackupFilesystem(bool saveRetained)
{
#if COMPILE_BACKUP
	DEBUG_LOG_LN("Saving config backup to File System...");

	if (status.config.backup.backupsDisabled)
	{
		DEBUG_LOG_LN("...Backups are disabled.");
		return false;
	}

	if (status.config.backup.filesystemBackedUp /*&& statusRetained.fileSizes.fileSystemBackup*/)
	{
		DEBUG_LOG_LN("...File System backup already saved.");
		return true;
	}

	if (!status.device.fsMounted)
	{
		DEBUG_LOG_LN("...Backup cannot be saved as the file system is not mounted.");
		return false;
	}
	
	DEBUG_LOG_LN("...Creating json document...");
	/*StaticJsonDocument<DOC_CONFIG_SERIALIZE_SIZE> serializeConfigDoc;
	if (SerializeConfig(&serializeConfigDoc))
		return SaveBackupFilesystem(&serializeConfigDoc, saveRetained);*/

	if (Documents::SerializeConfig(&configDoc))
		return SaveBackupFilesystem(&configDoc, saveRetained);
	
	return 0;
#endif
}

size_t Config::Backup::SaveBackupFilesystem(JsonDocument* doc, bool saveRetained, const uint32_t crc)
{
#if COMPILE_BACKUP
	if (status.config.backup.backupsDisabled)
	{
		DEBUG_LOG_LN("Backups are disabled.\r\n");
		return false;
	}

	if (!status.device.fsMounted)
	{
		DEBUG_LOG_LN("Backup cannot be saved as the file system is not mounted.\r\n");
		return false;
	}

	if (doc == nullptr)
	{
		DEBUG_LOG_LN("JsonDocument nullptr!\r\n");
		return 0;
	}

	if (status.config.backup.filesystemBackedUp /*&& statusRetained.fileSizes.fileSystemBackup*/ /*&& statusRetained.crcs.recentBackup == statusRetained.crcs.fileSystemBackupFile*/)
	{
		DEBUG_LOG_LN("File System backup already saved.\r\n");
		return 1;
	}

	DEBUG_LOG("Serializing backup to File System...");

	//Check CRC. If matches do not save.
	uint32_t docCRC = crc > 0 ? crc : FileManager::GetSerializedCRC(*doc);


	if (docCRC == statusRetained.crcs.fileSystemBackupFile)
	{
		DEBUG_LOG_LN("CRC Matching, Already backed up.\r\n");
		return 1;	//CRC matches, no need to save.
	}

	File file;

	/*File file;*/
	String backupPath;
	if (!Documents::GenerateBackupPath(status.config.path, &backupPath))
	{
		DEBUG_LOG_LN("Failed to generate backup path.\r\n");
		return 0;
	}

	if (!FileManager::OpenFile(&file, backupPath.c_str(), "w+"))
	{
		DEBUG_LOG_LN("Could not open backup file for writing!\r\n");
		return 0;
	}

	size_t size = serializeJson(*doc, file);
	//file.close();

#warning testing : check if CRC is failing or the backup
#pragma region Test
DEBUG_LOG_LN("FS Test start");
	doc->clear();
	file.seek(0);



	Config_t fsConfig;
	FileManager::ParseFile(&file, *doc, status.config.path, true);
	String test;
	serializeJson(*doc, test);
	DEBUG_LOG_LN(test);

	convertFromJson(*doc, fsConfig);
	if (memcmp(&config, &fsConfig, sizeof(config)) == 0)
	{
		DEBUG_LOG_LN("Backup matches");
	}
	else 
	{
		DEBUG_LOG_LN("Backup Doesn't match");
	}

	doc->clear();
	if (!FileManager::OpenAndParseFile(status.config.path, *doc))
	{
		DEBUG_LOG_LN("Could not open backup file for writing!\r\n");
		return 0;
	}
#pragma endregion
	

	DEBUG_LOG_F("...FS Backup serialized and saved...Size : %dbytes...Checking CRC...", size);

	if (size)
	{
		//Get Saved CRC to validate
		uint32_t crc = FileManager::GetFileCRC(backupPath.c_str());

		if (crc != docCRC)
		{
			DEBUG_LOG_F("Failed!\r\nSaved File Corrupted! CRC does not match Document CRC!\r\nFile : %0000000X\r\nDoc : %0000000X\r\n", crc, docCRC);
			size = 0;
			crc = 0;
		}

		DEBUG_LOG_F("Backup saved to File System. Size : %d bytes\r\n", size);
		status.config.backup.filesystemBackedUp = true;
		statusRetainedMonitor.crcs.recentBackup = statusRetained.crcs.recentBackup != crc;
		statusRetainedMonitor.crcs.fileSystemBackupFile = statusRetained.crcs.fileSystemBackupFile != crc;
		statusRetainedMonitor.fileSizes.recentBackup = statusRetained.fileSizes.recentBackup != size;
		statusRetainedMonitor.fileSizes.fileSystemBackup = statusRetained.fileSizes.fileSystemBackup != size;
		statusRetained.crcs.recentBackup = crc;
		statusRetained.crcs.fileSystemBackupFile = crc;
		statusRetained.fileSizes.recentBackup = size;
		statusRetained.fileSizes.fileSystemBackup = size;
	}
	else
	{
		DEBUG_LOG_LN("...Saving backup to File System failed. Size 0.");
	}

	if (saveRetained)
		Status::SaveRetainedStatus();

	return size;
#endif
}

bool Config::Backup::DeserializeEepromBackupConfig(JsonDocument& doc)
{
#if COMPILE_BACKUP
	/*DEBUG_LOG_LN("Reading Config Backup from EEPROM...");
	EEPROM.get(0, settings);
	DEBUG_LOG_LN("Read Complete.");*/

	DEBUG_LOG_LN("Reading and deserializing config backup from EEPROM...");

	EepromStream eepromStream(sizeof(StatusRetained_t), DOC_CONFIG_DESERIALIZE_SIZE);
	eepromStream.setTimeout(FILE_STREAM_TIMEOUT);

	DeserializationError derror = deserializeJson(doc, eepromStream);

	if (derror)
	{
		DEBUG_LOG("Error parsing backup config : ");
		DEBUG_LOG_LN(derror.c_str());
		configDoc.clear();
		return false;
	}
	else
	{
		DEBUG_LOG_LN("Backup config read and parsed.");
		status.config.configRead = true;
	}

	return true;
#endif
}

/// <summary>
/// Allow backups again.
/// Must be manually set by the user.
/// </summary>
void Config::Backup::DisableBackups()
{
#if COMPILE_BACKUP
	DEBUG_LOG_LN("Auto-backup disabled. To enable, login to the ESP in a browser.");
	status.config.backup.backupsDisabled = true;
#endif
}


/// <summary>
/// Allow backups again.
/// Must be manually set by the user.
/// </summary>
void Config::Backup::EnableBackups()
{
#if COMPILE_BACKUP
	DEBUG_LOG_LN("Auto-backup enabled.");
	status.config.backup.backupsDisabled = false;
#endif
}

void Config::Backup::SetBackupFlags(uint32_t crc)
{
#if COMPILE_BACKUP
	status.config.backup.eepromBackedUp = crc == statusRetained.crcs.eepromBackupFile && statusRetained.fileSizes.eepromBackup > 0 && statusRetained.crcs.eepromBackupFile != statusRetained.crcs.recentBackup;
	status.config.backup.filesystemBackedUp = crc == statusRetained.crcs.fileSystemBackupFile && statusRetained.fileSizes.fileSystemBackup > 0 && statusRetained.crcs.fileSystemBackupFile != statusRetained.crcs.recentBackup;
#endif
}


#pragma endregion




void Config::ApplySettings()
{
	#warning  wip

	EspSense::RestartDevice();
}

/// <summary>
///  Restore settings to default. Overwrites config.json, and eeprom.
/// *Wifi and MQTT broker settings will not be overwritten if connected.
/// </summary>
/// <param name="restart">Restart device?</param>
/// /// <param name="keepConnectionSettings">If Wifi or MQTT is connected, it will keep the settings.</param>
void Config::FactoryReset(bool format, bool restart, bool keepConnectionSettings, bool overwriteConfig)
{
	DEBUG_LOG_LN("Performing factory reset.");

	//Config

	WifiConfig_t wifi = config.wifi;
	MqttBrokerConfig_t mqttBroker = config.mqtt.broker;

	Defaults::SetAll();

	if (keepConnectionSettings)
	{
		if (status.wifi.station.connected || status.wifi.accessPoint.ipAssigned)
		{
			config.wifi.channel = wifi.channel;
			//config.wifi.powerLevel = wifi.powerLevel;
		}

		if (status.wifi.station.connected)
		{
			//Wifi is connected, keep the SSID and password.
			config.wifi.station.ssid = wifi.station.ssid;
			config.wifi.station.pass = wifi.station.pass;			
		}

		if (status.wifi.accessPoint.ipAssigned)
		{
			//Access Point is connected, keep the SSID and password.
			config.wifi.accessPoint.ssid = wifi.accessPoint.ssid;
			config.wifi.accessPoint.pass = wifi.accessPoint.pass;
		}

		if (status.mqtt.connected && !mqttBroker.autoDetectIp)
		{
			//MQTT is connected, keep these connection settings.
			config.mqtt.broker.ip = mqttBroker.ip;
			//memset(config.mqtt.broker.ip, 0, sizeof(config.mqtt.broker.ip));
			//strlcpy(config.mqtt.broker.ip, mqttBroker.ip, sizeof(config.mqtt.broker.ip));
			config.mqtt.broker.user = mqttBroker.user;
			config.mqtt.broker.pass = mqttBroker.pass;
		}
	}

	//Need to implement saving of other config files first.
	//if (format)
	//	ESP_FS.format();

#if COMPILE_BACKUP
	Backup::AutoSaveBackupConfig();
#endif

	if (overwriteConfig)
		Documents::SaveConfig();

	//status.bootSettings.freshBoot = true;
	//ChangeBootSource(ConfigSource::CFG_DEFAULT);

	DEBUG_LOG_LN("Default Settings Recovered. Restarting device...");

	EspSense::RestartDevice();
}

void Config::ChangeBootSource(enum ConfigSource source, bool saveRetained)
{
	statusRetainedMonitor.boot.bootSource = statusRetained.boot.bootSource != source;
	statusRetained.boot.bootSource = source;

	if(saveRetained)
		Status::SaveRetainedStatus();
}


#pragma region Status

int Config::Status::PackDeviceStatus(JsonObject& doc)
{
	JsonObject statusObj = doc.createNestedObject("status");

	statusObj["status"].set(status);
	statusObj["statusRetained"].set(statusRetained);

	return 0;
}


bool Config::Status::SaveRetainedStatus()
{
	if (!status.device.eepromMounted)
	{
		DEBUG_LOG_LN("Cannot Save Retained Status : EEPROM Not Mounted!");
		return false;
	}

	//if (status.disableRetainCommits) return;

	//Settings have not been changed
	if (!statusRetainedMonitor.bitmap) return true;

	EEPROM.put(0, statusRetained);
	if (!EEPROM.commit())
	{
		DEBUG_LOG_LN("Cannot Save Retained Status : EEPROM Commit Failed!");
		return false;
	}

	status.config.saveRetainedLoop = true;

	//Blink hotspot LED
	if (config.wifi.accessPoint.ledGpio != -1)
	{
		//Make sure number is even so that previous state is recovered
		for (uint8_t i = 0; i < 10; i++)
		{
			LedToggle(config.wifi.accessPoint.ledGpio);
			EspSense::YieldWatchdog(250);
		}
	}

	statusRetainedMonitor.bitmap = 0;
	return true;
}

bool Config::Status::SetRetainedConfigPath(bool saveRetained)
{
#warning unfinished
	//statusRetainedMonitor.lastConfigPath = strcmp(statusRetained.lastConfigPath, status.config.path) != 0;
	//memset(statusRetained.lastConfigPath, 0, CONFIG_PATH_MAX_LENGTH);
	//strlcpy(statusRetained.lastConfigPath, status.config.path, CONFIG_PATH_MAX_LENGTH);

	//if (saveRetained)
	//	SaveRetainedStatus();
}

#pragma endregion




