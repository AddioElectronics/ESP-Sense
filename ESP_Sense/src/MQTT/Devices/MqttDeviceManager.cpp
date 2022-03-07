#include "MqttDeviceManager.h"


#include "ArduinoJson.hpp"
#include "ArduinoJson.h"

#include <StreamUtils.hpp>
#include <StreamUtils.h>	

#if defined(ESP8266)
#include <LittleFS.h>
#elif defined(ESP32)
#include <FS.h>
#include <LITTLEFS.h>
#endif

#include "../MqttManager.h"
#include "../MqttHelper.h"
#include "../../FileManager.h"
#include "../../HelperFunctions.h"
#include "../../Macros.h"
#include "../../EspSenseValidation.h"
#include "../../Network/Server/FtpManager.h"

MqttSensor** mqttSensors;
MqttBinarySensor** mqttBinarySensors;

extern Mqtt::MqttMessagesSent_t mqttMessagesSent;

DynamicJsonDocument* doc;

TaskHandle_t taskDeviceManagerLoop;

typedef bool (*ISVALIDDEVICEKEY)(const char* deviceKey);
typedef MqttDevice* (*CONSTRUCT_MQTT_DEVICE)(int deviceIndex, int subTypeIndex, const char* name, const char* device);

JsonVariantConst GetBaseTopicObject(JsonDocument& doc);
bool GetValidDeviceCount(JsonArrayConst& deviceArray, device_count_t& validCount, ISVALIDDEVICEKEY isValidFunction, bool** out_validDevices);

device_count_t CreateDevicesOfSubType(const char* deviceTypeName, JsonArrayConst& deviceArray, bool*& validDevices, CONSTRUCT_MQTT_DEVICE constructDeviceFunc, device_count_t& subTypeCount);

//template <class MqttGenericDevice>

MqttDevice* CreateMqttDeviceSensor(int deviceIndex, int sensorIndex, const char* name, const char* device)
{
	if (strcmp(device, "scd4x") == 0)
	{
		return mqttSensors[sensorIndex] = new Scd4xSensor(name, deviceIndex);
	}
	else if (strcmp(device, "sht4x") == 0)
	{
		return mqttSensors[sensorIndex] = new Sht4xSensor(name, deviceIndex);
	}
	else
	{
		return nullptr;
	}
}

typedef struct {
	uint32_t* deviceIndex;
	device_count_t* subTypeIndex; 
	const char** name; 
	const char** device;
}ConstructMqttDeviceParameters_t;

#pragma region MQTT Device Configuration

int Mqtt::DeviceManager::ConfigureDevices()
{

	DEBUG_LOG_LN("Configuring MQTT devices..");
	if (status.mqtt.devicesConfigured)
	{
		DEBUG_LOG_LN("...Already Configured. Must unconfigure first.");
		return -1;
	}


	File file;
	size_t filesize = GetMaxFileSize();
	size_t docsize = FileManager::GetDocumentFileSize(filesize);
	doc = new DynamicJsonDocument(docsize);

	DEBUG_LOG_F("Calculated required document size.\r\nMax File Size : %d\r\nDocument Size : %d\r\n", filesize, docsize);

	int errorCount = 0;

	errorCount += ConfigureSensors();
	errorCount += ConfigureBinarySensors();
	errorCount += ConfigureLights();
	errorCount += ConfigureButtons();
	errorCount += ConfigureSwitches();

	//status.mqtt.devices.deviceCount =  status.mqtt.devices.binarySensorCount + status.mqtt.devices.buttonCount + status.mqtt.devices.lightCount + status.mqtt.devices.sensorCount + status.mqtt.devices.switchCount;

	DEBUG_LOG_F("MQTT Devices configured. %d devices were unable to be configured.\r\n", errorCount);

	free(doc);

	status.mqtt.devicesConfigured = true;

	return errorCount;
}

int Mqtt::DeviceManager::UnconfigureDevices()
{
	DEBUG_LOG_LN("!!! Not Implmented !!!");
	//status.mqtt.devicesConfigured = false;
	return 0;
}

device_count_t Mqtt::DeviceManager::ConfigureSensors()
{
	DEBUG_LOG_LN("Configuring Sensors...");

	device_count_t deviceIndex = 0;

	doc->clear();

	if (FileManager::OpenAndParseFile(FILE_MQTT_SENSORS_PATH, *doc) != 0)
	{
		return false;
	}

	DEBUG_LOG_LN(FILE_MQTT_SENSORS_NAME " read and parsed.");

	EspSense::YieldWatchdog(50);
	JsonArrayConst deviceArray;

	//Find out how many sensors are in the config file.
	if (doc->containsKey("sensors"))
	{
		deviceArray = (*doc)["sensors"].as<JsonArray>();

		if (deviceArray.size() == 0)
		{
			goto NoSensorsAvailableMessage;
		}
	}
	else
	{
	NoSensorsAvailableMessage:
		DEBUG_LOG_LN("No Sensors available.");
		return false;
	}

	//Set global base device topic.
	JsonVariantConst baseVar = GetBaseTopicObject(*doc);
	Mqtt::Helper::CreateBaseTopicString(baseVar, MqttSensor::mqttSensorBaseTopic, TOPIC_BASE_MQTT_SENSOR, config.mqtt.baseTopics.base);
	//Mqtt::CreateBaseTopic(topicsObj["jCommand"].as<JsonVariant>(), MqttSensor::mqttSensorBaseTopic, TOPIC_BASE_MQTT_SENSOR, config.mqtt.baseTopics.base);
	//Mqtt::CreateBaseTopic(topicsObj["jState"].as<JsonVariant>(), MqttSensor::mqttSensorBaseTopic, TOPIC_BASE_MQTT_SENSOR, config.mqtt.baseTopics.base);
		//MqttSensor::mqttSensorBaseTopic = doc["baseTopics"].as<String>();

	EspSense::YieldWatchdog(50);
	device_count_t validCount;
	bool* validDevices;
	if (!GetValidDeviceCount(deviceArray, validCount, IsValidSensor, &validDevices))
	{
		DEBUG_LOG_LN("No Valid Sensors.");
		return false;
	}

	//Allocate and create
	EspSense::YieldWatchdog(50);
	mqttSensors = (MqttSensor**)malloc(sizeof(MqttSensor*) * validCount);
	device_count_t initializedCount = CreateDevicesOfSubType("Sensor", deviceArray, validDevices, (CONSTRUCT_MQTT_DEVICE)CreateMqttSensor, status.mqtt.devices.sensorCount);
	//deviceIndex = 0; //Reset
	//device_count_t initializedCount = 0;

	//for (device_count_t i = 0; i < deviceArray.size(); i++)
	//{
	//	EspSense::YieldWatchdog(50);

	//	//Sensor was invalid. Skip configuring
	//	if (!validDevices[i]) continue;

	//	JsonVariant v = deviceArray[i];

	//	//Reset name variable to Null terminator.
	//	//memset(name, 0, NAME_MAX_LENGTH);
	//	String name;
	//	String deviceString = v["device"].as<String>();

	//	//Get/Create name
	//	if (v.containsKey("name"))
	//	{
	//		name = v["name"].as<String>();
	//		//strlcpy(name, v["name"], NAME_MAX_LENGTH); 

	//		if (name.isEmpty()) 
	// goto SetBasicName;
	//	}
	//	else
	//	{
	//	SetBasicName:
	//		//Name does not exist. Give it a generic name.
	//		//Use "i" so it reflects the config file, instead of the actual index.
	//		//sprintf(name, "Sensor", i);
	//		name = /*"Sensor" + */deviceString + "_" + status.mqtt.devices.sensorCount;
	//	}

	//	DEBUG_LOG_F("Sensor %s Initializing\r\n", name.c_str());
	//	EspSense::YieldWatchdog(50);
	//	//Create Sensor object.
	//	MqttSensor* device = CreateMqttSensor(status.mqtt.devices.deviceCount, status.mqtt.devices.sensorCount, name.c_str(), deviceString.c_str());

	//	if (device == nullptr)
	//	{
	//		DEBUG_LOG_F("Cannot configure sensor %s. Sensor of type %s is not supported.\r\n", name.c_str(), deviceString.c_str());
	//		validDevices[i] = false;
	//		continue;
	//	}

	//	MqttDeviceGlobalStatus_t* deviceGlobalStatus = device->GetGlobalDeviceStatus();
	//	EspSense::YieldWatchdog(50);
	//	//Read global config
	//	if (!deviceGlobalStatus->configRead)
	//	{
	//		//#warning  may need to free the document to parse the global settings, and then reopen to finish.
	//		device->SetDefaultGlobalSettings();
	//		//device->SetDefaultGlobalMqttTopics();
	//		device->ReadGlobalConfig(); 
	// deviceGlobalStatus->configRead = true;
	//	}

	//	//#warning  would need to reopen here.

	//	//Read device specific settings
	//	device->SetDefaultSettings();
	//	device->SetDefaultMqttTopics();
	//	EspSense::YieldWatchdog(50);
	//	if (v.containsKey("config"))
	//	{
	//		JsonObject configObj = v["config"];

	//		device->ReadConfigObject(configObj);
	//		//MqttDevice::ReadConfigObject(device, configObj, false);
	//	}

	//	//Temporary until custom topics are finished.
	//	device->GenerateTopics();

	//	//deviceGlobalStatus->configRead = true;

	//	EspSense::YieldWatchdog(50);

	//	status.mqtt.devices.deviceCount++;
	//	status.mqtt.devices.sensorCount++;

	//	if (device->Init(false))
	//	{
	//		initializedCount++;
	//	}
	//	else
	//	{
	//		DEBUG_LOG_F("Sensor %s failed to connect or configure! It may succeed later.", device->name.c_str());
	//	}
	//}

	free(validDevices);

	//status.mqtt.devices.sensorCount = validCount;

	DEBUG_LOG_F("Finished Configuring Sensors. %d/%d created, and %d successfully connected and configured.", status.mqtt.devices.sensorCount, deviceArray.size(), initializedCount);
	EspSense::YieldWatchdog(50);
	DEBUG_NEWLINE();
	return deviceArray.size() - validCount;
}

device_count_t Mqtt::DeviceManager::ConfigureBinarySensors()
{
	DEBUG_LOG_LN("Configuring Binary Sensors...");

	device_count_t deviceIndex = 0;

	doc->clear();

	if (FileManager::OpenAndParseFile(FILE_MQTT_BINARYSENSORS_PATH, *doc) != 0)
	{
		return false;
	}

	DEBUG_LOG_LN(FILE_MQTT_BINARYSENSORS_NAME " read and parsed.");

	EspSense::YieldWatchdog(50);
	JsonArrayConst deviceArray;

	//Find out how many sensors are in the config file.
	if (doc->containsKey("binarySensors"))
	{
		deviceArray = (*doc)["binarySensors"].as<JsonArray>();

		if (deviceArray.size() == 0)
		{
			goto NoSensorsAvailableMessage;
		}
	}
	else
	{
	NoSensorsAvailableMessage:
		DEBUG_LOG_LN("No Binary Sensors available.");
		return false;
	}

	//Set global base device topic.
	JsonVariantConst baseVar = GetBaseTopicObject(*doc);
	Mqtt::Helper::CreateBaseTopicString(baseVar, MqttBinarySensor::mqttBinarySensorBaseTopic, TOPIC_BASE_MQTT_BINARYSENSOR, config.mqtt.baseTopics.base);


	EspSense::YieldWatchdog(50);
	device_count_t validCount;
	bool* validDevices;
	if (!GetValidDeviceCount(deviceArray, validCount, IsValidBinarySensor, &validDevices))
	{
		DEBUG_LOG_LN("No Valid Binary Sensors.");
		return false;
	}

	//Allocate and create
	EspSense::YieldWatchdog(50);
	mqttBinarySensors = (MqttBinarySensor**)malloc(sizeof(MqttBinarySensor*) * validCount);
	device_count_t initializedCount = CreateDevicesOfSubType("Sensor", deviceArray, validDevices, (CONSTRUCT_MQTT_DEVICE)CreateMqttBinarySensor, status.mqtt.devices.binarySensorCount);
	////deviceIndex = 0; //Reset
	//device_count_t initializedCount = 0;

	//for (device_count_t i = 0; i < deviceArray.size(); i++)
	//{
	//	EspSense::YieldWatchdog(50);

	//	//Sensor was invalid. Skip configuring
	//	if (!validDevices[i]) continue;

	//	JsonVariant v = deviceArray[i];

	//	//Reset name variable to Null terminator.
	//	//memset(name, 0, NAME_MAX_LENGTH);
	//	String name;
	//	String deviceString = v["device"].as<String>();

	//	//Get/Create name
	//	if (v.containsKey("name"))
	//	{
	//		name = v["name"].as<String>();
	//		//strlcpy(name, v["name"], NAME_MAX_LENGTH); 

	//		if (name.isEmpty()) 
	// goto SetBasicName;
	//	}
	//	else
	//	{
	//	SetBasicName:
	//		//Name does not exist. Give it a generic name.
	//		//Use "i" so it reflects the config file, instead of the actual index.
	//		//sprintf(name, "Sensor", i);
	//		name = /*"Sensor" + */deviceString + "_" + status.mqtt.devices.sensorCount;
	//	}

	//	DEBUG_LOG_F("Binary Sensor %s Initializing\r\n", name.c_str());
	//	EspSense::YieldWatchdog(50);
	//	//Create Sensor object.
	//	MqttSensor* device = CreateMqttSensor(status.mqtt.devices.deviceCount, status.mqtt.devices.sensorCount, name.c_str(), deviceString.c_str());

	//	if (device == nullptr)
	//	{
	//		DEBUG_LOG_F("Cannot configure binary sensor %s. Binary sensor of type %s is not supported.\r\n", name.c_str(), deviceString.c_str());
	//		validDevices[i] = false;
	//		continue;
	//	}

	//	MqttDeviceGlobalStatus_t* deviceGlobalStatus = device->GetGlobalDeviceStatus();
	//	EspSense::YieldWatchdog(50);
	//	//Read global config
	//	if (!deviceGlobalStatus->configRead)
	//	{
	//		//#warning  may need to free the document to parse the global settings, and then reopen to finish.
	//		device->SetDefaultGlobalSettings();
	//		//device->SetDefaultGlobalMqttTopics();
	//		device->ReadGlobalConfig(); 
	// deviceGlobalStatus->configRead = true;
	//	}

	//	//#warning  would need to reopen here.

	//	//Read device specific settings
	//	device->SetDefaultSettings();
	//	device->SetDefaultMqttTopics();
	//	EspSense::YieldWatchdog(50);
	//	if (v.containsKey("config"))
	//	{
	//		JsonObject configObj = v["config"];

	//		device->ReadConfigObject(configObj);
	//		//MqttDevice::ReadConfigObject(device, configObj, false);
	//	}

	//	//Temporary until custom topics are finished.
	//	device->GenerateTopics();

	//	//deviceGlobalStatus->configRead = true;

	//	EspSense::YieldWatchdog(50);

	//	status.mqtt.devices.deviceCount++;
	//	status.mqtt.devices.sensorCount++;

	//	if (device->Init(false))
	//	{
	//		initializedCount++;
	//	}
	//	else
	//	{
	//		DEBUG_LOG_F("Sensor %s failed to connect or configure! It may succeed later.", device->name.c_str());
	//	}
	//}

	free(validDevices);

	//status.mqtt.devices.sensorCount = validCount;

	DEBUG_LOG_F("Finished Configuring Binary Sensors. %d/%d configured, and %d successfully connected and configured.", status.mqtt.devices.binarySensorCount, deviceArray.size(), initializedCount);
	EspSense::YieldWatchdog(50);
	DEBUG_NEWLINE();
	return deviceArray.size() - validCount;
}

device_count_t Mqtt::DeviceManager::ConfigureLights()
{
	return false;
}

device_count_t Mqtt::DeviceManager::ConfigureButtons()
{
	return false;
}

device_count_t Mqtt::DeviceManager::ConfigureSwitches()
{
	return false;
}

#pragma region Shared functions

JsonVariantConst GetBaseTopicObject(JsonDocument& doc)
{
	JsonVariantConst baseVar; /*= NULL;*/

	if (doc.containsKey("mqtt"))
		if (doc["mqtt"].containsKey("topics"))
		{
			JsonObject topicsObj = doc["topics"];

			JsonVariantConst baseVar = topicsObj["base"].as<JsonVariantConst>();
		}
	#warning need to see how this is passed(should I malloc and return pointer ? ), and if need to set to null
		return baseVar;
}

bool GetValidDeviceCount(JsonArrayConst& deviceArray, device_count_t& validCount, ISVALIDDEVICEKEY isValidFunction, bool** out_validDevices)
{
	validCount = 0;
	bool* validDevices = (bool*)malloc(deviceArray.size());

	//Count valid sensors. We do not want to allocate memory for invalid sensors.
	for (device_count_t i = 0; i < deviceArray.size(); i++)
	{
		JsonVariantConst v = deviceArray[i];

		if (!v.containsKey("device") || !isValidFunction((const char*)v["device"]))
		{
			DEBUG_LOG_F("Device %d cannot be created, as the \"device\" key is missing or invalid.\r\n", i);
			continue;
		}
		validDevices[i] = true;
		validCount++;
	}

	if (validCount == 0)
		free(validDevices);
	else
		*out_validDevices = validDevices;

	return validCount;
}

//template <class MqttGenericDevice>
device_count_t CreateDevicesOfSubType(const char* deviceTypeName, JsonArrayConst& deviceArray, bool*& validDevices, CONSTRUCT_MQTT_DEVICE constructDeviceFunc, device_count_t& subTypeCount)
{
	device_count_t initializedCount = 0;

	for (device_count_t i = 0; i < deviceArray.size(); i++)
	{
		EspSense::YieldWatchdog(50);

		//Sensor was invalid. Skip configuring
		if (!validDevices[i]) continue;

		JsonVariantConst deviceObj = deviceArray[i];

		//Reset name variable to Null terminator.
		//memset(name, 0, NAME_MAX_LENGTH);
		String name;
		String deviceString = deviceObj["device"].as<String>();

		//Get/Create name
		if (deviceObj.containsKey("name"))
		{
			name = deviceObj["name"].as<String>();
			//strlcpy(name, deviceObj["name"], NAME_MAX_LENGTH); 

			if (name.isEmpty()) 
				goto SetBasicName;
		}
		else
		{
		SetBasicName:
			//Name does not exist. Give it a generic name.
			//Use "i" so it reflects the config file, instead of the actual index.
			//sprintf(name, "Sensor", i);
			name = /*"Sensor" + */deviceString + "_" + status.mqtt.devices.sensorCount;
		}

		DEBUG_LOG_F("%s %s Initializing\r\n", deviceTypeName, name.c_str());
		EspSense::YieldWatchdog(50);

		//Create Device Subclass object.
		MqttDevice* device = constructDeviceFunc(status.mqtt.devices.deviceCount++, subTypeCount++, name.c_str(), deviceString.c_str());
		//device = /*(MqttGenericDevice*)*/(constructDeviceFunc(status.mqtt.devices.deviceCount++, subTypeCount++, name.c_str(), deviceString.c_str()));

		//device key was already checked, if a nullptr is returned something is wrong.
		//VALIDATE_ASSERT(device);

		MqttDeviceGlobalStatus_t* deviceGlobalStatus = device->GetGlobalDeviceStatus();

		EspSense::YieldWatchdog(50);
		//Read global config
		if (!deviceGlobalStatus->configRead)
		{
			#warning  may need to free the main document to parse the global device settings, and then reopen to finish. (If not enough ram)
			device->SetDefaultGlobalSettings();
			//device->SetDefaultGlobalMqttTopics();
			device->ReadGlobalConfig(); 
			deviceGlobalStatus->configRead = true;
		}

		#warning  would need to reopen here.

			//Read device specific settings
		device->SetDefaultSettings();
		device->SetDefaultMqttTopics();
		EspSense::YieldWatchdog(50);
		if (deviceObj.containsKey("config"))
		{
			JsonVariantConst configObj = deviceObj["config"];

			device->ReadConfigObject(configObj);
			//MqttDevice::ReadConfigObject(device, configObj, false);
		}

		//Temporary until custom topics are finished.
		device->GenerateTopics();

		//deviceGlobalStatus->configRead = true;

		EspSense::YieldWatchdog(50);

		//status.mqtt.devices.deviceCount++;
		//subTypeCount++;

		if (device->Init(false))
		{
			initializedCount++;
		}
		else
		{
			DEBUG_LOG_F("%s %s failed to connect or configure! It may succeed later.", deviceTypeName, device->name.c_str());
		}
	}

	return initializedCount;
}

#pragma endregion
#pragma endregion

void Mqtt::DeviceManager::EnableAll(bool initial)
{
	if (!status.mqtt.devicesConfigured) return;

	for (device_count_t i = 0; i < status.mqtt.devices.sensorCount; i++)
	{
		if (mqttSensors[i]->deviceConfig.initiallyEnabled || !initial)
			mqttSensors[i]->Enable();

		EspSense::YieldWatchdog(5);
	}

	for (device_count_t i = 0; i < status.mqtt.devices.binarySensorCount; i++)
	{
		if (mqttBinarySensors[i]->deviceConfig.initiallyEnabled || !initial)
			mqttBinarySensors[i]->Enable();

		EspSense::YieldWatchdog(5);
	}
	DEBUG_NEWLINE();
}

void Mqtt::DeviceManager::DisableAll()
{
	if (!status.mqtt.devicesConfigured) return;

	for (device_count_t i = 0; i < status.mqtt.devices.sensorCount; i++)
	{
		mqttSensors[i]->Disable();
		EspSense::YieldWatchdog(1);
	}

	for (device_count_t i = 0; i < status.mqtt.devices.binarySensorCount; i++)
	{
		mqttBinarySensors[i]->Disable();
		EspSense::YieldWatchdog(1);
	}
	DEBUG_NEWLINE();
}

int Mqtt::DeviceManager::SubscribeAll()
{
	if (!status.mqtt.devicesConfigured || !status.mqtt.connected) return 0;

	int failed = 0;

	for (device_count_t i = 0; i < status.mqtt.devices.sensorCount; i++)
	{
		if (!mqttSensors[i]->Subscribe()) failed++;

		EspSense::YieldWatchdog(5);
	}

	for (device_count_t i = 0; i < status.mqtt.devices.binarySensorCount; i++)
	{
		if (!mqttBinarySensors[i]->Subscribe()) failed++;
		EspSense::YieldWatchdog(5);
	}

	DEBUG_NEWLINE();
	return failed;
}

int Mqtt::DeviceManager::UnsubscribeAll()
{
	if (!status.mqtt.devicesConfigured || !status.mqtt.connected) return 0;

	int failed = 0;

	for (device_count_t i = 0; i < status.mqtt.devices.sensorCount; i++)
	{
		if (!mqttSensors[i]->Unsubscribe()) failed++;

		EspSense::YieldWatchdog(1);
	}

	for (device_count_t i = 0; i < status.mqtt.devices.binarySensorCount; i++)
	{
		if (!mqttBinarySensors[i]->Unsubscribe()) failed++;
		EspSense::YieldWatchdog(1);
	}

	DEBUG_NEWLINE();
	return failed;
}

void Mqtt::DeviceManager::PublishAvailability()
{
	if (!status.mqtt.devicesConfigured || !status.mqtt.connected) return;

	for (device_count_t i = 0; i < status.mqtt.devices.sensorCount; i++)
	{
		mqttSensors[i]->PublishAvailability();
		EspSense::YieldWatchdog(10);
#warning add configurable option for delay 
	}

	for (device_count_t i = 0; i < status.mqtt.devices.binarySensorCount; i++)
	{
		mqttBinarySensors[i]->PublishAvailability();
		EspSense::YieldWatchdog(10);
	}

	DEBUG_NEWLINE();
	/*EspSense::YieldWatchdog(1);*/
}
#warning move this function to MqttSensors as static, create Sensor loop which handles calls this.
void Mqtt::DeviceManager::ReadAll()
{
	if (!status.mqtt.devicesConfigured) return;

	for (device_count_t i = 0; i < status.mqtt.devices.sensorCount; i++)
	{
		mqttSensors[i]->Read();
		EspSense::YieldWatchdog(10);
	}

	for (device_count_t i = 0; i < status.mqtt.devices.binarySensorCount; i++)
	{
		mqttBinarySensors[i]->Read();
		EspSense::YieldWatchdog(10);
	}

	DEBUG_NEWLINE();
}

void Mqtt::DeviceManager::PublishAll()
{
	if (!status.mqtt.devicesConfigured || !status.mqtt.connected) return;

	for (device_count_t i = 0; i < status.mqtt.devices.sensorCount; i++)
	{
		mqttSensors[i]->Publish();
		EspSense::YieldWatchdog(10);
	}

	for (device_count_t i = 0; i < status.mqtt.devices.binarySensorCount; i++)
	{
		mqttBinarySensors[i]->Publish();
		EspSense::YieldWatchdog(10);
	}
	DEBUG_NEWLINE();
}

void Mqtt::DeviceManager::Loop()
{
	if (!status.mqtt.devicesConfigured) return;

	//if (!status.device.tasks.mqttDeviceManagerTaskRunning)
	//{
	//	if (millis() < status.mqtt.nextPublish) return;
	//	status.mqtt.nextPublish = millis() + config.mqtt.publish.rate;
	//}

	for (device_count_t i = 0; i < status.mqtt.devices.sensorCount; i++)
	{
		mqttSensors[i]->Loop();
		EspSense::YieldWatchdog(10);
	}

	for (device_count_t i = 0; i < status.mqtt.devices.binarySensorCount; i++)
	{
		mqttBinarySensors[i]->Loop();
		EspSense::YieldWatchdog(10);
	}

	if (!status.mqtt.publishingDisabled)
	{
		PublishAll();
		ReadAll();
		DEBUG_NEWLINE();
	}

	mqttMessagesSent.devices = true;

}



void Mqtt::DeviceManager::Callback(char* topic, byte* payload, unsigned int length)
{
	if (!status.mqtt.devicesConfigured) return;

#warning Need to only send to sensors who it is targeted for. Or possibly deserializeJson data here and send them doc pointer.
#warning add custom subscribing class which has a key and value, key being topic, and value being a list of functions.

	for (device_count_t i = 0; i < status.mqtt.devices.sensorCount; i++)
	{
		mqttSensors[i]->ReceiveCommand(topic, payload, length);
		EspSense::YieldWatchdog(5);
	}

	for (device_count_t i = 0; i < status.mqtt.devices.binarySensorCount; i++)
	{
		mqttBinarySensors[i]->ReceiveCommand(topic, payload, length);
		EspSense::YieldWatchdog(5);
	}
}



size_t Mqtt::DeviceManager::GetMaxFileSize()
{
	File file;

	size_t max = 0;

	//if (GetFileSize(FILE_MQTT_DEVICES_PATH) > max)
	//	max = file.size();

	if (FileManager::GetFileSize(&file, FILE_MQTT_SENSORS_PATH, true) > max)
		max = file.size();

	if (FileManager::GetFileSize(&file, FILE_MQTT_BINARYSENSORS_PATH, true) > max)
		max = file.size();

	if (FileManager::GetFileSize(&file, FILE_MQTT_LIGHTS_PATH, true) > max)
		max = file.size();

	if (FileManager::GetFileSize(&file, FILE_MQTT_BUTTONS_PATH, true) > max)
		max = file.size();

	if (FileManager::GetFileSize(&file, FILE_MQTT_SWITCHES_PATH, true) > max)
		max = file.size();

	file.close();

	return max;
}


bool Mqtt::DeviceManager::IsValidSensor(const char* device)
{
	if (strcmp(device, "scd4x"))
	{
		return true;
	}
	else if (strcmp(device, "sht4x"))
	{
		return true;
	}
	else
	{
		return false;
	}
}

MqttSensor* Mqtt::DeviceManager::CreateMqttSensor(int deviceIndex, int sensorIndex, const char* name, const char* device)
{
	if (strcmp(device, "scd4x") == 0)
	{
		return mqttSensors[sensorIndex] = new Scd4xSensor(name, deviceIndex);
	}
	else if (strcmp(device, "sht4x") == 0)
	{
		return mqttSensors[sensorIndex] = new Sht4xSensor(name, deviceIndex);
	}
	else
	{
		return nullptr;
	}
}


bool Mqtt::DeviceManager::IsValidBinarySensor(const char* device)
{
	if (strcmp(device, "llc200d3sh") == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

MqttBinarySensor* Mqtt::DeviceManager::CreateMqttBinarySensor(int deviceIndex, int sensorIndex, const char* name, const char* device)
{
	if (strcmp(device, "llc200d3sh") == 0)
	{
		return mqttBinarySensors[sensorIndex] = new Llc200d3sh_Sensor(name, deviceIndex);
	}
	else
	{
		return nullptr;
	}
}


#pragma region Tasks



bool Mqtt::Tasks::StartDeviceManagerLoopTask()
{
	if (!status.mqtt.devicesConfigured) return false;
	if (status.device.tasks.mqttDeviceManagerTaskRunning) return true;
	status.device.tasks.mqttDeviceManagerTaskRunning = true;

	xTaskCreatePinnedToCore(
		DeviceManagerLoopTask,						// Task function
		"MqttDeviceManager",						// Name
		8192,										// Stack size
		NULL,										// Parameters
		2,											// Priority
		&taskDeviceManagerLoop,						// Task handle
		1);											// Core

	return true;
}

void Mqtt::Tasks::StopDeviceManagerLoopTask()
{
	if (!status.device.tasks.mqttDeviceManagerTaskRunning) return;

	vTaskDelete(taskDeviceManagerLoop);
	status.device.tasks.mqttDeviceManagerTaskRunning = false;
}

void Mqtt::Tasks::DeviceManagerLoopTask(void* pvParameters)
{
	status.device.tasks.mqttDeviceManagerTaskRunning = true;

	unsigned long nextAliveMessage;

	////Allow devices to reconnect and configure if needed.
	//Mqtt::DeviceManager::Loop();
	//EspSense::YieldWatchdog(10);

	//if (!status.mqtt.publishingDisabled)
	//{
	//	//Get devices to read and have their data ready.
	//	Mqtt::DeviceManager::ReadAll();
	//	EspSense::YieldWatchdog(config.mqtt.publish.rate);
	//}

	//if (!status.mqtt.publishingDisabled)
//{
//	PublishAll();
//	EspSense::YieldWatchdog(10);
//	ReadAll();
//}

//mqttMessagesSent.devices = true;
//DEBUG_NEWLINE();

	/*bool reading = false;*/

	for (;;)
	{
		if (!status.mqtt.connected)
			break;

		////Publish and read right away
		//if (!status.mqtt.publishingDisabled)
		//{
		//	//Mqtt::DeviceManager::PublishAll();
		//	//EspSense::YieldWatchdog(10);
		//	//Mqtt::DeviceManager::ReadAll();

		//	//Perform operations on opposite loops.
		//	if (reading)
		//	{
		//		Mqtt::DeviceManager::ReadAll();
		//	}
		//	else
		//	{

		//		Mqtt::DeviceManager::PublishAll();
		//	}

		//	EspSense::YieldWatchdog(config.mqtt.publish.rate / 2);
		//	reading = !reading;
		//}

		DeviceManager::Loop();

		EspSense::YieldWatchdog(config.mqtt.publish.rate);

#if COMPILE_ALIVE_MSG
		if (millis() > nextAliveMessage)
		{
			nextAliveMessage = millis() + 10000;
			DEBUG_LOG_LN("DeviceManagerLoopTask Alive");
			EspSense::DisplayMemoryInfo();
			DEBUG_NEWLINE();
		}
#endif
	}

	status.device.tasks.mqttDeviceManagerTaskRunning = false;

	vTaskDelete(NULL);
}

#pragma endregion