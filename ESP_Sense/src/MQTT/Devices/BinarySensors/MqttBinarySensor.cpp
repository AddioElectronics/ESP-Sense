#include "MqttBinarySensor.h"

String MqttBinarySensor::mqttBinarySensorBaseTopic;

void MqttBinarySensor::AddStatusData(JsonObject& addTo)
{
	MqttDevice::AddStatusData(addTo);
	addTo["binarySensorStatus"].set<MqttBinarySensorStatus_t>(binarySensorStatus);
}

int MqttBinarySensor::ReadAndPublish()
{
	if (!Read()) return 1;

	if (!Publish()) return 2;

	return 0;
}


#pragma region JSON UDFs


bool canConvertFromJson(JsonVariantConst src, const MqttBinarySensorStatus_t&)
{
	return src.containsKey("newData") || src.containsKey("binarySensorStatus");
}

void convertFromJson(JsonVariantConst src, MqttBinarySensorStatus_t& dst)
{
	JsonVariantConst sensorStatusObj = src;

	if (src.containsKey("binarySensorStatus"))
		sensorStatusObj = src["binarySensorStatus"];

	if (sensorStatusObj.containsKey("newData"))
		dst.newData = sensorStatusObj["newData"];

}

bool convertToJson(const MqttBinarySensorStatus_t& src, JsonVariant dst)
{
	dst["newData"] = src.newData;
}

#pragma endregion