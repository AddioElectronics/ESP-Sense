#ifndef _MQTTHELPER_h
#define _MQTTHELPER_h

#include <Arduino.h>

#include <ArduinoJson.hpp>
#include <ArduinoJson.h>

#include <cstdarg>

#include "../Config/config_mqtt.h"
#include "../GlobalDefs.h"

namespace Mqtt
{
	namespace Helper
	{
		int GetSwitchState(String payload);

		bool ButtonPressed(String payload);

		bool PayloadIsInteger(String payload);

		bool PayloadIsFloat(String payload);

		String GetAvailabilityString(bool state);

		char* GetBinarySensorString(bool state);

		bool IsBaseTopic(String& base);

		bool IsBaseTopic(char* base);

		//String CreateTopicString(String& base, String& topics, String* setBase, char* defaultBaseTopic);

		//String CreateTopicString(String& base, char* topics, String* setBase, char* defaultBaseTopic);

		bool CreateTopicString(String* setTopic, char* jsonTopic, String& baseParent, String* setBase, char* defaultBaseTopic);

		bool CreateTopicString(char* setTopic, char* jsonTopic, String& baseParent, String* setBase, char* defaultBaseTopic);

		#warning  testing, remember to remove the ones not in use

		bool CreateBaseTopicString(char* jsonTopic, String* setBase, char* defaultBaseTopic);

		bool CreateBaseTopicString(char* jsonTopic, String& baseParent, String* setBase, char* defaultBaseTopic);

		void CreateBaseTopicString(JsonVariantConst& jsonTopic, String& setBase, const char* defaultBaseTopic, String& parentBase);

		void CreateBaseTopicString(JsonVariantConst& jsonTopic, String& setBase, const char* defaultBaseTopic, const char* parentBase);

		String CombineTopics(int count, ...);
	}
}
#endif

