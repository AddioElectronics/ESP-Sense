#ifndef _BINARYSENSOR_h
#define _BINARYSENSOR_h

#include <Arduino.h>

#include "../../../../ESPSense.h"

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

	MqttBinarySensor(const char* _name, int _index) : MqttDevice(_name, _index) {}

	virtual bool Read()
	{
		return false;
	}

	virtual int ReadAndPublish();

};


#endif

