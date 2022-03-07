#include "MqttSensor.h"

String MqttSensor::mqttSensorBaseTopic;

bool MqttSensor::Init(bool enable)
{
	if (!status.mqtt.devicesConfigured)
	{
		memset(&deviceStatus, 0, sizeof(MqttDeviceStatus_t));
		memset(&sensorStatus, 0, sizeof(MqttSensorStatus_t));
	}

	//deviceStatus.enabled = true;

	Connect();

	if (!sensorStatus.connected) return false;

	return Configure();
}

void MqttSensor::Loop()
{
	if (!deviceStatus.enabled) return;

	if (!sensorStatus.connected)
	{
		if (Connect())
		{
			MarkReconnected();
			if (deviceStatus.configured)
				DEBUG_LOG_F(MQTT_DMSG_RECONNECTED, name.c_str());
		}

#warning mark on website that the device was unable to configure and set controls to try and retry
		//if (!deviceStatus.configured)
		//	Configure();

		return;
	}
}

void MqttSensor::ResetStatus()
{
	memset(&deviceStatus, 0, sizeof(MqttDeviceStatus_t));
	memset(&sensorStatus, 0, sizeof(MqttSensorStatus_t));
	MqttDevice::ResetStatus();
}

bool MqttSensor::Connect()
{
	sensorStatus.connected = deviceStatus.enabled;
	return true;
}

bool MqttSensor::IsConnected()
{
	return sensorStatus.connected;
}


int MqttSensor::ReadAndPublish()
{
	if (!Read()) return 1;

	if (!Publish()) return 2;

	return 0;
}

bool MqttSensor::Publish()
{
	if (!deviceStatus.enabled || !sensorStatus.newData || !status.mqtt.connected)
		return false;

	sensorStatus.newData = false;

	if (sensorStatus.connected)
	{
		return MqttDevice::Publish();
	}
	else if (!sensorStatus.connected || sensorStatus.failedReads >= MQTT_SENSOR_MAX_FAILED_READS)
	{
		PublishNoConnection();
	}
	else
	{
		DEBUG_LOG_F("%s cannot publish. No new Sensor Data.\r\n", name.c_str());
	}

	return false;
}

//virtual bool PublishAvailability() override
//{
//	return mqttClient.publish(topics.availability.c_str(), Mqtt::Helper::GetAvailabilityString(sensorStatus.connected).c_str());
//}


bool MqttSensor::PublishNoConnection()
{
	const char* topic = deviceMqttSettings.useParentTopics ? (*GetParentBaseTopic()).c_str() : topics.jsonState.c_str();

	String payload;

	if (deviceMqttSettings.useParentTopics || deviceMqttSettings.json)
		payload = GenerateJsonPayload();
	else
		payload = SENSOR_DATA_UNKNOWN;

	return mqttClient.publish(topic, SENSOR_DATA_UNKNOWN);
}


void MqttSensor::FailedRead()
{
	sensorStatus.failedReads++;

	if (sensorStatus.failedReads >= MQTT_SENSOR_MAX_FAILED_READS)
		sensorStatus.connected = false;
}