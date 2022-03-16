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
extern DeviceStatus_t status;
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
	config.mqtt.publish.unknownPayload = WIFI_SSID;
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
		configMonitor.mqtt.publish.unknownPayload = config.mqtt.publish.unknownPayload != SENSOR_DATA_UNKNOWN;

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
	config.mqtt.publish.unknownPayload = SENSOR_DATA_UNKNOWN;

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
/// Checks to see if the config's CRC matches the previous boot.
/// </summary>
/// <returns>1 if the file has been updated or if it is a different file, 0 if there has been no changes, and -1 if the file does not exist.</returns>
int Config::Documents::CheckConfigCrc()
{
	uint32_t result = FileManager::GetFileCRC(status.config.path);

	if (result == 0) return -1;

	statusRetainedMonitor.crcs.configFile = statusRetained.crcs.configFile != result;
	statusRetained.crcs.configFile = result;

	Backup::SetBackupFlags(result);	

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
		status.backup.backupsDisabled = !status.device.freshBoot;
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
		if (!status.backup.filesystemBackedUp)
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
			if (status.config.configSource == ConfigSource::CFG_BACKUP_FILESYSTEM || !status.backup.filesystemBackedUp)
			{
				if (status.backup.eepromBackedUp)
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
		status.backup.backupsDisabled = true;

		#if COMPILE_BACKUP
			//Config failed, try backup or EEPROM.
			if (status.backup.filesystemBackedUp)
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
				if (status.backup.eepromBackedUp) {
					DEBUG_LOG_LN("...Getting config from EEPROM...");
					status.config.configSource = ConfigSource::CFG_EEPROM;
					if (!Backup::DeserializeEepromBackupConfig())
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

	if (!status.storage.fsMounted)
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
	CheckConfigCrc();
	
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

//bool SerializeSerialDatabits(JsonVariant* obj, uint32_t portConfigBitmap)
//{
//	if (obj == nullptr) return false;
//
//	UART_DATABITS databits = (UART_DATABITS)(portConfigBitmap & UART_CONFIG_DATABITS_MASK);
//
//	switch (databits)
//	{
//	case DATABITS_5:
//		(*obj)["dataBits"] = 5;
//		break;
//	case DATABITS_6:
//		(*obj)["dataBits"] = 6;
//		break;
//	case DATABITS_7:
//		(*obj)["dataBits"] = 7;
//		break;
//	case DATABITS_8:
//		(*obj)["dataBits"] = 8;
//		break;
//	default: return false;
//	}
//	return true;
//}
//
//bool SerializeSerialParity(JsonVariant* obj, uint32_t portConfigBitmap)
//{
//	if (obj == nullptr) return false;
//
//	UART_PARITY parity = (UART_PARITY)(portConfigBitmap & UART_CONFIG_PARITY_MASK);
//
//	switch (parity)
//	{
//	case PARITY_NONE:
//		(*obj)["parity"] = "none";
//		break;
//	case PARITY_EVEN:
//		(*obj)["parity"] = "even";
//		break;
//	case PARITY_ODD:
//		(*obj)["parity"] = "odd";
//		break;
//	default: return false;
//	}
//	return true;
//}
//
//bool SerializeSerialStopbits(JsonVariant* obj, uint32_t portConfigBitmap)
//{
//	if (obj == nullptr) return false;
//
//	UART_STOPBITS stopbits = (UART_STOPBITS)(portConfigBitmap & UART_CONFIG_STOPBITS_MASK);
//
//	switch (stopbits)
//	{
//	case STOPBITS_1:
//		(*obj)["stopBits"] = 1;
//		break;
//	case STOPBITS_2:
//		(*obj)["stopBits"] = 2;
//		break;
//	default: return false;
//	}
//	return true;
//}

bool Config::Documents::SerializeConfig(JsonDocument* doc)
{
	if (doc == nullptr) return false;

	doc->clear();

	JsonObject wifi = doc->createNestedObject("wifi");
	wifi["useDefaults"] = config.wifi.useDefaults;
	wifi["channel"] = config.wifi.channel;
	wifi["power"].set((WifiPower)config.wifi.powerLevel);
	//wifi["mode"] = (uint8_t)config.wifi.mode;

	JsonObject station = wifi.createNestedObject("station");
	station["enabled"] = config.wifi.station.enabled;
	station["ssid"] = config.wifi.station.ssid;
	station["pass"] = config.wifi.station.pass;
	station["ledGpio"] = config.wifi.station.ledGpio;
	station["ledOn"] = config.wifi.station.ledOn;


	JsonObject wifi_hotspot = wifi.createNestedObject("accessPoint");
	wifi_hotspot["useDefaults"] = config.wifi.accessPoint.useDefaults;
	wifi_hotspot["enabled"] = config.wifi.accessPoint.enabled;
	wifi_hotspot["configOnly"] = config.wifi.accessPoint.configOnly;
	wifi_hotspot["ssid"] = config.wifi.accessPoint.ssid;
	wifi_hotspot["pass"] = config.wifi.accessPoint.pass;
	wifi_hotspot["hidden"] = config.wifi.accessPoint.hidden;
	wifi_hotspot["maxConnections"] = config.wifi.accessPoint.maxConnections;
	wifi_hotspot["buttonGpio"] = config.wifi.accessPoint.buttonGpio;
	wifi_hotspot["buttonPullup"] = config.wifi.accessPoint.buttonPullup;
	wifi_hotspot["buttonPress"] = config.wifi.accessPoint.buttonPress;
	wifi_hotspot["holdtime"] = config.wifi.accessPoint.holdTime;
	wifi_hotspot["ledGpio"] = config.wifi.accessPoint.ledGpio;
	wifi_hotspot["ledOn"] = config.wifi.accessPoint.ledOn;

#if DEVELOPER_MODE
	wifi["taskSettings"].set<TaskConfig_t>(config.wifi.taskSettings);
#endif

	JsonObject mqtt = doc->createNestedObject("mqtt");
	mqtt["useDefaults"] = config.mqtt.useDefaults;
	mqtt["ledGpio"] = config.mqtt.ledGpio;
	mqtt["ledOn"] = config.mqtt.ledOn;

	JsonObject mqtt_publish = mqtt.createNestedObject("publish");
	mqtt_publish["bufferSize"] = config.mqtt.publish.bufferSize;
	mqtt_publish["rate"] = config.mqtt.publish.rate / 1000;
	mqtt_publish["errorRate"] = config.mqtt.publish.errorRate / 1000;
	mqtt_publish["availabilityRate"] = config.mqtt.publish.availabilityRate / 1000;
	mqtt_publish["json"] = config.mqtt.publish.json;
	mqtt_publish["asIndividualTopics"] = config.mqtt.publish.onIndividualTopics;
	mqtt_publish["unknownPayload"] = config.mqtt.publish.unknownPayload;

	JsonObject mqtt_broker = mqtt.createNestedObject("broker");
	mqtt_broker["timeout"] = config.mqtt.broker.timeout;
	mqtt_broker["autoDetectIP"] = config.mqtt.broker.autoDetectIp;
	//mqtt_broker["wifiMode"] = (uint8_t)config.mqtt.broker.wifiMode;
	mqtt_broker["wifiMode"].set((WifiMode)config.mqtt.broker.wifiMode);
	mqtt_broker["autoTimeout"] = config.mqtt.broker.autoTimeout;
	mqtt_broker["attemptRate"] = config.mqtt.broker.connectInterval;
	mqtt_broker["maxRetries"] = config.mqtt.broker.maxRetries;
	mqtt_broker["autoMaxRetries"] = config.mqtt.broker.autoMaxRetries;
	mqtt_broker["ip"].set(config.mqtt.broker.ip);
	mqtt_broker["ipAP"].set(config.mqtt.broker.ipAP);

	//mqtt_broker["ip"] = config.mqtt.broker.ip;
	mqtt_broker["port"] = config.mqtt.broker.port;
	mqtt_broker["user"] = config.mqtt.broker.user;
	mqtt_broker["pass"] = config.mqtt.broker.pass;
	
	
	

	JsonObject mqtt_baseTopics = mqtt.createNestedObject("baseTopics");
	mqtt_baseTopics["base"] = config.mqtt.baseTopics.base;
	mqtt_baseTopics["availability"] = config.mqtt.baseTopics.availability;
	mqtt_baseTopics["jCommand"] = config.mqtt.baseTopics.jsonCommand;
	mqtt_baseTopics["jState"] = config.mqtt.baseTopics.jsonState;
	mqtt_baseTopics["command"] = config.mqtt.baseTopics.command;
	mqtt_baseTopics["state"] = config.mqtt.baseTopics.state;

	JsonObject mqtt_topics = mqtt.createNestedObject("topics");
	mqtt_topics["availability"] = config.mqtt.topics.availability;
	mqtt_topics["jCommand"] = config.mqtt.topics.jsonCommand;
	mqtt_topics["jState"] = config.mqtt.topics.jsonState;

	JsonObject device = doc->createNestedObject("device");
	device["useDefaults"] = config.device.useDefaults;
	device["autoBackupMode"].set((ConfigAutobackupMode)config.device.autoBackupMode);
	//device["autoBackupMode"] = (uint8_t)config.device.autoBackupMode;
	//device["crcOnBackup"] = config.device.crcOnBackup;
	

	JsonObject device_serial = device.createNestedObject("serial");
	device_serial["useDefaults"] = config.device.serial.useDefaults;
	device_serial["messagePort"] = config.device.serial.messagePort;
	device_serial["debugPort"] = config.device.serial.debugPort;

	JsonObject device_serial_port0 = device_serial.createNestedObject("port0");
	device_serial_port0["useDefaults"] = config.device.serial.port0.useDefaults;
	//device_serial_port0["enabled"] = config.device.serial.port0.enabled;
	device_serial_port0["baud"] = config.device.serial.port0.baud;
	device_serial_port0["timeout"] = config.device.serial.port0.timeout;

	JsonObject device_serial_port0_config = device_serial_port0.createNestedObject("config");
	//device_serial_port0_config["bitmap"] = config.device.serial.port0.config.bitmap;

	JsonVariant device_serial_port0_config_databits = device_serial_port0_config.createNestedObject("dataBits");
	JsonVariant device_serial_port0_config_parity = device_serial_port0_config.createNestedObject("parity");
	JsonVariant device_serial_port0_config_stopbits = device_serial_port0_config.createNestedObject("stopBits");

	device_serial_port0_config_databits.set<UART_DATABITS>((UART_DATABITS)(config.device.serial.port0.config & UART_CONFIG_DATABITS_MASK));
	device_serial_port0_config_parity.set<UART_PARITY>((UART_PARITY)(config.device.serial.port0.config & UART_CONFIG_PARITY_MASK));
	device_serial_port0_config_stopbits.set<UART_STOPBITS>((UART_STOPBITS)(config.device.serial.port0.config & UART_CONFIG_STOPBITS_MASK));

	//SerializeSerialDatabits(&device_serial_port0_config_databits, config.device.serial.port0.config & UART_CONFIG_DATABITS_MASK);
	//SerializeSerialParity(&device_serial_port0_config_parity, config.device.serial.port0.config & UART_CONFIG_PARITY_MASK);
	//SerializeSerialStopbits(&device_serial_port0_config_stopbits, config.device.serial.port0.config & UART_CONFIG_STOPBITS_MASK);

	JsonObject device_serial_port1 = device_serial.createNestedObject("port1");
	device_serial_port1["useDefaults"] = config.device.serial.port1.useDefaults;
	device_serial_port1["enabled"] = config.device.serial.port1.enabled;
	device_serial_port1["baud"] = config.device.serial.port1.baud;
	device_serial_port1["rxGpio"] = config.device.serial.port1.rxGpio;
	device_serial_port1["txGpio"] = config.device.serial.port1.txGpio;
	device_serial_port1["invert"] = config.device.serial.port1.invert;
	device_serial_port1["timeout"] = config.device.serial.port1.timeout;
	//device_serial_port1["config"] = config.device.serial.port1.config.bitmap;

	JsonObject device_serial_port1_config = device_serial_port1.createNestedObject("config");
	//device_serial_port1_config["bitmap"] = config.device.serial.port1.config.bitmap;

	JsonVariant device_serial_port1_config_databits = device_serial_port1_config.createNestedObject("dataBits");
	JsonVariant device_serial_port1_config_parity = device_serial_port1_config.createNestedObject("parity");
	JsonVariant device_serial_port1_config_stopbits = device_serial_port1_config.createNestedObject("stopBits");

	device_serial_port1_config_databits.set((UART_DATABITS)(config.device.serial.port1.config& UART_CONFIG_DATABITS_MASK));
	device_serial_port1_config_parity.set((UART_DATABITS)(config.device.serial.port1.config& UART_CONFIG_DATABITS_MASK));
	device_serial_port1_config_stopbits.set((UART_DATABITS)(config.device.serial.port1.config& UART_CONFIG_DATABITS_MASK));

	//SerializeSerialDatabits(&device_serial_port1_config_databits, config.device.serial.port1.config);
	//SerializeSerialParity(&device_serial_port1_config_parity, config.device.serial.port1.config);
	//SerializeSerialStopbits(&device_serial_port1_config_stopbits, config.device.serial.port1.config);


	JsonObject device_i2c = device.createNestedObject("i2c");
	device_i2c["useDefaults"] = config.device.i2c.useDefaults;
	device_i2c["enabled"] = config.device.i2c.enabled;
	device_i2c["sclGpio"] = config.device.i2c.sclGpio;
	device_i2c["sdaGpio"] = config.device.i2c.sdaGpio;
	device_i2c["freq"] = config.device.i2c.freq;


	JsonObject server = doc->createNestedObject("server");
	server["useDefaults"] = config.server.useDefaults;
	server["dns"] = config.server.dns;
	server["hostname"] = config.server.hostname;
	server["authenticate"] = config.server.authenticate;
	server["user"] = config.server.user;
	server["pass"] = config.server.pass;
	server["sessionTimeout"] = config.server.sessionTimeout;

#if COMPILE_FTP
	JsonObject ftp = server.createNestedObject("ftp");
	ftp["useDefaults"] = config.server.ftp.useDefaults;
	ftp["enabled"] = config.server.ftp.enabled;
	ftp["anonymous"] = config.server.ftp.anonymous;
	ftp["user"] = config.server.ftp.user;
	ftp["pass"] = config.server.ftp.pass;
	ftp["timeout"] = config.server.ftp.timeout;

#if DEVELOPER_MODE
	ftp["taskSettings"].set<TaskConfig_t>(config.server.ftp.taskSettings);
#endif
#endif
	
#if COMPILE_OTA
	JsonObject server_ota = server.createNestedObject("ota");
	server_ota["useDefaults"] = config.server.ota.useDefaults;
	server_ota["enabled"] = config.server.ota.enabled;

#if DEVELOPER_MODE
	server_ota["taskSettings"].set<TaskConfig_t>(config.server.ota.taskSettings);
#endif
#endif

	JsonObject server_browser = server.createNestedObject("browser");
	server_browser["useDefaults"] = config.server.browser.useDefaults;
	server_browser["enabled"] = config.server.browser.enabled;
	server_browser["config"] = config.server.browser.config;
	server_browser["console"] = config.server.browser.console;
	server_browser["updater"] = config.server.browser.updater;
	server_browser["mqttDevices"] = config.server.browser.mqttDevices;
	server_browser["ssl"] = config.server.browser.ssl;

#if COMPILE_BROWSER_TOOLS
	JsonObject server_browserTools = server_browser.createNestedObject("tools");
	server_browserTools["fileEditor"] = config.server.browser.tools.fileEditor;
	server_browserTools["jsonVerify"] = config.server.browser.tools.jsonVerify;
#endif

	return true;
}


size_t Config::Documents::SaveConfig()
{
	//#warning  wip

	DEBUG_LOG_LN("Saving config...");

	if (!status.storage.fsMounted)
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



bool Config::Documents::SetConfigFromDoc()
{
	DEBUG_LOG_LN("Setting config to values from JSON document...");

	if (!status.config.configRead)
	{
		DEBUG_LOG_LN("Configuration cannot be set. Config Document has not been deserialized.");
		return false;
	}

	//Set required data to true.
	//If we are missing data it will be reset to false.
	status.config.hasRequiredData = true;

	if (configBitmap.device)
		SetDeviceFromDoc();

	if (configBitmap.wifi)
		SetWifiFromDoc();

	if (configBitmap.mqtt)
		SetMqttFromDoc();
	//SetDocFromDoc();

#if COMPILE_FTP
	if (configBitmap.ftp)
		SetFtpFromDoc();
#endif

#if COMPILE_SERVER
	if (configBitmap.server)
		SetServerFromDoc();
#endif

	configDoc.clear();
	status.config.configRead = false;

	DEBUG_LOG_F("SSID : %s\r\nPASS : %s\r\nMQTT IP : %s\r\nPort : %d\r\nUser : %s\r\nPass : %s\r\n", config.wifi.station.ssid.c_str(), config.wifi.station.pass.c_str(), config.mqtt.broker.ip.toString().c_str(), config.mqtt.broker.port, config.mqtt.broker.user.c_str(), config.mqtt.broker.pass.c_str());

	return status.config.settingsConfigured = true;
}


bool Config::Documents::SetDeviceFromDoc()
{
	DEBUG_LOG_LN("Setting Device Settings...");

	if (!configDoc.containsKey("device"))
	{
		missing_key("device");
		return false;
	}

	JsonObject deviceObj = configDoc["device"];

	if (deviceObj.containsKey("useDefaults"))
		config.device.useDefaults = deviceObj["useDefaults"];
			//goto ReadSerial;


	if (deviceObj.containsKey("autoBackupMode"))
	{
#warning JSON UDF
		JsonVariantConst autobackupObj = deviceObj["autoBackupMode"];
		config.device.autoBackupMode = (ConfigAutobackupMode_t)(autobackupObj.as<ConfigAutobackupMode>());

		//config.device.autoBackupMode = deviceObj["autoBackupMode"].as<ConfigAutobackupMode_t>();
/*		JsonObject autobackupObj = deviceObj["autoBackupMode"];

		int8_t abmint = deviceObj["autoBackupMode"];

		if (abmint >= 0 && abmint <= 3)
			config.device.autoBackupMode = (ConfigAutobackupMode_t)abmint;*/
	}


	//if (deviceObj.containsKey("crcOnBackup"))
	//	if (config.device.crcOnBackup = deviceObj["crcOnBackup"])

ReadSerial:

	if (deviceObj.containsKey("serial"))
	{
		DEBUG_LOG_LN("Setting Device Serial Settings...");
		JsonObject serialObj = deviceObj["serial"];

		if (serialObj.containsKey("useDefaults"))
			config.device.serial.useDefaults = serialObj["useDefaults"];
				//goto ReadPorts;

		if (serialObj.containsKey("messagePort"))
			config.device.serial.messagePort = serialObj["messagePort"];

		if (serialObj.containsKey("debugPort"))
			config.device.serial.debugPort = serialObj["debugPort"];

	ReadPorts:

		if (serialObj.containsKey("port0"))
		{
			JsonObject port0Obj = serialObj["port0"];
			Parsers::ParseSerialPort(port0Obj, config.device.serial.port0, configMonitor.device.serial.port0);
		}

		if (serialObj.containsKey("port1"))
		{
			JsonObject port1Obj = serialObj["port1"];
			Parsers::ParseSerialPort(port1Obj, config.device.serial.port1, configMonitor.device.serial.port1);
		}
	}


	if (deviceObj.containsKey("i2c"))
	{
		DEBUG_LOG_LN("Setting Device I2C Settings...");

		JsonObject i2cObj = deviceObj["i2c"];

		if (i2cObj.containsKey("useDefaults"))
			config.device.i2c.useDefaults = i2cObj["useDefaults"];
				//return true;


		if (i2cObj.containsKey("enabled"))
			config.device.i2c.enabled = i2cObj["enabled"];

		if (i2cObj.containsKey("sclGpio"))
		{
			int8_t gpio = i2cObj["sclGpio"];
			if (IsGpioValidPin(gpio, GPIOV_I2C))
			{
				configMonitor.device.i2c.sclGpio = config.device.i2c.sclGpio != gpio;
				config.device.i2c.sclGpio = gpio;
			}
		}

		if (i2cObj.containsKey("sdaGpio"))
		{
			int8_t gpio = i2cObj["sdaGpio"];
			if (IsGpioValidPin(gpio, GPIOV_I2C))
			{
				configMonitor.device.i2c.sdaGpio = config.device.i2c.sdaGpio != gpio;
				config.device.i2c.sdaGpio = gpio;
			}
		}

		//Mqtt::ReadKey(config.device.i2c.freq, i2cObj, "freq");
		if (i2cObj.containsKey("freq"))
			config.device.i2c.freq = i2cObj["freq"];
	}

	return true;
}

bool Config::Documents::SetWifiFromDoc()
{
	if (!configDoc.containsKey("wifi"))
	{
		missing_required_key("wifi");

		if (config.wifi.station.ssid.length() == 0 || config.wifi.station.pass.length() == 0)
		{
			DEBUG_LOG_LN("Default settings do not contain SSID or Password. Will not be able to connect unless EEPROM has backup config.");
		}
		return false;
	}


	JsonObject wifiObj = configDoc["wifi"];

	if (wifiObj.containsKey("useDefaults"))
		config.wifi.useDefaults = wifiObj["useDefaults"];

	////Using defaults. Don't read the rest.
	//if (config.wifi.useDefaults)
	//	goto ReadStationSettings;


	if (wifiObj.containsKey("channel"))
		config.wifi.channel = wifiObj["channel"];

	if (wifiObj.containsKey("power"))
	{
#warning JSON UDF
		JsonVariantConst powerVar = wifiObj["power"];
		config.wifi.powerLevel = (wifi_power_t)(powerVar.as<WifiPower>());
	}

ReadStationSettings:
	JsonObject stationObj = wifiObj["station"];

	if (stationObj.containsKey("enabled"))
		config.wifi.station.enabled = stationObj["enabled"];

	if (stationObj.containsKey("ledOn"))
		config.wifi.station.ledOn = stationObj["ledOn"];

	if (stationObj.containsKey("ledGpio"))
	{
		int8_t gpio = stationObj["ledGpio"];

		if (gpio == -1)
			goto SkipStationLedValidation;

		if (IsGpioValidPin(gpio, false, true))
		{
		SkipStationLedValidation:
			configMonitor.wifi.station.ledGpio = config.wifi.station.ledGpio != gpio;
			config.wifi.station.ledGpio = gpio;
		}
	}

	if (stationObj.containsKey("ssid"))
		config.wifi.station.ssid = (const char*)stationObj["ssid"];
	else
		missing_required_key("wifi/ssid");

	if (stationObj.containsKey("pass"))
		config.wifi.station.pass = (const char*)stationObj["pass"];
	else
		missing_required_key("wifi/pass");


Label_SkipStation:
#if DEVELOPER_MODE
	if (stationObj.containsKey("taskSettings"))
	{
		config.wifi.taskSettings = stationObj["taskSettings"].as<TaskConfig_t>();
	}
#endif

	

ReadHotspotSettings:
	if (wifiObj.containsKey("accessPoint"))
	{
		JsonObject hotspotObj = wifiObj["accessPoint"];

		if (hotspotObj.containsKey("useDefaults"))
			config.wifi.accessPoint.useDefaults = hotspotObj["useDefaults"];

		////Using defaults. Don't read the rest.
		//if (config.wifi.accessPoint.useDefaults)
		//	return true;

		if (hotspotObj.containsKey("configOnly"))
			config.wifi.accessPoint.configOnly = hotspotObj["configOnly"];

		if (hotspotObj.containsKey("enabled"))
			config.wifi.accessPoint.enabled = hotspotObj["enabled"];

		if (hotspotObj.containsKey("ssid"))
			config.wifi.accessPoint.ssid = (const char*)hotspotObj["ssid"];

		if (hotspotObj.containsKey("pass"))
			config.wifi.accessPoint.pass = (const char*)hotspotObj["pass"];

		if (hotspotObj.containsKey("buttonPullup"))
			config.wifi.accessPoint.buttonPullup = hotspotObj["buttonPullup"];

		if (hotspotObj.containsKey("buttonPress"))
			config.wifi.accessPoint.buttonPress = hotspotObj["buttonPress"];

		if (hotspotObj.containsKey("buttonGpio"))
		{
			int8_t gpio = hotspotObj["buttonGpio"];

			if (gpio == -1)
				goto SkipHotspotButtonValidation;

			if (IsGpioValidPin(gpio, (GpioPull_t)config.wifi.accessPoint.buttonPullup, false) || gpio == -1)	//-1 = disabled
			{
			SkipHotspotButtonValidation:
				configMonitor.wifi.accessPoint.buttonGpio = config.wifi.accessPoint.buttonGpio != gpio;
				config.wifi.accessPoint.buttonGpio = gpio;
			}
		}

		if (hotspotObj.containsKey("ledGpio"))
		{
			int8_t gpio = hotspotObj["ledGpio"];

			if (gpio == -1)
				goto SkipHotspotLedValidation;

			if (IsGpioValidPin(gpio, false, true))
			{
				SkipHotspotLedValidation:
				configMonitor.wifi.accessPoint.ledGpio = config.wifi.accessPoint.ledGpio != gpio;
				config.wifi.accessPoint.ledGpio = gpio;
			}
		}

		//if (hotspotObj.containsKey("buttonPress"))
		//	config.wifi.accessPoint.buttonPress = hotspotObj["buttonPress"];

		if (hotspotObj.containsKey("ledOn"))
			config.wifi.accessPoint.ledOn = hotspotObj["ledOn"];

		if (hotspotObj.containsKey("holdtime"))
			config.wifi.accessPoint.holdTime = hotspotObj["holdtime"];

		if (hotspotObj.containsKey("hidden"))
			config.wifi.accessPoint.hidden = hotspotObj["hidden"];


		if (hotspotObj.containsKey("maxConnections"))
			config.wifi.accessPoint.maxConnections = hotspotObj["maxConnections"];
	}


	return true;
}

bool Config::Documents::SetMqttFromDoc()
{

	if (!configDoc.containsKey("mqtt"))
	{
		missing_required_key("mqtt");

	MqttDefaultSettingsCheck:
		if (config.mqtt.broker.user.length() == 0 || config.mqtt.broker.pass.length() == 0 || (config.mqtt.broker.ip[0] == 0 && !config.mqtt.broker.autoDetectIp))
		{
			DEBUG_LOG_LN("Default settings do not contain MQTT settings. Will not be able to connect unless EEPROM has backup config.");
		}
		return false;
	}

	JsonObject mqttObj = configDoc["mqtt"];

	if (mqttObj.containsKey("useDefaults"))
		config.mqtt.useDefaults = mqttObj["useDefaults"];

	//if (config.mqtt.useDefaults)
	//	return true;

	if (mqttObj.containsKey("ledGpio"))
		config.mqtt.ledGpio = mqttObj["ledGpio"];

	if (mqttObj.containsKey("ledOn"))
		config.mqtt.ledOn = mqttObj["ledOn"];

	if (mqttObj.containsKey("publish"))
	{
		JsonObject publishObj = mqttObj["publish"];

		if (publishObj.containsKey("bufferSize"))
			config.mqtt.publish.bufferSize = publishObj["bufferSize"];

		//Mqtt::ReadKey(config.mqtt.publish.rate, publishObj, "rate");
		if (publishObj.containsKey("rate"))
			config.mqtt.publish.rate = ((int)publishObj["rate"]) * 1000;

		//Mqtt::ReadKey(config.mqtt.publish.errorRate, publishObj, "errorRate");
		if (publishObj.containsKey("errorRate"))
			config.mqtt.publish.errorRate = ((int)publishObj["errorRate"]) * 1000;

		//Mqtt::ReadKey(config.mqtt.publish.availabilityRate, publishObj, "availabilityRate");
		if (publishObj.containsKey("availabilityRate"))
			config.mqtt.publish.availabilityRate = ((int)publishObj["availabilityRate"]) * 1000;

		if (publishObj.containsKey("json"))
			config.mqtt.publish.json = publishObj["json"];

		if (publishObj.containsKey("asIndividualTopics"))
			config.mqtt.publish.onIndividualTopics = publishObj["asIndividualTopics"];

		//Mqtt::ReadKey(config.mqtt.publish.unknownPayload, publishObj, "unknownPayload");
		if (publishObj.containsKey("unknownPayload"))
			config.mqtt.publish.unknownPayload = (const char*)publishObj["unknownPayload"];

	}

	if (mqttObj.containsKey("baseTopics"))
	{
		JsonObject topicsObj = mqttObj["baseTopics"];


		//Mqtt::ReadKey(config.mqtt.baseTopics.base, topicsObj, "base");
		if (topicsObj.containsKey("base"))
			config.mqtt.baseTopics.base = (const char*)topicsObj["base"];

		//if (Mqtt::ReadKey(config.mqtt.baseTopics.availability, topicsObj, "availability"))
		//{
		//	config.mqtt.topics.availability = config.mqtt.baseTopics.base + config.mqtt.baseTopics.availability;
		//}
		if (topicsObj.containsKey("availability"))
		{
			config.mqtt.baseTopics.availability = (const char*)topicsObj["availability"];
			config.mqtt.topics.availability = config.mqtt.baseTopics.base + config.mqtt.baseTopics.availability;
		}

		//if (Mqtt::ReadKey(config.mqtt.baseTopics.jsonCommand, topicsObj, "jCommand"))
		//{
		//	config.mqtt.topics.jsonCommand = config.mqtt.baseTopics.base + config.mqtt.baseTopics.jsonCommand;
		//}
		if (topicsObj.containsKey("jCommand"))
		{
			config.mqtt.baseTopics.jsonCommand = (const char*)topicsObj["jCommand"];
			config.mqtt.topics.jsonCommand = config.mqtt.baseTopics.base + config.mqtt.baseTopics.jsonCommand;
		}

		//if (Mqtt::ReadKey(config.mqtt.baseTopics.jsonState, topicsObj, "jState"))
		//{
		//	config.mqtt.topics.jsonState = config.mqtt.baseTopics.base + config.mqtt.baseTopics.jsonState;
		//}
		if (topicsObj.containsKey("jState"))
		{
			config.mqtt.baseTopics.jsonState = (const char*)topicsObj["jState"];
			config.mqtt.topics.jsonState = config.mqtt.baseTopics.base + config.mqtt.baseTopics.jsonState;
		}

		//Mqtt::ReadKey(config.mqtt.baseTopics.command, topicsObj, "command");
		if (topicsObj.containsKey("command"))
			config.mqtt.baseTopics.command = (const char*)topicsObj["command"];

		//Mqtt::ReadKey(config.mqtt.baseTopics.state, topicsObj, "state");
		if (topicsObj.containsKey("state"))
			config.mqtt.baseTopics.state = (const char*)topicsObj["state"];
	}

#if DEVELOPER_MODE
	if (mqttObj.containsKey("taskSettings"))
	{
		config.mqtt.taskSettings = mqttObj["taskSettings"].as<TaskConfig_t>();
	}
#endif

	SkipTaskSettings:
	if (mqttObj.containsKey("broker"))
	{
		JsonObject brokerObj = mqttObj["broker"];


		if (brokerObj.containsKey("autoDetectIP"))
			config.mqtt.broker.autoDetectIp = brokerObj["autoDetectIP"];

		if (brokerObj.containsKey("maxRetries"))
			config.mqtt.broker.maxRetries = brokerObj["maxRetries"];

		if (brokerObj.containsKey("autoMaxRetries"))
			config.mqtt.broker.autoMaxRetries = brokerObj["autoMaxRetries"];

		if (brokerObj.containsKey("wifiMode"))
		{
			JsonVariantConst modeVar = brokerObj["wifiMode"];
			config.mqtt.broker.wifiMode = (wifi_mode_t)(modeVar.as<WifiMode>());
		}

		bool hasIp = false;

		if (brokerObj.containsKey("ip"))
		{
#if VALIDATE_JSON
			if (brokerObj["ip"].is<IPAddress>())
			{
				config.mqtt.broker.ip = brokerObj["ip"].as<IPAddress>();

				if(config.wifi.station.enabled)
					hasIp = true;
			}
			else goto ElseBrokerIP;
#else
			config.mqtt.broker.ip = brokerObj["ip"].as<IPAddress>();

			if (config.wifi.mode != WIFI_MODE_ACCESSPOINT)
				hasIp = true;
#endif			
		}
		else
		{
		ElseBrokerIP:
			if (!config.mqtt.broker.autoDetectIp && !config.wifi.station.enabled)
				missing_required_key("mqtt/broker/ip\r\nMQTT may not be able to connect!");
			else
				missing_key("mqtt/broker/ip");
		}

		if (brokerObj.containsKey("ipAP"))
		{
#if VALIDATE_JSON
			if (brokerObj["ipAP"].is<IPAddress>())
			{
				config.mqtt.broker.ipAP = brokerObj["ipAP"].as<IPAddress>();

				if (config.wifi.accessPoint.enabled)
					hasIp = true;
				else
					goto ElseBrokerIPAP;					
			}
			else goto ElseBrokerIPAP;
#else
			config.mqtt.broker.ipAP = brokerObj["ipAP"].as<IPAddress>();
#endif

		}
		else 
		{
		ElseBrokerIPAP:

			if (hasIp)
				missing_key("mqtt/broker/ipAP\r\nMQTT will try to use \"mqtt/broker/ip\", but may not be able to connect.");
			else
			{
				missing_required_key("mqtt/broker/ip\r\nMQTT Will not be able to connect!");
			}
		}

		if((config.wifi.station.enabled && config.mqtt.broker.ip == IPAddress()) && (config.wifi.accessPoint.enabled && config.mqtt.broker.ipAP == IPAddress() && config.mqtt.broker.ip == IPAddress()))
			status.mqtt.missingRequiredInfo = true;

		if (brokerObj.containsKey("user"))
			config.mqtt.broker.user = (const char*)brokerObj["user"];
		else
			missing_required_key("mqtt/broker/user");

		if (brokerObj.containsKey("pass"))
			config.mqtt.broker.pass = (const char*)brokerObj["pass"];
		else
			missing_required_key("mqtt/broker/pass");

		if (brokerObj.containsKey("pass"))
			config.mqtt.broker.pass = (const char*)brokerObj["pass"];
		else
			missing_required_key("mqtt/broker/pass");

		/*if (brokerObj.containsKey("autoMaxAttempts"))
			config.mqtt.broker.autoMaxAttempts = brokerObj["autoMaxAttempts"];*/

		if (brokerObj.containsKey("autoTimeout"))
			config.mqtt.broker.autoTimeout = brokerObj["autoTimeout"];

		if (brokerObj.containsKey("timeout"))
			config.mqtt.broker.timeout = brokerObj["timeout"];

		if (brokerObj.containsKey("attemptRate"))
			config.mqtt.broker.connectInterval = ((int)brokerObj["attemptRate"]);
	}
	else
	{
		missing_required_key("mqtt/broker");
		goto MqttDefaultSettingsCheck;
	}

	return true;
}


//bool Config::SetDocFromDoc()
//{
//	return false;
//	//if (configDoc.containsKey("docs"))
////{
////	JsonObject docsObj = configDoc["docs"];
//
////	if (docsObj.containsKey("deviceSizes"))
////	config.docs.devicesJsonSize = docsObj["deviceSizes"];
//
////	if (docsObj.containsKey("sensorsSize"))
////	config.docs.sensorsJsonSize = docsObj["sensorsSize"];
//
////	if (docsObj.containsKey("binarySensorsSize"))
////	config.docs.binarySensorsJsonSize = docsObj["binarySensorsSize"];
//
////	if (docsObj.containsKey("lightsSize"))
////	config.docs.lightsJsonSize = docsObj["lightsSize"];
//
////	if (docsObj.containsKey("buttonsSize"))
////	config.docs.buttonsJsonSize = docsObj["buttonsSize"];
//
////	if (docsObj.containsKey("switchesSize"))
////	config.docs.switchesJsonSize = docsObj["switchesSize"];
////}
//}

#if COMPILE_FTP
bool Config::Documents::SetFtpFromDoc()
{
	DEBUG_LOG_LN("Setting FTP Settings...");

	if (!configDoc.containsKey("server") || !configDoc["server"].containsKey("ftp"))
	{
		missing_key("server:ftp");
		return false;
	}

	JsonObject ftpObj = configDoc["server"]["ftp"];

	if (ftpObj.containsKey("useDefaults"))
		config.server.ftp.useDefaults = ftpObj["useDefaults"];

	//if (config.server.ftp.useDefaults)
	//	return true;

	if (ftpObj.containsKey("enabled"))
		config.server.ftp.enabled = ftpObj["enabled"];

	if (ftpObj.containsKey("anonymous"))
		config.server.ftp.anonymous = ftpObj["anonymous"];

	//Mqtt::ReadKey(config.server.ftp.user, ftpObj, "user");
	if (ftpObj.containsKey("user"))
		config.server.ftp.user = (const char*)ftpObj["user"];

	//Mqtt::ReadKey(config.server.ftp.pass, ftpObj, "pass");
	if (ftpObj.containsKey("pass"))
		config.server.ftp.pass = (const char*)ftpObj["pass"];

	//Mqtt::ReadKey(config.server.ftp.timeout, ftpObj, "freq");
	if (ftpObj.containsKey("timeout"))
		config.server.ftp.timeout = ftpObj["timeout"];

#if DEVELOPER_MODE
	if (ftpObj.containsKey("taskSettings"))
	{
		config.server.ftp.taskSettings = ftpObj["taskSettings"].as<TaskConfig_t>();
	}
#endif


	return true;
}
#endif

#if COMPILE_SERVER
bool Config::Documents::SetServerFromDoc()
{
	DEBUG_LOG_LN("Setting Server Settings...");

	if (!configDoc.containsKey("server"))
	{
		missing_key("server");
		return false;
	}

	JsonObject serverObj = configDoc["server"];

	if (serverObj.containsKey("useDefaults"))
		config.server.useDefaults = serverObj["useDefaults"];

	//if (config.server.useDefaults)
	//	return true;

	if (serverObj.containsKey("dns"))
		config.server.dns = serverObj["dns"];

	if (serverObj.containsKey("hostname"))
		config.server.hostname = (const char*)serverObj["hostname"];

	if (serverObj.containsKey("authenticate"))
		config.server.authenticate = serverObj["authenticate"];

	if (serverObj.containsKey("user"))
	{
		config.server.user = (const char*)serverObj["user"];

		//if (!serverObj.containsKey("ftp") || !serverObj["ftp"].containsKey("user"))
		//{
		//	config.server.ftp.user = (const char*)serverObj["user"];
		//	DEBUG_LOG_LN("FTP does not contain the user key. Using server user");
		//}
	}

	if (serverObj.containsKey("pass"))
	{
		config.server.pass = (const char*)serverObj["pass"];

		//if (!serverObj.containsKey("ftp") || !serverObj["ftp"].containsKey("pass"))
		//{
		//	config.server.ftp.pass = (const char*)serverObj["pass"];
		//	DEBUG_LOG_LN("FTP does not contain the pass key. Using server pass");
		//}
	}

	if (serverObj.containsKey("sessionTimeout"))
		config.server.sessionTimeout = serverObj["sessionTimeout"];

#if COMPILE_OTA
	if (serverObj.containsKey("ota"))
	{
		//config.server.updater.mode = serverObj["updater"].as<DeviceUpdaterMode_t>();

		JsonObject otaObj = serverObj["ota"];

		if (otaObj.containsKey("useDefaults"))
			config.server.ota.useDefaults = otaObj["useDefaults"];

		//if (config.server.ota.useDefaults)
		//	goto SkipUpdater;

		if (otaObj.containsKey("enabled"))
			config.server.ota.enabled = otaObj["enabled"];


#if DEVELOPER_MODE
		if (otaObj.containsKey("taskSettings"))
		{
			config.server.ota.taskSettings = otaObj["taskSettings"].as<TaskConfig_t>();
		}
#endif

		
	}
#endif

SkipUpdater:
	if (serverObj.containsKey("browser"))
	{
		JsonObject browserObj = serverObj["browser"];

		if (browserObj.containsKey("useDefaults"))
			config.server.browser.useDefaults = browserObj["useDefaults"];

		//if (config.server.browser.useDefaults)
		//	goto SkipBrowser;

		if (browserObj.containsKey("enabled"))
			config.server.browser.enabled = browserObj["enabled"];

		if (browserObj.containsKey("config"))
			config.server.browser.config = browserObj["config"];

		if (browserObj.containsKey("console"))
			config.server.browser.console = browserObj["console"];

		if (browserObj.containsKey("updater"))
			config.server.browser.updater = browserObj["updater"];

		if (browserObj.containsKey("mqttDevices"))
			config.server.browser.mqttDevices = browserObj["mqttDevices"];

		if (browserObj.containsKey("ssl"))
			config.server.browser.ssl = browserObj["ssl"];

#if COMPILE_BROWSER_TOOLS
		if (browserObj.containsKey("tools"))
		{
			JsonObject toolsObj = browserObj["tools"];

			if (toolsObj.containsKey("fileEditor"))
				config.server.browser.tools.fileEditor = toolsObj["fileEditor"];

			if (toolsObj.containsKey("jsonVerify"))
				config.server.browser.tools.jsonVerify = toolsObj["jsonVerify"];
		}
#endif
	}

	SkipBrowser:
	DEBUG_LOG_LN("...Server Settings Set.");
	return true;
}
#endif

#pragma endregion

#pragma region Backup

bool Config::Backup::SaveBackupConfig(bool fs, bool eeprom, bool saveRetained, size_t* out_sizeFileSystem, size_t* out_sizeEeprom)
{
#if COMPILE_BACKUP
	DEBUG_NEWLINE();
	DEBUG_LOG_LN("Saving backup config...");

	if (status.backup.backupsDisabled)
	{
		DEBUG_LOG_LN("...Backups are disabled.");
		return false;
	}

	if (!status.config.settingsConfigured || !status.config.hasRequiredData)
	{
		DEBUG_LOG_LN("...Backup could not be saved. Settings not configured or required data is missing.");
		return 0;
	}

	if (status.backup.filesystemBackedUp /*&& status.backup.eepromBackedUp && statusRetained.fileSizes.eepromBackup*/)
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

	#warning Backups disabled until they can be fixed.
	return 0;

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

	if (eeprom && status.backup.ableToBackupEeprom)
	{
		DEBUG_LOG("...Saving to EEPROM...");
		sizeEeprom = SaveBackupEeprom(&serializeConfigDoc, docCRC);

		if (sizeEeprom > 1)
			DEBUG_LOG_F("...Backed up to EEPROM with size of %dbytes...", sizeEeprom);
	}

	if (out_sizeEeprom != nullptr)
		*out_sizeEeprom = sizeEeprom;

	if (!status.backup.ableToBackupEeprom && sizeFS > 1)
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

	if (status.backup.backupsDisabled)
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

	if (status.backup.backupsDisabled)
	{
		DEBUG_LOG_LN("...Backups are disabled.");
		return false;
	}

	if (!status.storage.eepromMounted)
	{
		DEBUG_LOG_LN("...Backup cannot be saved as the EEPROM is not mounted.");
		return false;
	}

	if (!status.backup.eepromBackedUp /*&& statusRetained.fileSizes.eepromBackup*/)
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
	if (status.backup.backupsDisabled)
	{
		DEBUG_LOG_LN("Backups are disabled.\r\n");
		return false;
	}

	if (!status.storage.eepromMounted)
	{
		DEBUG_LOG_LN("Backup cannot be saved as the EEPROM is not mounted.\r\n");
		return false;
	}

	if (doc == nullptr)
	{
		DEBUG_LOG_LN("JsonDocument nullptr!\r\n");
		return 0;
	}

	if (status.backup.eepromBackedUp /*&& statusRetained.fileSizes.eepromBackup*/)
	{
		DEBUG_LOG_LN("EEPROM backup already saved.\r\n");
		return 1;
	}

	size_t serializedSize = 0;

	if(status.backup.filesystemBackedUp)



	DEBUG_LOG("Serializing backup to EEPROM...");
	EepromStream eepromStream(sizeof(StatusRetained_t), DOC_CONFIG_SERIALIZE_SIZE);

	uint32_t docCRC = crc > 0 ? crc : FileManager::GetSerializedCRC(*doc);

	if (docCRC == statusRetained.crcs.eepromBackupFile)
	{
		DEBUG_LOG_LN("CRC Matching, Already backed up.\r\n");

		return 1;	//CRC matches, no need to save.
	}

	size_t size = serializeJson(*doc, eepromStream);


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
		status.backup.eepromBackedUp = true;
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

	if (status.backup.backupsDisabled)
	{
		DEBUG_LOG_LN("...Backups are disabled.");
		return false;
	}

	if (status.backup.filesystemBackedUp /*&& statusRetained.fileSizes.fileSystemBackup*/)
	{
		DEBUG_LOG_LN("...File System backup already saved.");
		return true;
	}

	if (!status.storage.fsMounted)
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
	if (status.backup.backupsDisabled)
	{
		DEBUG_LOG_LN("Backups are disabled.\r\n");
		return false;
	}

	if (!status.storage.fsMounted)
	{
		DEBUG_LOG_LN("Backup cannot be saved as the file system is not mounted.\r\n");
		return false;
	}

	if (doc == nullptr)
	{
		DEBUG_LOG_LN("JsonDocument nullptr!\r\n");
		return 0;
	}

	if (status.backup.filesystemBackedUp /*&& statusRetained.fileSizes.fileSystemBackup*/ /*&& statusRetained.crcs.recentBackup == statusRetained.crcs.fileSystemBackupFile*/)
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
	file.close();

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
		status.backup.filesystemBackedUp = true;
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

bool Config::Backup::DeserializeEepromBackupConfig()
{
#if COMPILE_BACKUP
	/*DEBUG_LOG_LN("Reading Config Backup from EEPROM...");
	EEPROM.get(0, settings);
	DEBUG_LOG_LN("Read Complete.");*/

	DEBUG_LOG_LN("Reading and deserializing config backup from EEPROM...");

	EepromStream eepromStream(sizeof(Boot_bm), DOC_CONFIG_DESERIALIZE_SIZE);
	eepromStream.setTimeout(FILE_STREAM_TIMEOUT);

	DeserializationError derror = deserializeJson(configDoc, eepromStream);

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
	status.backup.backupsDisabled = true;
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
	status.backup.backupsDisabled = false;
#endif
}

void Config::Backup::SetBackupFlags(uint32_t crc)
{
#if COMPILE_BACKUP
	status.backup.eepromBackedUp = crc == statusRetained.crcs.eepromBackupFile && statusRetained.fileSizes.eepromBackup > 0 && statusRetained.crcs.eepromBackupFile != statusRetained.crcs.recentBackup;
	status.backup.filesystemBackedUp = crc == statusRetained.crcs.fileSystemBackupFile && statusRetained.fileSizes.fileSystemBackup > 0 && statusRetained.crcs.fileSystemBackupFile != statusRetained.crcs.recentBackup;
#endif
}


#pragma endregion

#pragma region Parsers

	void Config::Parsers::ParseSerialPort(JsonObject& portObj, SerialPortConfig_t& port, SerialPortConfigMonitor_t& portMonitor)
	{
		if (portObj.containsKey("useDefaults"))
			port.useDefaults = portObj["useDefaults"];

		//if (port.useDefaults) return;

		if (portObj.containsKey("enabled"))
			port.enabled = portObj["enabled"];

		if (portObj.containsKey("baud"))
		{
			unsigned long baud = portObj["baud"];
			if (IsBaudrateValid(baud))
				port.baud = baud;
		}

		if (portObj.containsKey("rxGpio"))
		{
			int8_t rx = portObj["rxGpio"];
			if (IsGpioValidPin(rx, GPIOV_UART))
			{
				portMonitor.rxGpio = port.rxGpio != rx;
				port.rxGpio = rx;
			}
		}


		if (portObj.containsKey("txGpio"))
		{
			int8_t tx = portObj["txGpio"];
			if (IsGpioValidPin(tx, GPIOV_UART))
			{
				portMonitor.txGpio = port.txGpio != tx;
				port.txGpio = tx;
			}
		}

		if (portObj.containsKey("config"))
		{
			JsonObject portConfigObj = portObj["config"];
			uint32_t bitmap = UART_CONFIG_CONSTANT;

			UART_DATABITS databits = (UART_DATABITS)0;
			UART_PARITY parity = (UART_PARITY)0;
			UART_STOPBITS stopbits = (UART_STOPBITS)0;

			//#warning Not working properly. Come back when debugger is working again...

			if (portConfigObj.containsKey("bitmap"))
			{
				JsonVariant bmVar = portObj["bitmap"];

				uint32_t test = portObj["bitmap"];
				#warning not detecting hex string as char*
					if (bmVar.is<const char*>())
					{
						const char* bmStr = bmVar;

						if (bmStr[1] == 'x' || bmStr[1] == 'X')
						{
							bmStr += 2;
							bitmap = (int)strtol(bmStr, NULL, 16);
						}
						else
						{
							DEBUG_LOG("Cannot set Serial Port Config Bitmap : ");
							DEBUG_LOG_LN("Incorrect HEX format");
						}
					}
					else if (bmVar.is<int>())
					{
						bitmap = bmVar;
					}
			}
			else
			{
				#warning Json UDF
					if (portConfigObj.containsKey("dataBits"))
					{
						JsonVariantConst dbVar = portConfigObj["stopBits"];
						databits = (UART_DATABITS)(dbVar.as<UART_DATABITS>());
						/*uint8_t db = portConfigObj["dataBits"];
						switch (db)
						{
						case 5:
							bitmap |= UART_DATABITS_5;
							break;
						case 6:
							bitmap |= UART_DATABITS_6;
							break;
						case 7:
							bitmap |= UART_DATABITS_7;
							break;
						case 8:
							bitmap |= UART_DATABITS_8;
							break;
						default:
							DEBUG_LOG_LN("Cannot set Serial Port Data Bits : Value NAN or Out of range");
							break;
						}*/
					}

				if (portConfigObj.containsKey("parity"))
				{
					JsonVariantConst parVar = portConfigObj["stopBits"];
					parity = (UART_PARITY)(parVar.as<UART_PARITY>());
					/*JsonVariant parityVar = portConfigObj["parity"];

					if (parityVar.is<const char*>())
					{
						const char* parity = parityVar;

						if (strcmp(parity, "none") == 0)
							bitmap |= (uint32_t)UART_PARITY_NONE;
						else if (strcmp(parity, "even") == 0)
							bitmap |= (uint32_t)UART_PARITY_EVEN;
						else if (strcmp(parity, "odd") == 0)
							bitmap |= (uint32_t)UART_PARITY_ODD;
						else
							DEBUG_LOG_LN("Cannot set Serial Port Parity : Invalid value");
					}
					else if (parityVar.is<int>())
					{
						uint32_t parity = parityVar;
						bitmap |= parity;
					}
					else
					{
						DEBUG_LOG_LN("Cannot set Serial Port Parity : Invalid value");
					}		*/
				}

				if (portConfigObj.containsKey("stopBits"))
				{
					JsonVariantConst sbVar = portConfigObj["stopBits"];
					stopbits = (UART_STOPBITS)(sbVar.as<UART_STOPBITS>());
					/*uint8_t stopBits = portConfigObj["stopBits"];

					switch (stopBits)
					{
					case 1:
						bitmap |= (uint32_t)UART_STOPBITS_1;
						break;
					case 2:
						bitmap |= (uint32_t)UART_STOPBITS_2;
						break;
					default:
						DEBUG_LOG_LN("Cannot set Serial Port Stop Bits : Value NAN or Out of range");
						break;
					}*/
				}
			}

			bitmap |= (int)databits;
			bitmap |= (int)parity;
			bitmap |= (int)stopbits;

			//Make sure bitmap is correct format before setting.
			if (bitmap & 0x8000000 != 0 &&
				bitmap & ~0x800003f == 0 &&
				(bitmap & UART_CONFIG_STOPBITS_MASK) >> 4 != 0b10 &&
				bitmap & UART_CONFIG_PARITY_MASK != 0b10)
			{
				//Format is correct, set.
				port.config = bitmap;
			}
			else
			{
				DEBUG_LOG("Cannot set Serial Port Config : ");
				DEBUG_LOG_LN("Bitmap is incorrect format");
			}
		}
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

//size_t Config::Status::SerializeDeviceStatus(String* serializeTo, DynamicJsonDocument** out_doc)
//{
//	if (serializeTo == nullptr && out_doc == nullptr) return 0;
//
//	DynamicJsonDocument* doc = new DynamicJsonDocument(3072);
//
//	JsonObject statusObj = doc->createNestedObject("statusObj");
//	
//	statusObj["status"].set(status);
//	statusObj["retainedStatus"].set(statusRetained);
//
////	//Device
////	{
////		JsonObject status_device = statusObj.createNestedObject("device");
////		status_device["freshboot"] = status.device.freshBoot;
////		status_device["i2cInitialized"] = status.device.i2cInitialized;
////		status_device["retainedStatusLoaded"] = status.device.retainedStatusLoaded;
////		status_device["retainedSnextAliveMessagetatusLoaded"] = status.device.nextAliveMessage;
////
////		JsonObject status_dualcore = status_device.createNestedObject("tasks");
////		status_dualcore["enabled"] = status.device.tasks.enabled;
////		status_dualcore["wifiTaskRunning"] = status.device.tasks.wifiTaskRunning;
////		status_dualcore["ftpTaskRunning"] = status.device.tasks.ftpTaskRunning;
////		status_dualcore["otaTaskRunning"] = status.device.tasks.otaTaskRunning;
////		status_dualcore["mqttTaskRunning"] = status.device.tasks.mqttTaskRunning;
////		status_dualcore["mqttDeviceManagerTaskRunning"] = status.device.tasks.mqttDeviceManagerTaskRunning;
////		status_dualcore["mqttPublishAvailabilityTaskRunning"] = status.device.tasks.mqttPublishAvailabilityTaskRunning;
////	}
////
////	//Config
////	{
////		JsonObject status_config = statusObj.createNestedObject("config");
////		status_config["setupComplete"] = status.config.setupComplete;
////		status_config["configRead"] = status.config.configRead;
////		status_config["hasRequiredData"] = status.config.hasRequiredData;
////		status_config["settingsConfigured"] = status.config.settingsConfigured;
////		status_config["saveRetainedLoop"] = status.config.saveRetainedLoop;
////		//status_config["configSource"] = (uint8_t)status.config.configSource;
////		status_config["configSource"].set(status.config.configSource);
////		status_config["pathSet"] = status.config.pathSet;
////		status_config["testingConfig"] = status.config.testingConfig;
////		status_config["path"] = status.config.path;
////		status_config["fileName"] = status.config.fileName;
////	}
////
////	//Wifi
////	{
////		JsonObject status_wifi = statusObj.createNestedObject("wifi");
////		status_wifi["connected"] = status.wifi.connected;
////		status_wifi["configMode"] = status.wifi.configMode;
////		status_wifi["eventsRegistered"] = status.wifi.eventsRegistered;
////		status_wifi["nextDisplayMessage"] = status.wifi.nextDisplayMessage;
////		//status_wifi["hotspotEnabled"] = status.wifi.hotspotEnabled;
////		//status_wifi["mode"]= (uint8_t)status.wifi.mode;
////		status_wifi["mode"].set<WifiMode>((WifiMode)status.wifi.mode);
////		status_wifi["powerLevel"].set((WifiPower)status.wifi.powerLevel);
////		//status_wifi["powerLevel"] = EnumTo(status.wifi.powerLevel, wifi_power_strings, 12, (int*)wifi_power_values);
////
////		JsonObject status_station = status_wifi.createNestedObject("station");
////		status_station["enabled"] = status.wifi.station.enabled;
////		status_station["missingRequiredInfo"] = status.wifi.station.missingRequiredInfo;
////		status_station["connected"] = status.wifi.station.connected;
////		status_station["eventsRegistered"] = status.wifi.station.eventsRegistered;
////		status_station["ip"].set(status.wifi.station.ip);
////		status_station["gotIP"] = status.wifi.station.gotIP;
////		//status_station["startedConnecting"] = status.wifi.station.startedConnecting;
////		status_station["wlstatus"] = (int)WiFi.status();
////#warning move WiFi status to its own WiFi object
////
////		JsonObject status_ap = status_wifi.createNestedObject("accessPoint");
////		status_ap["enabled"] = status.wifi.accessPoint.enabled;
////		status_ap["connected"] = status.wifi.accessPoint.connected;
////		status_ap["clientCount"] = status.wifi.accessPoint.clientCount;
////		status_ap["ip"].set(status.wifi.accessPoint.ip);
////		status_ap["ipAssigned"] = status.wifi.accessPoint.ipAssigned;
////	}
////
////	//MQTT
////	{
////		JsonObject status_mqtt = statusObj.createNestedObject("mqtt");
////		status_mqtt["connected"] = status.mqtt.connected;
////		status_mqtt["missingRequiredInfo"] = status.mqtt.missingRequiredInfo;
////		status_mqtt["devicesConfigured"] = status.mqtt.devicesConfigured;
////		//status_mqtt["publishingEnabled"] = status.mqtt.publishingEnabled;
////		status_mqtt["publishingDisabled"] = status.mqtt.publishingDisabled;
////		status_mqtt["serverSet"] = status.mqtt.serverSet;
////		status_mqtt["nextPublish"] = status.mqtt.nextPublish;
////		status_mqtt["nextDisplayMessages"] = status.mqtt.nextDisplayMessages;
////		status_mqtt["nextPublishAvailability"] = status.mqtt.nextPublishAvailability;
////		status_mqtt["nextMqttConnectAttempt"] = status.mqtt.nextMqttConnectAttempt;
////		status_mqtt["nextWarningBlink"] = status.mqtt.nextWarningBlink;
////
////		{
////			JsonObject status_mqtt_devices = status_mqtt.createNestedObject("devices");
////			status_mqtt_devices["deviceCount"] = status.mqtt.devices.deviceCount;
////			status_mqtt_devices["binarySensorCount"] = status.mqtt.devices.binarySensorCount;
////			status_mqtt_devices["buttonCount"] = status.mqtt.devices.buttonCount;
////			status_mqtt_devices["lightCount"] = status.mqtt.devices.lightCount;
////			status_mqtt_devices["sensorCount"] = status.mqtt.devices.sensorCount;
////			status_mqtt_devices["switchCount"] = status.mqtt.devices.switchCount;
////
////#warning create UDF
////			JsonObject status_functioning_devices = status_mqtt_devices.createNestedObject("functioningDevices");
////			status_functioning_devices["bitmap0"] = status.mqtt.devices.functioningDevices.bitmap0;
////			status_functioning_devices["bitmap1"] = status.mqtt.devices.functioningDevices.bitmap1;
////			status_functioning_devices["bitmap2"] = status.mqtt.devices.functioningDevices.bitmap2;
////			status_functioning_devices["bitmap3"] = status.mqtt.devices.functioningDevices.bitmap3;
////
////			JsonObject status_functioning_devices_important = status_mqtt_devices.createNestedObject("functioningDevicesImportant");
////			status_functioning_devices_important["bitmap0"] = status.mqtt.devices.functioningDevices.bitmap0;
////			status_functioning_devices_important["bitmap1"] = status.mqtt.devices.functioningDevices.bitmap1;
////			status_functioning_devices_important["bitmap2"] = status.mqtt.devices.functioningDevices.bitmap2;
////			status_functioning_devices_important["bitmap3"] = status.mqtt.devices.functioningDevices.bitmap3;
////		}
////
////		{
////			JsonObject status_mqtt_ip = status_mqtt.createNestedObject("ipStatus");
////			status_mqtt_ip["ip"].set(status.mqtt.ipStatus.ip);
////			status_mqtt_ip["ipIndex"] = status.mqtt.ipStatus.ipIndex;
////			status_mqtt_ip["mode"] = (uint8_t)status.mqtt.ipStatus.mode;
////			status_mqtt_ip["mode"].set(status.mqtt.ipStatus.mode);
////			//status_mqtt_ip["totalAttemptsCounter"] = status.mqtt.ipStatus.totalAttemptsCounter;
////			status_mqtt_ip["currentAttemptsCounter"] = status.mqtt.ipStatus.currentAttemptsCounter;
////			status_mqtt_ip["maxRetries"] = status.mqtt.ipStatus.maxRetries;
////			status_mqtt_ip["changed"] = status.mqtt.ipStatus.changed;
////			status_mqtt_ip["triedRetainedIP"] = status.mqtt.ipStatus.triedRetainedIP;
////			status_mqtt_ip["triedConfigStation"] = status.mqtt.ipStatus.triedConfigStation;
////			status_mqtt_ip["triedConfigAP"] = status.mqtt.ipStatus.triedConfigAP;
////			status_mqtt_ip["stationAutoExhausted"] = status.mqtt.ipStatus.stationAutoExhausted;
////			status_mqtt_ip["accessPointAutoExhausted"] = status.mqtt.ipStatus.accessPointAutoExhausted;
////		}
////	}
////
////	//Storage
////	{
////		JsonObject status_storage = statusObj.createNestedObject("storage");
////		status_storage["fsMounted"] = status.storage.fsMounted;
////		status_storage["eepromMounted"] = status.storage.eepromMounted;
////	}
////
////
////
////#if COMPILE_BACKUP
////	//Backup
////	{
////		JsonObject status_backup = statusObj.createNestedObject("backup");
////
////		status_backup["ableToBackupEeprom"] = status.backup.ableToBackupEeprom;
////		status_backup["backupsDisabled"] = status.backup.backupsDisabled;
////		status_backup["eepromBackedUp"] = status.backup.eepromBackedUp;
////		status_backup["filesystemBackedUp"] = status.backup.filesystemBackedUp;
////	}
////
////#endif	/*COMPILE_BACKUP*/
////
////#if COMPILE_SERVER
////	//Server
////	{
////		JsonObject status_server = statusObj.createNestedObject("server");
////
////		status_server["enabled"] = status.server.enabled;
////		status_server["configured"] = status.server.configured;
////		status_server["authenticated"] = status.server.authenticated;
////		status_server["authConfigured"] = status.server.authConfigured;
////		status_server["specialRequestsConfigured"] = status.server.specialRequestsConfigured;
////		status_server["updating"] = (int)status.server.updating;	
////		status_server["clientIP"] = status.server.clientIP;
////		status_server["sessionEnd"] = status.server.sessionEnd;
////		#warning convert to string
////
////		//DNS
////		{
////			JsonObject status_dns = status_server.createNestedObject("dns");
////			status_dns["enabled"] = status.server.dns.enabled;
////			status_dns["configured"] = status.server.dns.configured;
////		}
////
////#if COMPILE_OTA
////		//OTA
////		{
////			JsonObject status_ota = status_server.createNestedObject("ota"); 
////			status_ota["enabled"] = status.server.ota.enabled;
////			status_ota["configured"] = status.server.ota.configured;
////			status_ota["updating"] = status.server.ota.updating;
////		}
////#endif
////
////		//Browser
////		JsonObject status_browser = status_server.createNestedObject("browser");
////		{
////			status_browser["enabled"] = status.server.browser.enabled;
////
////#endif	/*COMPILE_SERVER*/
////
////#if COMPILE_SERVER_CONSOLE
////			{
////				JsonObject status_server_console = status_browser.createNestedObject("console");
////				status_server_console["enabled"] = status.server.browser.console.enabled;
////				status_server_console["configured"] = status.server.browser.console.configured;
////			}
////#endif
////			
////#if COMPILE_CONFIG_BROWSER
////			{
////				JsonObject status_server_configBrowser = status_browser.createNestedObject("configBrowser");
////				status_server_configBrowser["enabled"] = status.server.browser.configBrowser.enabled;
////				status_server_configBrowser["configured"] = status.server.browser.configBrowser.configured;
////			}
////#endif	/*COMPILE_CONFIG_BROWSER*/
////
////#if COMPILE_CONFIG_BROWSER_MQTT
////			{
////				JsonObject status_server_mqttConfigBrowser = status_browser.createNestedObject("mqttDevices");
////				status_server_mqttConfigBrowser["enabled"] = status.server.browser.mqttDevices.enabled;
////				status_server_mqttConfigBrowser["configured"] = status.server.browser.mqttDevices.configured;
////			}
////#endif	/*COMPILE_CONFIG_BROWSER*/
////
////#if COMPILE_WEBUPDATE
////			{
////				JsonObject status_server_webUpdate = status_browser.createNestedObject("webUpdate");
////				status_server_webUpdate["enabled"] = status.server.browser.updater.enabled;
////				status_server_webUpdate["configured"] = status.server.browser.updater.configured;
////				status_server_webUpdate["updating"] = status.server.browser.updater.updating;
////			}
////#endif	/*COMPILE_WEBUPDATE*/
////
////#if COMPILE_BROWSER_TOOLS
////			JsonObject status_tools = status_browser.createNestedObject("tools");
////			{
////#if COMPILE_FILE_EDITOR
////				{
////					JsonObject status_fileEditor = status_tools.createNestedObject("fileEditor");
////					status_fileEditor["enabled"] = status.server.browser.tools.fileEditor.enabled;
////					status_fileEditor["configured"] = status.server.browser.tools.fileEditor.configured;
////				}
////#endif
////#if COMPILE_WEB_JSON_VALIDATOR
////				{
////					JsonObject status_JsonVerify = status_tools.createNestedObject("jsonVerify");
////					status_JsonVerify["enabled"] = status.server.browser.tools.jsonVerify.enabled;
////					status_JsonVerify["configured"] = status.server.browser.tools.jsonVerify.configured;
////				}
////#endif
////			}
////#endif
////		}
////
////#if COMPILE_FTP
////		//FTP
////		{
////			JsonObject status_ftp = statusObj.createNestedObject("ftp");
////			status_ftp["enabled"] = status.server.ftp.enabled;
////		}
////#endif
////	}
////
////	//Misc
////	{
////		JsonObject status_misc = statusObj.createNestedObject("misc");
////		status_misc["developerMode"] = status.misc.developerMode;
////	}
////
////	//Retained Status
////	{
////		JsonObject retainedStatusObj = statusObj.createNestedObject("retainedStatus");
////
////		{
////			JsonObject retainedStatus_boot = retainedStatusObj.createNestedObject("boot");
////			retainedStatus_boot["freshBoot"] = statusRetained.boot.freshBoot;
////			retainedStatus_boot["bootSource"] = (uint8_t)statusRetained.boot.bootSource;
////		}
////
////		{
////			JsonObject retainedStatus_crcs = retainedStatusObj.createNestedObject("crcs");
////			retainedStatus_crcs["bootFile"] = statusRetained.crcs.bootFile;
////			retainedStatus_crcs["configPath"] = statusRetained.crcs.configPath;
////			retainedStatus_crcs["configFile"] = statusRetained.crcs.configFile;
////			retainedStatus_crcs["recentBackup"] = statusRetained.crcs.recentBackup;
////			retainedStatus_crcs["fileSystemBackupFile"] = statusRetained.crcs.fileSystemBackupFile;
////			retainedStatus_crcs["eepromBackupFile"] = statusRetained.crcs.eepromBackupFile;
////		}
////
////		{
////			JsonObject retainedStatus_fileSizes = retainedStatusObj.createNestedObject("fileSizes");
////			retainedStatus_fileSizes["recentBackup"] = statusRetained.fileSizes.recentBackup;
////			retainedStatus_fileSizes["fileSystemBackup"] = statusRetained.fileSizes.fileSystemBackup;
////			retainedStatus_fileSizes["eepromBackup"] = statusRetained.fileSizes.eepromBackup;
////
////			JsonObject retainedStatus_mqtt = retainedStatusObj.createNestedObject("mqtt");
////			retainedStatus_mqtt["ip"].set(statusRetained.mqtt.ip);
////		}
////	}
//
//	size_t size = 1;
//
//	if (serializeTo != nullptr)
//		size = serializeJson(*doc, *serializeTo);
//
//	if (out_doc != nullptr)
//	{
//		*out_doc == doc;
//	}
//	else
//	{
//		doc->clear();
//		free(doc);
//	}
//
//	return size;
//}
//
//size_t Config::Status::SerializeDeviceStatus(DynamicJsonDocument** out_doc)
//{
//	return SerializeDeviceStatus(nullptr, out_doc);
//}


bool Config::Status::SaveRetainedStatus()
{
	if (!status.storage.eepromMounted)
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
	//statusRetainedMonitor.lastConfigPath = strcmp(statusRetained.lastConfigPath, status.config.path) != 0;
	//memset(statusRetained.lastConfigPath, 0, CONFIG_PATH_MAX_LENGTH);
	//strlcpy(statusRetained.lastConfigPath, status.config.path, CONFIG_PATH_MAX_LENGTH);

	//if (saveRetained)
	//	SaveRetainedStatus();
}

#pragma endregion


#pragma region Json UDFs

bool convertToJson(const DeviceStatus_t& src, JsonVariant dst)
{
	//Device
	{
		JsonObject status_device = dst.createNestedObject("device");
		status_device["freshboot"] = src.device.freshBoot;
		status_device["i2cInitialized"] = src.device.i2cInitialized;
		status_device["retainedStatusLoaded"] = src.device.retainedStatusLoaded;
		status_device["retainedSnextAliveMessagetatusLoaded"] = src.device.nextAliveMessage;

		JsonObject status_dualcore = status_device.createNestedObject("tasks");
		status_dualcore["enabled"] = src.device.tasks.enabled;
		status_dualcore["wifiTaskRunning"] = src.device.tasks.wifiTaskRunning;
		status_dualcore["ftpTaskRunning"] = src.device.tasks.ftpTaskRunning;
		status_dualcore["otaTaskRunning"] = src.device.tasks.otaTaskRunning;
		status_dualcore["mqttTaskRunning"] = src.device.tasks.mqttTaskRunning;
		status_dualcore["mqttDeviceManagerTaskRunning"] = src.device.tasks.mqttDeviceManagerTaskRunning;
		status_dualcore["mqttPublishAvailabilityTaskRunning"] = src.device.tasks.mqttPublishAvailabilityTaskRunning;
	}

	//Config
	{
		JsonObject status_config = dst.createNestedObject("config");
		status_config["setupComplete"] = src.config.setupComplete;
		status_config["configRead"] = src.config.configRead;
		status_config["hasRequiredData"] = src.config.hasRequiredData;
		status_config["settingsConfigured"] = src.config.settingsConfigured;
		status_config["saveRetainedLoop"] = src.config.saveRetainedLoop;
		//status_config["configSource"] = (uint8_t)src.config.configSource;
		status_config["configSource"].set(src.config.configSource);
		status_config["pathSet"] = src.config.pathSet;
		status_config["testingConfig"] = src.config.testingConfig;
		status_config["path"] = src.config.path;
		status_config["fileName"] = src.config.fileName;
	}

	//Wifi
	{
		JsonObject status_wifi = dst.createNestedObject("wifi");
		status_wifi["connected"] = src.wifi.connected;
		status_wifi["configMode"] = src.wifi.configMode;
		status_wifi["eventsRegistered"] = src.wifi.eventsRegistered;
		status_wifi["nextDisplayMessage"] = src.wifi.nextDisplayMessage;
		//status_wifi["hotspotEnabled"] = src.wifi.hotspotEnabled;
		//status_wifi["mode"]= (uint8_t)src.wifi.mode;
		status_wifi["mode"].set<WifiMode>((WifiMode)src.wifi.mode);
		status_wifi["powerLevel"].set((WifiPower)src.wifi.powerLevel);
		//status_wifi["powerLevel"] = EnumTo(src.wifi.powerLevel, wifi_power_strings, 12, (int*)wifi_power_values);

		JsonObject status_station = status_wifi.createNestedObject("station");
		status_station["enabled"] = src.wifi.station.enabled;
		status_station["missingRequiredInfo"] = src.wifi.station.missingRequiredInfo;
		status_station["connected"] = src.wifi.station.connected;
		status_station["eventsRegistered"] = src.wifi.station.eventsRegistered;
		status_station["ip"].set(src.wifi.station.ip);
		status_station["gotIP"] = src.wifi.station.gotIP;
		//status_station["startedConnecting"] = src.wifi.station.startedConnecting;
		status_station["wlstatus"] = (int)WiFi.status();
		#warning move WiFi status to its own WiFi object

			JsonObject status_ap = status_wifi.createNestedObject("accessPoint");
		status_ap["enabled"] = src.wifi.accessPoint.enabled;
		status_ap["connected"] = src.wifi.accessPoint.connected;
		status_ap["clientCount"] = src.wifi.accessPoint.clientCount;
		status_ap["ip"].set(src.wifi.accessPoint.ip);
		status_ap["ipAssigned"] = src.wifi.accessPoint.ipAssigned;
	}

	//MQTT
	{
		JsonObject status_mqtt = dst.createNestedObject("mqtt");
		status_mqtt["connected"] = src.mqtt.connected;
		status_mqtt["missingRequiredInfo"] = src.mqtt.missingRequiredInfo;
		status_mqtt["devicesConfigured"] = src.mqtt.devicesConfigured;
		//status_mqtt["publishingEnabled"] = src.mqtt.publishingEnabled;
		status_mqtt["publishingDisabled"] = src.mqtt.publishingDisabled;
		status_mqtt["serverSet"] = src.mqtt.serverSet;
		status_mqtt["nextPublish"] = src.mqtt.nextPublish;
		status_mqtt["nextDisplayMessages"] = src.mqtt.nextDisplayMessages;
		status_mqtt["nextPublishAvailability"] = src.mqtt.nextPublishAvailability;
		status_mqtt["nextMqttConnectAttempt"] = src.mqtt.nextMqttConnectAttempt;
		status_mqtt["nextWarningBlink"] = src.mqtt.nextWarningBlink;

		{
			JsonObject status_mqtt_devices = status_mqtt.createNestedObject("devices");
			status_mqtt_devices["deviceCount"] = src.mqtt.devices.deviceCount;
			status_mqtt_devices["binarySensorCount"] = src.mqtt.devices.binarySensorCount;
			status_mqtt_devices["buttonCount"] = src.mqtt.devices.buttonCount;
			status_mqtt_devices["lightCount"] = src.mqtt.devices.lightCount;
			status_mqtt_devices["sensorCount"] = src.mqtt.devices.sensorCount;
			status_mqtt_devices["switchCount"] = src.mqtt.devices.switchCount;

			#warning create UDF
				JsonObject status_functioning_devices = status_mqtt_devices.createNestedObject("functioningDevices");
			status_functioning_devices["bitmap0"] = src.mqtt.devices.functioningDevices.bitmap0;
			status_functioning_devices["bitmap1"] = src.mqtt.devices.functioningDevices.bitmap1;
			status_functioning_devices["bitmap2"] = src.mqtt.devices.functioningDevices.bitmap2;
			status_functioning_devices["bitmap3"] = src.mqtt.devices.functioningDevices.bitmap3;

			JsonObject status_functioning_devices_important = status_mqtt_devices.createNestedObject("functioningDevicesImportant");
			status_functioning_devices_important["bitmap0"] = src.mqtt.devices.functioningDevices.bitmap0;
			status_functioning_devices_important["bitmap1"] = src.mqtt.devices.functioningDevices.bitmap1;
			status_functioning_devices_important["bitmap2"] = src.mqtt.devices.functioningDevices.bitmap2;
			status_functioning_devices_important["bitmap3"] = src.mqtt.devices.functioningDevices.bitmap3;
		}

		{
			JsonObject status_mqtt_ip = status_mqtt.createNestedObject("ipStatus");
			status_mqtt_ip["ip"].set(src.mqtt.ipStatus.ip);
			status_mqtt_ip["ipIndex"] = src.mqtt.ipStatus.ipIndex;
			status_mqtt_ip["mode"] = (uint8_t)src.mqtt.ipStatus.mode;
			status_mqtt_ip["mode"].set(src.mqtt.ipStatus.mode);
			//status_mqtt_ip["totalAttemptsCounter"] = src.mqtt.ipStatus.totalAttemptsCounter;
			status_mqtt_ip["currentAttemptsCounter"] = src.mqtt.ipStatus.currentAttemptsCounter;
			status_mqtt_ip["maxRetries"] = src.mqtt.ipStatus.maxRetries;
			status_mqtt_ip["changed"] = src.mqtt.ipStatus.changed;
			status_mqtt_ip["triedRetainedIP"] = src.mqtt.ipStatus.triedRetainedIP;
			status_mqtt_ip["triedConfigStation"] = src.mqtt.ipStatus.triedConfigStation;
			status_mqtt_ip["triedConfigAP"] = src.mqtt.ipStatus.triedConfigAP;
			status_mqtt_ip["stationAutoExhausted"] = src.mqtt.ipStatus.stationAutoExhausted;
			status_mqtt_ip["accessPointAutoExhausted"] = src.mqtt.ipStatus.accessPointAutoExhausted;
		}
	}

	//Storage
	{
		JsonObject status_storage = dst.createNestedObject("storage");
		status_storage["fsMounted"] = src.storage.fsMounted;
		status_storage["eepromMounted"] = src.storage.eepromMounted;
	}



#if COMPILE_BACKUP
	//Backup
	{
		JsonObject status_backup = dst.createNestedObject("backup");

		status_backup["ableToBackupEeprom"] = src.backup.ableToBackupEeprom;
		status_backup["backupsDisabled"] = src.backup.backupsDisabled;
		status_backup["eepromBackedUp"] = src.backup.eepromBackedUp;
		status_backup["filesystemBackedUp"] = src.backup.filesystemBackedUp;
	}

#endif	/*COMPILE_BACKUP*/

#if COMPILE_SERVER
	//Server
	{
		JsonObject status_server = dst.createNestedObject("server");

		status_server["enabled"] = src.server.enabled;
		status_server["configured"] = src.server.configured;
		status_server["authenticated"] = src.server.authenticated;
		status_server["authConfigured"] = src.server.authConfigured;
		status_server["specialRequestsConfigured"] = src.server.specialRequestsConfigured;
		status_server["updating"] = (int)src.server.updating;
		status_server["clientIP"] = src.server.clientIP;
		status_server["sessionEnd"] = src.server.sessionEnd;
		#warning convert to string

			//DNS
		{
			JsonObject status_dns = status_server.createNestedObject("dns");
			status_dns["enabled"] = src.server.dns.enabled;
			status_dns["configured"] = src.server.dns.configured;
		}

#if COMPILE_OTA
			//OTA
		{
			JsonObject status_ota = status_server.createNestedObject("ota");
			status_ota["enabled"] = src.server.ota.enabled;
			status_ota["configured"] = src.server.ota.configured;
			status_ota["updating"] = src.server.ota.updating;
		}
#endif

		//Browser
		JsonObject status_browser = status_server.createNestedObject("browser");
		{
			status_browser["enabled"] = src.server.browser.enabled;

#endif	/*COMPILE_SERVER*/

#if COMPILE_SERVER_CONSOLE
			{
				JsonObject status_server_console = status_browser.createNestedObject("console");
				status_server_console["enabled"] = src.server.browser.console.enabled;
				status_server_console["configured"] = src.server.browser.console.configured;
			}
#endif

#if COMPILE_CONFIG_BROWSER
			{
				JsonObject status_server_configBrowser = status_browser.createNestedObject("configBrowser");
				status_server_configBrowser["enabled"] = src.server.browser.configBrowser.enabled;
				status_server_configBrowser["configured"] = src.server.browser.configBrowser.configured;
			}
#endif	/*COMPILE_CONFIG_BROWSER*/

#if COMPILE_CONFIG_BROWSER_MQTT
			{
				JsonObject status_server_mqttConfigBrowser = status_browser.createNestedObject("mqttDevices");
				status_server_mqttConfigBrowser["enabled"] = src.server.browser.mqttDevices.enabled;
				status_server_mqttConfigBrowser["configured"] = src.server.browser.mqttDevices.configured;
			}
#endif	/*COMPILE_CONFIG_BROWSER*/

#if COMPILE_WEBUPDATE
			{
				JsonObject status_server_webUpdate = status_browser.createNestedObject("webUpdate");
				status_server_webUpdate["enabled"] = src.server.browser.updater.enabled;
				status_server_webUpdate["configured"] = src.server.browser.updater.configured;
				status_server_webUpdate["updating"] = src.server.browser.updater.updating;
			}
#endif	/*COMPILE_WEBUPDATE*/

#if COMPILE_BROWSER_TOOLS
			JsonObject status_tools = status_browser.createNestedObject("tools");
			{
#if COMPILE_FILE_EDITOR
				{
					JsonObject status_fileEditor = status_tools.createNestedObject("fileEditor");
					status_fileEditor["enabled"] = src.server.browser.tools.fileEditor.enabled;
					status_fileEditor["configured"] = src.server.browser.tools.fileEditor.configured;
				}
#endif
#if COMPILE_WEB_JSON_VALIDATOR
				{
					JsonObject status_JsonVerify = status_tools.createNestedObject("jsonVerify");
					status_JsonVerify["enabled"] = src.server.browser.tools.jsonVerify.enabled;
					status_JsonVerify["configured"] = src.server.browser.tools.jsonVerify.configured;
				}
#endif
			}
#endif
		}

#if COMPILE_FTP
		//FTP
		{
			JsonObject status_ftp = dst.createNestedObject("ftp");
			status_ftp["enabled"] = src.server.ftp.enabled;
		}
#endif
	}

	//Misc
	{

		JsonObject status_misc = dst.createNestedObject("misc");
		status_misc["version"].set(src.misc.version);
		status_misc["developerMode"] = src.misc.developerMode;
	}

}


bool canConvertFromJson(JsonVariantConst src, const StatusRetained_t&)
{
	return src.containsKey("boot") && src.containsKey("crcs") && src.containsKey("fileSizes") && src.containsKey("mqtt");
}

void convertFromJson(JsonVariantConst src, StatusRetained_t& dst)
{
	//if(src.containsKey("boot"))
	JsonVariantConst boot = src["boot"];
	dst.boot.freshBoot = boot["freshBoot"];
	dst.boot.bootSource = boot["bootSource"].as<ConfigSource>();
	dst.boot.wifiMode = (wifi_mode_t)((WifiMode)boot["wifiMode"].as<WifiMode>());

	JsonVariantConst crcs = src["crcs"];
	dst.crcs.bootFile = crcs["bootFile"];
	dst.crcs.configFile = crcs["configFile"];
	dst.crcs.configPath= crcs["configPath"];
	dst.crcs.eepromBackupFile = crcs["eepromBackupFile"];
	dst.crcs.fileSystemBackupFile = crcs["fileSystemBackupFile"];
	dst.crcs.recentBackup= crcs["recentBackup"];

	JsonVariantConst fileSizes = src["fileSizes"];
	dst.fileSizes.eepromBackup = fileSizes["eepromBackup"];
	dst.fileSizes.fileSystemBackup = fileSizes["fileSystemBackup"];
	dst.fileSizes.recentBackup = fileSizes["recentBackup"];

	JsonVariantConst mqtt = src["mqtt"];
	dst.mqtt.ip = mqtt["ip"].as<IPAddress>();
}

bool convertToJson(const StatusRetained_t& src, JsonVariant dst)
{
	JsonObject retainedStatus_boot = dst.createNestedObject("boot");
	retainedStatus_boot["freshBoot"] = statusRetained.boot.freshBoot;
	retainedStatus_boot["bootSource"].set(statusRetained.boot.bootSource);
	retainedStatus_boot["wifiMode"].set<WifiMode>((WifiMode)statusRetained.boot.wifiMode);

	JsonObject retainedStatus_crcs = dst.createNestedObject("crcs");
	retainedStatus_crcs["bootFile"] = statusRetained.crcs.bootFile;
	retainedStatus_crcs["configPath"] = statusRetained.crcs.configPath;
	retainedStatus_crcs["configFile"] = statusRetained.crcs.configFile;
	retainedStatus_crcs["recentBackup"] = statusRetained.crcs.recentBackup;
	retainedStatus_crcs["fileSystemBackupFile"] = statusRetained.crcs.fileSystemBackupFile;
	retainedStatus_crcs["eepromBackupFile"] = statusRetained.crcs.eepromBackupFile;

	JsonObject retainedStatus_fileSizes = dst.createNestedObject("fileSizes");
	retainedStatus_fileSizes["recentBackup"] = statusRetained.fileSizes.recentBackup;
	retainedStatus_fileSizes["fileSystemBackup"] = statusRetained.fileSizes.fileSystemBackup;
	retainedStatus_fileSizes["eepromBackup"] = statusRetained.fileSizes.eepromBackup;

	JsonObject retainedStatus_mqtt = dst.createNestedObject("mqtt");
	retainedStatus_mqtt["ip"].set(statusRetained.mqtt.ip);
}

#pragma endregion

