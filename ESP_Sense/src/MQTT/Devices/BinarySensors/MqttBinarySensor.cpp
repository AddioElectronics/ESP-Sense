#include "MqttBinarySensor.h"

String MqttBinarySensor::mqttBinarySensorBaseTopic;


int MqttBinarySensor::ReadAndPublish()
{
	if (!Read()) return 1;

	if (!Publish()) return 2;

	return 0;
}