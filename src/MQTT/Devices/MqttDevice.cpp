#include "MqttDevice.h"

#include "../MqttManager.h"
#include "../../FileManager.h"
#include "../../HelperFunctions.h"
#include "../../Macros.h"

extern PubSubClient mqttClient;

void MqttDevice::MarkDisconnected()
{
	uint64_t mask = 1 << (index < 64 ? index : index % 64);
	uint8_t bmIndex = index < 64 ? 0 : (index / 64);

	*((&status.mqtt.devices.functioningDevices.bitmap0) + bmIndex) |= mask;

	if (deviceConfig.important)
		*((&status.mqtt.devices.functioningDevicesImportant.bitmap0) + bmIndex) |= mask;

	DEBUG_LOG_F("Mark Disconnected\r\n-Device Index : %d\r\n-Important : %d\r\n", index, deviceConfig.important);

	if (config.mqtt.ledGpio == -1 || !mqttClient.connected() || status.device.tasks.mqttBlinkTaskRunning) return;

	if (!Mqtt::AllImportantDevicesFunctional())
		Mqtt::Tasks::StartBlinkTask();
}

void MqttDevice::MarkReconnected()
{
	uint64_t mask = 1 << (index < 64 ? index : index % 64);
	uint8_t bmIndex = index < 64 ? 0 : (index / 64);

	*((&status.mqtt.devices.functioningDevices.bitmap0) + bmIndex) &= ~mask;

	if (deviceConfig.important)
		*((&status.mqtt.devices.functioningDevicesImportant.bitmap0) + bmIndex) &= ~mask;

	DEBUG_LOG_F("Mark Reconnected\r\n-Device Index : %d\r\n-Important : %d\r\n", index, deviceConfig.important);

	if (config.mqtt.ledGpio == -1 || !mqttClient.connected() || !status.device.tasks.mqttBlinkTaskRunning) return;

	if (Mqtt::AllImportantDevicesFunctional())
		Mqtt::Tasks::StopBlinkTask();
}

bool MqttDevice::Enable()
{
	if (deviceStatus.enabled) return true;

	deviceStatus.enabled = true;
	status.mqtt.devices.enabledCount++;
	Subscribe();

	return true;
}

bool MqttDevice::Disable()
{
	if (!deviceStatus.enabled) return true;

	Unsubscribe();
	deviceStatus.enabled = false;
	status.mqtt.devices.enabledCount--;
	return true;
}

bool MqttDevice::Subscribe()
{
	if (deviceStatus.subscribed || !deviceStatus.enabled)
		return true;

	if (mqttClient.subscribe(topics.jsonCommand.c_str()))
	{
		deviceStatus.subscribed = true;
		status.mqtt.devices.subscribedCount++;
		return true;
	}

	return false;

	#warning  useParents settings.
	/*if (deviceMqttSettings.json)
	{
		return mqttClient.subscribe(topics.jsonCommand.c_str());
	}
	else
	{
		return mqttClient.subscribe(deviceTopics.command.c_str());
	}*/
}
bool MqttDevice::Unsubscribe()
{
	if (!deviceStatus.subscribed)
		return true;

	if (mqttClient.unsubscribe(topics.jsonCommand.c_str()))
	{
		deviceStatus.subscribed = false;
		status.mqtt.devices.subscribedCount--;
		return true;
	}

	return false;
}

bool MqttDevice::Publish()
{
	if (!deviceStatus.enabled || !status.mqtt.connected)
		return false;

	DEBUG_LOG_F("Publishing %s Data...\r\n", name.c_str());

	String jdata = GenerateJsonPayload();

	if (jdata.isEmpty()) return false;

	const char* topic = deviceMqttSettings.useParentTopics ? (*GetParentBaseTopic()).c_str() : topics.jsonState.c_str();

	DEBUG_LOG_F("-Topic : %s\r\n", topic);

	bool success = mqttClient.publish(topic, (uint8_t*)jdata.c_str(), jdata.length(), deviceMqttSettings.retain);

	if (success)
	{
		DEBUG_LOG_LN("...Publish Successful.");
		deviceStatus.publishTimestamp = millis();
	}
	else
	{
		DEBUG_LOG_LN("...Publishing Failed!");
	}

	DEBUG_NEWLINE(); 

	return success;
}

bool MqttDevice::PublishAvailability()
{
	if (!deviceStatus.enabled) return false;

	return mqttClient.publish(topics.availability.c_str(), Mqtt::Helper::GetAvailabilityString(/*deviceStatus.enabled && */deviceStatus.enabled).c_str());
}

bool MqttDevice::PublishDisabled()
{
	const char* topic = deviceMqttSettings.useParentTopics ? (*GetParentBaseTopic()).c_str() : topics.jsonState.c_str();
	return PublishDisabled(topic);
}

bool MqttDevice::PublishDisabled(const char* topic)
{
	return mqttClient.publish(topic, MQTT_DEVICE_DISABLED);
}

bool MqttDevice::ReadGlobalConfig()
{
	const char* filepath = GetConfigFilePath();

	//Get Document Size
	size_t docSize = FileManager::GetDocumentFileSize(FileManager::GetFileSize(filepath, true));

	//Error reading config document.
	if (docSize == 0) return false;

	DynamicJsonDocument doc(docSize);

	const char* filename = GetConfigFileName();
	const char* objkey = GetConfigObjKey();

	if (FileManager::OpenAndParseFile(filepath, doc) != 0)
	{
		return false;
	}

	DEBUG_LOG_F("%s read and parsed.\r\n", filename);

	JsonVariantConst configObj = doc.as<JsonVariantConst>();


	if (doc.containsKey(objkey))
		configObj = doc[objkey];
	else
	{
		DEBUG_LOG_F("Document does not contain key \"%s\"! Will attmpt to parse, assuming document contains objects.", objkey);
	}
	ReadGlobalConfigObject(configObj);
	//ReadConfigObject(*this, configObj, true);
	return true;
}

void MqttDevice::ReadConfigObject(JsonVariantConst& obj)
{
	JsonVariantConst topicsObj;
	bool hasTopics;

	ReadConfigObject(obj, deviceConfig, deviceConfigMonitor, deviceMqttSettings, deviceMqttSettingsMonitor, &topicsObj, &hasTopics);
	//ReadTopicsObject(outTopicsObj);
	ReadConfigObjectUnique(obj);

	const char* defaultBaseTopic = GetDefaultBaseTopic();
	String parentBaseTopic = *GetParentBaseTopic();

	if(hasTopics)
		SetBaseTopic(topicsObj, defaultBaseTopic, parentBaseTopic);

	/*	if (obj.containsKey("device"))
			ReadConfigDeviceSettings(obj["device"].as<JsonObject>(), deviceConfig, deviceConfigMonitor);

		if (obj.containsKey("mqtt"))
		{
			ReadConfigMqttSettings(obj["mqtt"].as<JsonObject>(), deviceMqttSettings, deviceMqttSettingsMonitor);

			if (obj["mqtt"].containsKey("topics"))
			{
				JsonObject topicsObj = obj["mqtt"]["topics"];

				ReadTopicsObject(topicsObj);
			}
		}*/
}

void MqttDevice::SetDefaultSettings()
{
	MqttDevice::SetDefaultDeviceSettings(deviceConfig, deviceConfigMonitor);
	MqttDevice::SetDefaultMqttSettings(deviceMqttSettings, deviceMqttSettingsMonitor);
	SetDefaultUniqueSettings();
}


#pragma region Static Functions


//void MqttDevice::ReadConfigObject(MqttDevice& device, JsonObject& obj, bool global)
//{
//	if (obj.containsKey("device"))
//		ReadConfigDeviceSettings(obj["device"].as<JsonObject>(), global ? device.GetGlobalDeviceConfig() : &device.deviceConfig);
//
//	if (obj.containsKey("mqtt"))
//	{
//		ReadConfigMqttSettings(obj["mqtt"].as<JsonObject>(), global ? device.GetGlobalMqttConfig() : &device.deviceMqttSettings);
//
//		if (obj["mqtt"].containsKey("topics"))
//		{
//			JsonObject topicsObj = obj["mqtt"]["topics"];
//
//			//ReadConfigMqttTopics(device, topicsObj, global ? device.GetGlobalMqttTopics() : &device.topics, global ? device.GetGlobalMqttDeviceTopics() : &device.deviceTopics);
//
//			if (!global)
//			{
//				device.ReadTopicsObject(topicsObj);
//			}
//		}
//	}
//
//	device.ReadConfigObjectUnique(obj, global);
//}

void MqttDevice::ReadConfigObject(JsonVariantConst& obj, MqttDeviceConfig_t& deviceConfig, MqttDeviceConfigMonitor_t& deviceConfigMonitor, MqttDeviceMqttSettings_t& mqttSettings, MqttDeviceMqttSettingsMonitor_t& mqttSettingsMonitor, JsonVariantConst* out_topicsObj, bool* out_hasTopics)
{
	if (obj.containsKey("device"))
	{
		JsonVariantConst deviceObj = obj["device"];
		ReadConfigDeviceSettings(deviceObj, deviceConfig, deviceConfigMonitor);
	}

	if (obj.containsKey("mqtt"))
	{
		JsonVariantConst mqttObj = obj["mqtt"];
		ReadConfigMqttSettings(mqttObj, mqttSettings, mqttSettingsMonitor);

		if (out_hasTopics != nullptr)
		{
			*out_hasTopics = false;

			if (out_topicsObj != nullptr)
				if (mqttObj.containsKey("topics"))
				{
					*out_hasTopics = true;
					*out_topicsObj = mqttObj["topics"];
				}
		}
	}
}

void MqttDevice::SetDefaultDeviceSettings(MqttDeviceConfig_t& deviceConfig, MqttDeviceConfigMonitor_t& deviceConfigMonitor)
{
	deviceConfigMonitor.useDefaults = deviceConfig.useDefaults != false;
	deviceConfig.useDefaults = false;

	deviceConfigMonitor.useGlobalConfig = deviceConfig.useGlobalConfig != MQTTDEVICE_USE_GLOBAL_CONFIG;
	deviceConfig.useGlobalConfig = MQTTDEVICE_USE_GLOBAL_CONFIG;

	deviceConfigMonitor.initiallyEnabled = deviceConfig.initiallyEnabled != MQTTDEVICE_INITIALLY_ENABLED;
	deviceConfig.initiallyEnabled = MQTTDEVICE_INITIALLY_ENABLED;

	deviceConfigMonitor.important = deviceConfig.important != MQTTDEVICE_IMPORTANT;
	deviceConfig.important = MQTTDEVICE_IMPORTANT;	
}

void MqttDevice::SetDefaultMqttSettings(MqttDeviceMqttSettings_t& mqttSettings, MqttDeviceMqttSettingsMonitor_t& mqttSettingsMonitor)
{
	mqttSettingsMonitor.useDefaults = mqttSettings.useDefaults != false;
	mqttSettings.useDefaults = false;

	mqttSettingsMonitor.useParentTopics = mqttSettings.useParentTopics != MQTT_USE_PARENT_TOPICS;
	mqttSettingsMonitor.retain = mqttSettings.retain != MQTT_PUBLISH_RETAIN;

	mqttSettings.useParentTopics = MQTT_USE_PARENT_TOPICS;
	mqttSettings.retain = MQTT_PUBLISH_RETAIN;

	mqttSettingsMonitor.json = mqttSettings.json != mqttSettings.useParentTopics ? true : MQTT_PUBLISH_AS_JSON;
	mqttSettings.json = mqttSettings.useParentTopics ? true : MQTT_PUBLISH_AS_JSON;
}

void MqttDevice::SetDefaultMqttTopics(MqttTopics_t& topics, MqttDeviceTopics_t& deviceTopics, String& baseTopic, const char* defaultDeviceBase, String& parentBase)
{
	baseTopic = parentBase + defaultDeviceBase;

	topics.availability = baseTopic + config.mqtt.baseTopics.availability;
	topics.jsonCommand = baseTopic + config.mqtt.baseTopics.jsonCommand;
	topics.jsonState = baseTopic + config.mqtt.baseTopics.jsonState;

	deviceTopics.command = baseTopic + config.mqtt.baseTopics.command;
	deviceTopics.state = baseTopic + config.mqtt.baseTopics.state;
}

//template<typename T>
//bool ReadKey(T& setValue, JsonVariantConst& obj, const char* key)
//{
//	if (obj.containsKey(key))
//	{
//		setValue = obj[key];
//		return true;
//	}
//	return false;
//}

void MqttDevice::ReadConfigDeviceSettings(JsonVariantConst& deviceObj, MqttDeviceConfig_t& deviceConfig, MqttDeviceConfigMonitor_t& deviceConfigMonitor)
{
	if (deviceObj.containsKey("useDefaults"))
		deviceConfig.useDefaults = deviceObj["useDefaults"];

	//if (deviceConfig.useDefaults)
	//	return;

	if (deviceObj.containsKey("initEnable"))
		deviceConfig.initiallyEnabled = deviceObj["initEnable"];

	if (deviceObj.containsKey("useGlobalConfig"))
		deviceConfig.useGlobalConfig = deviceObj["useGlobalConfig"];

	if (deviceObj.containsKey("important"))
		deviceConfig.important = deviceObj["important"];
}

void MqttDevice::ReadConfigMqttSettings(JsonVariantConst& mqttObj, MqttDeviceMqttSettings_t& mqttSettings, MqttDeviceMqttSettingsMonitor_t& mqttSettingsMonitor)
{
	if (mqttObj.containsKey("useDefaults"))
		mqttSettings.useDefaults = mqttObj["useDefaults"];


	if (mqttObj.containsKey("useParentTopics"))
		mqttSettings.useParentTopics = mqttObj["useParentTopics"];

	if (mqttSettings.useParentTopics)
	{
		mqttSettings.json = true;
	}
	else
	{
	#warning Only JSON publishing is supported at the moment.
		if (mqttObj.containsKey("json"))
			mqttSettings.json = mqttObj["json"];

		mqttSettings.json = true;	//Temp, only JSON supported
	}

	if (mqttObj.containsKey("retain"))
		mqttSettings.retain = mqttObj["retain"];
}

//void MqttDevice::ReadConfigMqttTopics(JsonObject& topicsObj, MqttTopics_t& topics, MqttDeviceTopics_t& deviceTopics, String& baseTopic, const char* defaultDeviceBase, String& parentBase)
//{
//	if (topicsObj.containsKey("base"))
//	{
//		Mqtt::CreateBaseTopicString(topicsObj["base"].as<JsonVariant>(), baseTopic, defaultDeviceBase, parentBase);
//	}
//
//	if (topicsObj.containsKey("availability"))
//	{
//		Mqtt::CreateTopicString(&topics.availability, topicsObj["availability"], baseTopic, nullptr, TOPIC_BASE_AVAILABILITY);
//	}
//
//	if (topicsObj.containsKey("jCommand"))
//	{
//		Mqtt::CreateTopicString(&topics.jsonCommand, topicsObj["jCommand"], baseTopic, nullptr, TOPIC_BASE_JSON_COMMAND);
//	}
//
//	if (topicsObj.containsKey("jState"))
//	{
//		Mqtt::CreateTopicString(&topics.jsonState, topicsObj["jState"], baseTopic, nullptr, TOPIC_BASE_JSON_STATE);
//	}
//
//	if (topicsObj.containsKey("command"))
//	{
//		Mqtt::CreateTopicString(&deviceTopics.command, topicsObj["command"], baseTopic, nullptr, TOPIC_BASE_COMMAND);
//	}
//
//	if (topicsObj.containsKey("state"))
//	{
//		Mqtt::CreateTopicString(&deviceTopics.state, topicsObj["state"], baseTopic, nullptr, TOPIC_BASE_STATE);
//	}
//}

//bool MqttDevice::ReadConfigMqttTopics(MqttDevice* const device, JsonObject& topicsObj, MqttTopics_t* topics, MqttDeviceTopics_t* deviceTopics)
//{
//	return false;
//}

#pragma endregion

