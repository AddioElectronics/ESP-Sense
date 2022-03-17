#pragma once

#include <WiFi.h>

#include "../GlobalDefs.h"
#include "config_mqtt.h"
#include "config_master.h"

typedef struct{
	bool enabled : 1;
	bool configured : 1;
}GenericModuleStatus_t;

#pragma region Device Status

typedef struct {
	bool enabled : 1;
	bool wifiTaskRunning : 1;
	bool wifiBlinkTaskRunning : 2;
	bool ftpTaskRunning : 1;
	bool otaTaskRunning : 1;
	bool mqttTaskRunning : 1;
	bool mqttPublishAvailabilityTaskRunning : 1;
	bool mqttDeviceManagerTaskRunning : 1;
	bool mqttBlinkTaskRunning : 1;
}GlobalStatusTasks_t;

typedef struct {
	bool freshBoot : 1;
	bool retainedStatusLoaded : 1;			//Was the retained status succesfully loaded?
	bool i2cInitialized : 1;
	bool fsMounted : 1;						//Was the file system successfully mounted?
	bool eepromMounted : 1;
	unsigned long nextAliveMessage;
	GlobalStatusTasks_t tasks;
}GlobalStatusDevice_t;

#pragma endregion

#pragma region Config

typedef struct {
	uint32_t configCRC;						//Current CRC of the global config after configuration.
	bool ableToBackupEeprom : 1;			//Is the config file small enough to fit on the EEPROM?
	bool backupsDisabled : 1;
	bool eepromBackedUp : 1;				//A backup has been saved to the EEPROM since the last time settings were changed.
	bool filesystemBackedUp : 1;			//A backup has been saved to the File System since the last time settings were changed.
}BackupStatus_t;

typedef struct {
	bool setupComplete : 1;
	bool configRead : 1;					//Was config.json parsed successfully?
	bool hasRequiredData : 1;				//True if config contains all the required data to connect and run. If false, saving to backup will be disabled.
	bool settingsConfigured : 1;			//Has config.json been parsed in to the config structure? (Or default settings set)
	//bool settingsChanged : 1;				//Config has been changed during runtime. **Using combination of eepromBackedUp and filesystemBackedUp instead.
	ConfigSource configSource : 3;			//Where was the config configured from this boot?
	bool pathSet : 1;						//Has the config path been set?
	bool testingConfig : 1;
	bool saveRetainedLoop : 1;				//Has retained status been saved in one of the setup loops? Safety measure incase AP button stuck on.
	char path[CONFIG_PATH_MAX_LENGTH];
	char fileName[CONFIG_PATH_MAX_LENGTH];	
	BackupStatus_t backup;
}GlobalStatusConfig_t;

#pragma endregion

#pragma region Wifi

typedef struct {
	bool enabled : 1;					//If connected is intended to be true. (If connected is false and enabled is true, device will attempt to reconnect)
	bool connected : 1;					//If connected to a network.
	//bool startedConnecting : 1;		//Set when WiFi.begin has been called.
	bool missingRequiredInfo : 1;
	bool gotIP : 1;
	bool eventsRegistered : 1;
	bool blink : 1;
	IPAddress ip;
	unsigned long attemptTimestamp;
}WifiStationStatus_t;

typedef struct {
	bool enabled : 1;					//If connected is intended to be true. (If connected is false and enabled is true, device will attempt to start AP mode.)
	bool connected : 1;					//If softAP mode was succesfully started.
	bool ipAssigned : 1;				//Atleast 1 client has been assigned an IP.
	bool eventsRegistered : 1;
	bool blink : 1;
	uint8_t clientCount;
	IPAddress ip;
}WifiAccessPointStatus_t;

typedef struct {
	//bool enabled : 1;
	bool connected : 1;
	wifi_mode_t mode : 3;
	//WifiMode_t mode : 2;				//Soon to be deprecated.
	bool eventsRegistered : 1;			//Soon to be deprecated, station and access point have their own flag.
	bool configMode : 1;				//Is the device currently in config mode?
	wifi_power_t powerLevel;
	uint32_t connectAttempts;
	unsigned long nextDisplayMessage;
	WifiStationStatus_t station;
	WifiAccessPointStatus_t accessPoint;
}WifiStatus_t;


#pragma endregion

#pragma region Server

//typedef struct {
//	bool enabled : 1;
//	bool configured : 1;
//}DnsStatus_t;

typedef struct  {
	bool enabled : 1;
	bool configured : 1;
	bool updating : 1;
}ServerOtaStatus_t;

typedef struct {
	GenericModuleStatus_t fileEditor;
	GenericModuleStatus_t jsonVerify;
}BrowserToolsStatus_t;

typedef struct {
	bool enabled : 1;
	bool configured : 1;
	GenericModuleStatus_t console;
	GenericModuleStatus_t configBrowser;
	GenericModuleStatus_t mqttDevices;
	ServerOtaStatus_t updater;
	BrowserToolsStatus_t tools;
}BrowserStatus_t;

typedef struct {
	bool enabled : 1;
}FtpStatus_t;

typedef struct {
	bool enabled : 1;
	bool configured : 1;
	bool authenticated : 1;
	bool authConfigured : 1;
	bool specialRequestsConfigured : 1;
	IPAddress clientIP;
	UpdateMode updating;
	unsigned long sessionEnd;
	GenericModuleStatus_t dns;
	BrowserStatus_t browser;
	FtpStatus_t ftp;
	ServerOtaStatus_t ota;
}ServerStatus_t;

#pragma endregion

#pragma region Misc

typedef struct {
	Version_t version;
	bool developerMode : 1;					//Developer mode is only compiled if the definition is true, but can still be disabled at runtime.
}GlobalMiscStatus_t;

#pragma endregion



typedef struct {
	GlobalStatusDevice_t device;
	GlobalStatusConfig_t config;
	WifiStatus_t wifi;
	GlobalMqttStatus_t mqtt;
	ServerStatus_t server;
	GlobalMiscStatus_t misc;
}GlobalStatus_t;



#pragma region Retained

typedef struct {
	uint32_t bootFile;
	uint32_t configPath;
	uint32_t configFile;
	uint32_t recentBackup;
	uint32_t fileSystemBackupFile;
	uint32_t eepromBackupFile;
}Crcs_t;

typedef struct {
	size_t recentBackup;		//Size of the most recent backup.
	size_t fileSystemBackup;	//Size of the backup on the file system
	size_t eepromBackup;		//Size of the backup on the EEPROM.
}FileSizes_t;

///// <summary>
///// Save which modules were functioning to determine if a backup 
///// </summary>
//typedef struct {
//	bool wifiStation : 1;		//Able to connect to WIFI?
//	bool wifiAccessPoint : 1;	//Clients able to connect to access point?
//	bool mqtt : 1;				//Able to connect to the MQTT server?
//	uint16_t mqttDevices;		//How many MQTT devices were able to be configured?
//}FunctionalStatus_t;

typedef struct {
	//char ip[16];				//Last IP that was able to connect.
	IPAddress ip;
}MqttRetainedStatus_t;

typedef struct {
	Boot_bm boot;
	//char lastConfigPath[CONFIG_PATH_MAX_LENGTH];
	Crcs_t crcs;
	FileSizes_t fileSizes;
	MqttRetainedStatus_t mqtt;
	//FunctionalStatus_t functional;
}StatusRetained_t;

typedef union {
	struct {
		BootMonitor_bm boot;
		//bool eepromBackup : 1;
		//bool filesystemBackup : 1;
		//bool lastConfigPath : 1;
		struct {
			bool bootFile : 1;
			bool configPath : 1;
			bool configFile : 1;
			bool recentBackup : 1;
			bool fileSystemBackupFile : 1;
			bool eepromBackupFile : 1;
		}crcs;
		struct {
			bool recentBackup : 1;
			bool fileSystemBackup : 1;
			bool eepromBackup : 1;
		}fileSizes;
		struct {
			bool ip : 1;				//Last IP that was able to connect.
		}mqtt;
	};
	uint32_t bitmap;
}StatusRetainedMonitor_t;

#pragma endregion