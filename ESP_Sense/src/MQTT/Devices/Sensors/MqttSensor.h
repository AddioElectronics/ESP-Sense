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
extern DeviceStatus_t status;

class MqttSensor : public MqttDevice
{
public:

	static String mqttSensorBaseTopic;

	/// <summary>
	/// MQTT Sensor Status 
	/// </summary>
	MqttSensorStatus_t sensorStatus;

	MqttSensor(const char* _name, int _index) : MqttDevice(_name, _index) {}

	virtual bool Init(bool enable = true);

	virtual void Loop() override;

	virtual void ResetStatus() override;

	virtual bool Connect();

	virtual bool IsConnected();

	virtual bool Read() = 0;

	virtual int ReadAndPublish();

	virtual bool Publish() override;

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


#endif

