// MqttDevice.h

#ifndef _MQTTDEVICE_h
#define _MQTTDEVICE_h

#include <Arduino.h>

#include <ArduinoJson.hpp>
#include <ArduinoJson.h>
#include <PubSubClient.h>

#include "../../../ESP_Sense.h"
#include "../../Config/config_mqtt.h"
#include "MqttDevice_Config.h"
#include "../MqttHelper.h"


//#include <ArduinoJson.hpp>
//#include <ArduinoJson.h>
//
//#include <StreamUtils.hpp>
//#include <StreamUtils.h>
//#include <LittleFS.h>



enum class ecode{EXIT_UNSUPPORTED = -1, EXIT_FAILED = 0, EXIT_OK = 1};
typedef int8_t ecode_t;

/// <summary>
/// Topics used by individual devices.
/// </summary>
typedef struct {
	String state;
	String command;
}MqttDeviceTopics_t;

typedef struct {
	bool useDefaults : 1;
	bool initiallyEnabled : 1;			//Enabled by default.
	bool useGlobalConfig : 1;
	bool important : 1;			//Is the MQTT device required for proper functioning?
	uint8_t reserved : 5;
}MqttDeviceConfig_t;

typedef struct {
	bool useDefaults : 1;
	bool initiallyEnabled : 1;
	bool useGlobalConfig : 1;
	bool important : 1;
	uint8_t reserved : 6;
}MqttDeviceConfigMonitor_t;

typedef struct {
	bool useDefaults : 1;
	bool json : 1;
	bool retain : 1;
	bool useParentTopics : 1;
}MqttDeviceMqttSettings_t;

typedef struct {
	bool useDefaults : 1;
	bool json : 1;
	bool retain : 1;
	bool useParentTopics : 1;
}MqttDeviceMqttSettingsMonitor_t;

typedef struct {
	struct {
		bool configured : 1;
		bool enabled : 1;
		bool subscribed : 1;
		bool markedDisconnected : 1;
		uint8_t reserved : 4;
	};
	unsigned long publishTimestamp;
	unsigned long publishErrorTimestamp;
}MqttDeviceStatus_t;

typedef struct {
	struct {
		bool configRead : 1;
		uint8_t reserved : 7;
	};
}MqttDeviceGlobalStatus_t;

typedef struct {
	bool enabled : 1;
	bool configured : 1;
}MqttDeviceWebpageStatus_t;

extern Config_t config;
extern DeviceStatus_t status;
extern PubSubClient mqttClient;

class MqttDevice
{
public:

	/// <summary>
	/// Index of the MQTT device in the order it was configured.
	/// </summary>
	uint8_t index;

	/// <summary>
	/// Index of the MQTT device for the specific type. (Binary Sensor, Sensor, ect...)
	/// </summary>
	uint8_t indexType;

	/// <summary>
	/// Custom name for the MQTT device.
	/// </summary>
	String name;

	/// <summary>
	/// MQTT Device Configuration
	/// </summary>
	MqttDeviceConfig_t deviceConfig;
	MqttDeviceConfigMonitor_t deviceConfigMonitor;

	/// <summary>
	/// Device MQTT Settings
	/// </summary>
	MqttDeviceMqttSettings_t deviceMqttSettings;
	MqttDeviceMqttSettingsMonitor_t deviceMqttSettingsMonitor;

	String baseTopic;

	MqttTopics_t topics;
	MqttDeviceTopics_t deviceTopics;

	MqttDeviceStatus_t deviceStatus;
	MqttDeviceWebpageStatus_t webpageStatus;

	virtual const char* GetConfigFilePath() = 0;
	virtual const char* GetConfigFileName() = 0;
	virtual const char* GetConfigObjKey() = 0;

	virtual const char* GetDefaultBaseTopic() = 0;
	virtual String* GetParentBaseTopic() = 0;


	MqttDevice(const char* _name, int _index)
	{
		//strlcpy(name, _name, NAME_MAX_LENGTH);
		name = _name;
		index = _index;
	}


	virtual bool Init(bool enable = true);

	virtual void ResetStatus();

	virtual void Loop() {return;}
	virtual bool Configure() { return true; }			//Requires no configuration
	virtual bool Reconfigure() { return Configure(); }
	virtual void Denit() {return;}

	virtual void MarkDisconnected();
	virtual void MarkReconnected();

	virtual bool Enable();
	virtual bool Disable();

	virtual bool Subscribe();
	virtual bool Unsubscribe();
	virtual bool Publish();
	virtual bool PublishAvailability();

	virtual bool PublishDisabled();
	static bool PublishDisabled(const char* topic);

	virtual String GenerateJsonPayload() = 0;

	//virtual String GenerateStatusJsonPayload() = 0;

//protected:
	//virtual bool GenerateStatusJsonPayload(JsonDocument& doc);	//Adds deviceStatus to a json string (each overloading function should call its parent) 

	virtual bool InitWebpage() { return true; }								//Start hosting configuration webpage
	virtual void StopHostingWebpage() { webpageStatus.enabled = false; }	//Stop hosting configuration webpage
	virtual void DenitWebpage() {}

	/// <summary>
	/// Receive MQTT command.
	/// </summary>
	/// <returns></returns>
	virtual int ReceiveCommand(char* topics, byte* payload, size_t length) { return -1; }	//Not supported

//protected:

	virtual String CreateTopic(String base)
	{
		if (base[base.length()] != '/')
		{
			return base + "/" + name;
		}

		return base + name;
	}



	

	//virtual void* GetUniqueConfig() = 0;
	//virtual void* GetUniqueConfigMonitor() = 0;
	//virtual void* GetGlobalUniqueConfig() = 0;
	//virtual void* GetGlobalUniqueConfigMonitor() = 0;
	//virtual MqttDeviceConfig_t* GetGlobalDeviceConfig() = 0;
	//virtual MqttDeviceConfigMonitor_t* GetGlobalDeviceConfigMonitor() = 0;
	//virtual MqttDeviceMqttSettings_t* GetGlobalMqttConfig() = 0;
	//virtual MqttDeviceMqttSettingsMonitor_t* GetGlobalMqttConfigMonitor() = 0;
	//virtual MqttTopics_t* GetGlobalMqttBaseTopics() {
	//	return nullptr;
	//}
	//virtual MqttTopics_t* GetGlobalMqttTopics() {
	//	return nullptr;
	//}
	//virtual MqttDeviceTopics_t* GetGlobalMqttDeviceTopics() {
	//	return nullptr;
	//}
	virtual MqttDeviceGlobalStatus_t* GetGlobalDeviceStatus() = 0;

	virtual void SetBaseTopic(JsonVariantConst& topicsObj, const char* defaultDeviceBase, String& parentBase)
	{
		if (topicsObj.containsKey("base"))
		{
			String base = topicsObj["base"].as<String>();
			if(base[0] == '/')
				baseTopic = parentBase + base;
			else
				baseTopic = base;
		}
		else
		{
			if (defaultDeviceBase[0] == '/')
				baseTopic = parentBase + defaultDeviceBase;
			else
				baseTopic = defaultDeviceBase;
		}
	}

	virtual void GenerateTopics()
	{
		topics.availability = baseTopic + config.mqtt.baseTopics.availability;
		topics.jsonCommand = baseTopic + config.mqtt.baseTopics.jsonCommand;
		topics.jsonState = baseTopic + config.mqtt.baseTopics.jsonState;
		
		//deviceTopics.command = baseTopic + config.mqtt.baseTopics.command;
		//deviceTopics.state = baseTopic + config.mqtt.baseTopics.state;
	}

	virtual void SetDefaultSettings();
	virtual void SetDefaultGlobalSettings() = 0;
	//virtual void UseGlobalSettings() = 0;
	virtual void SetDefaultUniqueSettings() = 0;
	virtual void SetDefaultGlobalUniqueSettings() = 0;
	virtual void SetDefaultMqttTopics() = 0;
	//virtual void SetDefaultGlobalMqttTopics() = 0;
	virtual bool ReadGlobalConfig();
	//virtual void ReadTopicsObject(JsonObject& topicsObj) = 0;
	//virtual void ReadGlobalBaseTopicsObject(JsonObject& topicsObj) = 0;
	virtual void ReadConfigObjectUnique(JsonVariantConst& obj) = 0;
	virtual void ReadGlobalUniqueConfigObject(JsonVariantConst& obj) = 0;

	/// <summary>
	/// Parse the device config from either a device from an array, or a devices json file.
	/// Passes the non global structures from the MqttDevice.
	/// </summary>
	/// <param name="obj"></param>
	virtual void ReadConfigObject(JsonVariantConst& obj);

	/// <summary>
	/// Parse the device config from either a device from an array, or a devices json file.
	/// Passes the global structures associated with the MqttDevice.
	/// </summary>
	/// <param name="obj"></param>
	virtual void ReadGlobalConfigObject(JsonVariantConst& obj) = 0;

#pragma region Static Functions

	//static void ReadConfigObject(MqttDevice& device, JsonObject& obj, bool global);

	static void ReadConfigObject(JsonVariantConst& obj, MqttDeviceConfig_t& deviceConfig, MqttDeviceConfigMonitor_t& deviceConfigMonitor, MqttDeviceMqttSettings_t& mqttSettings, MqttDeviceMqttSettingsMonitor_t& mqttSettingsMonitor, JsonVariantConst* out_topicsObj = nullptr, bool* out_hasTopics = nullptr);

	static void SetDefaultDeviceSettings(MqttDeviceConfig_t& deviceConfig, MqttDeviceConfigMonitor_t& deviceConfigMonitor);

	static void SetDefaultMqttSettings(MqttDeviceMqttSettings_t& mqttSettings, MqttDeviceMqttSettingsMonitor_t& mqttSettingsMonitor);

	static void SetDefaultMqttTopics(MqttTopics_t& topics, MqttDeviceTopics_t& deviceTopics, String& baseTopic, const char* defaultDeviceBase, String& parentBase);

	static void ReadConfigDeviceSettings(JsonVariantConst& deviceObj, MqttDeviceConfig_t& deviceConfig, MqttDeviceConfigMonitor_t& deviceConfigMonitor);

	static void ReadConfigMqttSettings(JsonVariantConst& mqttObj, MqttDeviceMqttSettings_t& mqttSettings, MqttDeviceMqttSettingsMonitor_t& mqttSettingsMonitor);

	//static void ReadConfigMqttTopics(JsonObject& topicsObj, MqttTopics_t& topics, MqttDeviceTopics_t& deviceTopics, String& baseTopic, const char* defaultDeviceBase, String& parentBase);

#pragma endregion
};


#endif

