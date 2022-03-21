// mushbox_LLC200D3SH.h

#ifndef _SENSOR_LLC200D3SH_h
#define _SENSOR_LLC200D3SH_h

#include <Arduino.h>

#include <ArduinoJson.hpp>
#include <ArduinoJson.h>
#include <PubSubClient.h>

#include "../../MqttDevice.h"
#include "../MqttBinarySensor.h"

#include "LLC200D3SH_config.h"

#include "../../../../EspSenseValidation.h"

typedef struct {
	bool gpioValid : 1;
}Llc200d3sh_Status_t;

class Llc200d3sh_Sensor : public MqttBinarySensor
{
public:

	static const char* deviceName;		//Used for print statements
	static const char* deviceKey;		//Used for keys, and filtering

	const char* DeviceName() override
	{
		return deviceName;
	}
	const char* DeviceKey() override
	{
		return deviceKey;
	}

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
	static Llc200d3sh_Config_t globalUniqueConfig;
	static Llc200d3sh_ConfigMonitor_bm globalUniqueConfigMonitor;

	static MqttDeviceGlobalStatus_t globalDeviceStatus;

#pragma endregion

#pragma region Variables

	Llc200d3sh_Config_t uniqueConfig;
	Llc200d3sh_ConfigMonitor_bm uniqueConfigMonitor;

	Llc200d3sh_Status_t uniqueStatus;


	bool measurement;

#pragma endregion

	const char* GetConfigFilePath() override {
		return LLC200D3SH_CONFIG_PATH;
	}
	const char* GetConfigFileName() override {
		return LLC200D3SH_CONFIG_NAME;
	}
	const char* GetConfigObjKey() override {
		return LLC200D3SH_CONFIG_OBJECT_KEY;
	}
	const char* GetDefaultBaseTopic() override {
		return TOPIC_SENSOR_LLC200D3SH_BASE;
	}
	String* GetParentBaseTopic() override {
		return &mqttBinarySensorBaseTopic;
	}

	Llc200d3sh_Sensor(const char* _name, int _index, int _subIndex) : MqttBinarySensor(_name, _index, _subIndex) 
	{
		ResetStatus();
	}

#pragma region MqttDevice Functions

	virtual void ResetStatus() override
	{
		memset(&uniqueStatus, 0, sizeof(Llc200d3sh_Status_t));
		MqttBinarySensor::ResetStatus();
	}

	//bool Configure() override;

	bool Configure() override
	{
		uniqueStatus.gpioValid = IsGpioValidPin(uniqueConfig.program.gpio, 0);
		deviceStatus.configured = uniqueStatus.gpioValid;

		if (uniqueStatus.gpioValid)
			pinMode(uniqueConfig.program.gpio, INPUT);
	}

	bool Enable() override
	{
		if (!deviceStatus.configured) return false;

		deviceStatus.enabled = true;
	}

	bool Subscribe() override;
	bool Unsubscribe() override;

	void AddStatePayload(JsonVariant& addTo) override;		//Payload for MQTT state topic
	void AddStatusData(JsonVariant& addTo) override;								//device, binary/sensor/ect.., and unique status
	void AddConfigData(JsonVariant& addTo) override;								//device, binary/sensor/ect.., and unique config

	int ReceiveCommand(char* topic, byte* payload, size_t length) override;

	template<class T>
	int GetSensorObject(T** object)
	{
		*object = nullptr;
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
	static void SetDefaultUniqueSettings(Llc200d3sh_Config_t uConfig, Llc200d3sh_ConfigMonitor_bm uConfigMonitor);

	/// <summary>
	/// Parse the unique objects from either the Llc200d3sh.json, or the "config" key from one of the sensors in the array in config_sensors.json.
	/// Pass either global or non global structures.
	/// </summary>
	/// <param name="obj"></param>
	/// <param name="uConfig"></param>
	/// <param name="uConfigMonitor"></param>
	static void ReadConfigObjectUnique(JsonVariantConst& obj, Llc200d3sh_Config_t& uConfig, Llc200d3sh_ConfigMonitor_bm& uConfigMonitor);

#pragma region MqttSensor Functions

	bool Read() override;


#pragma endregion

};


#pragma region JSON UDFs



//bool canConvertFromJson(JsonVariantConst src, const Llc200d3sh_Config_t&);
void convertFromJson(JsonVariantConst src, Llc200d3sh_Config_t& dst);
bool convertToJson(const Llc200d3sh_Config_t& src, JsonVariant dst);

//bool canConvertFromJson(JsonVariantConst src, const Llc200d3sh_Status_t&);
void convertFromJson(JsonVariantConst src, Llc200d3sh_Status_t& dst);
bool convertToJson(const Llc200d3sh_Status_t& src, JsonVariant dst);


#pragma endregion


#endif

