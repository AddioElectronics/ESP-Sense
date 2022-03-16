#ifndef _SETTINGS_h
#define _SETTINGS_h

#warning Reminder to clean this file up, its a bloody mess!

#include <Arduino.h>

#include <ArduinoJson.hpp>
#include <ArduinoJson.h>

#include "config_master.h"

namespace Config
{
	//#warning reminder to clean this up, and reorder CPP file.

	namespace Defaults
	{
		void SetAll(bool flagMonitor = true);

		void InitializeStrings();

		void RestoreDeviceDefaults(bool flagMonitor = true);

		void RestoreWifiDefaults(bool flagMonitor = true);

		void RestoreMqttDefaults(bool flagMonitor = true);

		//void RestoreDocDefaults(bool flagMonitor);

#if COMPILE_FTP
		void RestoreFtpDefaults(bool flagMonitor = true);
#endif

#if COMPILE_SERVER
		void RestoreServerDefaults(bool flagMonitor = true);
#endif
	}

	namespace Documents
	{
		void LoadBootSettings();

		int CheckConfigCrc();
		int CheckConfigPathCrc();

		bool LoadConfiguration();
		void Reconfigure();

		/// <summary>
		/// Selects the which file to use depending on program state.
		/// </summary>
		/// <param name="backup">Will select backup config on the file system.</param>
		/// <returns>True if the file exists, false if it was unable to find the config.</returns>
		bool SelectConfigPath(bool backup);

		/// <summary>
		/// Sets status.config.path and status.config.fileName,
		/// only if the file exists.
		/// </summary>
		/// <param name="path"></param>
		/// <param name="filename">(Optional)If left null, the filename will be extracted from the path.</param>
		/// <returns>If the path and filename were set. False if the file did not exist.</returns>
		bool SetConfigPathAndFileName(const char* path, const char* filename = nullptr);

		bool GenerateBackupPath(const char* path, String* backupPath, String* backupName = nullptr);

		bool DeserializeConfig();
		//void SerializeConfig();
		bool SerializeConfig(JsonDocument* out_doc);
		size_t SaveConfig();


		bool SetConfigFromDoc();

		bool SetDeviceFromDoc();
		bool SetWifiFromDoc();
		bool SetMqttFromDoc();
		//bool SetDocFromDoc();
#if COMPILE_FTP
		bool SetFtpFromDoc();
#endif

#if COMPILE_SERVER
		bool SetServerFromDoc();
#endif


	}

	namespace Backup
	{
		//#warning if config path has changed, should handle saving backups differently.
		bool SaveBackupConfig(bool fs = true, bool eeprom = true, bool saveRetained = false, size_t* out_sizeFileSystem = nullptr, size_t* out_sizeEeprom = nullptr);
		bool AutoSaveBackupConfig(bool saveRetained = false, size_t* out_sizeFileSystem = nullptr, size_t* out_sizeEeprom = nullptr);
		size_t SaveBackupEeprom(bool saveRetained = false);
		size_t SaveBackupEeprom(JsonDocument* doc, bool saveRetained = false, const uint32_t crc = 0);
		size_t SaveBackupFilesystem(bool saveRetained);
		size_t SaveBackupFilesystem(JsonDocument* doc, bool saveRetained = false, const uint32_t crc = 0);
		bool DeserializeEepromBackupConfig();

		void DisableBackups();
		void EnableBackups();
		void SetBackupFlags(uint32_t crc);
	}


	namespace Status
	{
		/// <summary>
		/// Adds status(DeviceStatus_t) and statusRetained (RetainedStatus_t) into a JSON document.
		/// </summary>
		int PackDeviceStatus(JsonObject& obj);

		///// <summary>
		///// Serializes the device status structure to a String.
		///// If you do not wish to serialize at this time, pass nullptr to "serializeTo", but you must pass a document pointer to "out_doc".
		///// </summary>
		///// <param name="serializeTo">String to serialize to. If nullptr the document will not be serialized(must pass out_doc).</param>
		///// <param name="pretty">True will serialize to human readable form(Newlines, tabs, ect..), false will serialize all to a single line.</param>
		///// <param name="out_doc">Get the serialized JsonDocument after serialization. Warning : Must free if used.</param>
		///// <returns>Size of the serialized document.</returns>
		//size_t SerializeDeviceStatus(String* serializeTo, DynamicJsonDocument** out_doc = nullptr);
		//size_t SerializeDeviceStatus(DynamicJsonDocument** out_doc);

		bool SaveRetainedStatus();

		bool SetRetainedConfigPath(bool saveRetained = false);
	}

	void ApplySettings();

	void ChangeBootSource(enum ConfigSource source, bool saveRetained = false);

	void FactoryReset(bool format = true, bool restart = true, bool keepConnectionSettings = true, bool overwriteConfig = false);


	namespace Parsers
	{
		void ParseSerialPort(JsonObject& portObj, SerialPortConfig_t& port, SerialPortConfigMonitor_t& portMonitor);
	}

}


#pragma region Json UDFs

bool convertToJson(const DeviceStatus_t& src, JsonVariant dst);

bool canConvertFromJson(JsonVariantConst src, const StatusRetained_t&);
void convertFromJson(JsonVariantConst src, StatusRetained_t& dst);
bool convertToJson(const StatusRetained_t& src, JsonVariant dst);


//bool canConvertFromJson(JsonVariantConst src, const Boot_bm&);
void convertFromJson(JsonVariantConst src, Boot_bm& dst);
bool convertToJson(const Boot_bm& src, JsonVariant dst);

//bool canConvertFromJson(JsonVariantConst src, const WifiStationConfig_t&);
void convertFromJson(JsonVariantConst src, WifiStationConfig_t& dst);
bool convertToJson(const WifiStationConfig_t& src, JsonVariant dst);

//bool canConvertFromJson(JsonVariantConst src, const WifiAccessPointConfig_t&);
void convertFromJson(JsonVariantConst src, WifiAccessPointConfig_t& dst);
bool convertToJson(const WifiAccessPointConfig_t& src, JsonVariant dst);

//bool canConvertFromJson(JsonVariantConst src, const WifiConfig_t&);
void convertFromJson(JsonVariantConst src, WifiConfig_t& dst);
bool convertToJson(const WifiConfig_t& src, JsonVariant dst);

//bool canConvertFromJson(JsonVariantConst src, const FtpConfig_t&);
void convertFromJson(JsonVariantConst src, FtpConfig_t& dst);
bool convertToJson(const FtpConfig_t& src, JsonVariant dst);

//bool canConvertFromJson(JsonVariantConst src, const BrowserConfig_t&);
void convertFromJson(JsonVariantConst src, BrowserConfig_t& dst);
bool convertToJson(const BrowserConfig_t& src, JsonVariant dst);

//bool canConvertFromJson(JsonVariantConst src, const OtaConfig_t&);
void convertFromJson(JsonVariantConst src, OtaConfig_t& dst);
bool convertToJson(const OtaConfig_t& src, JsonVariant dst);

//bool canConvertFromJson(JsonVariantConst src, const ServerConfig_t&);
void convertFromJson(JsonVariantConst src, ServerConfig_t& dst);
bool convertToJson(const ServerConfig_t& src, JsonVariant dst);

//bool canConvertFromJson(JsonVariantConst src, const SerialPortConfig_t&);
void convertFromJson(JsonVariantConst src, SerialPortConfig_t& dst);
bool convertToJson(const SerialPortConfig_t& src, JsonVariant dst);

//bool canConvertFromJson(JsonVariantConst src, const SerialConfig_t&);
void convertFromJson(JsonVariantConst src, SerialConfig_t& dst);
bool convertToJson(const SerialConfig_t& src, JsonVariant dst);

//bool canConvertFromJson(JsonVariantConst src, const ConfigDevice_t&);
void convertFromJson(JsonVariantConst src, ConfigDevice_t& dst);
bool convertToJson(const ConfigDevice_t& src, JsonVariant dst);

//bool canConvertFromJson(JsonVariantConst src, const Config_t&);
void convertFromJson(JsonVariantConst src, Config_t& dst);
bool convertToJson(const Config_t& src, JsonVariant dst);

//bool canConvertFromJson(JsonVariantConst src, const I2cConfig_t&);
void convertFromJson(JsonVariantConst src, I2cConfig_t& dst);
bool convertToJson(const I2cConfig_t& src, JsonVariant dst);

//bool canConvertFromJson(JsonVariantConst src, const Config_t&);
void convertFromJson(JsonVariantConst src, Config_t& dst);
bool convertToJson(const Config_t& src, JsonVariant dst);

#pragma region Mqtt UDFs

//bool canConvertFromJson(JsonVariantConst src, const MqttPublishConfig_t&);
void convertFromJson(JsonVariantConst src, MqttPublishConfig_t& dst);
bool convertToJson(const MqttPublishConfig_t& src, JsonVariant dst);

//bool canConvertFromJson(JsonVariantConst src, const MqttBrokerConfig_t&);
void convertFromJson(JsonVariantConst src, MqttBrokerConfig_t& dst);
bool convertToJson(const MqttBrokerConfig_t& src, JsonVariant dst);

//bool canConvertFromJson(JsonVariantConst src, const MqttTopics_t&);
void convertFromJson(JsonVariantConst src, MqttTopics_t& dst);
bool convertToJson(const MqttTopics_t& src, JsonVariant dst);

//bool canConvertFromJson(JsonVariantConst src, const MqttBaseTopics_t&);
void convertFromJson(JsonVariantConst src, MqttBaseTopics_t& dst);
bool convertToJson(const MqttBaseTopics_t& src, JsonVariant dst);

//bool canConvertFromJson(JsonVariantConst src, const MqttConfig_t&);
void convertFromJson(JsonVariantConst src, MqttConfig_t& dst);
bool convertToJson(const MqttConfig_t& src, JsonVariant dst);

//bool canConvertFromJson(JsonVariantConst src, const DevicesFunctioning_t&);
//void convertFromJson(JsonVariantConst src, DevicesFunctioning_t& dst);
bool convertToJson(const DevicesFunctioning_t& src, JsonVariant dst);

//bool canConvertFromJson(JsonVariantConst src, const MqttDevicesStatus_t&);
//void convertFromJson(JsonVariantConst src, MqttDevicesStatus_t& dst);
bool convertToJson(const MqttDevicesStatus_t& src, JsonVariant dst);

#pragma endregion




#pragma endregion


#endif


