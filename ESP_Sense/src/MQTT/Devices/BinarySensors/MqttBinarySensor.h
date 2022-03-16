#ifndef _BINARYSENSOR_h
#define _BINARYSENSOR_h

#include <Arduino.h>

#include "../../../../ESP_Sense.h"

#include "../../MqttHelper.h"
#include "../MqttDevice.h"

typedef struct {
	bool newData : 1;
}MqttBinarySensorStatus_t;

class MqttBinarySensor : public MqttDevice
{
public:

	static String mqttBinarySensorBaseTopic;

	static MqttTopics_t globalTopics;
	static MqttDeviceMqttSettings_t globalMqttSettings;

	MqttBinarySensorStatus_t binarySensorStatus;

	MqttBinarySensor(const char* _name, const char* _device, int _index, int _subIndex) : MqttDevice(_name, _device, _index, _subIndex) {}

	virtual void ResetStatus() override
	{
		memset(&binarySensorStatus, 0, sizeof(binarySensorStatus));
		MqttDevice::ResetStatus();
	}

	MqttDeviceType GetDeviceType() override
	{
		return MqttDeviceType::MQTT_BINARY_SENSOR;
	}

	virtual void AddStatusData(JsonObject& addTo) override;		//device, binary/sensor/ect.., and unique status

	virtual bool Read()
	{
		return false;
	}

	virtual int ReadAndPublish();

	bool SaveConfig() override
	{
		#warning load config_sensors.json, modify values for each sensor, and save.
			return false;
	}

};

#pragma region JSON UDFs

bool canConvertFromJson(JsonVariantConst src, const MqttBinarySensorStatus_t&);
void convertFromJson(JsonVariantConst src, MqttBinarySensorStatus_t& dst);
bool convertToJson(const MqttBinarySensorStatus_t& src, JsonVariant dst);

#pragma endregion

#endif

