#include "MqttHelper.h"

#include "../Macros.h"

int Mqtt::Helper::GetSwitchState(String payload)
{
	if (payload == MQTT_SWITCH_ON)
	{
		return true;
	}
	else if (payload == MQTT_SWITCH_OFF)
	{
		return false;
	}

	return -1;
}

bool Mqtt::Helper::ButtonPressed(String payload)
{
	DEBUG_LOG_F("Comparing %s to %s", (payload), MQTT_BUTTON_PRESS);
	return (payload).compareTo(MQTT_BUTTON_PRESS) == 0;
}

bool Mqtt::Helper::PayloadIsInteger(String payload)
{
	//#warning  Function not finished
	return true;
}

bool Mqtt::Helper::PayloadIsFloat(String payload)
{
	return true;
}

/// <summary>
/// Converts a bool into a string payload for the availability topic.
/// </summary>
String Mqtt::Helper::GetAvailabilityString(bool state)
{
	return state ? MQTT_AVAILABLE : MQTT_UNAVAILABLE;
}

char* Mqtt::Helper::GetBinarySensorString(bool state)
{
	if (state)
		return MQTT_BINARY_SENSOR_ON;
	else
		return MQTT_BINARY_SENSOR_OFF;
}

bool Mqtt::Helper::IsBaseTopic(String& base)
{
	return base[0] == '/';
}

bool Mqtt::Helper::IsBaseTopic(char* base)
{
	return base[0] == '/';
}

//String CreateTopicString(String& base, String& topics, String* setBase, char* defaultBaseTopic)
//{
//	if (topics[0] == '/')
//	{
//		if (setBase != 0)
//			*setBase = topics;
//		return String(base + topics);
//	}
//	else
//	{
//		if (setBase != 0 && defaultBaseTopic != 0)
//			*setBase = *defaultBaseTopic;

//		return topics;
//	}


//	//DEBUG_LOG_LN("Cannot set topic. Format invalid.");
//}

//String CreateTopicString(String& base, char* topics, String* setBase, char* defaultBaseTopic)
//{
//	if (topics[0] == '/' && base.length() > 0)
//	{
//		if (setBase != 0)
//			*setBase = topics;

//		return String(base + topics);
//	}
//	else if (topics[0] != '/')
//	{
//		if (setBase != 0 && defaultBaseTopic != 0)
//			*setBase = *defaultBaseTopic;

//		return String(topics);
//	}

//	//DEBUG_LOG_LN("Cannot set topic. Format invalid.");
//}

bool Mqtt::Helper::CreateTopicString(String* setTopic, char* jsonTopic, String& baseParent, String* setBase, char* defaultBaseTopic)
{
	if (jsonTopic == NULL && setTopic != NULL && baseParent.length() != 0 && defaultBaseTopic != NULL)
	{
		*setTopic = baseParent + defaultBaseTopic;

		if (setBase != NULL)
			*setBase = defaultBaseTopic;
	}
	else if (jsonTopic[0] == '/' && setTopic != NULL && baseParent.length() != 0)
	{
		*setTopic = baseParent + jsonTopic;

		if (setBase != NULL)
			*setBase = jsonTopic;
	}
	else if (jsonTopic[0] != '/' && setTopic != NULL)
	{
		*setTopic = jsonTopic;

		if (setBase != NULL)
			*setBase = defaultBaseTopic;
	}
	else
		return false;

	return true;
}

bool Mqtt::Helper::CreateTopicString(char* setTopic, char* jsonTopic, String& baseParent, String* setBase, char* defaultBaseTopic)
{
	if (jsonTopic == NULL && setTopic != NULL && baseParent.length() != 0 && defaultBaseTopic != NULL)
	{
		strcpy(setTopic, baseParent.c_str());
		strcpy(setTopic + baseParent.length(), defaultBaseTopic);

		if (setBase != NULL)
			*setBase = defaultBaseTopic;
	}
	else if (jsonTopic[0] == '/' && setTopic != NULL && baseParent.length() != 0)
	{
		strcpy(setTopic, baseParent.c_str());
		strcpy(setTopic + baseParent.length(), jsonTopic);

		if (setBase != NULL)
			*setBase = jsonTopic;
	}
	else if (jsonTopic != NULL && setTopic != NULL)
	{
		strcpy(setTopic, jsonTopic);

		if (setBase != NULL)
			*setBase = defaultBaseTopic;
	}
	else
		return false;

	return true;
}




bool Mqtt::Helper::CreateBaseTopicString(char* jsonTopic, String* setBase, char* defaultBaseTopic)
{
	if (jsonTopic == NULL && defaultBaseTopic != NULL && setBase != NULL)
	{
		*setBase = defaultBaseTopic;
	}
	else if (jsonTopic[0] == '/' && setBase != NULL)
	{
		*setBase = jsonTopic;
	}
	else if (jsonTopic != NULL && defaultBaseTopic != NULL && setBase != NULL)
	{
		*setBase = defaultBaseTopic;
	}
	else
		return false;

	return true;
}
bool Mqtt::Helper::CreateBaseTopicString(char* jsonTopic, String& baseParent, String* setBase, char* defaultBaseTopic)
{
	if (jsonTopic == NULL && defaultBaseTopic != NULL && setBase != NULL)
	{
		*setBase = baseParent + defaultBaseTopic;
	}
	else if (jsonTopic[0] == '/' && setBase != NULL)
	{
		*setBase = baseParent + jsonTopic;
	}
	else if (jsonTopic != NULL && defaultBaseTopic != NULL && setBase != NULL)
	{
		*setBase = defaultBaseTopic;
	}
	else
		return false;

	return true;
}

void Mqtt::Helper::CreateBaseTopicString(JsonVariantConst& jsonTopic, String& setBase, const char* defaultBaseTopic, String& parentBase)
{
	const char* topic;

	if (!jsonTopic.isNull())
		topic = jsonTopic;
	else
		topic = defaultBaseTopic;

	if (topic[0] == '/')
		setBase = parentBase + defaultBaseTopic;
	else
		setBase = defaultBaseTopic;
}

void Mqtt::Helper::CreateBaseTopicString(JsonVariantConst& jsonTopic, String& setBase, const char* defaultBaseTopic, const char* parentBase)
{
	const char* topic;

	if (!jsonTopic.isNull())
		topic = jsonTopic;
	else
		topic = defaultBaseTopic;

	if (topic[0] == '/')
	{
		setBase = parentBase;
		setBase += defaultBaseTopic;
	}
	else
		setBase = defaultBaseTopic;
}

String Mqtt::Helper::CombineTopics(int count, ...)
{

	String topic;
	va_list ap;
	va_start(ap, count);
	for (int i = 0; i < count; i++)
	{
		char* s = va_arg(ap, char*);
		int len = strlen(s);

		if ((i == 0 || topic[topic.length() - 1] == '/') && s[0] == '/')
		{
			s++;
			len--;
		}

		topic += s;
	}

	va_end(ap);

	return topic;
}

/*template<typename T>
bool Mqtt::Helper::ReadKey(T& setValue, JsonObject& obj, const char* key)
{
	if (obj.containsKey(key))
	{
		setValue = obj[key];
		return true;
	}
	return false;
}

template<typename T>
bool Mqtt::Helper::ReadKey(T& setValue, JsonDocument& doc, const char* key)
{
	if (doc.containsKey(key))
	{
		setValue = doc[key];
		return true;
	}
	return false;
}

template<typename T>
bool Mqtt::Helper::ReadKey(T& setValue, JsonVariant& doc, const char* key)
{
	if (doc.containsKey(key))
	{
		setValue = doc[key];
		return true;
	}
	return false;
}

bool Mqtt::Helper::ReadKey(String& setValue, JsonObject& obj, const char* key)
{
	if (obj.containsKey(key))
	{
		setValue = obj[key].as<String>();
		return true;
	}
	return false;
}

bool Mqtt::Helper::ReadKey(String& setValue, JsonDocument& doc, const char* key)
{
	if (doc.containsKey(key))
	{
		setValue = doc[key].as<String>();
		return true;
	}
	return false;
}

bool Mqtt::Helper::ReadKey(String& setValue, JsonVariant& doc, const char* key)
{
	if (doc.containsKey(key))
	{
		setValue = doc[key].as<String>();
		return true;
	}
	return false;
}*/

