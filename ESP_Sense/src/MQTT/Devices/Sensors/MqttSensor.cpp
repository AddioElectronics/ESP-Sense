#include "MqttSensor.h"

String MqttSensor::mqttSensorBaseTopic;

const char* MqttSensor::deviceTypeName = "Sensor";
const char* MqttSensor::deviceTypeKey = "sensors";

bool MqttSensor::Init()
{
	DEBUG_LOG_F("Initializing Sensor %s(SCD4x)\r\n", name.c_str());

	Connect();

	return MqttDevice::Init();
}

void MqttSensor::Loop()
{
	if (!deviceStatus.enabled) return;

	if (!sensorStatus.connected)
	{
		if (Connect())
		{
			if (deviceStatus.configured)
				DEBUG_LOG_F(MQTT_DMSG_RECONNECTED, name.c_str());
		}

		MarkFunctionalBitmap();

		//if (!deviceStatus.configured)
		//	Configure();

		return;
	}

	MqttDevice::Loop();
}

void MqttSensor::ResetStatus()
{
	memset(&sensorStatus, 0, sizeof(MqttSensorStatus_t));
	MqttDevice::ResetStatus();
}

void MqttSensor::SetDeviceState()
{
	if (deviceStatus.configured)
	{
		if (deviceStatus.enabled && sensorStatus.connected && !sensorStatus.sleeping)
			deviceStatus.state = (DeviceState_t)DeviceState::DEVICE_OK;
		else
			deviceStatus.state = (DeviceState_t)DeviceState::DEVICE_DISABLED;
	}
	else
	{
		deviceStatus.state = (DeviceState_t)DeviceState::DEVICE_ERROR;
	}
}

//bool MqttSensor::Connect()
//{
//	sensorStatus.connected = deviceStatus.enabled;
//	SetDeviceState();
//	return true;
//}
//
//bool MqttSensor::IsConnected()
//{
//	return sensorStatus.connected;
//}


int MqttSensor::ReadAndPublish()
{
	if (!Read()) return 1;

	if (!Publish()) return 2;

	return 0;
}

bool MqttSensor::Publish()
{
	DEBUG_LOG_F("%s MqttSensor::Publish()\r\n", name.c_str());

	if (deviceStatus.state != (DeviceState_t)DeviceState::DEVICE_OK || !sensorStatus.newData || !status.mqtt.connected)
	{
		DEBUG_LOG_LN("Failed : No new data, not enabled, and/or connected to mqtt broker.\r\n");
		return false;
	}

	sensorStatus.newData = false;

	if (sensorStatus.connected)
	{
		return MqttDevice::Publish();
	}
	//else if (!sensorStatus.connected || sensorStatus.failedReads >= MQTT_SENSOR_MAX_FAILED_READS)
	//{
	//	PublishNoConnection();
	//}
	else
	{
		DEBUG_LOG_F("%s cannot publish. No new Sensor Data.\r\n", name.c_str());
	}

	return false;
}

bool MqttSensor::IsFunctional()
{
	return sensorStatus.connected;
}


void MqttSensor::AddStatusData(JsonVariant& addTo)
{
	MqttDevice::AddStatusData(addTo);
	addTo["sensorStatus"].set<MqttSensorStatus_t>(sensorStatus);
}


//virtual bool PublishAvailability() override
//{
//	return mqttClient.publish(topics.availability.c_str(), Mqtt::Helper::GetAvailabilityString(sensorStatus.connected).c_str());
//}


bool MqttSensor::PublishNoConnection()
{
	const char* topic = deviceMqttSettings.useParentTopics ? (*GetParentBaseTopic()).c_str() : topics.jsonState.c_str();

	String payload;

	//if (deviceMqttSettings.useParentTopics || deviceMqttSettings.json)
	//	payload = 
	//else
		payload = SENSOR_DATA_UNKNOWN;

	return mqttClient.publish(topic, SENSOR_DATA_UNKNOWN);
}


void MqttSensor::FailedRead()
{
	sensorStatus.failedReads++;

	if (sensorStatus.failedReads >= MQTT_SENSOR_MAX_FAILED_READS)
		sensorStatus.connected = false;
}

#pragma region JSON UDFs


//bool canConvertFromJson(JsonVariantConst src, const MqttSensorStatus_t&)
//{
//	return src.containsKey("measurementTimestamp") || src.containsKey("sensorStatus");
//}

//void convertFromJson(JsonVariantConst src, MqttSensorStatus_t& dst)
//{
//	JsonVariantConst sensorStatusObj = src;
//
//	if (src.containsKey("sensorStatus"))
//		sensorStatusObj = src["sensorStatus"];
//
//
//	if (sensorStatusObj.containsKey("connected"))
//		dst.connected = sensorStatusObj["connected"];
//
//	if (sensorStatusObj.containsKey("sleeping"))
//		dst.sleeping = sensorStatusObj["sleeping"];
//
//	if (sensorStatusObj.containsKey("newData"))
//		dst.newData = sensorStatusObj["newData"];
//
//	if (sensorStatusObj.containsKey("failedReads"))
//		dst.failedReads = sensorStatusObj["failedReads"];
//
//	if (sensorStatusObj.containsKey("measurementTimestamp"))
//		dst.measurementTimestamp = sensorStatusObj["measurementTimestamp"];
//}

bool convertToJson(const MqttSensorStatus_t& src, JsonVariant dst)
{
	dst["connected"] = src.connected;
	dst["sleeping"] = src.sleeping;
	dst["newData"] = src.newData;
	dst["failedReads"] = src.failedReads;
	dst["measurementTimestamp"] = src.measurementTimestamp;
}

#pragma endregion
