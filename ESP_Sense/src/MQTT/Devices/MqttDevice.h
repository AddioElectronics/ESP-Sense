// MqttDevice.h

#ifndef _MQTTDEVICE_h
#define _MQTTDEVICE_h

#include <Arduino.h>

#include <functional>

#include <ArduinoJson.hpp>
#include <ArduinoJson.h>
#include <PubSubClient.h>

#include "../../../ESP_Sense.h"
#include "../../Config/config_mqtt.h"
#include "MqttDevice_Config.h"
#include "../MqttHelper.h"
#include "MqttDeviceWeb.h"
#include "../../JsonHelper.h"

//#include <ArduinoJson.hpp>
//#include <ArduinoJson.h>
//
//#include <StreamUtils.hpp>
//#include <StreamUtils.h>
//#include <LittleFS.h>



enum class ecode{EXIT_UNSUPPORTED = -1, EXIT_FAILED = 0, EXIT_OK = 1};
typedef int8_t ecode_t;

//typedef bool(MqttDevice::* ADD_PAYLOAD_FUNC)(void);

typedef std::function<void(JsonVariant&)> ADD_PAYLOAD_FUNC;

enum class MqttDeviceType
{
	MQTT_BINARY_SENSOR,
	MQTT_BUTTON,
	MQTT_LIGHT,
	MQTT_SENSOR,
	MQTT_SWITCH
};

enum class DeviceState
{
	DEVICE_OK,			//Functioning and publishing
	DEVICE_DISABLED,	//Functioning but publishing disabled
	DEVICE_ERROR		//Not functioning
};
typedef uint8_t DeviceState_t;

extern const char* device_state_strings[3];

/// <summary>
/// Topics used by individual devices.
/// </summary>
typedef struct {
	String state;
	String command;
}MqttDeviceTopics_t;

typedef struct {
	bool hostWebsite : 1;
	bool configurable : 1;
}MqttDeviceWebConfig_t;

typedef struct {
	bool useDefaults : 1;
	bool initiallyEnabled : 1;			//Enabled by default.
	bool useGlobalConfig : 1;
	bool important : 1;			//Is the MQTT device required for proper functioning?
	uint8_t reserved : 4;
	MqttDeviceWebConfig_t website;
}MqttDeviceConfig_t;

typedef struct {
	bool useDefaults : 1;
	bool initiallyEnabled : 1;
	bool useGlobalConfig : 1;
	bool important : 1;
	uint8_t reserved : 4;
	struct {
		bool hostWebsite : 1;
		bool configurable : 1;
	}website;
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

//typedef struct {
//	bool enabled : 1;
//	bool configured : 1;
//}MqttDeviceWebpageStatus_t;

typedef struct {
	struct {
		bool configured : 1;
		bool enabled : 1;
		DeviceState_t state : 2;
		bool subscribed : 1;
		bool markedDisconnected : 1;
		bool configModified : 1;			//Has its config been modified during runtime?
		uint8_t reserved : 1;
	};
	unsigned long publishTimestamp;
	unsigned long publishErrorTimestamp;
	//MqttDeviceWebpageStatus_t web;
}MqttDeviceStatus_t;

typedef struct {
	struct {
		bool configRead : 1;
		uint8_t reserved : 7;
	};
}MqttDeviceGlobalStatus_t;



extern Config_t config;
extern GlobalStatus_t status;
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
	uint8_t subTypeIndex;

	/// <summary>
	/// Custom name for the MQTT device.
	/// </summary>
	String name;
	
	///// <summary>
	///// Name of the IC or feature.
	///// *Make pointer, and store string in device type's global config.
	///// </summary>
	//String deviceName;

	virtual const char* DeviceName() = 0;
	virtual const char* DeviceKey() = 0;
	virtual const char* DeviceTypeName() = 0;
	virtual const char* DeviceTypeKey() = 0;
	//virtual device_count_t* GetDeviceTypeCount() = 0;

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
	

	MqttDeviceWeb* website = nullptr;

protected:
	DynamicJsonDocument* document = nullptr;	//Change to non pointer? Document contains pointer to data, may be no benefits to using pointer.
	JsonVariant documentRoot;
public:

	virtual const char* GetConfigFilePath() = 0;
	virtual const char* GetConfigFileName() = 0;
	virtual const char* GetConfigObjKey() = 0;

	virtual const char* GetDefaultBaseTopic() = 0;
	virtual String* GetParentBaseTopic() = 0;


	MqttDevice(const char* _name, int _index, int _subIndex)
	{
		//strlcpy(name, _name, NAME_MAX_LENGTH);
		name = _name;
		index = _index;
		subTypeIndex = _subIndex;
		document = nullptr;
		website = nullptr;
		ResetStatus();
	}


	virtual bool Init();

	virtual void ResetStatus();

	virtual void Loop();
	virtual bool Configure() { return true; }			//Requires no configuration
	virtual bool Reconfigure() { return Configure(); }
	virtual void Denit() { /*website->Deinitialize();*/  return; }

	virtual void MarkDisconnected();
	virtual void MarkReconnected();

	virtual bool Enable();
	virtual bool Disable();

	virtual void SetDeviceState();

	virtual bool Subscribe();
	virtual bool Unsubscribe();
	virtual bool Publish();
	virtual bool PublishAvailability();

	virtual bool PublishDisabled();
	static bool PublishDisabled(const char* topic);

	virtual bool ParseConfigFromWeb(String& jsonData) { return false; };
	virtual bool SaveConfig() = 0;	//Save config to file.


	//virtual int SetConfig(JsonDocument& doc);		//Set config with data from web
	//virtual String GenerateJsonConfig() = 0;		//Generate config data as string.

	String GenerateJsonData(ADD_PAYLOAD_FUNC, const char* dataType);
	virtual String GenerateJsonStatePayload();
	String GenerateJsonStatus();
	String GenerateJsonConfig();
	String GenerateJsonAll();

	virtual bool CreateJsonDocument();
	bool FreshJsonDocument();
	void FreeJsonDocument();
	bool IsDocumentCreated(bool create = true);
	size_t SerializeDocument(String* out_string, bool freeDoc = true);
	size_t StreamDocument(AsyncWebServerRequest* request);

	virtual void AddStatePayload(JsonVariant& addTo) = 0;		//Payload for MQTT state topic
	virtual void AddStatusData(JsonVariant& addTo);				//device, binary/sensor/ect.., and unique status
	virtual void AddConfigData(JsonVariant& addTo);				//device, binary/sensor/ect.., and unique config

//protected:
	//virtual bool GenerateStatusJsonPayload(JsonDocument& doc);	//Adds deviceStatus to a json string (each overloading function should call its parent) 

	virtual bool InitWebpage();									//Start hosting configuration webpage
	virtual void DenitWebpage();

	/// <summary>
	/// Receive MQTT command.
	/// </summary>
	/// <returns></returns>
	virtual int ReceiveCommand(char* topics, byte* payload, size_t length) { return -1; }	//Not supported

	virtual MqttDeviceType GetDeviceType() = 0;

	//MqttDeviceType GetDeviceType()
	//{
	//	MqttBinarySensor* binarySensor = dynamic_cast<MqttBinarySensor*>(this);
	//	if (binarySensor != nullptr) return MqttDeviceType::MQTT_BINARY_SENSOR;

	//	//MqttButton* button = dynamic_cast<MqttButton*>(this);
	//	//if (button != nullptr) return MqttDeviceType::MQTT_BUTTON;

	//	MqttSensor* sensor = dynamic_cast<MqttSensor*>(this);
	//	if (sensor != nullptr) return MqttDeviceType::MQTT_SENSOR;

	//	//MqttLight* light = dynamic_cast<MqttLight*>(this);
	//	//if (light != nullptr) return MqttDeviceType::MQTT_LIGHT;

	//	//MqttSwitch* mqttSwitch = dynamic_cast<MqttSwitch*>(this);
	//	//if (mqttSwitch != nullptr) return MqttDeviceType::MQTT_SWITCH;
	//}

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


#pragma region JSON UDFs


//bool canConvertFromJson(JsonVariantConst src, const MqttDeviceType&);
void convertFromJson(JsonVariantConst src, MqttDeviceType& dst);
bool convertToJson(const MqttDeviceType& src, JsonVariant dst);

//bool canConvertFromJson(JsonVariantConst src, const MqttDeviceTopics_t&);
void convertFromJson(JsonVariantConst src, MqttDeviceTopics_t& dst);
bool convertToJson(const MqttDeviceTopics_t& src, JsonVariant dst);

//bool canConvertFromJson(JsonVariantConst src, const MqttDeviceConfig_t&);
void convertFromJson(JsonVariantConst src, MqttDeviceConfig_t& dst);
bool convertToJson(const MqttDeviceConfig_t& src, JsonVariant dst);

//bool canConvertFromJson(JsonVariantConst src, const MqttDeviceStatus_t&);
//void convertFromJson(JsonVariantConst src, MqttDeviceStatus_t& dst);
bool convertToJson(const MqttDeviceStatus_t& src, JsonVariant dst);

//bool canConvertFromJson(JsonVariantConst src, const MqttDeviceWebConfig_t&);
void convertFromJson(JsonVariantConst src, MqttDeviceWebConfig_t& dst);
bool convertToJson(const MqttDeviceWebConfig_t& src, JsonVariant dst);

//bool canConvertFromJson(JsonVariantConst src, const MqttDeviceMqttSettings_t&);
void convertFromJson(JsonVariantConst src, MqttDeviceMqttSettings_t& dst);
bool convertToJson(const MqttDeviceMqttSettings_t& src, JsonVariant dst);

//bool canConvertFromJson(JsonVariantConst src, const MqttDeviceGlobalStatus_t&);
//void convertFromJson(JsonVariantConst src, MqttDeviceGlobalStatus_t& dst);
bool convertToJson(const MqttDeviceGlobalStatus_t& src, JsonVariant dst);

////bool canConvertFromJson(JsonVariantConst src, const MqttDeviceWebpageStatus_t&);
//void convertFromJson(JsonVariantConst src, MqttDeviceWebpageStatus_t& dst);
//bool convertToJson(const MqttDeviceWebpageStatus_t& src, JsonVariant dst);


//bool canConvertFromJson(JsonVariantConst src, const DeviceState&);
//void convertFromJson(JsonVariantConst src, DeviceState& dst);
bool convertToJson(const DeviceState& src, JsonVariant dst);

#pragma endregion




#endif

