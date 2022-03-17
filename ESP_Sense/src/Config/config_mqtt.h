#pragma once

#include <WiFi.h>
#include "../GlobalDefs.h"

//////////////////////////////////////////////////////
//					Broker Settings
//////////////////////////////////////////////////////
#define MQTT_AUTODETECTIP					false							//Try to detect IP address if config fails.
#define MQTT_IP								192,168,0,32					//Pi IP Address
#define MQTT_IP_AP							192,168,4,32					//PI IP Address when connected to ESP Access Point.
//#define mqtt_broker						  "localhost"					//Not Needed 
#define MQTT_WIFI_MODE						WIFI_MODE_APSTA					//How is the PI connected to the network? (Default(Wifi), Router, ESP AP, or both?) *Only valid for "apsta" mode.
#define MQTT_PORT							1883							//Port (Non SSL)
#define MQTT_USER							"homeassistant"					//Broker Username in MQTT Settings
#define MQTT_PASS							"homeassistant"					//Broker Password in MQTT Settings
#define MQTT_TIMEOUT						5								//How many seconds before connection attempt is deemed a failure. *Needs to be low enough to allow FTP and other server functions to run, and high enough to not give up too soon.
#define MQTT_AUTO_IP_TIMEOUT				1								//When auto-detecting IP, it will take n seconds before trying the next IP.
#define MQTT_CONNECTION_INTERVAL			500								//Only try and connect to MQTT every n milliseconds to allow for server handling.
#define MQTT_MAX_RETRIES					10								//How many times will an IP try to connect before moving on to the next?
#define MQTT_AUTO_MAX_RETRIES				2								//How many times will a local IP try to connect before moving on to the next? (Auto-detect Station IPs only)

//Task only calls PubSubClient.Loop()
#define MQTT_TASK_CORE						1
#define MQTT_TASK_PRIORITY					1
#define MQTT_TASK_RECURRATE					1
#define MQTT_TASK_STACKSIZE					8192
//
//#define MQTT_TASK_PUBAV_RECURRATE			5000
//#define MQTT_TASK_DEVMAN_RECURRATE			5000


//LED which is enabled when MQTT is connected.
#if defined(ESP8266)
#define MQTT_LED_GPIO						-1
#elif defined(ESP32)
#define MQTT_LED_GPIO						5
#endif

#define MQTT_LED_ON							LOW						//What voltage level turns the LED on.

//////////////////////////////////////////////////////
//					Publish Settings
//////////////////////////////////////////////////////
#define MQTT_BUFFER_SIZE					256						//The maximum packet size for an MQTT message.
#define MQTT_PUBLISH_RATE					5						//How often should sensor data be sent to Server(Seconds)
#define MQTT_ERROR_PUBLISH_RATE				10						//If a sensor is not connected, how often should the message display (Seconds)?
#define MQTT_AVAILABILITY_PUBLISH_RATE		5						//How often should availability be published?
#define MQTT_PUBLISH_AS_JSON				true					//If true, data will be sent as JSON data.
#define MQTT_PUBLISH_RETAIN					false					//If true, MQTT data will be retained after publishing. *Warning!!! If a sensor stops working, home assistant may not realize.
//#define MQTT_PUBLISH_ON_SINGLE_TOPICS		true					//If true, data will be sent on invidual topics.
#define MQTT_USE_PARENT_TOPICS				false					//(JSON Only) Example: All sensors would publish to "*/sensors/json/state", instead of "*/sensors/waterlevel/json/state"

//Only JSON publishing is supported at the moment.
#if !MQTT_PUBLISH_AS_JSON
#undef MQTT_PUBLISH_AS_JSON
#define MQTT_PUBLISH_AS_JSON true
#endif

//////////////////////////////////////////////////////
//					MQTT Payload Strings
//////////////////////////////////////////////////////
//Move to separate configs
#define MQTT_BINARY_SENSOR_ON			"ON"
#define MQTT_BINARY_SENSOR_OFF			"OFF"

#define MQTT_BUTTON_PRESS				"PRESS"

#define MQTT_SWITCH_ON					"ON"
#define MQTT_SWITCH_OFF					"OFF"

#define MQTT_AVAILABLE					"online"
#define MQTT_UNAVAILABLE				"offline"
#define MQTT_DEVICE_DISABLED			"DISABLED"


//////////////////////////////////////////////////////
//					MQTT Topics
//////////////////////////////////////////////////////
#define TOPIC_BASE									"esp_sense"
#define TOPIC_BASE_AVAILABILITY						"/availability"		
#define TOPIC_BASE_JSON_COMMAND						"/json/command"											
#define TOPIC_BASE_JSON_STATE						"/json/state"													
#define TOPIC_BASE_COMMAND							"/command"
#define TOPIC_BASE_STATE							"/state"

//#define TOPIC_DEVICE_AVAILABILITY					TOPIC_BASE TOPIC_BASE_AVAILABILITY	
//#define TOPIC_JSON_COMMAND							TOPIC_BASE TOPIC_BASE_JSON_COMMAND							//From HA
//#define TOPIC_JSON_STATE							TOPIC_BASE TOPIC_BASE_JSON_STATE							//To HA




#define TOPIC_MESSAGE								TOPIC_BASE "/message"										//To HA		| For displaying a string
#define TOPIC_DEBUG									TOPIC_BASE "/debug"											//To HA		| For displaying debug messages which would usually be displayed in the Serial Console.

//////////////////////////////////////////////////////
//					JSON
//////////////////////////////////////////////////////

#define NAME_MAX_LENGTH			16		//Max length of names(for sensors, and other objects)
#define JSON_STRING_MAX_LENGTH	128		//Max length for any JSON string


//////////////////////////////////////////////////////
//					Debug Messages
//////////////////////////////////////////////////////

#define MQTT_DMSG_RECONNECTED	"Sensor %s reconnected.\r\n"
#define MQTT_DMSG_DISCONNECTED	"Sensor %s lost connection!\r\n"


//Using One JSON topic instead.
//#define TOPIC_SETTINGS_JSON_OUT						TOPIC_BASE "/settings/json/out"								//From HA
//#define TOPIC_SETTINGS_JSON_IN						TOPIC_BASE "/settings/json/in"								//To HA
//
//
//#define TOPIC_SETTINGS_SAVE							TOPIC_BASE "/settings/save"									//From HA
//#define TOPIC_SETTINGS_LOAD_SETTINGS_ON_BOOT		TOPIC_BASE "/settings/loadOnBoot"							//From HA	| Load settings from the ESP8266 EEPROM on boot.
//#define TOPIC_SETTINGS_RESTORE_DEFAULT				TOPIC_BASE "/settings/restoreDefaults"						//From HA	| Erases the saved settings in the EEPROM and restores defaults.
//#define TOPIC_SETTINGS_SERIAL_BAUDRATE_SET			TOPIC_BASE "/settings/serial_baudrate/set"					//From HA	| Set Serial port baud rate
//#define TOPIC_SETTINGS_SERIAL_BAUDRATE_GET			TOPIC_BASE "/settings/serial_baudrate/get"					//To HA		| Get Serial port baud rate
//#define TOPIC_SETTINGS_I2C_FREQUENCY_SET			TOPIC_BASE "/settings/i2c_frequency/set"					//From HA	| Set I2C Frequency
//#define TOPIC_SETTINGS_I2C_FREQUENCY_GET			TOPIC_BASE "/settings/i2c_frequency/get"					//To HA		| Get I2C Frequency
//#define TOPIC_SETTINGS_PUBLISH_INTERVAL_SET			TOPIC_BASE "/settings/publish_interval/set"					//From HA	| Set Minimum publish interval
//#define TOPIC_SETTINGS_PUBLISH_INTERVAL_GET			TOPIC_BASE "/settings/publish_interval/get"					//To HA		| Get Minimum publish interval
//#define TOPIC_SETTINGS_ERROR_PUBLISH_INTERVAL_SET	TOPIC_BASE "/settings/error_publish_interval/set"			//From HA	| Set Minimum error publish interval
//#define TOPIC_SETTINGS_ERROR_PUBLISH_INTERVAL_GET	TOPIC_BASE "/settings/error_publish_interval/get"			//To HA		| Get Minimum error publish interval
////#define TOPIC_SETTINGS_WAIT_TIMEOUT_SET			TOPIC_BASE "/settings/wait_timeout/set"						//From HA
////#define TOPIC_SETTINGS_WAIT_TIMEOUT_GET			TOPIC_BASE "/settings/wait_timeout/get"						//To HA
//
////Temporary Settings
//#define TOPIC_SETTINGS_PUBLISH_ENABLED_SET			TOPIC_BASE "/settings/publish_enabled/set"					//From HA	| Set if the ESP should publish sensor data.
//#define TOPIC_SETTINGS_PUBLISH_ENABLED_GET			TOPIC_BASE "/settings/publish_enabled/get"					//To HA		| Get if the ESP should publish sensor data.

#define MQTT_MAX_DEVICE_COUNT	255

#warning Preprocess to change type depending on MQTT_MAX_DEVICE_COUNT
typedef uint8_t device_count_t;

typedef struct {
	uint64_t bitmap0;
	uint64_t bitmap1;
	uint64_t bitmap2;
	uint64_t bitmap3;
}DevicesFunctioning_t;


typedef struct {
	uint32_t subscribedCount;
	uint32_t enabledCount;
	uint32_t deviceCount;
	device_count_t buttonCount;
	device_count_t switchCount;
	device_count_t lightCount;
	device_count_t sensorCount;
	device_count_t binarySensorCount;
	DevicesFunctioning_t functioningDevices;
	DevicesFunctioning_t functioningDevicesImportant;
}MqttDevicesStatus_t;


#pragma region Config Structures

#warning reminder to remove json or individual
typedef struct {
	uint16_t bufferSize;
	uint16_t rate;
	uint16_t errorRate;
	uint16_t availabilityRate;
	bool json;
	bool onIndividualTopics;
	//String unknownPayload;
}MqttPublishConfig_t;

typedef struct {
	bool autoDetectIp : 1;
	wifi_mode_t wifiMode : 2;
	//char ip[16];
	IPAddress ip;
	IPAddress ipAP;
	uint16_t port;
	String user;
	String pass;
	uint8_t maxRetries;
	uint8_t autoMaxRetries;
	uint16_t autoTimeout;
	uint16_t timeout;
	uint16_t connectInterval;
}MqttBrokerConfig_t;

typedef struct {
	String base;
	String availability;
	String jsonCommand;
	String jsonState;
	String command;
	String state;
}MqttBaseTopics_t;


typedef struct {
	String availability;
	String jsonCommand;		//For receiving commands from HA
	String jsonState;		//For sending commands to HA
	//String command;
	//String state;
}MqttTopics_t;

typedef struct {
	bool useDefaults : 1;
	bool ledOn : 1;
	int8_t ledGpio;
	MqttPublishConfig_t publish;
	MqttBrokerConfig_t broker;
	MqttBaseTopics_t baseTopics;
	MqttTopics_t topics;	
	TaskConfig_t taskSettings;
	//TaskConfig_t taskAvailability;
	//TaskConfig_t taskDeviceManager;
	//TaskConfig_t taskBlink;
}MqttConfig_t;

#pragma endregion

#pragma region Monitor Bitmaps

typedef union {
	struct {
		bool bufferSize : 1;
		bool rate : 1;
		bool errorRate : 1;
		bool availabilityRate : 1;
		bool json : 1;
		bool onIndividualTopics : 1;
		bool unknownPayload : 1;
	};
	uint8_t bitmap;
}MqttPublishConfigMonitor_t;


typedef union {
	struct {
		bool autoDetectIp : 1;
		bool wifiMode : 1;
		bool ip : 1;
		bool ipAP : 1;
		bool port : 1;
		bool user : 1;
		bool pass : 1;
		bool autoTimeout : 1;
		bool timeout : 1;
		bool connectInterval : 1;
		bool maxRetries : 1;
		bool autoMaxRetries : 1;
	};
	uint8_t bitmap;
}MqttBrokerConfigMonitor_t;

typedef union {
	struct {
		bool base : 1;
		bool availability : 1;
		bool jsonCommand : 1;
		bool jsonState : 1;
		bool command : 1;
		bool state : 1;
	};
	uint8_t bitmap;
}MqttBaseTopicsMonitor_t;


typedef union {
	struct {
		bool availability : 1;
		bool jsonCommand : 1;		//For receiving commands from HA
		bool jsonState : 1;		//For sending commands to HA
	//bool command: 1;
	//bool state: 1;
	};
	uint8_t bitmap;
}MqttTopicsMonitor_t;

typedef union {
	struct {
		bool useDefaults : 1;
		bool ledOn : 1;
		bool ledGpio : 1;
		MqttPublishConfigMonitor_t publish;
		MqttBrokerConfigMonitor_t broker;
		MqttBaseTopicsMonitor_t baseTopics;
		MqttTopicsMonitor_t topics;
		TaskConfigMonitor_t taskSettings;
	};
	uint64_t bitmap;
}MqttConfigMonitor_t;
#pragma endregion

typedef struct {
	IPAddress ip;
	uint8_t ipIndex;
	//uint8_t totalAttemptsCounter;		//How many IP's have tried to connect?
	uint8_t currentAttemptsCounter;
	uint8_t maxRetries;
	wifi_mode_t mode : 2;
	bool triedRetainedIP : 1;
	bool triedConfigStation : 1;
	bool triedConfigAP : 1;
	bool stationAutoExhausted : 1;
	bool accessPointAutoExhausted : 1;
	bool changed : 1;
	bool autoDetectingStation : 1;
}GlobalMqttIpStatus_t;

typedef struct {
	bool enabled : 1;
	bool connected : 1;
	bool missingRequiredInfo : 1;
	bool subscribed : 1;
	bool devicesConfigured : 1;
	bool publishingDisabled : 1;
	bool serverSet : 1;
	//bool canPublishErrors : 1;
	uint32_t connectAttempts;
	unsigned long nextPublish;
	unsigned long nextDisplayMessages;
	unsigned long nextPublishAvailability;
	unsigned long nextMqttConnectAttempt;
	unsigned long nextWarningBlink;
	MqttDevicesStatus_t devices;
	GlobalMqttIpStatus_t ipStatus;
}GlobalMqttStatus_t;

