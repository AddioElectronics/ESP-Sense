#pragma once

#if defined(ESP8266)
#include <HardwareSerial.h>
#elif defined(ESP32)
#include <esp32-hal-uart.h>
#endif

#include <WiFi.h>

#include "../GlobalDefs.h"
#include "config_mqtt.h"
//#include "global_status.h"

#pragma region Default Definitions

#pragma region Compiler
//////////////////////////////////////////////////////
//					Compiler
//////////////////////////////////////////////////////

enum ESP_MEMSIZE
{
	MEMSIZE_NONE = 0,
	MEMSIZE_4MB = 4,
	MEMSIZE_8MB = 8,
	MEMSIZE_16MB = 16
};


/*
	Do not set to enum!
	Set value to what enum equals.
	Preprocessor does not know enum values.
*/
#define ESP_FLASH_SIZE				8	
#define ESP_PSRAM_SIZE				0

#define CPU_DUALCORE				true						//(ESP32 Only)Using a dual core ESP32?

#define COMPILE_DEBUG_CODE			true						//Compile extra code which makes debugging easier, but increases program size.
#define SHOW_DEBUG_MESSAGES			true						//Compiles DEBUG_LOG print functions.
#define DEVELOPER_MODE				true						//Compile extra code which makes life easier when frequently flashing or switching devices(ESP32->8266) *Barely used
#define SERIALIZE_ENUMS_TO_STRING	true						//If true, when serializing config files for saving, enums will be displayed as strings, else serialize to index.
//#define SERIALIZE_ENUMS_TO_INDEX	true						//If true and TO_STRING is false, enums will be displayed as an index from 0 and incrementing, even if the true value is something else.

#define NEWLINE						\r\n


//Max amount of seconds or connection attempts(depending on where loop is) before retained status will save.
//It usually saves at end of setup, but if it can't get there it needs a way to save earlier.
//If EEPROM wear is not a worry to you, set to 0.
//If you have pressed the hotspot button, make sure to wait to allow it to save.
//Will display message on Serial Monitor when saved, as well as blink wifi access point LED.
#define FORCE_SAVE_RETAINED_AFTER	50

//#if COMPILE_VALIDATION == false
//#undef VALIDATE_GPIO
////#undef VALIDATE_DEFAULT_SETTINGS
//#define VALIDATE_GPIO				true
////#define VALIDATE_DEFAULT_SETTINGS	false
//#endif


/*
	Enable or disable which modules are compiled.
	If using a device with small flash you may need to 
	disable features to be able to compile.

	*Makes the code fugly, but its useful. Will clean up later.
*/
#warning Do not change these at the moment, they are untested and could cause exceptions
#define COMPILE_TASKS				true
#define COMPILE_BACKUP				true
#define COMPILE_ACCESSPOINT			true
#define COMPILE_ALIVE_MSG			true

#define COMPILE_VALIDATION			true						//Compile validation code. If enabled your configuration will be checked before running, but the code size will be larger. *Does not enable/disable all validation code.
//#define VALIDATE_DEFAULT_SETTINGS	false						//Validate Default settings (*Should be done at compile time, no need for definition)
//#define VALIDATE_GPIO				false						//Validate if GPIO pins usage is valid or if already in use.
#define VALIDATE_JSON				true						//Validate config values before setting.
#define COMPILE_VALIDATION_ASSERT	true

#define COMPILE_SERVER				true


#define COMPILE_FTP					true
#define COMPILE_FILE_EDITOR			true
#define COMPILE_SERVER_CONSOLE		true
#define COMPILE_OTA					true
#define COMPILE_WEBUPDATE			true
#define COMPILE_MQTT_DEVICE_WEB		true
#define COMPILE_WEB_JSON_VALIDATOR	true
#define COMPILE_CONFIG_BROWSER		true
#define COMPILE_CONFIG_BROWSER_MQTT	true
#define COMPILE_BROWSER_TOOLS		true

#if ESP_FLASH_SIZE < 8
#undef COMPILE_OTA					
#undef COMPILE_WEBUPDATE			
#define COMPILE_OTA					false
#define COMPILE_WEBUPDATE			false
#endif

#if !COMPILE_SERVER
//#undef COMPILE_FTP					
#undef COMPILE_OTA				
#undef COMPILE_WEBUPDATE		
#undef COMPILE_FILE_EDITOR
#undef COMPILE_SERVER_CONSOLE
#undef COMPILE_CONFIG_BROWSER		
#undef COMPILE_MQTT_DEVICE_WEB		
#undef COMPILE_WEB_JSON_VALIDATOR
//#undef COMPILE_FTP false
#define COMPILE_OTA	false
#define COMPILE_WEBUPDATE false
#define COMPILE_FILE_EDITOR false
#define COMPILE_SERVER_CONSOLE false
#define COMPILE_CONFIG_BROWSER false
#define COMPILE_MQTT_DEVICE_WEB false
#define COMPILE_WEB_JSON_VALIDATOR false
#endif

#if /*!COMPILE_FTP && */!COMPILE_SERVER_CONSOLE && !COMPILE_OTA && !COMPILE_WEBUPDATE && !COMPILE_FILE_EDITOR && !COMPILE_CONFIG_BROWSER && !COMPILE_MQTT_DEVICE_WEB && !COMPILE_WEB_JSON_VALIDATOR
#undef COMPILE_SERVER
#define COMPILE_SERVER false
#endif

#if !COMPILE_BROWSER_TOOLS
#undef COMPILE_FILE_EDITOR				
#undef COMPILE_WEB_JSON_VALIDATOR	
#define COMPILE_FILE_EDITOR	false
#define COMPILE_WEB_JSON_VALIDATOR false
#endif

#if !COMPILE_FILE_EDITOR && !COMPILE_WEB_JSON_VALIDATOR
#undef COMPILE_BROWSER_TOOLS
#define COMPILE_BROWSER_TOOLS false
#endif

#pragma endregion

#pragma region GPIO/Pins
//////////////////////////////////////////////////////
//					Pins
//////////////////////////////////////////////////////
//#warning  Reminder to put GPIO in their related regions. (Example : I2C GPIO in an I2C region.)
/*
	Use the GPIO # as different ESP modules have different "D" pin designations.
*/
#define GPIO_LED1						2		//Node MCU ESP-12 (D4)
#if defined(ESP8266)
#define GPIO_LED2						16		//Node MCU ESP-12 (D0)
#elif defined(ESP32)
//#define GPIO_LED2						???	
#endif
#pragma endregion

#pragma region WIFI
//////////////////////////////////////////////////////
//					Wifi
//////////////////////////////////////////////////////
#define WIFI_STATION_ENABLE					true
#define WIFI_SSID							""
#define WIFI_PASS							""
#define WIFI_CHANNEL						0
#define WIFI_BSSID							0
#define WIFI_TX_POWER_LEVEL					WIFI_POWER_19_5dBm
#define WIFI_MESSAGE_DISPLAY_RATE			10


//LED which is enabled when WIFI is connected.
#if defined(ESP8266)
#define WIFI_LED_GPIO						2
#elif defined(ESP32)
#define WIFI_LED_GPIO						2
#endif
#define WIFI_LED_ON							LOW							//What voltage level turns the LED on.

#define WIFI_ACCESSPOINT_ENABLE				false
#define WIFI_ACCESSPOINT_CONFIG_ONLY		false						//Access point mode will only run features which allow configuration. (No MQTT)
#define WIFI_ACCESSPOINT_SSID				"ESP-Sense"
#define WIFI_ACCESSPOINT_PASS				"flashyourbits"					
#define WIFI_ACCESSPOINT_HIDDEN				false						//Hide the SSID?
#define WIFI_ACCESSPOINT_MAX_CONNECTIONS	2							//How many devices can connect to access point?
#define WIFI_ACCESSPOINT_PULLUP				true						//Button pinMode = INPUT_PULLUP?
#define WIFI_ACCESSPOINT_BUTTON_PRESSED		!WIFI_ACCESSPOINT_PULLUP	//What voltage level equals a button press?
#define WIFI_ACCESSPOINT_HOLDTIME			5000						//How long the button must be pressed until it enters.

#if defined(ESP8266)
#define WIFI_ACCESSPOINT_BUTTON_GPIO		-1							//Node MCU ESP-12 (D4)
#define WIFI_ACCESSPOINT_LED_GPIO			GPIO_LED1
#elif defined(ESP32)
#define WIFI_ACCESSPOINT_BUTTON_GPIO		-1							//Node MCU ESP-12 (D4)
#define WIFI_ACCESSPOINT_LED_GPIO			-1							//LED is enabled when client is connected to access point.
#endif
#define WIFI_ACCESSPOINT_LED_ON				LOW							//What voltage level turns the LED on.
#pragma endregion

#define WIFI_TASK_CORE						1							//(ESP32 Only)What core should the tasks run on? *Core 0 is used by internal WIFI SDK, Core 1 runs the Arduino Setup and Loop functions (CONFIG_ARDUINO_RUNNING_CORE). 
#define WIFI_TASK_PRIORITY					1
#define TASK_RECURRANCE_RATE				1000						//(ESP8266 Only)In milliseconds, how often should should the loop task be ran?
#define TASK_STACK_SIZE						8192						//Allocate how many bytes for the Loop task's stack?

#pragma region Device
//////////////////////////////////////////////////////
//					Device
//////////////////////////////////////////////////////
#define DEVICE_NAME						"ESP-Sense"
#define BOOT_DELAY						10
#define DEVICE_BOOT_ORDER				{ConfigSource::CFG_FILESYSTEM, ConfigSource::CFG_EEPROM, ConfigSource::CFG_BACKUP_FILESYSTEM, ConfigSource::CFG_BACKUP_FILESYSTEM}
#define AUTO_BACKUP_MODE				ConfigAutobackupMode::AUTOBACKUP_OFF
#define CRC_ON_BACKUP					true					//Only save backup if the serialized document is different than current backup. *Will take CPU time, but potentially save flash life.

#pragma region I2C

#define I2C_ENABLED						true
#define I2C_FREQUENCY					100000					//(Hz)Default is 100000 (100Khz)

#if defined(ESP8266)
#define GPIO_I2C_SCL					14						//Node MCU ESP-12 (D1)
#define GPIO_I2C_SDA					2						//Node MCU ESP-12 (D2)
#elif defined(ESP32)
#define GPIO_I2C_SCL					22
#define GPIO_I2C_SDA					21
#endif
#pragma endregion

#pragma region Serial

//#define SERIAL_COMMAND_CHAR				'%'					//The character which represents the start of a command.
#warning Serial0 seems to always be enabled for debug features and libraries or an exception happens				
#define SERIAL0_BAUDRATE				115200					//Serial Baudrate
//#define SERIAL0_INVERT					false
#define SERIAL0_TIMEOUT					20000
#define SERIAL0_CONFIG					SERIAL_8N1				//Data bits, Parity, stop bits
#if !defined(SERIAL0_CONFIG)
#define SERIAL0_DATABITS				UART_DATABITS_8	
#define SERIAL0_PARITY					UART_PARITY_NONE
#define SERIAL0_STOPBITS				UART_STOPBITS_1
#define SERIAL0_CONFIG					UART_CONFIG_CONSTANT | SERIAL0_DATABITS | SERIAL0_PARITY | SERIAL0_STOPBITS
#endif

#define SERIAL1_BAUDRATE				115200
#define SERIAL1_INVERT					false
#define SERIAL1_CONFIG					SERIAL_8N1				//Data bits, Parity, stop bits
#define SERIAL1_TIMEOUT					20000
#if !defined(SERIAL1_CONFIG)
#define SERIAL1_DATABITS				UART_DATABITS_8	
#define SERIAL1_PARITY					UART_PARITY_NONE
#define SERIAL1_STOPBITS				UART_STOPBITS_1
#define SERIAL1_CONFIG					UART_CONFIG_CONSTANT | SERIAL1_DATABITS | SERIAL1_PARITY | SERIAL1_STOPBITS
#endif

#if defined(ESP8266)
#define SERIAL_MESSAGE_PORT				0						//The Serial port used for program messages. (Serial0 is always used by libraries)
#define SERIAL_DEBUG_PORT				0						//The Serial port used for debug messsages. 
#elif defined(ESP32)
#define SERIAL_MESSAGE_PORT				1
#define SERIAL_DEBUG_PORT				1
#endif

#if defined(ESP8266)
//#define GPIO_SERIAL0_RX					3		
//#define GPIO_SERIAL0_TX					1

#define GPIO_SERIAL1_RX					-1
#define GPIO_SERIAL1_TX					-1
#elif defined(ESP32)
//#define GPIO_SERIAL0_RX					3		
//#define GPIO_SERIAL0_TX					1

#define GPIO_SERIAL1_RX					16
#define GPIO_SERIAL1_TX					17
#endif

#if (SERIAL_MESSAGE_PORT != 1 && SERIAL_DEBUG_PORT != 1) || (GPIO_SERIAL1_RX < 0 || GPIO_SERIAL1_TX < 0)
#define SERIAL1_ENABLED	false
#else
#define SERIAL1_ENABLED	true
#endif

#if SERIAL_MESSAGE_PORT < 0 || SERIAL_MESSAGE_PORT > 1
#undef SERIAL_MESSAGE_PORT
#define SERIAL_MESSAGE_PORT	0
#endif

#if SERIAL_DEBUG_PORT < 0 || SERIAL_DEBUG_PORT > 1
#undef SERIAL_DEBUG_PORT
#define SERIAL_DEBUG_PORT	0
#endif


#pragma endregion


#pragma region Files
//////////////////////////////////////////////////////
//					Files
//////////////////////////////////////////////////////

#define CONFIG_PATH_MAX_LENGTH			32				//Max length of the config's path.
#define CONFIG_NAME_MAX_LENGTH			16				//Max length of the config's file name.

#define CALCULATE_DOCSIZE_AT_RUNTIME	true			//Size needed to allocate json documents will be calculated(poorly) at runtime. *If ram is a scarce resource, it is recommended to set them manually.
#define FILE_STREAM_TIMEOUT				10000			//Stream timeout for the File object type.

#define DOCUMENT_MAX_SIZE				4096			//Maximum size allocated for a single document.
#define DOC_CONFIG_DESERIALIZE_SIZE		4096			//Amount of bytes allocated for deserializing config.json
#define DOC_CONFIG_SERIALIZE_SIZE		3072			//Amount of bytes allocated for serializing config.json

#define CONFIG_ROOT_PATH				"/"

#define FILE_BOOT_NAME					"boot.json"							
#define FILE_BOOT_PATH					CONFIG_ROOT_PATH FILE_BOOT_NAME		//Extra boot settings stored here.

#define FILE_CONFIG_NAME				"config.json"
#define FILE_CONFIG_PATH				CONFIG_ROOT_PATH FILE_CONFIG_NAME

#if DEVELOPER_MODE
#define FILE_CONFIG_ESP8266_NAME		"config_esp8266.json"
#define FILE_CONFIG_ESP32_NAME			"config_esp32.json"
#define FILE_CONFIG_ESP8266_PATH		CONFIG_ROOT_PATH FILE_CONFIG_ESP8266_NAME
#define FILE_CONFIG_ESP32_PATH			CONFIG_ROOT_PATH FILE_CONFIG_ESP32_NAME
#endif

#define FILE_MQTT_DEVICES_NAME			"config_devices.json"
#define FILE_MQTT_DEVICES_PATH			CONFIG_ROOT_PATH "mqtt/" FILE_MQTT_DEVICES_NAME
#define FILE_MQTT_SENSORS_NAME			"config_sensors.json"
#define FILE_MQTT_SENSORS_PATH			CONFIG_ROOT_PATH "mqtt/devices/" FILE_MQTT_SENSORS_NAME
#define FILE_MQTT_BINARYSENSORS_NAME	"config_binarySensors.json"
#define FILE_MQTT_BINARYSENSORS_PATH	CONFIG_ROOT_PATH "mqtt/devices/" FILE_MQTT_BINARYSENSORS_NAME
#define FILE_MQTT_LIGHTS_NAME			CONFIG_ROOT_PATH "config_lights.json"
#define FILE_MQTT_LIGHTS_PATH			CONFIG_ROOT_PATH "mqtt/devices/" FILE_MQTT_LIGHTS_NAME
#define FILE_MQTT_BUTTONS_NAME			"config_buttons.json"
#define FILE_MQTT_BUTTONS_PATH			CONFIG_ROOT_PATH "mqtt/devices/" FILE_MQTT_BUTTONS_NAME
#define FILE_MQTT_SWITCHES_NAME			"/config_switches.json"
#define FILE_MQTT_SWITCHES_PATH			CONFIG_ROOT_PATH "mqtt/devices/" FILE_MQTT_SWITCHES_NAME
#pragma endregion


#pragma region FTP
//////////////////////////////////////////////////////
//					FTP
//////////////////////////////////////////////////////
#define FTP_ENABLED						true
#define FTP_ANONYMOUS					true
#define FTP_USERNAME					"anonymous"
#define FTP_PASSWORD					"password"
#define FTP_TIMEOUT						30000
#define FTP_TASK_CORE					1
#define FTP_TASK_PRIORITY				1
#define FTP_TASK_RECURRATE				1
#define FTP_TASK_STACKSIZE				4096
#pragma endregion

#pragma region Server
//////////////////////////////////////////////////////
//					Server
/////////////////////////////////////////////////////
#define SERVER_HOSTNAME					"esp-sense"
#define SERVER_DNS_ENABLED				true
#define SERVER_AUTHENTICATE				true					//Require authentication for browser features?
#define SERVER_USER						"admin"
#define SERVER_PASS						"admin"
#define SERVER_UPDATER_ENABLED			false					//Enable WebUpdater during regular operation (non accessPoint mode) (Not currently supported)
#define SERVER_SESSION_TIMEOUT			10						//How many minutes before authentication is lost?


#define BROWSER_ENABLED					true					//Enable configuration browser during regular operation. (non accessPoint mode) (Not currently supported)
#define BROSWER_SSL						false					//Use SSL for browser. (Not currently supported)
#define BROWSER_CONFIG_ENABLED			true					//Host a webpage for configuring the device?
#define BROWSER_CONFIG_MQTT_ENABLED		true					//Host a webpage for configuring the connected MQTT devices?
#define BROWSER_CONSOLE_ENABLED			true					//Host a webpage which emulates the Serial port.
#define BROWSER_UPDATER_ENABLED			true					//Host a webpage for updating firmware?
#define BROWSER_TOOLS_FILE_EDITOR		true					//Host a webpage for editing files?
#define BROWSER_TOOLS_JSON_VERIFY		true					//Host a webpage for verifying JSON files?

#define OTA_ENABLED						true
#define OTA_TASK_CORE					1
#define OTA_TASK_PRIORITY				1
#define OTA_TASK_RECURRATE				1000
#define OTA_TASK_STACKSIZE				4096


#pragma endregion

#pragma endregion




#pragma region Config Structures

typedef union{
	struct {
		bool freshBoot : 1;
		ConfigSource bootSource : 3;
		wifi_mode_t wifiMode : 3;
		bool configMode : 1;
	};
	uint8_t bitmap;
}Boot_bm;


typedef struct {
	bool enabled : 1;
	bool ledOn : 1;
	String ssid;
	String pass;
	int8_t	ledGpio;
}WifiStationConfig_t;

typedef struct {
	bool useDefaults : 1;
	bool enabled : 1;
	bool configOnly : 1;
	bool buttonPullup : 1;
	bool buttonPress : 1;
	bool hidden : 1;
	bool ledOn : 1;
	uint8_t reserved : 1;
	String ssid;
	String pass;
	uint8_t maxConnections;
	uint8_t buttonGpio;
	uint16_t holdTime;
	int8_t ledGpio;
}WifiAccessPointConfig_t;

typedef struct {
	bool useDefaults : 1;
	int32_t channel;
	wifi_power_t powerLevel;

	WifiStationConfig_t station;

#if COMPILE_ACCESSPOINT
	WifiAccessPointConfig_t accessPoint;
#endif
	TaskConfig_t taskSettings;
}WifiConfig_t;


typedef struct {
	bool useDefaults : 1;
	bool enabled : 1;
	bool anonymous : 1;
	uint8_t reserved : 5;
	String user;
	String pass;
	uint32_t timeout;
	TaskConfig_t taskSettings;
}FtpConfig_t;

typedef struct {
	bool useDefaults : 1;
	bool enabled : 1;
	bool config : 1;				//Enable ConfigBrowser?
	bool console : 1;				//Enable debug console?
	bool updater : 1;
	bool mqttDevices : 1;		//Enable the ability to configure MQTT devices on a webpage?
	bool ssl : 1;
	uint8_t reserved : 2;
	struct {
		bool fileEditor : 1;
		bool jsonVerify : 1;
		uint8_t reserved : 6;
	}tools;
}BrowserConfig_t;

typedef struct {
	bool useDefaults : 1;
	bool enabled : 1;
	TaskConfig_t taskSettings;
}OtaConfig_t;

typedef struct {
	bool useDefaults : 1;
	bool dns : 1;
	bool authenticate : 1;				//Require authentication
	uint8_t reserved : 5;
	String hostname;
	String user;
	String pass;
	unsigned long sessionTimeout;
	BrowserConfig_t browser;
	FtpConfig_t ftp;
	OtaConfig_t ota;
}ServerConfig_t;

typedef struct {
	bool useDefaults : 1;
	bool enabled : 1;
	bool invert : 1;
	uint8_t reserved : 5;
	unsigned long baud;
	int8_t rxGpio;
	int8_t txGpio;
	uint32_t timeout;
	uint32_t config;
//	union {
//#warning union will not work on its own, remember to finish this.
//		UART_DATABITS dataBits;
//		UART_PARITY parity;
//		UART_STOPBITS stopBits;
//		uint32_t bitmap;
//	}config;
}SerialPortConfig_t;

typedef struct {
	bool useDefaults : 1;
	uint8_t reserved : 7;
	int8_t messagePort;
	int8_t debugPort;
	SerialPortConfig_t port0;
	SerialPortConfig_t port1;
}SerialConfig_t;

typedef struct {
	bool useDefaults : 1;
	bool enabled : 1;
	uint8_t reserved : 7;
	int8_t sclGpio;
	int8_t sdaGpio;
	uint32_t freq;
}I2cConfig_t;

typedef struct {
	bool useDefaults : 1;
	ConfigAutobackupMode_t autoBackupMode : 2;
	//bool crcOnBackup : 1;
	uint8_t reserved : 5;
	SerialConfig_t serial;
	I2cConfig_t i2c;
}ConfigDevice_t;

typedef struct{
	WifiConfig_t wifi;
	MqttConfig_t mqtt;
	ConfigDevice_t device;
	ServerConfig_t server;
}Config_t;

#pragma endregion

#pragma region Monitor Bitmaps

typedef union {
	struct {
		bool freshBoot : 1;
		bool bootSource : 1;
		bool wifiMode : 1;
		bool configMode : 1;
		uint8_t reserved : 4;
	};
	uint8_t bitmap;
}BootMonitor_bm;



typedef union {
	struct {
		bool useDefaults : 1;
		bool channel : 1;
		bool powerLevel : 1;
		struct {
			bool enabled : 1;
			bool ledOn : 1;
			bool ssid : 1;
			bool pass : 1;
			bool ledGpio : 1;
		}station;
		struct {
			bool useDefaults : 1;
			bool enabled : 1;
			bool configOnly : 1;
			bool ssid : 1;
			bool pass : 1;
			bool maxConnections : 1;
			bool hidden : 1;
			bool buttonGpio : 1;
			bool buttonPullup : 1;
			bool buttonPress : 1;	
			bool holdTime : 1;
			bool ledGpio : 1;
			bool ledOn : 1;
		}accessPoint;
		TaskConfigMonitor_t taskSettings;
	};
	uint32_t bitmap;
}WifiConfigMonitor_t;

typedef union {
	struct {
		bool useDefaults : 1;
		bool enabled : 1;
		bool anonymous : 1;
		bool user : 1;
		bool pass : 1;
		bool timeout : 1;
		TaskConfigMonitor_t taskSettings;
	};
	uint16_t bitmap;
}FtpConfigMonitor_t;

typedef union {
	struct {
		bool useDefaults : 1;
		bool dns : 1;
		bool hostname : 1;
		bool authenticate : 1;
		bool user : 1;
		bool pass : 1;
		bool sessionTimeout : 1;
		struct {
			bool useDefaults : 1;
			bool enabled : 1;			
			bool config : 1;
			bool console : 1;
			bool updater : 1;
			bool mqttDevices : 1;
			bool ssl : 1;
			struct {
				bool fileEditor : 1;
				bool jsonVerify : 1;
			}tools;
		}browser;
		FtpConfigMonitor_t ftp;
		struct {
			bool useDefaults : 1;
			bool enabled : 1;
			TaskConfigMonitor_t taskSettings;
		}ota;
	};
	uint32_t bitmap;
}ServerConfigMonitor_t;

typedef union {
	struct {
		bool useDefaults : 1;
		bool enabled : 1;
		bool invert : 1;
		bool baud : 1;
		bool rxGpio : 1;
		bool txGpio : 1;
		bool timeout : 1;
		bool config : 1;
	};
	uint8_t bitmap;
}SerialPortConfigMonitor_t;

typedef union {
	struct {
		bool useDefaults : 1;
		bool messagePort : 1;
		bool debugPort : 1;
		SerialPortConfigMonitor_t port0;
		SerialPortConfigMonitor_t port1;
	};
	uint32_t bitmap;
}SerialConfigMonitor_t;

typedef union {
	struct {
		bool useDefaults : 1;
		bool autoBackupMode : 1;
		//bool crcOnBackup : 1;
		SerialConfigMonitor_t serial;
		struct {
			bool useDefaults : 1;
			bool enabled : 1;
			bool sclGpio : 1;
			bool sdaGpio : 1;
			bool freq : 1;
		}i2c;
	};
	uint64_t bitmap;
}ConfigDeviceMonitor_t;

/// <summary>
/// Used to determine which settings have been changed.
/// </summary>
typedef struct {
	WifiConfigMonitor_t wifi;
	MqttConfigMonitor_t mqtt;
	ConfigDeviceMonitor_t device;
	ServerConfigMonitor_t server;
}ConfigMonitor_t;

#pragma endregion




/// <summary>
/// Used to select which parts of the config will be updated.
/// (Unused)
/// </summary>
typedef union
{
	struct {
		bool device : 1;
		bool wifi : 1;
		bool mqtt : 1;
		bool ftp : 1;
		bool server : 1;
		uint8_t reserved : 3;
	};
	uint8_t bitmap;
}ConfigBitmap_bm;