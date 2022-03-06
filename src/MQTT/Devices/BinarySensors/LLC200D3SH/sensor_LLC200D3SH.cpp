#include "sensor_LLC200D3SH.h"

#include "../../../MqttManager.h"
#include "../../../MqttHelper.h"

extern PubSubClient mqttClient;

#pragma region Global Variables

MqttDeviceConfig_t Llc200d3sh_Sensor::globalDeviceConfig;
MqttDeviceConfigMonitor_t Llc200d3sh_Sensor::globalDeviceConfigMonitor;

MqttDeviceMqttSettings_t Llc200d3sh_Sensor::globalDeviceMqttSettings;
MqttDeviceMqttSettingsMonitor_t Llc200d3sh_Sensor::globalDeviceMqttSettingsMonitor;

Llc200d3sh_Config_t Llc200d3sh_Sensor::globalUniqueConfig;
Llc200d3sh_ConfigMonitor_bm Llc200d3sh_Sensor::globalUniqueConfigMonitor;

MqttDeviceGlobalStatus_t Llc200d3sh_Sensor::globalDeviceStatus;

#pragma endregion


bool Llc200d3sh_Sensor::Read()
{
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

bool Llc200d3sh_Sensor::Subscribe()
{
	return MqttDevice::Subscribe();
}

bool Llc200d3sh_Sensor::Unsubscribe()
{
	return MqttDevice::Unsubscribe();
}

//bool Llc200d3sh_Sensor::Publish()
//{
//	if (deviceStatus.enabled && binarySensorStatus.newData)
//	{
//		if (deviceMqttSettings.json)
//		{
//
//		}
//		else
//		{
//			mqttClient.publish(deviceTopics.state.c_str(), Mqtt::Helper::GetBinarySensorString(measurement));
//		}
//
//		DEBUG_LOG("Water Level : ");
//		DEBUG_LOG(measurement ? MQTT_BINARY_SENSOR_ON : MQTT_BINARY_SENSOR_OFF);
//		DEBUG_LOG_LN();
//	}
//	
//	return false;
//}


String Llc200d3sh_Sensor::GenerateJsonPayload()
{
	DEBUG_LOG_F("...Generating %s(LLC200D3SH)\r\n-JSON Data :\r\n", name.c_str());

	String jdata;
	StaticJsonDocument<128> doc;
	JsonObject obj = doc.createNestedObject(name);

	JsonVariant waterlevel = obj.createNestedObject("waterlevel");
	waterlevel.set(measurement ? MQTT_BINARY_SENSOR_ON : MQTT_BINARY_SENSOR_OFF);

	serializeJson(doc, jdata);

	DEBUG_LOG_LN(jdata.c_str());
	DEBUG_NEWLINE();

	return jdata;
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