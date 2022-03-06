#ifndef _MQTTMANAGER_h
#define _MQTTMANAGER_h

#include <Arduino.h>

#include <ArduinoJson.hpp>
#include <ArduinoJson.h>

#include <cstdarg>

#include "Devices/MqttDeviceManager.h"
#include "../GlobalDefs.h"


namespace Mqtt {

	typedef union
	{
		struct {
			bool connect : 1;
			bool connectNoWifi : 1;
			bool reconnect : 1;
			bool clientConnect : 1;
			bool loop : 1;
			bool devices : 1;
			bool selectIp : 1;
			bool publishAvailability : 1;
		};
		uint8_t bitmap;
	}MqttMessagesSent_t;

	extern String baseTopic;


	void Initialize();

	void Configure();

	bool Connect();

	bool Reconnect();

	void Disconnect();

	void MonitorConnection();

	bool IsConnected();

	void SetConnectionLed();

	/// <summary>
	/// Check to see if all important MQTT devices are functioning properly.
	/// </summary>
	bool AllImportantDevicesFunctional();


	void Loop();

	bool Subscribe();

	bool Unsubscribe();

	void PublishAvailability();

	bool DisplayMessageIntervalMet();

	void Callback(char* topics, byte* payload, unsigned int length);   //Callback for receiving MQTT Messages

	namespace Tasks
	{
		void StartAllTasks();

		void StartMqttLoopTask();
		void StopLoopTask();
#if defined(ESP8266)
		bool MqttLoopTask()
#elif defined(ESP32)
		void MqttLoopTask(void* pvParameters);
#endif
		bool StartPublishAvailabilityTask();
		void StopPublishAvailabilityTask();
		void PublishAvailabilityTask(void* pvParameters);

		bool StartBlinkTask();
		void StopBlinkTask();
		void BlinkTask(void* pvParameters);
	}

	

//#warning  place ReadKey functions somewhere else.

//template<typename T>
//bool ReadKey(T& setValue, JsonObject& obj, const char* key);
//template<typename T>
//bool ReadKey(T& setValue, JsonDocument& doc, const char* key);
//template<typename T>
//bool ReadKey(T& setValue, JsonVariant& doc, const char* key);
//bool ReadKey(String& setValue, JsonObject& obj, const char* key);
//bool ReadKey(String& setValue, JsonDocument& doc, const char* key);
//bool ReadKey(String& setValue, JsonVariant& doc, const char* key);
// 
	//bool ReadKey(String& setValue, JsonObject& obj, char* key)
	//{
	//	if (obj.containsKey(key))
	//		setValue = obj[key];
	//}
	//bool ReadKey(char*& setValue, JsonObject& obj, char* key);
	//bool ReadKey(uint8_t& setValue, JsonObject& obj, char* key);
	//bool ReadKey(uint16_t& setValue, JsonObject& obj, char* key);
	//bool ReadKey(uint32_t& setValue, JsonObject& obj, char* key);
	//bool ReadKey(uint64_t& setValue, JsonObject& obj, char* key);

	namespace Strings {

	};
};







#endif

