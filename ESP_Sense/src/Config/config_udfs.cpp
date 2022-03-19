#include "config_udfs.h"

#include "../JsonHelper.h"
#include "../Macros.h"

extern Config_t config;
extern ConfigMonitor_t configMonitor;
extern GlobalStatus_t status;
extern ConfigBitmap_bm configBitmap;
extern StatusRetained_t statusRetained;
extern StatusRetainedMonitor_t statusRetainedMonitor;

bool canConvertFromJson(JsonVariantConst src, const Version_t&)
{
	return src.is<JsonArrayConst>() && src.as<JsonArrayConst>().size() == 3;
}

void convertFromJson(JsonVariantConst src, Version_t& dst)
{
	JsonArrayConst jarray = src.as<JsonArrayConst>();
	DEBUG_LOG_F("Convert JSON to Version : size %d\r\n", jarray.size());
	dst.major = src[0];
	dst.minor = src[1];
	dst.revision = src[2];

	DEBUG_LOG_F("Doc Version %d.%d.%d\r\n", dst.major, dst.minor, dst.revision);
}

bool convertToJson(const Version_t& src, JsonVariant dst)
{
	DEBUG_LOG_LN("Convert Version to JSON");
	DEBUG_LOG_F("Doc Version %d.%d.%d\r\n", src.major, src.minor, src.revision);
	JsonArray jarray = dst.as<JsonArray>();

	DEBUG_LOG_F("JARRAY NULL? %d\r\nis array? %d\r\n", jarray.isNull(), dst.is<JsonArray>());

	jarray.add(src.major);
	jarray.add(src.minor);
	jarray.add(src.revision);

	DEBUG_LOG_F("Array Version %d.%d.%d\r\n", (const char*)jarray[0], (const char*)jarray[1], (const char*)jarray[2]);

	return true;
}

bool convertToJson(const GenericModuleStatus_t& src, JsonVariant dst)
{
	dst["enabled"] = src.enabled;
	dst["configured"] = src.configured;
}

bool convertToJson(const GlobalStatusTasks_t& src, JsonVariant dst)
{
	dst["enabled"] = src.enabled;
	dst["wifiTaskRunning"] = src.wifiTaskRunning;
	dst["wifiBlinkTaskRunning"] = src.wifiBlinkTaskRunning;
	dst["ftpTaskRunning"] = src.ftpTaskRunning;
	dst["otaTaskRunning"] = src.otaTaskRunning;
	dst["mqttPublishAvailabilityTaskRunning"] = src.mqttPublishAvailabilityTaskRunning;
	dst["mqttDeviceManagerTaskRunning"] = src.mqttDeviceManagerTaskRunning;
	dst["mqttDeviceManagerTaskRunning"] = src.mqttDeviceManagerTaskRunning;
	dst["mqttBlinkTaskRunning"] = src.mqttBlinkTaskRunning;
}

bool convertToJson(const GlobalStatusDevice_t& src, JsonVariant dst)
{
	dst["freshBoot"] = src.freshBoot;
	dst["retainedStatusLoaded"] = src.retainedStatusLoaded;
	dst["i2cInitialized"] = src.i2cInitialized;
	dst["fsMounted"] = src.fsMounted;
	dst["eepromMounted"] = src.eepromMounted;
	dst["nextAliveMessage"] = src.nextAliveMessage;
	dst["tasks"].set(src.tasks);
}


bool convertToJson(const BackupStatus_t& src, JsonVariant dst)
{
	dst["configFileCRC"] = src.configFileCRC;
	dst["configPathCRC"] = src.configPathCRC;
	dst["configDocCRC"] = src.configDocCRC;
	dst["ableToBackupEeprom"] = src.ableToBackupEeprom;
	dst["backupsDisabled"] = src.backupsDisabled;
	dst["eepromBackedUp"] = src.eepromBackedUp;
	dst["filesystemBackedUp"] = src.filesystemBackedUp;
}

bool convertToJson(const GlobalStatusConfig_t& src, JsonVariant dst)
{
	dst["setupComplete"] = src.setupComplete;
	dst["configRead"] = src.configRead;
	dst["hasRequiredData"] = src.hasRequiredData;
	dst["settingsConfigured"] = src.settingsConfigured;
	dst["configSource"].set(src.configSource);
	dst["pathSet"] = src.pathSet;
	dst["testingConfig"] = src.testingConfig;
	dst["saveRetainedLoop"] = src.saveRetainedLoop;
	dst["path"] = src.path;
	dst["fileName"] = src.fileName;
	dst["backup"].set(src.backup);
}


bool convertToJson(const WifiStationStatus_t& src, JsonVariant dst)
{
	dst["enabled"] = src.enabled;
	dst["connected"] = src.connected;
	dst["missingRequiredInfo"] = src.missingRequiredInfo;
	dst["gotIP"] = src.gotIP;
	dst["eventsRegistered"] = src.eventsRegistered;
	dst["blink"] = src.blink;
	dst["ip"].set(src.ip);
	dst["attemptTimestamp"] = src.attemptTimestamp;
}

bool convertToJson(const WifiAccessPointStatus_t& src, JsonVariant dst)
{
	dst["enabled"] = src.enabled;
	dst["connected"] = src.connected;
	dst["ipAssigned"] = src.ipAssigned;
	dst["eventsRegistered"] = src.eventsRegistered;
	dst["blink"] = src.blink;
	dst["clientCount"] = src.clientCount;
	dst["ip"].set(src.ip);
}

bool convertToJson(const WifiStatus_t& src, JsonVariant dst)
{
	dst["connected"] = src.connected;
	dst["mode"].set<WifiMode>((WifiMode)src.mode);
	dst["eventsRegistered"] = src.eventsRegistered;
	dst["configMode"] = src.configMode;
	dst["powerLevel"].set<WifiPower>((WifiPower)src.powerLevel);
	dst["connectAttempts"] = src.connectAttempts;
	dst["nextDisplayMessage"] = src.nextDisplayMessage;
	dst["station"].set(src.station);
	dst["accessPoint"].set(src.accessPoint);
}


bool convertToJson(const BrowserToolsStatus_t& src, JsonVariant dst)
{
	dst["fileEditor"].set(src.fileEditor);
	dst["jsonVerify"].set(src.jsonVerify);
}

bool convertToJson(const BrowserStatus_t& src, JsonVariant dst)
{
	dst["enabled"] = src.enabled;
	dst["configured"] = src.configured;
	dst["console"].set(src.console);
	dst["configBrowser"].set(src.configBrowser);
	dst["mqttDevices"].set(src.mqttDevices);
	dst["updater"].set(src.updater);
	dst["tools"].set(src.tools);
}

bool convertToJson(const ServerOtaStatus_t& src, JsonVariant dst)
{
	dst["enabled"] = src.enabled;
	dst["configured"] = src.configured;
	dst["updating"] = src.updating;
}

bool convertToJson(const FtpStatus_t& src, JsonVariant dst)
{
	dst["enabled"] = src.enabled;
}


bool convertToJson(const ServerStatus_t& src, JsonVariant dst)
{
	dst["enabled"] = src.enabled;
	dst["configured"] = src.configured;
	dst["authenticated"] = src.authenticated;
	dst["authConfigured"] = src.authConfigured;
	dst["enabled"] = src.enabled;
	dst["specialRequestsConfigured"].set(src.specialRequestsConfigured);
	dst["clientIP"].set(src.clientIP);
	dst["sessionEnd"] = src.sessionEnd;
	dst["dns"].set(src.dns);
	dst["browser"].set(src.browser);
	dst["ftp"].set(src.ftp);
	dst["ota"].set(src.ota);
}

bool convertToJson(const GlobalMiscStatus_t& src, JsonVariant dst)
{
	dst["version"].set(src.version);
	dst["developerMode"] = src.developerMode;
}


bool convertToJson(const GlobalStatus_t& src, JsonVariant dst)
{
	dst["wifi"].set(src.wifi);
	dst["mqtt"].set(src.mqtt);
	dst["device"].set(src.device);
	dst["config"].set(src.config);
	dst["server"].set(src.server);
	dst["misc"].set(src.misc);
}

//bool canConvertFromJson(JsonVariantConst src, const Crcs_t&)
//{
//}

void convertFromJson(JsonVariantConst src, Crcs_t& dst)
{
	dst.bootFile = src["bootFile"];
	dst.configPath = src["configPath"];
	dst.configFile = src["configFile"];
	dst.configFileAtBackup = src["configFileAtBackup"];
	dst.recentBackup = src["recentBackup"];
	dst.fileSystemBackupFile = src["fileSystemBackupFile"];
	dst.eepromBackupFile = src["eepromBackupFile"];
}

bool convertToJson(const Crcs_t& src, JsonVariant dst)
{
	dst["bootFile"] = src.bootFile;
	dst["configPath"] = src.configPath;
	dst["configFile"] = src.configFile;
	dst["configFileAtBackup"] = src.configFileAtBackup;
	dst["recentBackup"] = src.recentBackup;
	dst["fileSystemBackupFile"] = src.fileSystemBackupFile;
	dst["eepromBackupFile"] = src.eepromBackupFile;
}

//bool canConvertFromJson(JsonVariantConst src, const FileSizes_t&)
//{
//}

void convertFromJson(JsonVariantConst src, FileSizes_t& dst)
{
	dst.eepromBackup = src["eepromBackup"];
	dst.fileSystemBackup = src["fileSystemBackup"];
	dst.recentBackup = src["recentBackup"];
}

bool convertToJson(const FileSizes_t& src, JsonVariant dst)
{
	dst["recentBackup"] = src.recentBackup;
	dst["fileSystemBackup"] = src.fileSystemBackup;
	dst["eepromBackup"] = src.eepromBackup;
}

//bool canConvertFromJson(JsonVariantConst src, const MqttRetainedStatus_t&)
//{
//}

void convertFromJson(JsonVariantConst src, MqttRetainedStatus_t& dst)
{
	dst.ip = src["ip"].as<IPAddress>();
}

bool convertToJson(const MqttRetainedStatus_t& src, JsonVariant dst)
{
	dst["ip"].set(src.ip);
}




//bool canConvertFromJson(JsonVariantConst src, const StatusRetained_t&)
//{
//	return src.containsKey("boot") && src.containsKey("crcs") && src.containsKey("fileSizes") && src.containsKey("mqtt");
//}

void convertFromJson(JsonVariantConst src, StatusRetained_t& dst)
{
	if(src.containsKey("boot"))
		convertFromJson(src["boot"], dst.boot);

	dst.fsFailedBackups = src["fsFailedBackups"];
	dst.eepromFailedBackups = src["eepromFailedBackups"];

	if (src.containsKey("crcs"))
		convertFromJson(src["crcs"], dst.crcs);

	if (src.containsKey("fileSizes"))
		convertFromJson(src["fileSizes"], dst.fileSizes);

	if (src.containsKey("mqtt"))
		convertFromJson(src["mqtt"], dst.mqtt);
}

bool convertToJson(const StatusRetained_t& src, JsonVariant dst)
{
	JsonObject retainedStatus_boot = dst.createNestedObject("boot");
	retainedStatus_boot["freshBoot"] = src.boot.freshBoot;
	retainedStatus_boot["bootSource"].set(src.boot.bootSource);
	retainedStatus_boot["wifiMode"].set<WifiMode>((WifiMode)src.boot.wifiMode);

	dst["fsFailedBackups"] = src.fsFailedBackups;
	dst["eepromFailedBackups"] = src.eepromFailedBackups;

	JsonObject retainedStatus_crcs = dst.createNestedObject("crcs");
	retainedStatus_crcs["bootFile"] = src.crcs.bootFile;
	retainedStatus_crcs["configPath"] = src.crcs.configPath;
	retainedStatus_crcs["configFile"] = src.crcs.configFile;
	retainedStatus_crcs["recentBackup"] = src.crcs.recentBackup;
	retainedStatus_crcs["fileSystemBackupFile"] = src.crcs.fileSystemBackupFile;
	retainedStatus_crcs["eepromBackupFile"] = src.crcs.eepromBackupFile;

	JsonObject retainedStatus_fileSizes = dst.createNestedObject("fileSizes");
	retainedStatus_fileSizes["recentBackup"] = src.fileSizes.recentBackup;
	retainedStatus_fileSizes["fileSystemBackup"] = src.fileSizes.fileSystemBackup;
	retainedStatus_fileSizes["eepromBackup"] = src.fileSizes.eepromBackup;

	JsonObject retainedStatus_mqtt = dst.createNestedObject("mqtt");
	retainedStatus_mqtt["ip"].set(src.mqtt.ip);
}





//bool canConvertFromJson(JsonVariantConst src, const Boot_bm&)
//{
//
//}

void convertFromJson(JsonVariantConst src, Boot_bm& dst)
{
	if (src.containsKey("freshBoot"))
		dst.freshBoot = src["freshBoot"];

	if (src.containsKey("bootSource"))
		dst.bootSource = src["bootSource"].as<ConfigSource>();

	if (src.containsKey("wifiMode"))
		dst.wifiMode = (wifi_mode_t)(src["wifiMode"].as<WifiMode>());

	if (src.containsKey("configMode"))
		dst.configMode = src["configMode"];
}

bool convertToJson(const Boot_bm& src, JsonVariant dst)
{
	dst["freshBoot"] = src.freshBoot;
	dst["bootSource"].set<ConfigSource>(src.bootSource);
	dst["wifiMode"].set<WifiMode>((WifiMode)src.wifiMode);
	dst["configMode"] = src.configMode;
}


//bool canConvertFromJson(JsonVariantConst src, const WifiStationConfig_t&)
//{
//
//}

void convertFromJson(JsonVariantConst src, WifiStationConfig_t& dst)
{
	if (src.containsKey("enabled"))
		dst.enabled = src["enabled"];

	if (src.containsKey("ledOn"))
		dst.ledOn = src["ledOn"];

	if (src.containsKey("ssid"))
		dst.ssid = (const char*)src["ssid"];

	if (src.containsKey("pass"))
		dst.pass = (const char*)src["pass"];

	if (src.containsKey("ledGpio"))
		dst.ledGpio = src["ledGpio"];
}

bool convertToJson(const WifiStationConfig_t& src, JsonVariant dst)
{
	dst["enabled"] = src.enabled;
	dst["ledOn"] = src.ledOn;
	dst["ssid"].set(src.ssid);
	dst["pass"].set(src.pass);
	dst["ledGpio"] = src.ledGpio;

}


//bool canConvertFromJson(JsonVariantConst src, const WifiAccessPointConfig_t&)
//{
//
//}

void convertFromJson(JsonVariantConst src, WifiAccessPointConfig_t& dst)
{
	if (src.containsKey("useDefaults"))
		dst.useDefaults = src["useDefaults"];

	if (src.containsKey("enabled"))
		dst.enabled = src["enabled"];

	if (src.containsKey("configOnly"))
		dst.configOnly = src["configOnly"];

	if (src.containsKey("buttonPullup"))
		dst.buttonPullup = src["buttonPullup"];

	if (src.containsKey("buttonPress"))
		dst.buttonPress = src["buttonPress"];

	if (src.containsKey("hidden"))
		dst.hidden = src["hidden"];

	if (src.containsKey("ledOn"))
		dst.ledOn = src["ledOn"];

	if (src.containsKey("ssid"))
		dst.ssid = (const char*)src["ssid"];

	if (src.containsKey("pass"))
		dst.pass = (const char*)src["pass"];

	if (src.containsKey("maxConnections"))
		dst.maxConnections = src["maxConnections"];

	if (src.containsKey("buttonGpio"))
		dst.buttonGpio = src["buttonGpio"];

	if (src.containsKey("holdTime"))
		dst.holdTime = ((uint16_t)src["holdTime"]) * 1000;

	if (src.containsKey("ledGpio"))
		dst.ledGpio = src["ledGpio"];


}

bool convertToJson(const WifiAccessPointConfig_t& src, JsonVariant dst)
{
	dst["useDefaults"] = src.useDefaults;
	dst["enabled"] = src.enabled;
	dst["configOnly"] = src.configOnly;
	dst["buttonPullup"] = src.buttonPullup;
	dst["buttonPress"] = src.buttonPress;
	dst["hidden"] = src.hidden;
	dst["ledOn"] = src.ledOn;
	dst["ssid"].set(src.ssid);
	dst["pass"].set(src.pass);
	dst["maxConnections"] = src.maxConnections;
	dst["buttonGpio"] = src.buttonGpio;
	dst["holdTime"] = src.holdTime / 1000;
	dst["ledGpio"] = src.ledGpio;
}

//bool canConvertFromJson(JsonVariantConst src, const WifiConfig_t&)
//{
//
//}

void convertFromJson(JsonVariantConst src, WifiConfig_t& dst)
{
	if (src.containsKey("useDefaults"))
		dst.useDefaults = src["useDefaults"];

	if (src.containsKey("channel"))
		dst.channel = src["channel"];

	if (src.containsKey("powerLevel"))
		dst.powerLevel = (wifi_power_t)(src["powerLevel"].as<WifiPower>());

	if (src.containsKey("station"))
		convertFromJson(src["station"], dst.station);

	if (src.containsKey("accessPoint"))
		convertFromJson(src["accessPoint"], dst.accessPoint);

	if (src.containsKey("taskSettings"))
		convertFromJson(src["taskSettings"], dst.taskSettings);
}

bool convertToJson(const WifiConfig_t& src, JsonVariant dst)
{
	dst["useDefaults"] = src.useDefaults;
	dst["channel"] = src.channel;
	dst["powerLevel"].set<WifiPower>((WifiPower)src.powerLevel);
	dst["station"].set(src.station);
	dst["accessPoint"].set(src.accessPoint);
	dst["taskSettings"].set(src.taskSettings);

}


//bool canConvertFromJson(JsonVariantConst src, const FtpConfig_t&)
//{
//
//}

void convertFromJson(JsonVariantConst src, FtpConfig_t& dst)
{
	if (src.containsKey("useDefaults"))
		dst.useDefaults = src["useDefaults"];

	if (src.containsKey("enabled"))
		dst.enabled = src["enabled"];

	if (src.containsKey("anonymous"))
		dst.anonymous = src["anonymous"];

	if (src.containsKey("user"))
		dst.user = (const char*)src["user"];

	if (src.containsKey("pass"))
		dst.pass = (const char*)src["pass"];

	if (src.containsKey("timeout"))
		dst.timeout = src["timeout"];

	if (src.containsKey("taskSettings"))
		convertFromJson(src["taskSettings"], dst.taskSettings);

}



bool convertToJson(const FtpConfig_t& src, JsonVariant dst)
{
	dst["useDefaults"] = src.useDefaults;
	dst["enabled"] = src.enabled;
	dst["anonymous"] = src.anonymous;
	dst["user"].set(src.user);
	dst["pass"].set(src.pass);
	dst["timeout"] = src.timeout;
	dst["taskSettings"].set(src.taskSettings);
}

//bool canConvertFromJson(JsonVariantConst src, const BrowserConfig_t&)
//{
//
//}

void convertFromJson(JsonVariantConst src, BrowserConfig_t& dst)
{
	if (src.containsKey("useDefaults"))
		dst.useDefaults = src["useDefaults"];

	if (src.containsKey("enabled"))
		dst.enabled = src["enabled"];

	if (src.containsKey("config"))
		dst.config = src["config"];

	if (src.containsKey("console"))
		dst.console = src["console"];

	if (src.containsKey("updater"))
		dst.updater = src["updater"];

	if (src.containsKey("mqttDevices"))
		dst.mqttDevices = src["mqttDevices"];

	if (src.containsKey("ssl"))
		dst.ssl = src["ssl"];

	if (src.containsKey("tools"))
	{
		if (src.containsKey("fileEditor"))
			dst.tools.fileEditor = src["fileEditor"];

		if (src.containsKey("jsonVerify"))
			dst.tools.jsonVerify = src["jsonVerify"];
	}
}



bool convertToJson(const BrowserConfig_t& src, JsonVariant dst)
{
	dst["useDefaults"] = src.useDefaults;
	dst["enabled"] = src.enabled;

	dst["config"] = src.config;
	dst["console"] = src.console;
	dst["updater"] = src.updater;
	dst["mqttDevices"] = src.mqttDevices;
	dst["ssl"] = src.ssl;

	dst["tools"]["fileEditor"] = src.tools.fileEditor;
	dst["jsonVerify"]["jsonVerify"] = src.tools.jsonVerify;
}



//bool canConvertFromJson(JsonVariantConst src, const OtaConfig_t&)
//{
//
//}

void convertFromJson(JsonVariantConst src, OtaConfig_t& dst)
{
	if (src.containsKey("useDefaults"))
		dst.useDefaults = src["useDefaults"];

	if (src.containsKey("enabled"))
		dst.enabled = src["enabled"];

	if (src.containsKey("rollbackMode"))
		dst.rollbackMode = src["rollbackMode"].as<OtaRollbackMode>();

	if (src.containsKey("rollbackTimer"))
		dst.rollbackTimer = src["rollbackTimer"];

	if (src.containsKey("taskSettings"))
		convertFromJson(src["taskSettings"], dst.taskSettings);

}

bool convertToJson(const OtaConfig_t& src, JsonVariant dst)
{
	dst["useDefaults"] = src.useDefaults;
	dst["enabled"] = src.enabled;
	dst["rollbackMode"].set(src.rollbackMode);
	dst["rollbackTimer"] = src.rollbackTimer;

	dst["taskSettings"].set(src.taskSettings);
}


//bool canConvertFromJson(JsonVariantConst src, const ServerConfig_t&)
//{
//
//}

void convertFromJson(JsonVariantConst src, ServerConfig_t& dst)
{
	if (src.containsKey("useDefaults"))
		dst.useDefaults = src["useDefaults"];

	if (src.containsKey("dns"))
		dst.dns = src["dns"];

	if (src.containsKey("authenticate"))
		dst.authenticate = src["authenticate"];

	if (src.containsKey("hostname"))
		dst.hostname = (const char*)src["hostname"];

	if (src.containsKey("user"))
		dst.user = (const char*)src["user"];

	if (src.containsKey("pass"))
		dst.pass = (const char*)src["pass"];

	if (src.containsKey("sessionTimeout"))
		dst.sessionTimeout = src["sessionTimeout"];

	if (src.containsKey("browser"))
		convertFromJson(src["browser"], dst.browser);

	if (src.containsKey("ftp"))
		convertFromJson(src["ftp"], dst.ftp);

	if (src.containsKey("ota"))
		convertFromJson(src["ota"], dst.ota);

}

bool convertToJson(const ServerConfig_t& src, JsonVariant dst)
{
	dst["useDefaults"] = src.useDefaults;
	dst["dns"] = src.dns;
	dst["authenticate"] = src.authenticate;
	dst["hostname"].set(src.hostname);
	dst["user"].set(src.user);
	dst["pass"].set(src.pass);
	dst["sessionTimeout"] = src.sessionTimeout;

	dst["browser"].set(src.browser);
	dst["ftp"].set(src.ftp);
	dst["ota"].set(src.ota);
}


//bool canConvertFromJson(JsonVariantConst src, const SerialPortConfig_t&)
//{
//
//}

void convertFromJson(JsonVariantConst src, SerialPortConfig_t& dst)
{
	if (src.containsKey("useDefaults"))
		dst.useDefaults = src["useDefaults"];

	if (src.containsKey("enabled"))
		dst.enabled = src["enabled"];

	if (src.containsKey("invert"))
		dst.invert = src["invert"];

	if (src.containsKey("baud"))
		dst.baud = src["baud"];

	if (src.containsKey("rxGpio"))
		dst.rxGpio = src["rxGpio"];

	if (src.containsKey("txGpio"))
		dst.txGpio = src["txGpio"];

	if (src.containsKey("timeout"))
		dst.timeout = src["timeout"];

	if (src.containsKey("config"))
		dst.config = src["config"];

	if (src.containsKey("useDefaults"))
		dst.useDefaults = src["useDefaults"];

}

bool convertToJson(const SerialPortConfig_t& src, JsonVariant dst)
{
	dst["useDefaults"] = src.useDefaults;
	dst["enabled"] = src.enabled;
	dst["invert"] = src.invert;
	dst["baud"] = src.baud;
	dst["rxGpio"] = src.rxGpio;
	dst["txGpio"] = src.txGpio;
	dst["timeout"] = src.timeout;
	dst["config"] = src.config;
}


//bool canConvertFromJson(JsonVariantConst src, const SerialPortConfig_t&)
//{
//
//}

void convertFromJson(JsonVariantConst src, SerialConfig_t& dst)
{
	if (src.containsKey("useDefaults"))
		dst.useDefaults = src["useDefaults"];

	if (src.containsKey("messagePort"))
		dst.messagePort = src["messagePort"];

	if (src.containsKey("debugPort"))
		dst.debugPort = src["debugPort"];

	if (src.containsKey("port0"))
		convertFromJson(src, dst.port0);

	if (src.containsKey("port1"))
		convertFromJson(src, dst.port1);
}

bool convertToJson(const SerialConfig_t& src, JsonVariant dst)
{
	dst["useDefaults"] = src.useDefaults;
	dst["messagePort"] = src.messagePort;
	dst["debugPort"] = src.debugPort;
	dst["port0"].set(src.port0);
	dst["port1"].set(src.port1);
}

//bool canConvertFromJson(JsonVariantConst src, const I2cConfig_t&)
//{
//
//}

void convertFromJson(JsonVariantConst src, I2cConfig_t& dst)
{
	if (src.containsKey("useDefaults"))
		dst.useDefaults = src["useDefaults"];

	if (src.containsKey("enabled"))
		dst.enabled = src["enabled"];

	if (src.containsKey("sclGpio"))
		dst.sclGpio = src["sclGpio"];

	if (src.containsKey("sdaGpio"))
		dst.sdaGpio = src["sdaGpio"];

	if (src.containsKey("freq"))
		dst.freq = src["freq"];
}

bool convertToJson(const I2cConfig_t& src, JsonVariant dst)
{
	dst["useDefaults"] = src.useDefaults;
	dst["enabled"] = src.enabled;
	dst["sclGpio"] = src.sclGpio;
	dst["sdaGpio"] = src.sdaGpio;
	dst["freq"] = src.freq;
}


//bool canConvertFromJson(JsonVariantConst src, const ConfigDevice_t&)
//{
//
//}

void convertFromJson(JsonVariantConst src, ConfigDevice_t& dst)
{
	if (src.containsKey("useDefaults"))
		dst.useDefaults = src["useDefaults"];

	if (src.containsKey("autoBackupMode"))
		dst.autoBackupMode = (ConfigAutobackupMode_t)(src["autoBackupMode"].as< ConfigAutobackupMode>());

	if (src.containsKey("maxFailedBackups"))
		dst.maxFailedBackups = src["maxFailedBackups"];

	if (src.containsKey("serial"))
		convertFromJson(src, dst.serial);

	if (src.containsKey("i2c"))
		convertFromJson(src, dst.i2c);

}

bool convertToJson(const ConfigDevice_t& src, JsonVariant dst)
{
	dst["useDefaults"] = src.useDefaults;
	dst["autoBackupMode"].set<ConfigAutobackupMode>((ConfigAutobackupMode)src.autoBackupMode);
	dst["maxFailedBackups"] = src.maxFailedBackups;

	dst["serial"].set(src.serial);
	dst["i2c"].set(src.i2c);
}


//bool canConvertFromJson(JsonVariantConst src, const Config_t&)
//{
//
//}

void convertFromJson(JsonVariantConst src, Config_t& dst)
{

	if (src.containsKey("wifi"))
		convertFromJson(src["wifi"], dst.wifi);

	if (src.containsKey("mqtt"))
		convertFromJson(src["mqtt"], dst.mqtt);

	if (src.containsKey("device"))
		convertFromJson(src["device"], dst.device);

	if (src.containsKey("server"))
		convertFromJson(src["server"], dst.server);



}

bool convertToJson(const Config_t& src, JsonVariant dst)
{
	dst["wifi"].set(src.wifi);
	dst["mqtt"].set(src.mqtt);
	dst["device"].set(src.device);
	dst["server"].set(src.server);
}

#pragma region Mqtt UDFs

//bool canConvertFromJson(JsonVariantConst src, const MqttPublishConfig_t&)
//{
//
//}



void convertFromJson(JsonVariantConst src, MqttPublishConfig_t& dst)
{
	if (src.containsKey("bufferSize"))
		dst.bufferSize = src["bufferSize"];

	if (src.containsKey("rate"))
	{
		dst.rate = src["rate"];
		dst.rate *= 1000;
	}

	if (src.containsKey("errorRate"))
	{
		dst.errorRate = src["errorRate"];
		dst.errorRate *= 1000;
	}

	if (src.containsKey("availabilityRate"))
	{
		dst.availabilityRate = src["availabilityRate"];
		dst.availabilityRate *= 1000;
	}

	if (src.containsKey("json"))
		dst.json = src["json"];

	//if (src.containsKey("onIndividualTopics"))
	//	dst.onIndividualTopics = src["onIndividualTopics"];

	//if (src.containsKey("unknownPayload"))
	//	dst.unknownPayload = src["unknownPayload"];

}

bool convertToJson(const MqttPublishConfig_t& src, JsonVariant dst)
{
	dst["bufferSize"] = src.bufferSize;
	dst["rate"] = src.rate / 1000;
	dst["errorRate"] = src.errorRate / 1000;
	dst["availabilityRate"] = src.availabilityRate / 1000;
	dst["json"] = src.json;
}



//bool canConvertFromJson(JsonVariantConst src, const MqttBrokerConfig_t&)
//{
//
//}


void convertFromJson(JsonVariantConst src, MqttBrokerConfig_t& dst)
{
	if (src.containsKey("autoDetectIP"))
		dst.autoDetectIp = src["autoDetectIP"];

	if (src.containsKey("wifiMode"))
		dst.wifiMode = (wifi_mode_t)(src["wifiMode"].as<WifiMode>());

	if (src.containsKey("ip"))
		dst.ip = src["ip"].as<IPAddress>();

	if (src.containsKey("ipAP"))
		dst.ipAP = src["ipAP"].as<IPAddress>();

	if (src.containsKey("port"))
		dst.port = src["port"];

	if (src.containsKey("user"))
		dst.user = (const char*)src["user"];

	if (src.containsKey("pass"))
		dst.pass = (const char*)src["pass"];

	if (src.containsKey("maxRetries"))
		dst.maxRetries = src["maxRetries"];

	if (src.containsKey("autoMaxRetries"))
		dst.autoMaxRetries = src["autoMaxRetries"];

	if (src.containsKey("autoTimeout"))
		dst.autoTimeout = src["autoTimeout"];

	if (src.containsKey("timeout"))
		dst.timeout = src["timeout"];

	if (src.containsKey("connectInterval"))
		dst.connectInterval = src["connectInterval"];
	else if (src.containsKey("attemptRate"))
		dst.connectInterval = src["attemptRate"];

}

bool convertToJson(const MqttBrokerConfig_t& src, JsonVariant dst)
{
	dst["autoDetectIp"] = src.autoDetectIp;
	dst["wifiMode"].set<WifiMode>((WifiMode)src.wifiMode);
	dst["ip"].set<IPAddress>(src.ip);
	dst["ipAP"].set<IPAddress>(src.ipAP);
	dst["port"] = src.port;
	dst["user"].set(src.user);
	dst["pass"].set(src.pass);
	dst["maxRetries"] = src.maxRetries;
	dst["autoMaxRetries"] = src.autoMaxRetries;
	dst["autoTimeout"] = src.autoTimeout;
	dst["timeout"] = src.timeout;
	dst["connectInterval"] = src.connectInterval;
}


//bool canConvertFromJson(JsonVariantConst src, const MqttBaseTopics_t&)
//{
//
//}


void convertFromJson(JsonVariantConst src, MqttBaseTopics_t& dst)
{
	if (src.containsKey("base"))
		dst.base = (const char*)src["base"];

	if (src.containsKey("availability"))
		dst.availability = (const char*)src["availability"];

	if (src.containsKey("jsonCommand"))
		dst.jsonCommand = (const char*)src["jsonCommand"];

	if (src.containsKey("jsonState"))
		dst.jsonState = (const char*)src["jsonState"];

	if (src.containsKey("command"))
		dst.command = (const char*)src["command"];

	if (src.containsKey("state"))
		dst.state = (const char*)src["state"];
}

bool convertToJson(const MqttBaseTopics_t& src, JsonVariant dst)
{
	dst["base"].set(src.base);
	dst["availability"].set(src.availability);
	dst["jsonCommand"].set(src.jsonCommand);
	dst["jsonState"].set(src.jsonState);
	dst["command"].set(src.command);
	dst["state"].set(src.state);
}

//bool canConvertFromJson(JsonVariantConst src, const MqttTopics_t&)
//{
//
//}


void convertFromJson(JsonVariantConst src, MqttTopics_t& dst)
{
	if (src.containsKey("availability"))
		dst.availability = (const char*)src["availability"];

	if (src.containsKey("jsonCommand"))
		dst.jsonCommand = (const char*)src["jsonCommand"];

	if (src.containsKey("jsonState"))
		dst.jsonState = (const char*)src["jsonState"];
}

bool convertToJson(const MqttTopics_t& src, JsonVariant dst)
{
	dst["availability"].set(src.availability);
	dst["jsonCommand"].set(src.jsonCommand);
	dst["jsonState"].set(src.jsonState);
}


//bool canConvertFromJson(JsonVariantConst src, const MqttConfig_t&)
//{
//
//}


void convertFromJson(JsonVariantConst src, MqttConfig_t& dst)
{
	if (src.containsKey("useDefaults"))
		dst.useDefaults = src["useDefaults"];

	if (src.containsKey("ledOn"))
		dst.ledOn = src["ledOn"];

	if (src.containsKey("ledGpio"))
		dst.ledGpio = src["ledGpio"];

	if (src.containsKey("publish"))
		convertFromJson(src["publish"], dst.publish);

	if (src.containsKey("broker"))
		convertFromJson(src["broker"], dst.broker);

	if (src.containsKey("baseTopics"))
		convertFromJson(src["baseTopics"], dst.baseTopics);

	if (src.containsKey("topics"))
		convertFromJson(src["topics"], dst.topics);

	if (src.containsKey("taskSettings"))
		convertFromJson(src["taskSettings"], dst.taskSettings);
}

bool convertToJson(const MqttConfig_t& src, JsonVariant dst)
{
	dst["useDefaults"] = src.useDefaults;
	dst["ledOn"] = src.ledOn;
	dst["ledGpio"] = src.ledGpio;

	dst["publish"].set(src.publish);
	dst["broker"].set(src.broker);
	dst["baseTopics"].set(src.baseTopics);
	dst["topics"].set(src.topics);
	dst["taskSettings"].set(src.taskSettings);
}




bool convertToJson(const DevicesFunctioning_t& src, JsonVariant dst)
{
	dst["bitmap0"] = src.bitmap0;
	dst["bitmap1"] = src.bitmap1;
	dst["bitmap2"] = src.bitmap2;
	dst["bitmap3"] = src.bitmap3;
}

bool convertToJson(const MqttDevicesStatus_t& src, JsonVariant dst)
{
	dst["subscribedCount"] = src.subscribedCount;
	dst["enabledCount"] = src.enabledCount;
	dst["deviceCount"] = src.deviceCount;
	dst["buttonCount"] = src.buttonCount;
	dst["switchCount"] = src.switchCount;
	dst["lightCount"] = src.lightCount;
	dst["sensorCount"] = src.sensorCount;
	dst["binarySensorCount"] = src.binarySensorCount;
	dst["functioningDevices"].set(src.functioningDevices);
	dst["functioningDevicesImportant"].set(src.functioningDevicesImportant);
}

bool convertToJson(const GlobalMqttIpStatus_t& src, JsonVariant dst)
{
	dst["ip"].set(src.ip);
	dst["ipIndex"] = src.ipIndex;
	dst["currentAttemptsCounter"] = src.currentAttemptsCounter;
	dst["maxRetries"] = src.maxRetries;
	dst["mode"].set<WifiMode>((WifiMode)src.mode);
	dst["triedRetainedIP"] = src.triedRetainedIP;
	dst["triedConfigStation"] = src.triedConfigStation;
	dst["triedConfigAP"] = src.triedConfigAP;
	dst["stationAutoExhausted"] = src.stationAutoExhausted;
	dst["accessPointAutoExhausted"] = src.accessPointAutoExhausted;
	dst["changed"] = src.changed;
	dst["autoDetectingStation"] = src.autoDetectingStation;

}


bool convertToJson(const GlobalMqttStatus_t& src, JsonVariant dst)
{
	dst["enabled"] = src.enabled;
	dst["connected"] = src.connected;
	dst["missingRequiredInfo"] = src.missingRequiredInfo;
	dst["subscribed"] = src.subscribed;
	dst["devicesConfigured"] = src.devicesConfigured;
	dst["publishingDisabled"] = src.publishingDisabled;
	dst["serverSet"] = src.serverSet;
	dst["connectAttempts"] = src.connectAttempts;
	dst["nextPublish"] = src.nextPublish;
	dst["nextDisplayMessages"] = src.nextDisplayMessages;
	dst["nextPublishAvailability"] = src.nextPublishAvailability;
	dst["nextMqttConnectAttempt"] = src.nextMqttConnectAttempt;
	dst["nextMqttConnectAttempt"] = src.nextMqttConnectAttempt;
	dst["nextWarningBlink"] = src.nextWarningBlink;
	dst["devices"].set(src.devices);
	dst["ipStatus"].set(src.ipStatus);
}