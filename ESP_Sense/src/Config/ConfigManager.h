#ifndef _SETTINGS_h
#define _SETTINGS_h

#warning Reminder to clean this file up, its a bloody mess!

#include <Arduino.h>

#include <ArduinoJson.hpp>
#include <ArduinoJson.h>

#include "config_master.h"
#include "global_status.h"
#include "config_udfs.h"

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
		bool SetConfigFromDoc();
		//void SerializeConfig();
		bool SerializeConfig(JsonDocument* out_doc);
		size_t SaveConfig();

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
		bool DeserializeEepromBackupConfig(JsonDocument& doc);

		void DisableBackups();
		void EnableBackups();
		void SetBackupFlags(uint32_t crc);
		void SetConfigCRCs();
	}


	namespace Status
	{
		/// <summary>
		/// Adds status(GlobalStatus_t) and statusRetained (RetainedStatus_t) into a JSON document.
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

		bool SaveRetainedStatus(bool force = false);

		bool SetRetainedConfigPath(bool saveRetained = false);
	}

	void ApplySettings();

	void ChangeBootSource(enum ConfigSource source, bool saveRetained = false);

	void FactoryReset(bool format = true, bool restart = true, bool keepConnectionSettings = true, bool overwriteConfig = false);



}





#endif


