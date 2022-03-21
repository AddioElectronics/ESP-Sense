// Sensor.h

#ifndef _MQTTSENSOR_h
#define _MQTTSENSOR_h

#include <Arduino.h>

#include <ArduinoJson.hpp>
#include <ArduinoJson.h>
#include <PubSubClient.h>

#include "../../MqttHelper.h"
#include "../MqttDevice.h"
#include "MqttSensor_Config.h"
#include "../../../Macros.h"

typedef struct {
	struct {
		bool connected : 1;
		bool sleeping : 1;
		bool newData : 1;
		uint8_t reserved : 5;
		uint8_t failedReads;
	};
	unsigned long measurementTimestamp;
}MqttSensorStatus_t;

extern PubSubClient mqttClient;
extern GlobalStatus_t status;

class MqttSensor : public MqttDevice
{
public:

	static const char* deviceTypeName;		//Used for print statements
	static const char* deviceTypeKey;	//Used for keys, and filtering

	const char* DeviceTypeName() override
	{
		return deviceTypeName;
	}
	const char* DeviceTypeKey() override
	{
		return deviceTypeKey;
	}

	//device_count_t* GetDeviceTypeCount() override
	//{
	//	return &status.mqtt.devices.sensorCount;
	//}

	static String mqttSensorBaseTopic;


	/// <summary>
	/// MQTT Sensor Status 
	/// </summary>
	MqttSensorStatus_t sensorStatus;

	MqttSensor(const char* _name, int _index, int _subIndex) : MqttDevice(_name, _index, _subIndex) {}

	virtual bool Init() override;

	virtual void Loop() override;

	virtual void ResetStatus() override;

	virtual void SetDeviceState() override;

	virtual bool Connect() = 0;

	virtual bool IsConnected() = 0;

	virtual bool Read() = 0;

	virtual int ReadAndPublish();

	virtual bool Publish() override;

	virtual bool IsFunctional() override;

	virtual void AddStatusData(JsonVariant& addTo) override;		//device, binary/sensor/ect.., and unique status

	bool SaveConfig() override
	{
#warning load config_sensors.json, modify values for each sensor, and save.
		return false;
	}

	MqttDeviceType GetDeviceType() override
	{
		return MqttDeviceType::MQTT_SENSOR;
	}

	//virtual bool PublishAvailability() override
	//{
	//	return mqttClient.publish(topics.availability.c_str(), Mqtt::Helper::GetAvailabilityString(sensorStatus.connected).c_str());
	//}


	virtual bool PublishNoConnection();

	/// <summary>
	/// Get the sensors library object.
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="object"></param>
	/// <returns></returns>
	template<class T> int GetSensorObject(T** object)
	{
		*object = NULL;
		return -1;
	};

	/// <summary>
	/// Power down if the sensor supports it.
	/// </summary>
	virtual bool PowerDown()
	{
		return true;
	}

	virtual bool Wake()
	{
		return true;
	}

	virtual void FailedRead();

};

#pragma region JSON UDFs

//bool canConvertFromJson(JsonVariantConst src, const MqttSensorStatus_t&);
//void convertFromJson(JsonVariantConst src, MqttSensorStatus_t& dst);
bool convertToJson(const MqttSensorStatus_t& src, JsonVariant dst);

#pragma endregion

#endif

