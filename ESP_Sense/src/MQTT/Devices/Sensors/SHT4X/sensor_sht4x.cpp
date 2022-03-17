#include "sensor_sht4x.h"

#include <StreamUtils.hpp>
#include <StreamUtils.h>	

#if defined(ESP8266)
#include <LittleFS.h>	
#elif defined(ESP32)
#include <FS.h>
#include <LITTLEFS.h>
#endif

#include "../../../../../ESP_Sense.h"
#include "../../../MqttManager.h"
#include "../../../MqttHelper.h"
#include "../../../../JsonHelper.h"
#include "../../../../HelperFunctions.h"
#include "../../../../macros.h"


extern PubSubClient mqttClient;
extern GlobalStatus_t status;

MqttDeviceConfig_t Sht4xSensor::globalDeviceConfig;
MqttDeviceConfigMonitor_t Sht4xSensor::globalDeviceConfigMonitor;

MqttDeviceMqttSettings_t Sht4xSensor::globalDeviceMqttSettings;
MqttDeviceMqttSettingsMonitor_t Sht4xSensor::globalDeviceMqttSettingsMonitor;

Sht4xConfig_t Sht4xSensor::globalUniqueConfig;
Sht4xConfigMonitor_bm Sht4xSensor::globalUniqueConfigMonitor;

MqttDeviceGlobalStatus_t Sht4xSensor::globalDeviceStatus;

#pragma region MqttDevice Functions

bool Sht4xSensor::Init()
{
	if (!status.device.i2cInitialized)
		if (!EspSense::InitializeI2C())
			return false;

	//if (!status.mqtt.devicesConfigured)
	//memset(&uniqueStatus, 0, sizeof(SHT4xStatus_t));

	return MqttSensor::Init();
	//memset(&deviceStatus, 0, sizeof(MqttDeviceStatus_t));
	//memset(&sensorStatus, 0, sizeof(MqttSensorStatus_t));

	//if (Connect())
	//	return Configure();

	//return false;
}

//void Sht4xSensor::Loop()
//{
//	if (!deviceStatus.enabled) return;
//
//	if (!sensorStatus.connected)
//	{
//		if (Connect())
//		{
//			MarkReconnected();
//			if (deviceStatus.configured)
//				DEBUG_LOG_F(MQTT_DMSG_RECONNECTED, name.c_str());
//		}
//
//		//if (!deviceStatus.configured)
//		//	Configure();
//
//		return;
//	}
//}

bool Sht4xSensor::Configure()
{
	if (!sensorStatus.connected) return false;

	DEBUG_LOG_LN("Configuring SHT40...");
	//DEBUG_LOG_LN("Configuring SHT40...");

	//If setup has completed and no settings have changed then return.
	if (deviceStatus.configured && uniqueConfigMonitor.bitmap == 0)
	{
		DEBUG_LOG_LN("Nothing to configure.");
		return true;
	}

	SetPrecision(uniqueConfig.internal.precision);
	SetHeater(uniqueConfig.internal.heater);

	if (sensor.getPrecision() != uniqueConfig.internal.precision || sensor.getHeater() != uniqueConfig.internal.heater)
	{
		DEBUG_LOG_LN("...Configuration Failed.");
		return false;
	}

	DEBUG_LOG_LN("...Configuration Complete.");
	deviceStatus.configured = true;

	return true;
}

bool Sht4xSensor::Enable()
{
	if (!sensorStatus.connected)
		if (!Connect())
			return false;

	return MqttDevice::Enable();
}


bool Sht4xSensor::Subscribe()
{
	return MqttDevice::Subscribe();

	//#warning  Also remember about the useParents settings.
		//if (deviceMqttSettings.json)
		//{
		//	
		//}
		//else
		//{
		//	
		//}

		//return false;
}
bool Sht4xSensor::Unsubscribe()
{
	return MqttDevice::Unsubscribe();

	//if (deviceMqttSettings.json)
	//{
	//	
	//}
	//else
	//{
	//	
	//}

	//return false;
}


//bool Sht4xSensor::PublishAvailability()
//{
//	return mqttClient.publish(topics.availability.c_str(), Mqtt::Helper::GetAvailabilityString(sensorStatus.connected).c_str());
//}


void Sht4xSensor::AddStatePayload(JsonObject& addTo)
{
	JsonObject obj = addTo;
	if (addTo.size() == 0)
		obj = addTo.createNestedObject("statePayload");

	if (uniqueConfig.mqtt.publishTemperature)
	{
		JsonVariant temp = obj.createNestedObject("temp");

		if (sensorStatus.connected)
			temp.set(measurementData.temperature.temperature);
		else
			temp.set(SENSOR_DATA_UNKNOWN);
	}

	if (uniqueConfig.mqtt.publishHumdiity)
	{
		JsonVariant humidity = obj.createNestedObject("humidity");

		if (sensorStatus.connected)
			humidity.set(measurementData.humidity.relative_humidity);
		else
			humidity.set(SENSOR_DATA_UNKNOWN);
	}
}

//void Sht4xSensor::AddStatusData(JsonObject& addTo)
//{
//	MqttSensor::AddStatusData(addTo);
//	addTo["uniqueStatus"].set<SHT4xStatus_t>(uniqueStatus);
//}

void Sht4xSensor::AddConfigData(JsonObject& addTo)
{
	MqttDevice::AddConfigData(addTo);
	addTo["uniqueConfig"].set<Sht4xConfig_t>(uniqueConfig);
}


int Sht4xSensor::ReceiveCommand(char* topic, byte* payload, size_t length)
{
//#warning  wip
	return 1;
}



//void* Sht4xSensor::GetUniqueConfig()
//{
//	return &uniqueConfig;
//}
//
//void* Sht4xSensor::GetUniqueConfigMonitor() {
//	return &uniqueConfigMonitor;
//}
//
//void* Sht4xSensor::GetGlobalUniqueConfig() {
//	return &globalUniqueConfig;
//}
//
//void* Sht4xSensor::GetGlobalUniqueConfigMonitor() {
//	return &globalUniqueConfigMonitor;
//}
//MqttDeviceConfig_t* Sht4xSensor::GetGlobalDeviceConfig() {
//	return &globalDeviceConfig;
//}
//MqttDeviceConfigMonitor_t* Sht4xSensor::GetGlobalDeviceConfigMonitor() {
//	return &globalDeviceConfigMonitor;
//}
//MqttDeviceMqttSettings_t* Sht4xSensor::GetGlobalMqttConfig() {
//	return &globalDeviceMqttSettings;
//}
//MqttDeviceMqttSettingsMonitor_t* Sht4xSensor::GetGlobalMqttConfigMonitor() {
//	return &globalDeviceMqttSettingsMonitor;
//}
MqttDeviceGlobalStatus_t* Sht4xSensor::GetGlobalDeviceStatus() {
	return &globalDeviceStatus;
}

void Sht4xSensor::GenerateTopics()
{
	MqttDevice::GenerateTopics();
}

void Sht4xSensor::SetDefaultSettings()
{
	MqttDevice::SetDefaultDeviceSettings(deviceConfig, deviceConfigMonitor);
	MqttDevice::SetDefaultMqttSettings(deviceMqttSettings, deviceMqttSettingsMonitor);
	SetDefaultUniqueSettings(uniqueConfig, uniqueConfigMonitor);
}

void Sht4xSensor::SetDefaultGlobalSettings()
{
	MqttDevice::SetDefaultDeviceSettings(globalDeviceConfig, globalDeviceConfigMonitor);
	MqttDevice::SetDefaultMqttSettings(globalDeviceMqttSettings, globalDeviceMqttSettingsMonitor);
	SetDefaultUniqueSettings(globalUniqueConfig, globalUniqueConfigMonitor);
}


void Sht4xSensor::SetDefaultUniqueSettings()
{
	SetDefaultUniqueSettings(uniqueConfig, uniqueConfigMonitor);
}

void Sht4xSensor::SetDefaultGlobalUniqueSettings()
{
	SetDefaultUniqueSettings(globalUniqueConfig, globalUniqueConfigMonitor);
}

void Sht4xSensor::SetDefaultMqttTopics()
{
	MqttDevice::SetDefaultMqttTopics(topics, deviceTopics, baseTopic, TOPIC_SENSOR_SHT4X_BASE, mqttSensorBaseTopic);

	//Add device specific topics here
}


//void Sht4xSensor::SetDefaultMqttTopics()
//{
//	MqttDevice::SetDefaultMqttTopics(topics, deviceTopics, baseTopic, TOPIC_SENSOR_SHT4X_BASE, mqttSensorBaseTopic);
//
//	//Add device specific topics here
//}
//
//void Sht4xSensor::SetDefaultGlobalMqttTopics()
//{
//	#warning WIP
//}

void Sht4xSensor::ReadConfigObjectUnique(JsonVariantConst& sht4xObject)
{
	ReadConfigObjectUnique(sht4xObject, uniqueConfig, uniqueConfigMonitor);
}

void Sht4xSensor::ReadGlobalConfigObject(JsonVariantConst& obj)
{
	JsonVariantConst* outTopicsObj;
	bool outHasTopics;
	ReadConfigObject(obj, deviceConfig, deviceConfigMonitor, deviceMqttSettings, deviceMqttSettingsMonitor/*, &outTopicsObj, &outHasTopics*/);
	//ReadTopicsObject(outTopicsObj);
	ReadConfigObjectUnique(obj);
}

void Sht4xSensor::ReadGlobalUniqueConfigObject(JsonVariantConst& sht4xObject)
{
	ReadConfigObjectUnique(sht4xObject, globalUniqueConfig, globalUniqueConfigMonitor);
}



//void Sht4xSensor::ReadTopicsObject(JsonObject& topicsObj)
//{
//	MqttDevice::ReadConfigMqttTopics(topicsObj, topics, deviceTopics, baseTopic, TOPIC_SENSOR_SHT4X_BASE, mqttSensorBaseTopic);
//
//	//Read special topics here
//}


void Sht4xSensor::SetDefaultUniqueSettings(Sht4xConfig_t& uConfig, Sht4xConfigMonitor_bm& uConfigMonitor)
{
	//Monitor
	uConfigMonitor.internal.useDefaults = uConfig.internal.useDefaults != false;

	uConfigMonitor.internal.precision = uConfig.internal.precision != SENSOR_SHT40_PRECISION;
	uConfigMonitor.internal.heater = uConfig.internal.heater != SENSOR_SHT40_HEATER;

	//MQTT Monitor
	uConfigMonitor.mqtt.publishTemperature = uConfig.mqtt.publishTemperature != SHT4X_PUBLISH_TEMPERATURE;
	uConfigMonitor.mqtt.publishHumdiity = uConfig.mqtt.publishHumdiity != SHT4X_PUBLISH_HUMIDITY;


	//Settings
	uConfigMonitor.internal.useDefaults = false;

	uConfig.internal.precision = SENSOR_SHT40_PRECISION;
	uConfig.internal.heater = SENSOR_SHT40_HEATER;

	//MQTT
	uConfig.mqtt.publishTemperature = SHT4X_PUBLISH_TEMPERATURE;
	uConfig.mqtt.publishHumdiity = SHT4X_PUBLISH_HUMIDITY;
}

void Sht4xSensor::ReadConfigObjectUnique(JsonVariantConst& sht4xObject, Sht4xConfig_t& uConfig, Sht4xConfigMonitor_bm& uConfigMonitor)
{

	if (sht4xObject.containsKey("internal"))
	{
		JsonVariantConst internalObj = sht4xObject["internal"];

		if (internalObj.containsKey("useDefaults"))
			uConfig.internal.useDefaults = internalObj["useDefaults"];

			//if (uConfig.internal.useDefaults)
			//	goto SkipInternal;

		if (internalObj.containsKey("precision"))
		{
			uConfig.internal.precision = (sht4x_precision_t)(internalObj["heater"].as<sht4x_precision>());
			//const char* pchar = internalObj["precision"];

			//if (strcmp(pchar, "high") == 0)
			//{
			//	uConfig.internal.precision = SHT4X_HIGH_PRECISION;
			//}
			//else if (strcmp(pchar, "med") == 0)
			//{
			//	uConfig.internal.precision = SHT4X_MED_PRECISION;
			//}
			//else if (strcmp(pchar, "low") == 0)
			//{
			//	uConfig.internal.precision = SHT4X_LOW_PRECISION;
			//}
			//else
			//{
			//	uint8_t pint = internalObj["precision"];
			//	if (pint >= 0 && pint <= 3)
			//	{
			//		uConfig.internal.precision = (sht4x_precision_t)pint;
			//	}
			//}
		}

		if (internalObj.containsKey("heater"))
		{
			uConfig.internal.heater = (sht4x_heater_t)(internalObj["heater"].as<sht4x_heater>());
			//uint8_t hint = internalObj["heater"];
			//if (hint >= 0 && hint <= 6)
			//{
			//	uConfig.internal.heater = (sht4x_heater_t)hint;
			//}
			//else
			//{
			//	const char* hchar = internalObj["heater"];

			//	if (strcmp(hchar, "none") == 0 || strcmp(hchar, "no") == 0)
			//	{
			//		uConfig.internal.heater = SHT4X_NO_HEATER;
			//	}
			//	else if (strcmp(hchar, "high1s") == 0)
			//	{
			//		uConfig.internal.heater = SHT4X_HIGH_HEATER_1S;
			//	}
			//	else if (strcmp(hchar, "high100ms") == 0)
			//	{
			//		uConfig.internal.heater = SHT4X_HIGH_HEATER_100MS;
			//	}
			//	else if (strcmp(hchar, "med1s") == 0)
			//	{
			//		uConfig.internal.heater = SHT4X_MED_HEATER_1S;
			//	}
			//	else if (strcmp(hchar, "med100ms") == 0)
			//	{
			//		uConfig.internal.heater = SHT4X_MED_HEATER_100MS;
			//	}
			//	else if (strcmp(hchar, "low1s") == 0)
			//	{
			//		uConfig.internal.heater = SHT4X_LOW_HEATER_1S;
			//	}
			//	else if (strcmp(hchar, "low100ms") == 0)
			//	{
			//		uConfig.internal.heater = SHT4X_LOW_HEATER_100MS;
			//	}
			//}
		}	
	}

	SkipInternal:
	if (sht4xObject.containsKey("mqtt"))
	{
		JsonVariantConst mqttObj = sht4xObject["mqtt"];

		if (mqttObj.containsKey("publishTemperature"))
			uConfig.mqtt.publishTemperature = mqttObj["publishTemperature"];

		if (mqttObj.containsKey("publishHumdiity"))
			uConfig.mqtt.publishHumdiity = mqttObj["publishHumdiity"];

	}
}

#pragma endregion

#pragma region MqttSensor Functions


bool Sht4xSensor::Connect()
{
	if (sensorStatus.connected) return true;

	//Connect to SHT4x
	if (sensor.begin())
	{
		sensorStatus.connected = IsConnected();
		DEBUG_LOG_LN("SHT4x Connected");
		//DEBUG_LOG_LN("SHT4x Connected");
		//client.publish(topic_debug, "SHT4x Connected");
	}
	else
	{
		sensorStatus.connected = false;

		//Only display error message on setup, or if an interval has passed.
		if (!status.config.setupComplete || millis() > deviceStatus.publishErrorTimestamp + (MQTT_ERROR_PUBLISH_RATE * 1000))
		{
			DEBUG_LOG_LN("Could not connect to SHT4x");
			//DEBUG_LOG_LN("Could not connect to SHT4x");
			//client.publish(topic_debug, "Could not connect to SHT4x");
			deviceStatus.publishErrorTimestamp = millis();
		}
	}

	return sensorStatus.connected;
}

bool Sht4xSensor::IsConnected()
{
	

	struct {
		sensors_event_t  temperature;
		sensors_event_t  humidity;
	}testData;

	bool currentStatus = sensorStatus.connected;

	//If sensor receives data deem it as connected.
	sensor.getEvent(&testData.humidity, &testData.temperature);

	sensorStatus.connected = testData.temperature.temperature != 0;

	DEBUG_LOG_F("%s(SHT4x) Connection Status : %d\r\n", name.c_str(), sensorStatus.connected);

	if ((currentStatus || !status.mqtt.devicesConfigured) && !sensorStatus.connected)
	{
		MarkDisconnected();

		if (deviceStatus.configured)
			DEBUG_LOG_F(MQTT_DMSG_DISCONNECTED, name.c_str());
	}

	
	return sensorStatus.connected;
}

bool Sht4xSensor::Read()
{	
	if (!deviceStatus.enabled) 
		return false;

	if (sensorStatus.connected && deviceStatus.enabled)
	{
		DEBUG_LOG_F("Reading %s(SHT4x) Measurement...", name.c_str());
		sensor.getEvent(&measurementData.humidity, &measurementData.temperature);
		sensorStatus.newData = measurementData.temperature.temperature != 0;

		if (!sensorStatus.newData)
		{
			DEBUG_LOG_LN("Failed.");
			FailedRead();
		}
		else
		{
			DEBUG_LOG("Success.\r\nTemperature : ");
			DEBUG_LOG(measurementData.temperature.temperature);
			DEBUG_LOG("°C\r\nHumidity : ");
			DEBUG_LOG(measurementData.humidity.relative_humidity);
			DEBUG_LOG_LN("%");
			DEBUG_NEWLINE();
		}

		return sensorStatus.newData;
	}
	return false;
}

#pragma endregion

#pragma region JSON UDFs

const char* sht4x_heater_strings[7] = { "none" , "high1s", "high100ms", "med1s", "med100ms", "low1s", "low100ms" };
const char* sht4x_precision_strings[3] = { "high" , "med", "low" };

bool canConvertFromJson(JsonVariantConst src, const sht4x_precision&)
{
	return JsonHelper::JsonParseEnum(src, 7, sht4x_precision_strings, nullptr) != -1;
}

void convertFromJson(JsonVariantConst src, sht4x_precision& dst)
{
	JsonHelper::UdfHelperConvertFromJsonEnums(src, (EnumClass_t&)dst, 7, "sht4x_precision", sht4x_precision_strings, nullptr);
}

bool convertToJson(const sht4x_precision& src, JsonVariant dst)
{
	JsonHelper::UdfHelperConvertToJsonEnums((EnumClass_t&)src, dst, 7, "sht4x_precision", sht4x_precision_strings, nullptr);
}

//void convertFromJson(JsonVariantConst src, sht4x_precision& dst)
//{
//	bool success;
//	sht4x_precision parseResult = (sht4x_precision)JsonHelper::JsonParseEnum(src, 7, sht4x_precision_strings, nullptr, &success);
//
//	if (success)
//		dst = parseResult;
//	else
//		DEBUG_LOG_LN("sht4x_precision Parsing Failed");
//}
//
//bool convertToJson(const sht4x_precision& src, JsonVariant dst)
//{
//#if SERIALIZE_ENUMS_TO_STRING
//	bool set = JsonHelper::EnumValueToJson(dst, (uint8_t)src, sht4x_precision_strings, 7);
//#else
//	bool set = dst.set((uint8_t)src);
//#endif
//
//	if (set) return true;
//
//	DEBUG_LOG_LN("sht4x_precision Conversion to JSON failed.");
//	return false;
//}


bool canConvertFromJson(JsonVariantConst src, const sht4x_heater&)
{
	return JsonHelper::JsonParseEnum(src, 7, sht4x_heater_strings, nullptr) != -1;
}

void convertFromJson(JsonVariantConst src, sht4x_heater& dst)
{
	JsonHelper::UdfHelperConvertFromJsonEnums(src, (EnumClass_t&)dst, 7, "sht4x_heater", sht4x_heater_strings, nullptr);
}

bool convertToJson(const sht4x_heater& src, JsonVariant dst)
{
	JsonHelper::UdfHelperConvertToJsonEnums((EnumClass_t&)src, dst, 7, "sht4x_heater", sht4x_heater_strings, nullptr);
}

//void convertFromJson(JsonVariantConst src, sht4x_heater& dst)
//{
//	bool success;
//	sht4x_heater parseResult = (sht4x_heater)JsonHelper::JsonParseEnum(src, 7, sht4x_heater_strings, nullptr, &success);
//
//	if (success)
//		dst = parseResult;
//	else
//		DEBUG_LOG_LN("sht4x_heater Parsing Failed");
//}
//
//bool convertToJson(const sht4x_heater& src, JsonVariant dst)
//{
//#if SERIALIZE_ENUMS_TO_STRING
//	bool set = JsonHelper::EnumValueToJson(dst, (uint8_t)src, sht4x_heater_strings, 7);
//#else
//	bool set = dst.set((uint8_t)src);
//#endif
//
//	if (set) return true;
//
//	DEBUG_LOG_LN("sht4x_heater Conversion to JSON failed.");
//	return false;
//}


//bool canConvertFromJson(JsonVariantConst src, const SHT4xStatus_t&)
//{
//	return src.containsKey("") || src.containsKey("");
//}
//
//void convertFromJson(JsonVariantConst src, SHT4xStatus_t& dst)
//{
//	JsonVariantConst uniqueStatusObj = src;
//
//	if (src.containsKey("uniqueStatus"))
//		uniqueStatusObj = src["uniqueStatus"];
//
//
//	if (uniqueStatusObj.containsKey("connected"))
//		dst.connected = uniqueStatusObj["connected"];
//
//}
//
//bool convertToJson(const SHT4xStatus_t& src, JsonVariant dst)
//{
//	dst["connected"] = src.connected;
//}

//bool canConvertFromJson(JsonVariantConst src, const Scd4xConfig_t&)
//{
//	return src.containsKey("internal") || src.containsKey("program");
//}

void convertFromJson(JsonVariantConst src, Sht4xConfig_t& dst)
{
	JsonVariantConst root = src;

	if (src.containsKey("uniqueStatus"))
		root = src["uniqueStatus"];


	if (root.containsKey("internal"))
	{
		JsonVariantConst obj = root["internal"];

		if (obj.containsKey("useDefaults"))
			dst.internal.useDefaults = obj["useDefaults"];

		if (obj.containsKey("heater"))
			dst.internal.heater = (sht4x_heater_t)(obj["heater"].as<sht4x_heater>());

		if (obj.containsKey("precision"))
			dst.internal.precision = (sht4x_precision_t)(obj["precision"].as<sht4x_precision>());
	}

	if (root.containsKey("mqtt"))
	{
		JsonVariantConst obj = root["mqtt"];

		if (obj.containsKey("publishHumdiity"))
			dst.mqtt.publishHumdiity = obj["publishHumdiity"];

		if (obj.containsKey("publishTemperature"))
			dst.mqtt.publishTemperature = obj["publishTemperature"];
	}

}

bool convertToJson(const Sht4xConfig_t& src, JsonVariant dst)
{
	JsonObject internalObj = dst.createNestedObject("internal");
	internalObj["useDefaults"] = src.internal.useDefaults;
	internalObj["heater"].set<sht4x_heater>((sht4x_heater)src.internal.heater);
	internalObj["precision"].set<sht4x_precision>((sht4x_precision)src.internal.precision);

	JsonObject mqttObj = dst.createNestedObject("mqtt");
	mqttObj["publishHumdiity"] = src.mqtt.publishHumdiity;
	mqttObj["publishTemperature"] = src.mqtt.publishTemperature;
}

#pragma endregion



