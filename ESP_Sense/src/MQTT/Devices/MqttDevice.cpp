#include "MqttDevice.h"

#include "../MqttManager.h"
#include "../../FileManager.h"
#include "../../HelperFunctions.h"
#include "../../Macros.h"
#include "../../Network/Website/WebStrings.h"

extern PubSubClient mqttClient;

bool MqttDevice::Init(bool enable)
{
	if (!status.mqtt.devicesConfigured)
		ResetStatus();

	if (deviceStatus.configured) return true;

	//memset(&deviceStatus, 0, sizeof(MqttDeviceStatus_t));
	//deviceStatus.enabled = true;
	Configure();

	if (deviceStatus.configured)
		Enable();
	else
		MarkDisconnected();

	InitWebpage();
	SetDeviceState();

	return deviceStatus.configured && (deviceStatus.enabled || !enable);
}

void MqttDevice::ResetStatus()
{
	memset(&deviceStatus, 0, sizeof(MqttDeviceStatus_t));
}

void MqttDevice::MarkDisconnected()
{
	if (deviceStatus.markedDisconnected) return;

	uint64_t mask = 1 << (index < 64 ? index : index % 64);
	uint8_t bmIndex = index < 64 ? 0 : (index / 64);

	*((&status.mqtt.devices.functioningDevices.bitmap0) + bmIndex) |= mask;

	if (deviceConfig.important)
		*((&status.mqtt.devices.functioningDevicesImportant.bitmap0) + bmIndex) |= mask;

	DEBUG_LOG_F("Mark Disconnected\r\n-Device Index : %d\r\n-Important : %d\r\n", index, deviceConfig.important);

	deviceStatus.markedDisconnected = true;

	if (config.mqtt.ledGpio == -1 || !mqttClient.connected() || status.device.tasks.mqttBlinkTaskRunning) return;

	if (!Mqtt::AllImportantDevicesFunctional())
		Mqtt::Tasks::StartBlinkTask();
}

void MqttDevice::MarkReconnected()
{
	if (!deviceStatus.markedDisconnected) return;

	uint64_t mask = 1 << (index < 64 ? index : index % 64);
	uint8_t bmIndex = index < 64 ? 0 : (index / 64);

	*((&status.mqtt.devices.functioningDevices.bitmap0) + bmIndex) &= ~mask;

	if (deviceConfig.important)
		*((&status.mqtt.devices.functioningDevicesImportant.bitmap0) + bmIndex) &= ~mask;

	DEBUG_LOG_F("Mark Reconnected\r\n-Device Index : %d\r\n-Important : %d\r\n", index, deviceConfig.important);

	deviceStatus.markedDisconnected = false;

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

void MqttDevice::SetDeviceState()
{
	if (deviceStatus.configured)
	{
		if (deviceStatus.enabled)
			deviceStatus.state = (DeviceState_t)DeviceState::DEVICE_OK;
		else
			deviceStatus.state = (DeviceState_t)DeviceState::DEVICE_DISABLED;
	}
	else
	{
		deviceStatus.state = (DeviceState_t)DeviceState::DEVICE_ERROR;
	}
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

	String jdata = GenerateJsonStatePayload();

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


/// <summary>
/// Quickly creates a fresh document for the device, calls 1 specific AddxxxxData function, 
/// serializes the document before freeing, and finally returns the serialized data as a string.
/// </summary>
/// <param name=""></param>
/// <param name="dataType"></param>
/// <returns></returns>
String MqttDevice::GenerateJsonData(ADD_PAYLOAD_FUNC addPayload, const char* dataType)
{
	DEBUG_LOG_F("Generating %s(%s) JSON %s Data :\r\n", name.c_str(), deviceName, dataType);

	String jdata = "";

	if (!FreshJsonDocument())
	{
		//Not enough heap memory
		return jdata;
	}

	//Add data to document
	addPayload(documentRoot);

	SerializeDocument(&jdata, true);

	DEBUG_LOG_LN(jdata.c_str());

	return jdata;
}

String MqttDevice::GenerateJsonStatePayload()
{

	return GenerateJsonData([this](JsonObject&) 
	{
		return this->AddStatePayload(this->documentRoot);
	}, "State Payload");
}

String MqttDevice::GenerateJsonStatus()
{
	return GenerateJsonData([this](JsonObject&)
	{
		return this->AddStatusData(this->documentRoot);
	}, "Status");
}

String MqttDevice::GenerateJsonConfig()
{
	return GenerateJsonData([this](JsonObject&)
	{
		return this->AddConfigData(this->documentRoot);
	}, "Config");
}

String MqttDevice::GenerateJsonAll()
{
	return GenerateJsonData([this](JsonObject&)
	{
		return this->AddStatusData(this->documentRoot);
		return this->AddConfigData(this->documentRoot);
		return this->AddStatePayload(this->documentRoot);
	}, "All");
}

bool MqttDevice::CreateJsonDocument()
{	
	if (document != nullptr) return false;

	#warning change size for each device
	document = JsonHelper::CreateDocument(2048);
	documentRoot = document->createNestedObject(name.c_str());

	return true;
}


bool MqttDevice::FreshJsonDocument()
{
	if (document == nullptr) return false;

	FreeJsonDocument();
	return CreateJsonDocument();
}

void MqttDevice::FreeJsonDocument()
{
	if (document != nullptr)
	{
		document->clear();
		free(document);
		document = nullptr;
	}
}

bool MqttDevice::IsDocumentCreated(bool create)
{
	if (document != nullptr)
		return true;

	if (create)
		return CreateJsonDocument();

	return false;
}

size_t MqttDevice::SerializeDocument(String* out_string, bool freeDoc)
{
	if(document == nullptr || out_string == nullptr)
	return 0;

	size_t size = serializeJson(*document, *out_string);

	if (freeDoc)
		FreeJsonDocument();

	return size;
}

size_t MqttDevice::StreamDocument(AsyncWebServerRequest* request)
{
	if (document == nullptr)
	{
		request->send(204);	//No Content
		return 0;
	}

	AsyncResponseStream* response = request->beginResponseStream(Network::Website::Strings::ContentType::appJSON);
	size_t size = serializeJson(*document, *response);
	request->send(response);

	return size;
}

void MqttDevice::AddStatusData(JsonObject& addTo)
{
	addTo["deviceStatus"].set<MqttDeviceStatus_t>(deviceStatus);

	if (this->website != nullptr)
		this->website->AddStatusData(this->documentRoot);
}

void MqttDevice::AddConfigData(JsonObject& addTo)
{
	addTo["deviceConfig"].set<MqttDeviceConfig_t>(deviceConfig);
	addTo["deviceMqttSettings"].set<MqttDeviceMqttSettings_t>(deviceMqttSettings);
}


bool MqttDevice::InitWebpage()
{
	if (website == nullptr)
		website = new MqttDeviceWeb();

	website->Initialize(this);
}

void MqttDevice::DenitWebpage()
{
	if (website == nullptr) return;

	website->Deinitialize();
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

	deviceConfigMonitor.website.hostWebsite = deviceConfig.website.hostWebsite != MQTTDEVICE_WEBSITE_HOST;
	deviceConfig.website.hostWebsite = MQTTDEVICE_WEBSITE_HOST;

	deviceConfigMonitor.website.configurable = deviceConfig.website.configurable != MQTTDEVICE_WEBSITE_CONFIG;
	deviceConfig.website.configurable = MQTTDEVICE_WEBSITE_CONFIG;
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

	if (deviceObj.containsKey("website"))
		convertFromJson(deviceObj["website"], deviceConfig.website);
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


#pragma region JSON UDFs


const char* mqtt_deviceType_strings[5] = { "binarysensor", "button", "light", "sensor", "switch" };

bool canConvertFromJson(JsonVariantConst src, const MqttDeviceType&)
{
	return JsonHelper::JsonParseEnum(src, 5, mqtt_deviceType_strings) != -1;
}

void convertFromJson(JsonVariantConst src, MqttDeviceType& dst)
{
	JsonHelper::UdfHelperConvertFromJsonEnums(src, (EnumClass_t&)dst, 5, "MqttDeviceType", mqtt_deviceType_strings, nullptr);
}

bool convertToJson(const MqttDeviceType& src, JsonVariant dst)
{
	JsonHelper::UdfHelperConvertToJsonEnums((EnumClass_t&)src, dst, 5, "MqttDeviceType", mqtt_deviceType_strings, nullptr);
}

//void convertFromJson(JsonVariantConst src, MqttDeviceType& dst)
//{
//
//	MqttDeviceType parseResult = (MqttDeviceType)JsonHelper::JsonParseEnum(src, 5, mqtt_deviceType_strings);
//
//	if ((int)parseResult != -1)
//		dst = parseResult;
//	else
//		DEBUG_LOG_LN("MqttDeviceType Parsing Failed");
//}
//
//bool convertToJson(const MqttDeviceType& src, JsonVariant dst)
//{
//#if SERIALIZE_ENUMS_TO_STRING
//	bool set = JsonHelper::EnumValueToJson(dst, (int)src, mqtt_deviceType_strings, 5);
//#else
//	bool set = dst.set(src);
//#endif
//
//	if (set) return true;
//
//	DEBUG_LOG_LN("MqttDeviceType Conversion to JSON failed.");
//	return false;
//}

//bool canConvertFromJson(JsonVariantConst src, const MqttDeviceTopics_t&)
//{

//}

void convertFromJson(JsonVariantConst src, MqttDeviceTopics_t& dst)
{
	JsonVariantConst obj = src;

	if (src.containsKey("deviceTopics"))
		obj = src["deviceTopics"];


	if (obj.containsKey("command"))
		dst.command = obj["command"].as<String>();

	if (obj.containsKey("state"))
		dst.state = obj["state"].as<String>();

}

bool convertToJson(const MqttDeviceTopics_t& src, JsonVariant dst)
{
	dst["command"] = src.command;
	dst["state"] = src.state;
}


//bool canConvertFromJson(JsonVariantConst src, const MqttDeviceConfig_t&)
//{

//}

void convertFromJson(JsonVariantConst src, MqttDeviceConfig_t& dst)
{
	JsonVariantConst obj = src;

	if (src.containsKey("deviceConfig"))
		obj = src["deviceConfig"];


	if (obj.containsKey("useDefaults"))
		dst.useDefaults = obj["useDefaults"];

	if (obj.containsKey("useGlobalConfig"))
		dst.useGlobalConfig = obj["useGlobalConfig"];

	if (obj.containsKey("initiallyEnabled"))
		dst.initiallyEnabled = obj["initiallyEnabled"];

	if (obj.containsKey("important"))
		dst.important = obj["important"];

	if (obj.containsKey("website"))
	convertFromJson(obj, dst);
}

bool convertToJson(const MqttDeviceConfig_t& src, JsonVariant dst)
{
	dst["important"] = src.useDefaults;
	dst["important"] = src.useGlobalConfig;
	dst["important"] = src.initiallyEnabled;
	dst["important"] = src.important;
	dst["website"] = src.website;
}



//bool canConvertFromJson(JsonVariantConst src, const MqttDeviceStatus_t&)
//{
//	return src.containsKey("deviceStatus") || src.containsKey("markedDisconnected");
//}

void convertFromJson(JsonVariantConst src, MqttDeviceStatus_t& dst)
{
	JsonVariantConst deviceStatusObj = src;

	if (src.containsKey("deviceStatus"))
		deviceStatusObj = src["deviceStatus"];

	if (deviceStatusObj.containsKey("configured"))
		dst.configured = deviceStatusObj["configured"];

	if (deviceStatusObj.containsKey("enabled"))
		dst.enabled = deviceStatusObj["enabled"];

	if (deviceStatusObj.containsKey("state"))
		dst.state = deviceStatusObj["state"];

	if (deviceStatusObj.containsKey("subscribed"))
		dst.subscribed = deviceStatusObj["subscribed"];

	if (deviceStatusObj.containsKey("markedDisconnected"))
		dst.markedDisconnected = deviceStatusObj["markedDisconnected"];

	if (deviceStatusObj.containsKey("configModified"))
		dst.configModified = deviceStatusObj["configModified"];

	if (deviceStatusObj.containsKey("publishTimestamp"))
		dst.publishTimestamp = deviceStatusObj["publishTimestamp"];

	if (deviceStatusObj.containsKey("publishErrorTimestamp"))
		dst.publishErrorTimestamp = deviceStatusObj["publishErrorTimestamp"];

}

bool convertToJson(const MqttDeviceStatus_t& src, JsonVariant dst)
{
	dst["configured"] = src.configured;
	dst["enabled"] = src.enabled;
	dst["state"] = src.state;
	dst["subscribed"] = src.subscribed;
	dst["markedDisconnected"] = src.markedDisconnected;
	dst["configModified"] = src.configModified;
	dst["publishTimestamp"] = src.publishTimestamp;
	dst["publishErrorTimestamp"] = src.publishErrorTimestamp;
}

//bool canConvertFromJson(JsonVariantConst src, const MqttDeviceWebConfig_t&)
//{
//	return src.containsKey("hostWebsite") || src.containsKey("website") || src.containsKey("configurable");
//}

void convertFromJson(JsonVariantConst src, MqttDeviceWebConfig_t& dst)
{
	JsonVariantConst websiteObj = src;

	if (src.containsKey("website"))
		websiteObj = src["website"];


	if (websiteObj.containsKey("hostWebsite"))
		dst.hostWebsite = websiteObj["hostWebsite"];

	if (websiteObj.containsKey("configurable"))
		dst.configurable = websiteObj["configurable"];
}

bool convertToJson(const MqttDeviceWebConfig_t& src, JsonVariant dst)
{
	dst["hostWebsite"] = src.hostWebsite;
	dst["configurable"] = src.configurable;
}

//bool canConvertFromJson(JsonVariantConst src, const MqttDeviceMqttSettings_t&)
//{

//}

void convertFromJson(JsonVariantConst src, MqttDeviceMqttSettings_t& dst)
{
	JsonVariantConst obj = src;

	if (src.containsKey("deviceMqttSettings"))
		obj = src["deviceMqttSettings"];


	if (obj.containsKey("useDefaults"))
		dst.useDefaults = obj["useDefaults"];

	if (obj.containsKey("useParentTopics"))
		dst.useParentTopics = obj["useParentTopics"];

	if (obj.containsKey("retain"))
		dst.retain = obj["retain"];

	if (obj.containsKey("json"))
		dst.json = obj["json"];
}

bool convertToJson(const MqttDeviceMqttSettings_t& src, JsonVariant dst)
{
	dst["useDefaults"] = src.useDefaults;
	dst["useParentTopics"] = src.useParentTopics;
	dst["json"] = src.json;
	dst["retain"] = src.retain;
}


//bool canConvertFromJson(JsonVariantConst src, const MqttDeviceGlobalStatus_t&)
//{

//}

void convertFromJson(JsonVariantConst src, MqttDeviceGlobalStatus_t& dst)
{
	JsonVariantConst obj = src;

	if (src.containsKey("globalDeviceStatus"))
		obj = src["globalDeviceStatus"];


	if (obj.containsKey("configRead"))
		dst.configRead = obj["configRead"];

}

bool convertToJson(const MqttDeviceGlobalStatus_t& src, JsonVariant dst)
{
	dst["configRead"] = src.configRead;
}


////bool canConvertFromJson(JsonVariantConst src, const MqttDeviceWebpageStatus_t&)
////{
//
////}
//
//void convertFromJson(JsonVariantConst src, MqttDeviceWebpageStatus_t& dst)
//{
//	JsonVariantConst webpageStatus = src;
//
//	if (src.containsKey("website"))
//		webpageStatus = src["website"];
//
//
//	if (webpageStatus.containsKey("enabled"))
//		dst.enabled = webpageStatus["enabled"];
//
//	if (webpageStatus.containsKey("configured"))
//		dst.configured = webpageStatus["configured"];
//}
//
//bool convertToJson(const MqttDeviceWebpageStatus_t& src, JsonVariant dst)
//{
//	dst["enabled"] = src.enabled;
//	dst["configured"] = src.configured;
//}

const char* device_state_strings[3] = { "ok", "disabled", "error" };

bool canConvertFromJson(JsonVariantConst src, const DeviceState&)
{
	return JsonHelper::JsonParseEnum(src, 3, device_state_strings, nullptr) != -1;
}

void convertFromJson(JsonVariantConst src, DeviceState& dst)
{
	JsonHelper::UdfHelperConvertFromJsonEnums(src, (EnumClass_t&)dst, 3, "DeviceState", device_state_strings, nullptr);
}

bool convertToJson(const DeviceState& src, JsonVariant dst)
{
	JsonHelper::UdfHelperConvertToJsonEnums((EnumClass_t&)src, dst, 3, "DeviceState", device_state_strings, nullptr);
}

//void convertFromJson(JsonVariantConst src, DeviceState& dst)
//{
//	DeviceState parseResult = (DeviceState)JsonHelper::JsonParseEnum(src, 3, device_state_strings, nullptr);
//
//	if ((int)parseResult != -1)
//		dst = parseResult;
//	else
//		DEBUG_LOG_LN("DeviceState Parsing Failed");
//
//}
//
//bool convertToJson(const DeviceState& src, JsonVariant dst)
//{
//#if SERIALIZE_ENUMS_TO_STRING
//	bool set = JsonHelper::EnumValueToJson(dst, (int)src, device_state_strings, 3);
//#else
//	bool set = dst.set(src);
//#endif
//
//	if (set) return true;
//
//	DEBUG_LOG_LN("DeviceState Conversion to JSON failed.");
//	return false;
//}

#pragma endregion
