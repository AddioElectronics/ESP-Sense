#include "sensor_LLC200D3SH.h"

#include "../../../MqttManager.h"
#include "../../../MqttHelper.h"

extern PubSubClient mqttClient;

const char* Llc200d3sh_Sensor::deviceName = "LLC200D3SH";
const char* Llc200d3sh_Sensor::deviceKey = "llc200d3sh";

#pragma region Global Variables

MqttDeviceConfig_t Llc200d3sh_Sensor::globalDeviceConfig;
MqttDeviceConfigMonitor_t Llc200d3sh_Sensor::globalDeviceConfigMonitor;

MqttDeviceMqttSettings_t Llc200d3sh_Sensor::globalDeviceMqttSettings;
MqttDeviceMqttSettingsMonitor_t Llc200d3sh_Sensor::globalDeviceMqttSettingsMonitor;

Llc200d3sh_Config_t Llc200d3sh_Sensor::globalUniqueConfig;
Llc200d3sh_ConfigMonitor_bm Llc200d3sh_Sensor::globalUniqueConfigMonitor;

MqttDeviceGlobalStatus_t Llc200d3sh_Sensor::globalDeviceStatus;

#pragma endregion

bool Llc200d3sh_Sensor::Subscribe()
{
	deviceStatus.subscribed = true;
	return true;
	return MqttDevice::Subscribe();
}

bool Llc200d3sh_Sensor::Unsubscribe()
{
	deviceStatus.subscribed = false;
	return true;
	return MqttDevice::Unsubscribe();
}

bool Llc200d3sh_Sensor::Read()
{
	if (deviceStatus.state != (DeviceState_t)DeviceState::DEVICE_OK)
		return false;

	DEBUG_LOG_F("Reading %s water level.\r\n-GPIO : %d\r\n", name.c_str(), uniqueConfig.program.gpio);

	if (uniqueConfig.program.gpio == -1)
	{
		DEBUG_LOG_LN("Failed : GPIO invalid.");
		return false;
	}

	measurement = digitalRead(uniqueConfig.program.gpio);	//Low Voltage = Water Detected, High Voltage = Water Not Detected

	if (uniqueConfig.program.invert)
		measurement = !measurement;

	binarySensorStatus.newData = true;

	DEBUG_LOG_F("Water Level : %s\r\n", uniqueConfig.program.waterPresent == measurement ? "ok" : "low");
	DEBUG_NEWLINE();

	return 0;
}


void Llc200d3sh_Sensor::AddStatePayload(JsonVariant& addTo)
{
	JsonVariant obj = addTo.getOrAddMember("statePayload");

	JsonVariant waterlevel = obj.getOrAddMember("waterlevel");
	waterlevel.set(measurement ? MQTT_BINARY_SENSOR_ON : MQTT_BINARY_SENSOR_OFF);
}

void Llc200d3sh_Sensor::AddStatusData(JsonVariant& addTo)
{
	MqttBinarySensor::AddStatusData(addTo);
	addTo["uniqueStatus"].set<Llc200d3sh_Status_t>(uniqueStatus);
}

void Llc200d3sh_Sensor::AddConfigData(JsonVariant& addTo)
{
	MqttDevice::AddConfigData(addTo);
	addTo["uniqueConfig"].set<Llc200d3sh_Config_t>(uniqueConfig);
}

int Llc200d3sh_Sensor::ReceiveCommand(char* topic, byte* payload, size_t length)
{
	#warning wip
		return 1;
}

//void* Llc200d3sh_Sensor::GetUniqueConfig() {
//	return &uniqueConfig;
//}
//
//void* Llc200d3sh_Sensor::GetUniqueConfigMonitor() {
//	return &uniqueConfigMonitor;
//}
//
//void* Llc200d3sh_Sensor::GetGlobalUniqueConfig() {
//	return &globalUniqueConfig;
//}
//
//void* Llc200d3sh_Sensor::GetGlobalUniqueConfigMonitor() {
//	return &globalUniqueConfigMonitor;
//}
//MqttDeviceConfig_t* Llc200d3sh_Sensor::GetGlobalDeviceConfig() {
//	return &globalDeviceConfig;
//}
//MqttDeviceConfigMonitor_t* Llc200d3sh_Sensor::GetGlobalDeviceConfigMonitor() {
//	return &globalDeviceConfigMonitor;
//}
//MqttDeviceMqttSettings_t* Llc200d3sh_Sensor::GetGlobalMqttConfig() {
//	return &globalDeviceMqttSettings;
//}
//MqttDeviceMqttSettingsMonitor_t* Llc200d3sh_Sensor::GetGlobalMqttConfigMonitor() {
//	return &globalDeviceMqttSettingsMonitor;
//}
MqttDeviceGlobalStatus_t* Llc200d3sh_Sensor::GetGlobalDeviceStatus() {
	return &globalDeviceStatus;
}



void Llc200d3sh_Sensor::GenerateTopics()
{
	MqttDevice::GenerateTopics();
}

void Llc200d3sh_Sensor::SetDefaultSettings()
{
	MqttDevice::SetDefaultDeviceSettings(deviceConfig, deviceConfigMonitor);
	MqttDevice::SetDefaultMqttSettings(deviceMqttSettings, deviceMqttSettingsMonitor);
	SetDefaultUniqueSettings(uniqueConfig, uniqueConfigMonitor);
}

void Llc200d3sh_Sensor::SetDefaultGlobalSettings()
{
	MqttDevice::SetDefaultDeviceSettings(globalDeviceConfig, globalDeviceConfigMonitor);
	MqttDevice::SetDefaultMqttSettings(globalDeviceMqttSettings, globalDeviceMqttSettingsMonitor);
	SetDefaultUniqueSettings(globalUniqueConfig, globalUniqueConfigMonitor);
}

void Llc200d3sh_Sensor::SetDefaultUniqueSettings()
{
	SetDefaultUniqueSettings(uniqueConfig, uniqueConfigMonitor);
}

void Llc200d3sh_Sensor::SetDefaultGlobalUniqueSettings()
{
	SetDefaultUniqueSettings(globalUniqueConfig, globalUniqueConfigMonitor);
}

void Llc200d3sh_Sensor::SetDefaultMqttTopics()
{
	MqttDevice::SetDefaultMqttTopics(topics, deviceTopics, baseTopic, TOPIC_SENSOR_LLC200D3SH_BASE, mqttBinarySensorBaseTopic);

	//Add device specific topics here
}



//void Llc200d3sh_Sensor::SetDefaultGlobalMqttTopics()
//{
//	#warning WIP
//}


void Llc200d3sh_Sensor::ReadConfigObjectUnique(JsonVariantConst& llc200Object)
{
	ReadConfigObjectUnique(llc200Object, uniqueConfig, uniqueConfigMonitor);
}

void Llc200d3sh_Sensor::ReadGlobalConfigObject(JsonVariantConst& obj)
{
	JsonVariantConst outTopicsObj;
	ReadConfigObject(obj, deviceConfig, deviceConfigMonitor, deviceMqttSettings, deviceMqttSettingsMonitor, &outTopicsObj);
	//ReadTopicsObject(outTopicsObj);
	ReadConfigObjectUnique(obj);
}


void Llc200d3sh_Sensor::ReadGlobalUniqueConfigObject(JsonVariantConst& scd4xObject)
{
	ReadConfigObjectUnique(scd4xObject, globalUniqueConfig, globalUniqueConfigMonitor);
}

//void Llc200d3sh_Sensor::ReadTopicsObject(JsonObject& topicsObj)
//{
//	MqttDevice::ReadConfigMqttTopics(topicsObj, topics, deviceTopics, baseTopic, TOPIC_SENSOR_LLC200D3SH_BASE, mqttBinarySensorBaseTopic);
//
//	//Read special topics here
//}



void Llc200d3sh_Sensor::SetDefaultUniqueSettings(Llc200d3sh_Config_t uConfig, Llc200d3sh_ConfigMonitor_bm uConfigMonitor)
{
	//Monitor
	uConfigMonitor.program.useDefaults = uConfig.program.useDefaults != false;

	uConfigMonitor.program.waterPresent = uConfig.program.waterPresent != LLC200D3SH_WATER_PRESENT_VOLTAGE;
	uConfigMonitor.program.invert = uConfig.program.invert != LLC200D3SH_INVERT_READING;
	uConfigMonitor.program.gpio = uConfig.program.gpio != LLC200D3SH_GPIO;


	//Settings
	uConfig.program.useDefaults = false;

	uConfig.program.waterPresent = LLC200D3SH_WATER_PRESENT_VOLTAGE;
	uConfig.program.invert = LLC200D3SH_INVERT_READING;
	uConfig.program.gpio = LLC200D3SH_GPIO;
}


void Llc200d3sh_Sensor::ReadConfigObjectUnique(JsonVariantConst& obj, Llc200d3sh_Config_t& uConfig, Llc200d3sh_ConfigMonitor_bm& uConfigMonitor)
{
	
	if (obj.containsKey("program"))
	{
		JsonVariantConst programObj = obj["program"];

		//Mqtt::ReadKey(uConfig.program.gpio, programObj, "gpio");

		if (programObj.containsKey("useDefaults"))
			uConfig.program.useDefaults = programObj["useDefaults"];

		//if (uConfig.program.useDefaults)
		//	return;

		if (programObj.containsKey("waterPresent"))
			uConfig.program.waterPresent = programObj["waterPresent"];

		if (programObj.containsKey("invert"))
			uConfig.program.invert = programObj["invert"];

		if (programObj.containsKey("gpio"))
			uConfig.program.gpio = programObj["gpio"];
	}
}


#pragma region JSON UDFs


//bool canConvertFromJson(JsonVariantConst src, const Llc200d3sh_Config_t&)
//{

//}

void convertFromJson(JsonVariantConst src, Llc200d3sh_Config_t& dst)
{
	JsonVariantConst obj = src;

	if (src.containsKey("uniqueConfig"))
		obj = src["uniqueConfig"];


	if (obj.containsKey("program"))
	{
		JsonVariantConst program = obj["program"];

		dst.program.useDefaults = obj["useDefaults"];
		dst.program.gpio = obj["gpio"];
		dst.program.waterPresent = obj["waterPresent"];
		dst.program.invert = obj["invert"];
	}
}

bool convertToJson(const Llc200d3sh_Config_t& src, JsonVariant dst)
{
	JsonObject obj = dst.createNestedObject("program");

	obj["configRead"] = src.program.useDefaults;
	obj["gpio"] = src.program.gpio;
	obj["waterPresent"] = src.program.waterPresent;
	obj["invert"] = src.program.invert;
}

bool canConvertFromJson(JsonVariantConst src, const Llc200d3sh_Status_t&)
{
	return src.containsKey("program") || src["uniqueConfig"].containsKey("program");
}

void convertFromJson(JsonVariantConst src, Llc200d3sh_Status_t& dst)
{
	JsonVariantConst obj = src;

	if (src.containsKey("uniqueStatus"))
		obj = src["uniqueStatus"];


	if (obj.containsKey("gpioValid"))
		dst.gpioValid = obj["gpioValid"];

}

bool convertToJson(const Llc200d3sh_Status_t& src, JsonVariant dst)
{
	dst["gpioValid"] = src.gpioValid;
}


#pragma endregion

