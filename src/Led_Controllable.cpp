///*
//	Gets LED pin configuration from "led_config.json"
//	Allows LED's to be controlled by name, within the program or via MQTT.
//	Publishes availability or LED states to MQTT server.
//*/
//#include "Led_Controllable.h"
//#include "ESPSense.h"
//
//#include <LittleFS.h>					//File system for loading config files.
//#include <ArduinoJson.hpp>				//Used for deserializing config files, and de/serializing data for/from Home Assistant
//#include <ArduinoJson.h>
//
//#include "error_codes.h"
//
//Led_t* controllableLeds;
//uint8_t controllableLedCount;
//
//String controllableLedTopic;
//String controllableLedAvailabilityTopic;
//String controllableLedArrayName;
//
// extern DeviceStatus_t status;
//
//bool led_init()
//{
//	Serial.println("Initializing Controllable LEDs");
//
//	if (!status.storage.fsMounted) return false;
//
//
//	File file = LittleFS.open("/led_config.json", "r");
//
//	StaticJsonDocument<1024> doc;
//
//
//	DeserializationError derror = deserializeJson(doc, file);
//
//	if (derror)
//	{
//		Serial.print("Error Parsing LED Config : ");
//		Serial.println(derror.c_str());
//		return ERROR_PARSING;
//	}
//	else
//	{
//		status.config.configRead = true;
//	}
//
//	JsonArray ledArray = doc["leds"].to<JsonArray>();
//	controllableLedCount = ledArray.size();
//
//	if (controllableLedCount == 0) return false;
//
//	controllableLedTopic = doc["topic"].as<String>();
//	controllableLedAvailabilityTopic = doc["availabilityTopic"].as<String>();
//	controllableLedArrayName = doc["arrayName"].as<String>();
//
//	controllableLeds = (Led_t*)malloc(sizeof(Led_t) * controllableLedCount);
//
//	for (uint8_t i = 0; i < controllableLedCount; i++)
//	{
//		controllableLeds[i].name = ledArray[i]["name"].as<String>();
//		controllableLeds[i].gpio = ledArray[i]["gpio"];
//	}
//
//	return true;
//}
//
//bool led_publish_availability()
//{
//	if (controllableLedCount == 0) return true;
//
//	if (!status.mqtt.connected) return false;
//
//	for (uint8_t i = 0; i < controllableLedCount; i++)
//	{
//#warning bad use of strings!
//		String topics = controllableLedAvailabilityTopic + controllableLeds[i].name;
//		mqttClient.publish(topics.c_str(), config.mqtt.payload.available.c_str());
//	}
//
//	return true;
//}
//
//bool led_publish_state()
//{
//	if (controllableLedCount == 0) return true;
//
//	if (!status.mqtt.connected) return false;
//
//	StaticJsonDocument<1024> doc;
//
//	JsonArray array = doc.createNestedArray(controllableLedArrayName);
//
//	for (uint8_t i = 0; i < controllableLedCount; i++)
//	{
//		array.add(controllableLeds[i]);
//	}
//
//	char buffer[512];
//	size_t size = serializeJson(doc, buffer);
//	mqttClient.publish(controllableLedTopic.c_str(), buffer, size);
//
//	return true;
//}
//
//bool led_enable(String name)
//{
//	for (uint8_t i = 0; i < controllableLedCount; i++)
//	{
//		if (controllableLeds[i].name.compareTo(name) == 0)
//		{
//			digitalWrite(controllableLeds[i].gpio, HIGH);
//			return true;
//		}
//	}
//	return false;
//}
//
//bool led_disable(String name)
//{
//	for (uint8_t i = 0; i < controllableLedCount; i++)
//	{
//		if (controllableLeds[i].name.compareTo(name) == 0)
//		{
//			digitalWrite(controllableLeds[i].gpio, LOW);
//			return true;
//		}
//	}
//	return false;
//}
//
//bool led_toggle(String name)
//{
//	for (uint8_t i = 0; i < controllableLedCount; i++)
//	{
//		if (controllableLeds[i].name.compareTo(name) == 0)
//		{
//			digitalWrite(controllableLeds[i].gpio, !digitalRead(controllableLeds[i].gpio));
//			return true;
//		}
//	}
//	return false;
//}