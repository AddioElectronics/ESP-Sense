#ifndef _MQTTDEVICEMANAGER_h
#define _MQTTDEVICEMANAGER_h

#include <Arduino.h>

#include <ArduinoJson.hpp>
#include <ArduinoJson.h>

#include "../../Config/config_master.h"
#include "../../Config/global_status.h"
#include "../../Config/config_mqtt.h"

#include "MqttDevice.h"

#include "Sensors/MqttSensor.h"
#include "Sensors/SHT4X/sensor_sht4x.h"
#include "Sensors/SCD4X/sensor_scd4x.h"

#include "BinarySensors/MqttBinarySensor.h"
#include "BinarySensors/LLC200D3SH/sensor_LLC200D3SH.h"

//#warning put somewhere else
//typedef struct {
//	String* name;
//	String* deviceName;
//	MqttDeviceType type;
//	String* url;
//}MqttDeviceInfo_t;

//#include "MqttLight.h"
//
//#include "MqttButton.h"
//
//#include "MqttSwitch.h"

namespace Mqtt
{
	class DeviceManager
	{
	public:
		static int ConfigureDevices();
		static int UnconfigureDevices();

	private:
		static device_count_t ConfigureSensors();
		static device_count_t ConfigureBinarySensors();
		static device_count_t ConfigureLights();
		static device_count_t ConfigureButtons();
		static device_count_t ConfigureSwitches();

	public:
		static void EnableAll(bool initial = false);
		static void DisableAll();
		static int SubscribeAll();
		static int UnsubscribeAll();
		static void PublishAvailability();
		static void ReadAll();
		static void PublishAll();
		static void Loop();

		/// <summary>
		/// Packs all MQTT device's names and deviceNames into a JSON document.
		/// </summary>
		static int PackDeviceInfo(JsonObject& doc);

		///// <summary>
		///// Packs all MQTT device's names and deviceNames into a JSON document,
		///// before serializing in to a string.
		///// *If string is nullptr, it will only pack and not serialize(out_doc must be passed).
		///// </summary>
		//static size_t GetJsonDeviceInfo(String* serializeTo, DynamicJsonDocument** out_doc = nullptr);

		///// <summary>
		///// Packs all MQTT device's names and deviceNames into a JSON document.
		///// </summary>
		//static size_t GetJsonDeviceInfo(DynamicJsonDocument** out_doc);

		/// <summary>
		/// Forwards the received MQTT message to all devices.
		/// *Todo: Only forward until the topic has been processed, unless the topic is JSON that is shared by multiple devices.
		/// </summary>
		static void Callback(char* topic, byte* payload, unsigned int length);

	private:
		static size_t GetMaxFileSize();

		static bool IsValidSensor(const char* device);
		static MqttSensor* CreateMqttSensor(int index, int subIndex, const char* name, const char* device);
		static bool IsValidBinarySensor(const char* device);
		static MqttBinarySensor* CreateMqttBinarySensor(int index, int subIndex, const char* name, const char* device);

	};

	namespace Tasks
	{
		bool StartDeviceManagerLoopTask();
		void StopDeviceManagerLoopTask();
		void DeviceManagerLoopTask(void* pvParameters);
	}
}


#endif

