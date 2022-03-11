#ifndef _SENSOR_SHT4X_h
#define _SENSOR_SHT4X_h

#include <Arduino.h>

#include <ArduinoJson.hpp>
#include <ArduinoJson.h>
#include <PubSubClient.h>

#include <Wire.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_SHT4x.h>

#include "../../../../Config/config_master.h"
#include "sht4x_config.h"

#include "../../MqttDevice.h"
#include "../MqttSensor.h"

#include "../../../../HelperFunctions.h"
#include "../../../../Macros.h"



typedef struct
{
	uint8_t reserved : 8;
}SHT4xStatus_t;

class Sht4xSensor : public MqttSensor
{
public:

	Adafruit_SHT4x sensor = Adafruit_SHT4x();

#pragma region Global Variables

	/// <summary>
	/// Global MQTT Device Configuration
	/// </summary>
	static MqttDeviceConfig_t globalDeviceConfig;
	static MqttDeviceConfigMonitor_t globalDeviceConfigMonitor;

	/// <summary>
	/// Global MQTT Settings
	/// </summary>
	static MqttDeviceMqttSettings_t globalDeviceMqttSettings;
	static MqttDeviceMqttSettingsMonitor_t globalDeviceMqttSettingsMonitor;

	/// <summary>
	/// SCD4X Configuration
	/// </summary>
	static Sht4xConfig_t globalUniqueConfig;
	static Sht4xConfigMonitor_bm globalUniqueConfigMonitor;

	static MqttDeviceGlobalStatus_t globalDeviceStatus;

#pragma endregion

#pragma region Variables

	Sht4xConfig_t uniqueConfig;
	Sht4xConfigMonitor_bm uniqueConfigMonitor;

	/// <summary>
	/// SCD4x Status
	/// </summary>
	SHT4xStatus_t uniqueStatus;


	struct {
		sensors_event_t  temperature;
		sensors_event_t  humidity;
	}measurementData;

#pragma endregion

	const char* GetConfigFilePath() override {
		return SHT4X_CONFIG_PATH;
	}
	const char* GetConfigFileName() override {
		return SHT4X_CONFIG_NAME;
	}
	const char* GetConfigObjKey() override {
		return SHT4X_CONFIG_OBJECT_KEY;
	}
	const char* GetDefaultBaseTopic() override {
		return TOPIC_SENSOR_SHT4X_BASE;
	}
	String* GetParentBaseTopic() override {
		return &mqttSensorBaseTopic;
	}

	Sht4xSensor(const char* _name, const char* _device, int _index) : MqttSensor(_name, _device, _index) {}

#pragma region MqttDevice Functions

	bool Init(bool enable) override;

	void ResetStatus() override
	{
		memset(&uniqueStatus, 0, sizeof(SHT4xStatus_t));
		MqttSensor::ResetStatus();
	}

	//void Loop() override;

	bool Configure() override;
	//bool Enable() override;
	//bool Disable() override;
	bool Subscribe() override;
	bool Unsubscribe() override;
	//bool Publish() override;
	//bool PublishAvailability() override;

	void AddStatePayload(JsonObject& addTo) override;				//Payload for MQTT state topic
	//void AddStatusData(JsonObject& addTo) override;					//device, binary/sensor/ect.., and unique status
	void AddConfigData(JsonObject& addTo) override;					//device, binary/sensor/ect.., and unique config

	int ReceiveCommand(char* topic, byte* payload, size_t length) override;

	template<class T>
	int GetSensorObject(T** object)
	{
		*object = &sensor;
		return 0;
	};

//protected:

	//void* GetUniqueConfig() override;
	//void* GetUniqueConfigMonitor() override;
	//void* GetGlobalUniqueConfig() override;
	//void* GetGlobalUniqueConfigMonitor() override;
	//MqttDeviceConfig_t* GetGlobalDeviceConfig()override;
	//MqttDeviceConfigMonitor_t* GetGlobalDeviceConfigMonitor()override;
	//MqttDeviceMqttSettings_t* GetGlobalMqttConfig() override;
	//MqttDeviceMqttSettingsMonitor_t* GetGlobalMqttConfigMonitor() override;
	MqttDeviceGlobalStatus_t* GetGlobalDeviceStatus() override;


	void GenerateTopics() override;

	void SetDefaultSettings() override;
	void SetDefaultGlobalSettings() override;

	void SetDefaultUniqueSettings() override;
	void SetDefaultGlobalUniqueSettings() override;
	void SetDefaultMqttTopics() override;
	//void SetDefaultGlobalMqttTopics() override;

	//bool ReadGlobalConfig() override;
	void ReadConfigObjectUnique(JsonVariantConst& obj) override;
	void ReadGlobalConfigObject(JsonVariantConst& obj) override;
	void ReadGlobalUniqueConfigObject(JsonVariantConst& obj) override;
	//void ReadTopicsObject(JsonObject& topicsObj) override;
	//void ReadGlobalBaseTopicsObject(JsonObject& topicsObj) override;

	private:
	static void SetDefaultUniqueSettings(Sht4xConfig_t& uConfig, Sht4xConfigMonitor_bm& uConfigMonitor);

	/// <summary>
	/// Parse the unique objects from either the sht4x.json, or the "config" key from one of the sensors in the array in config_sensors.json.
	/// Pass either global or non global structures.
	/// </summary>
	static void ReadConfigObjectUnique(JsonVariantConst& sht4xObject, Sht4xConfig_t& uConfig, Sht4xConfigMonitor_bm& uConfigMonitor);

#pragma endregion

#pragma region MqttSensor Functions

	bool Connect() override;
	bool IsConnected() override;
	bool Read() override;

#pragma endregion

	private:

		/// <summary>
		/// 
		/// </summary>
		/// <param name="force"></param>
		/// <returns>False equals already set.</returns>
		bool SetPrecision(sht4x_precision_t precision, bool force = false) 
		{
			if (uniqueConfigMonitor.internal.precision || !deviceStatus.configured || force)
			{
#warning add validation

				DEBUG_LOG_F("Setting %s(SHT4X) Precision : ", name.c_str());
				DEBUG_LOG_LN(uniqueConfig.internal.precision);
				sensor.setPrecision(uniqueConfig.internal.precision);
				uniqueConfigMonitor.internal.precision = false;

				return true;
			}
			return false;
		}

		bool SetHeater(sht4x_heater_t heater, bool force = false)
		{
			if (uniqueConfigMonitor.internal.heater || !deviceStatus.configured || force)
			{
				DEBUG_LOG_F("Setting %s(SHT4X) Heater : ", name.c_str());
				DEBUG_LOG_LN(uniqueConfig.internal.heater);
				sensor.setHeater(uniqueConfig.internal.heater);
				uniqueConfigMonitor.internal.heater = false;
				return true;
			}

			return false;
		}



};

#pragma region Json UDFs

extern const char* sht4x_heater_strings[7];
extern const char* sht4x_precision_strings[3];

//Classify enums so ArduinoJson can determine the correct function.
//(Regular enums are ints)
enum class sht4x_precision {};
enum class sht4x_heater {};

bool canConvertFromJson(JsonVariantConst src, const sht4x_precision&);
void convertFromJson(JsonVariantConst src, sht4x_precision& dst);
bool convertToJson(const sht4x_precision& src, JsonVariant dst);

bool canConvertFromJson(JsonVariantConst src, const sht4x_heater&);
void convertFromJson(JsonVariantConst src, sht4x_heater& dst);
bool convertToJson(const sht4x_heater& src, JsonVariant dst);

//bool canConvertFromJson(JsonVariantConst src, const SHT4xStatus_t&);
//void convertFromJson(JsonVariantConst src, SHT4xStatus_t& dst);
//bool convertToJson(const SHT4xStatus_t& src, JsonVariant dst);

bool canConvertFromJson(JsonVariantConst src, const Sht4xConfig_t&);
void convertFromJson(JsonVariantConst src, Sht4xConfig_t& dst);
bool convertToJson(const Sht4xConfig_t& src, JsonVariant dst);

#pragma endregion

#pragma region JSON UDFs



#pragma endregion

#endif

