#pragma once

#include <WiFi.h>

typedef struct {
	uint8_t major;
	uint8_t minor;
	uint16_t revision;
}Version_t;

enum class UpdateMode {
	ESP_UPDATE_NULL,
	ESP_UPDATE_WEB,
	ESP_UPDATE_OTA	
};

typedef struct {
	bool useDefaults : 1;
	uint8_t core : 2;
	int8_t priority;
	uint16_t stackSize;
	uint32_t recurRate;	//(ESP8266 only)How many microseconds until the loop task is restarted?
}TaskConfig_t;

typedef struct {
	bool useDefaults : 1;
	bool core : 1;
	bool priority : 1;
	bool stackSize : 1;
	bool recurRate : 1;
}TaskConfigMonitor_t;

/// <summary>
/// Used to set and monitor the configuration source.
/// </summary>
enum class ConfigSource
{
	CFG_DEFAULT = 0b00,				//Default source, used to determine when it hasn't been changed.
	CFG_FILESYSTEM = 0b01,
	CFG_EEPROM = 0b10,
	CFG_BACKUP_FILESYSTEM = 0b11,
	CFG_FIRMWARE = 0b100			//Firmware defaults
};
typedef uint8_t ConfigSource_t;

//Soon to be deprecated.
enum class WifiMode
{
	//WIFI_MODE_DEFAULT = 0,
	//WIFI_MODE_STATION = 1,
	//WIFI_MODE_ACCESSPOINT = 2,
	//WIFI_MODE_APSTATION = 3,
	//WIFI_MODE_MAX = 4
};
typedef uint8_t WifiMode_t;
#define WIFI_MODE_T	(wifi_mode_t)WifiMode

//enum class DeviceUpdaterMode
//{ 
//	USB_UPDATER = 0, //USB Only
//	HTTP_UPDATER = 1, 
//	COM_UPDATER = 2,
//	HTTP_AND_COM_UPDATER = 3
//};
//typedef uint8_t DeviceUpdaterMode_t;
//#define DEVICEUPDATERMODE_T	(DeviceUpdaterMode_t)DeviceUpdaterMode


enum class ConfigAutobackupMode
{ 
	AUTOBACKUP_OFF = 0, 
	AUTOBACKUP_MODE_FS = 1, 
	AUTOBACKUP_MODE_EEPROM = 2, 
	AUTOBACKUP_FS_AND_EEPROM = 3
};
typedef uint8_t ConfigAutobackupMode_t;
#define CONFIGAUTOBACKUPMODE_T	(ConfigAutobackupMode_t)ConfigAutobackupMode

enum ConfigBackup
{
	CONFIG_BACKUP_NONE = 0,
	CONFIG_BACKUP_FS = 1,
	CONFIG_BACKUP_EEPROM = 2
};
typedef uint8_t ConfigBackup_t;


#pragma region UART Config

#define UART_CONFIG_CONSTANT		0x8000000

#define UART_CONFIG_DATABITS_MASK	0b001100
#define UART_CONFIG_PARITY_MASK		0b000011
#define UART_CONFIG_STOPBITS_MASK	0b110000

enum class UART_DATABITS
{
	DATABITS_5 = 0b0000,
	DATABITS_6 = 0b0100,
	DATABITS_7 = 0b1000,
	DATABITS_8 = 0b1100
};
typedef uint8_t UART_DATABITS_t;
#define UART_DATABITS_T	(UART_DATABITS_t)UART_DATABITS



enum class UART_PARITY
{
	PARITY_NONE = 0b00,
	PARITY_EVEN = 0b10,
	PARITY_ODD = 0b11
};
typedef uint8_t UART_PARITY_t;
#define UART_PARITY_T	(UART_PARITY_t)UART_PARITY


enum class UART_STOPBITS
{
	STOPBITS_1 = 0b010000,
	STOPBITS_2 = 0b110000
};
typedef uint8_t UART_STOPBITS_t;
#define UART_STOPBITS_T	(UART_STOPBITS_t)UART_STOPBITS

#pragma endregion

typedef void (*FUNC_PTR)(void);

typedef void (*MQTT_CB)(char* topic, byte* payload, unsigned int length);


extern const char* config_source_strings[5];
extern const char* wifi_power_strings[12];
extern const char* esp_updater_strings[4];
extern const char* esp_autobackup_strings[4];
extern const char* wifi_mode_strings[4];
extern const char* uart_parity_strings[3];

extern ConfigSource config_source_values[5];
extern wifi_power_t wifi_power_values[12];
extern UART_DATABITS uart_databits_values[4];
extern UART_PARITY uart_parity_values[3];
extern UART_STOPBITS uart_stopbits_values[2];



#pragma region JSON UDFs

bool canConvertFromJson(JsonVariantConst src, const Version_t&);
void convertFromJson(JsonVariantConst src, Version_t& dst);
bool convertToJson(const Version_t& src, JsonVariant dst);

#pragma endregion
