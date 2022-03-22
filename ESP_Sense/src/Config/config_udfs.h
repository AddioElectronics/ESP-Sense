#ifndef _CONFIG_UDFS_h
#define _CONFIG_UDFS_h

#include <Arduino.h>

#include <ArduinoJson.hpp>
#include <ArduinoJson.h>

#include "config_master.h"
#include "global_status.h"

#pragma region Json UDFs

#pragma region Status

bool canConvertFromJson(JsonVariantConst src, const Version_t&);
void convertFromJson(JsonVariantConst src, Version_t& dst);
bool convertToJson(const Version_t& src, JsonVariant dst);

bool convertToJson(const GenericModuleStatus_t& src, JsonVariant dst);


bool convertToJson(const GlobalStatusTasks_t& src, JsonVariant dst);
bool convertToJson(const GlobalStatusDevice_t& src, JsonVariant dst);

bool convertToJson(const BackupStatus_t& src, JsonVariant dst);
bool convertToJson(const GlobalStatusConfig_t& src, JsonVariant dst);

bool convertToJson(const WifiStationStatus_t& src, JsonVariant dst);
bool convertToJson(const WifiAccessPointStatus_t& src, JsonVariant dst);
bool convertToJson(const WifiStatus_t& src, JsonVariant dst);

bool convertToJson(const BrowserToolsStatus_t& src, JsonVariant dst);
bool convertToJson(const BrowserStatus_t& src, JsonVariant dst);
bool convertToJson(const ServerOtaStatus_t& src, JsonVariant dst);
bool convertToJson(const FtpStatus_t& src, JsonVariant dst);
bool convertToJson(const ServerStatus_t& src, JsonVariant dst);

bool convertToJson(const GlobalMiscStatus_t& src, JsonVariant dst);


bool convertToJson(const GlobalStatus_t& src, JsonVariant dst);



//bool canConvertFromJson(JsonVariantConst src, const Crcs_t&);
void convertFromJson(JsonVariantConst src, Crcs_t& dst);
bool convertToJson(const Crcs_t& src, JsonVariant dst);

//bool canConvertFromJson(JsonVariantConst src, const FileSizes_t&);
void convertFromJson(JsonVariantConst src, FileSizes_t& dst);
bool convertToJson(const FileSizes_t& src, JsonVariant dst);

//bool canConvertFromJson(JsonVariantConst src, const MqttRetainedStatus_t&);
void convertFromJson(JsonVariantConst src, MqttRetainedStatus_t& dst);
bool convertToJson(const MqttRetainedStatus_t& src, JsonVariant dst);

//bool canConvertFromJson(JsonVariantConst src, const Boot_bm&);
void convertFromJson(JsonVariantConst src, Boot_bm& dst);
bool convertToJson(const Boot_bm& src, JsonVariant dst);

//bool canConvertFromJson(JsonVariantConst src, const StatusRetained_t&);
void convertFromJson(JsonVariantConst src, StatusRetained_t& dst);
bool convertToJson(const StatusRetained_t& src, JsonVariant dst);

#pragma endregion

#pragma region Config

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

//bool canConvertFromJson(JsonVariantConst src, const I2cConfig_t&);
void convertFromJson(JsonVariantConst src, I2cConfig_t& dst);
bool convertToJson(const I2cConfig_t& src, JsonVariant dst);

//bool canConvertFromJson(JsonVariantConst src, const ConfigDevice_t&);
void convertFromJson(JsonVariantConst src, ConfigDevice_t& dst);
bool convertToJson(const ConfigDevice_t& src, JsonVariant dst);

//bool canConvertFromJson(JsonVariantConst src, const Config_t&);
void convertFromJson(JsonVariantConst src, Config_t& dst);
bool convertToJson(const Config_t& src, JsonVariant dst);

#pragma endregion

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


bool convertToJson(const GlobalMqttIpStatus_t& src, JsonVariant dst);
bool convertToJson(const GlobalMqttStatus_t& src, JsonVariant dst);

#pragma endregion




#pragma endregion

#endif

