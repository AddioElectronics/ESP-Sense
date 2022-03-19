#include "sensor_scd4x.h"

#include "../../../../../ESP_Sense.h"
#include "../../../MqttManager.h"
#include "../../../MqttHelper.h"
#include "../../../../JsonHelper.h"
#include "../../../../HelperFunctions.h"
#include "../../../../macros.h"

extern PubSubClient mqttClient;
extern GlobalStatus_t status;

extern TwoWire Wire;

#pragma region Global Variables

MqttDeviceConfig_t Scd4xSensor::globalDeviceConfig;
MqttDeviceConfigMonitor_t Scd4xSensor::globalDeviceConfigMonitor;

MqttDeviceMqttSettings_t Scd4xSensor::globalDeviceMqttSettings;
MqttDeviceMqttSettingsMonitor_t Scd4xSensor::globalDeviceMqttSettingsMonitor;

Scd4xConfig_t Scd4xSensor::globalUniqueConfig;
Scd4xConfigMonitor_bm Scd4xSensor::globalUniqueConfigMonitor;

MqttDeviceGlobalStatus_t Scd4xSensor::globalDeviceStatus;

//SCD4xSpecialBaseTopics_t Scd4xSensor::globalUniqueBaseTopics;

const char* busyMessage = "%s cannot %s. Another action is being performed.";

char Scd4xSensor::errorMessage[];

#pragma endregion


#pragma region MqttDevice Functions



bool Scd4xSensor::Init()
{
	DEBUG_LOG_F("Initializing %s(SCD4x)\r\n", name.c_str());
	if (!status.device.i2cInitialized)
	{
		if (!EspSense::InitializeI2C())
			return false;
	}

	//if(!status.mqtt.devicesConfigured)
	//ResetStatus();

	sensor.begin(Wire);

	return MqttSensor::Init();

	/*ResetStatus(true);
	deviceStatus.enabled = false;
	deviceStatus.subscribed = false;
	sensorStatus.connected = false;

	if (Connect())
		return Configure();

	return false;*/
}

void Scd4xSensor::Loop()
{
	if (!deviceStatus.enabled) return;

	if (!sensorStatus.connected)
	{
		if(Connect())	
		{		
			MarkReconnected();	

			if (deviceStatus.configured)			
				DEBUG_LOG_F(MQTT_DMSG_RECONNECTED, name.c_str());
		}

		//if (!deviceStatus.configured)
		//	Configure();

		return;
	}

	if (uniqueStatus.performingFactoryReset)
	{
		PerformFactoryReset();
		return;
	}
	else if (uniqueStatus.performingSelfTest)
	{
		PerformSelfTest();
		return;
	}
	else if (uniqueStatus.performingCalibration)
	{
		PerformForcedCalibration(calibrationSettings.targetCo2Concentration, calibrationSettings.frcCorrection);
		return;
	}


	if (!uniqueStatus.periodicallyMeasuring && uniqueConfig.program.periodicMeasure)
	{
		StartPeriodicMeasurements();
	}
}

bool Scd4xSensor::Configure()
{
	if (!sensorStatus.connected)
	{
		DEBUG_LOG_F("Cannot configure %s(SCD4x) : Not Connected.\r\n", name.c_str());
		return false;
	}

	
	int errors = 0;
	DEBUG_LOG_F("Configuring %s(SCD4x)...", name.c_str());

#warning currently no way to save settings to EEPROM, so no way to load them. Will be added with browser tool.
	goto Label_SkipConfigTest;


	////If setup has completed and no settings have changed then return.
	//if (deviceStatus.configured && uniqueConfigMonitor.bitmap == 0)
	//{
	//	DEBUG_LOG_LN("Nothing to configure.");
	//	return true;
	//}


	//if (!deviceStatus.configured && uniqueConfig.internal.retainSettings)
	//{
	//	DEBUG_LOG_LN("Using data stored on Sensor's EEPROM. Warning : Settings must be configured before retainSettings has been set, or it will use the default settings that were stored on the device.");

	//	uniqueStatus.error = sensor.reinit();

	//	if (ReInit())
	//	{
	//		DEBUG_LOG_LN("...Configuration Complete.");
	//		return true;			
	//	}

	//	errors++;
	//	DisplayError("Error Configuring from Sensor's EEPROM", uniqueStatus.error, false);

	//	//Set flag to show that retainSettings was unable to be set.
	//	uniqueStatus.retainedSettingsInitialized = false;

	//	DEBUG_LOG_LN("Attempting to configure with ESP's Settings.");
	//}

	


EspSettings:
	DEBUG_LOG_LN("...From Config...");

	if (!status.config.setupComplete)
	{
		#warning move to their own functions.



		if(!SetAltitude(uniqueConfig.internal.altitude, !deviceStatus.configured)) errors++;
		if (!SetAmbientPressure(uniqueConfig.internal.ambientPressure, !deviceStatus.configured)) errors++;
		if (!SetTemperatureOffset(uniqueConfig.internal.tempOffset, !deviceStatus.configured)) errors++;
		if (!SetSelfCalibration(uniqueConfig.internal.selfCalibrate, !deviceStatus.configured)) errors++;
		//if (!SetRetainSettings(uniqueConfig.internal.retainSettings, !deviceStatus.configured)) errors++;

		
	}

	DEBUG_LOG_F("...Configuration Finished with %d/%d successful commands.\r\n", errors, 5);
	DEBUG_NEWLINE();
	Label_SkipConfigTest:
	deviceStatus.configured = true;

	//Set Period Measurements.
	if (uniqueConfig.program.periodicMeasure)
	{
		StartPeriodicMeasurements();
	}
	else
	{
		StopPeriodicMeasurements();
	}

	return true;
}


bool Scd4xSensor::Enable()
{
	if (deviceStatus.enabled) return true;

	DEBUG_LOG_F("Enabling %s (SCD4x)\r\n", name.c_str());


	if (!sensorStatus.connected)
		if (!Connect())
			return false;

	MqttDevice::Enable();

	if (!Wake()) 
		return false;

	if (uniqueConfig.program.periodicMeasure)
	{
		if (!StartPeriodicMeasurements()) return false;
	}

	uniqueStatus.performingSelfTest = false;
	uniqueStatus.reenable = false;
	//deviceStatus.enabled = true;

	return true;
}

bool Scd4xSensor::Disable()
{ 
	if (!deviceStatus.enabled) return true;	

	DEBUG_LOG_F("Disabling %s (SCD4x)\r\n", name);

	if (uniqueConfig.program.periodicMeasure)
	{
		if (!StopPeriodicMeasurements()) {
			DEBUG_LOG_F("Could not disable %s!\r\n", name.c_str());
		}
	}

	MqttDevice::Disable();
	deviceStatus.enabled = false;
	PowerDown();

	return true;
}

#if COMPILE_DEBUG_CODE == true
uint32_t successfulReads;
#endif

bool Scd4xSensor::Subscribe()
{
	return MqttDevice::Subscribe();

//#warning  Also remember about the useParents settings.
	//if (deviceMqttSettings.json)
	//{
	//	
	//}
	//else
	//{
	//	
	//}

	//return false;
}

bool Scd4xSensor::Unsubscribe()
{
	return MqttDevice::Unsubscribe();

	//if (deviceMqttSettings.json)
	//{
	//	
	//}
	//else
	//{
	//	
	//}

	//return false;
}

bool Scd4xSensor::Publish()
{	
	//if (!deviceStatus.enabled || !sensorStatus.newData) return false;
	if (uniqueStatus.standardOperationDisabled) return false;

	//if (measurementData.co2 == 0)
	//{
	//	DEBUG_LOG_F("%s cannot publish! Measurement data contains bad samples!\r\n", name.c_str());
	//	return false;
	//}

	return MqttSensor::Publish();
}


//bool Scd4xSensor::PublishAvailability()
//{
//	return mqttClient.publish(topics.availability.c_str(), Mqtt::Helper::GetAvailabilityString(sensorStatus.connected).c_str());
//}

void Scd4xSensor::AddStatePayload(JsonVariant& addTo)
{
	JsonObject obj = addTo;
	if (addTo.size() == 0)
		obj = addTo.createNestedObject("statePayload");

	if (uniqueConfig.mqtt.publishCo2)
	{
		JsonVariant co2 = obj.createNestedObject("co2");

		if (sensorStatus.connected)
			co2.set(measurementData.co2);
		else
			co2.set(SENSOR_DATA_UNKNOWN);
	}

	if (uniqueConfig.mqtt.publishTemperature)
	{
		JsonVariant temp = obj.createNestedObject("temp");

		if (sensorStatus.connected)
			temp.set(measurementData.temperature);
		else
			temp.set(SENSOR_DATA_UNKNOWN);
	}

	if (uniqueConfig.mqtt.publishHumdiity)
	{
		JsonVariant humidity = obj.createNestedObject("humidity");

		if (sensorStatus.connected)
			humidity.set(measurementData.humidity);
		else
			humidity.set(SENSOR_DATA_UNKNOWN);
	}
}

void Scd4xSensor::AddStatusData(JsonVariant& addTo)
{
	MqttSensor::AddStatusData(addTo);
	addTo["uniqueStatus"].set<SCD4xStatus_t>(uniqueStatus);
}

void Scd4xSensor::AddConfigData(JsonVariant& addTo)
{
	MqttDevice::AddConfigData(addTo);
	addTo["uniqueConfig"].set<Scd4xConfig_t>(uniqueConfig);
}


int Scd4xSensor::ReceiveCommand(char* topic, byte* payload, size_t length)
{
	//#warning  wip
		return 1;
}

//void* Scd4xSensor::GetUniqueConfig(){
//	return &uniqueConfig;
//}
//
//void* Scd4xSensor::GetUniqueConfigMonitor(){
//	return &uniqueConfigMonitor;
//}
//
//void* Scd4xSensor::GetGlobalUniqueConfig(){
//	return &globalUniqueConfig;
//}
//
//void* Scd4xSensor::GetGlobalUniqueConfigMonitor(){
//	return &globalUniqueConfigMonitor;
//}
//MqttDeviceConfig_t* Scd4xSensor::GetGlobalDeviceConfig(){
//	return &globalDeviceConfig;
//}
//MqttDeviceConfigMonitor_t* Scd4xSensor::GetGlobalDeviceConfigMonitor(){
//	return &globalDeviceConfigMonitor;
//}
//MqttDeviceMqttSettings_t* Scd4xSensor::GetGlobalMqttConfig(){
//	return &globalDeviceMqttSettings;
//}
//MqttDeviceMqttSettingsMonitor_t* Scd4xSensor::GetGlobalMqttConfigMonitor(){
//	return &globalDeviceMqttSettingsMonitor;
//}
MqttDeviceGlobalStatus_t* Scd4xSensor::GetGlobalDeviceStatus(){
	return &globalDeviceStatus;
}

void Scd4xSensor::GenerateTopics()
{
	MqttDevice::GenerateTopics();
}

void Scd4xSensor::SetDefaultSettings()
{
	MqttDevice::SetDefaultDeviceSettings(deviceConfig, deviceConfigMonitor);
	MqttDevice::SetDefaultMqttSettings(deviceMqttSettings, deviceMqttSettingsMonitor);
	SetDefaultUniqueSettings(uniqueConfig, uniqueConfigMonitor);
}

void Scd4xSensor::SetDefaultGlobalSettings()
{
	MqttDevice::SetDefaultDeviceSettings(globalDeviceConfig, globalDeviceConfigMonitor);
	MqttDevice::SetDefaultMqttSettings(globalDeviceMqttSettings, globalDeviceMqttSettingsMonitor);
	SetDefaultUniqueSettings(globalUniqueConfig, globalUniqueConfigMonitor);
}

void Scd4xSensor::SetDefaultUniqueSettings()
{
	SetDefaultUniqueSettings(uniqueConfig, uniqueConfigMonitor);
}

void Scd4xSensor::SetDefaultGlobalUniqueSettings()
{
	SetDefaultUniqueSettings(globalUniqueConfig, globalUniqueConfigMonitor);
}

void Scd4xSensor::SetDefaultMqttTopics()
{
	MqttDevice::SetDefaultMqttTopics(topics, deviceTopics, baseTopic, TOPIC_SENSOR_SCD4X_BASE, mqttSensorBaseTopic);

	//Add device specific topics here
}

//void Scd4xSensor::SetDefaultGlobalMqttTopics()
//{
//	globalUniqueBaseTopics.co2 = TOPIC_SENSOR_SCD4X_CO2_BASE;
//	globalUniqueBaseTopics.temperature = TOPIC_SENSOR_SCD4X_TEMP_BASE;
//	globalUniqueBaseTopics.humidity = TOPIC_SENSOR_SCD4X_HUMIDITY_BASE;
//}


void Scd4xSensor::ReadConfigObjectUnique(JsonVariantConst& scd4xObject)
{
	ReadConfigObjectUnique(scd4xObject, uniqueConfig, uniqueConfigMonitor);
}

void Scd4xSensor::ReadGlobalConfigObject(JsonVariantConst& obj)
{
	JsonVariantConst outTopicsObj;
	bool outHasTopics;
	ReadConfigObject(obj, deviceConfig, deviceConfigMonitor, deviceMqttSettings, deviceMqttSettingsMonitor/*, &outTopicsObj, &outHasTopics*/);
	//ReadTopicsObject(outTopicsObj);
	ReadConfigObjectUnique(obj);
}


void Scd4xSensor::ReadGlobalUniqueConfigObject(JsonVariantConst& scd4xObject)
{
	ReadConfigObjectUnique(scd4xObject, globalUniqueConfig, globalUniqueConfigMonitor);
}

//void Scd4xSensor::ReadTopicsObject(JsonObject& topicsObj)
//{
//
//	MqttDevice::ReadConfigMqttTopics(topicsObj, topics, deviceTopics, baseTopic, TOPIC_SENSOR_SCD4X_BASE, mqttSensorBaseTopic);
//
//	//make it change parents depending if json is null. if not, use baseTopic, if it is null use globalBase
//	Mqtt::CreateTopicString(&uniqueTopics.co2State, topicsObj["co2"], baseTopic, NULL, TOPIC_SENSOR_SCD4X_CO2_BASE);
//	Mqtt::CreateTopicString(&uniqueTopics.temperatureState, topicsObj["temperature"], baseTopic, NULL, TOPIC_SENSOR_SCD4X_TEMP_BASE);
//	Mqtt::CreateTopicString(&uniqueTopics.humidityState, topicsObj["humidity"], baseTopic, NULL, TOPIC_SENSOR_SCD4X_HUMIDITY_BASE);
//	//Read special topics here
//}
//
//void Scd4xSensor::ReadGlobalBaseTopicsObject(JsonObject& topicsObj)
//{
//	Mqtt::CreateBaseTopicString(topicsObj["co2"], &globalUniqueBaseTopics.co2, TOPIC_SENSOR_SCD4X_CO2_BASE);
//	Mqtt::CreateBaseTopicString(topicsObj["temperature"], &globalUniqueBaseTopics.temperature, TOPIC_SENSOR_SCD4X_TEMP_BASE);
//	Mqtt::CreateBaseTopicString(topicsObj["humidity"], &globalUniqueBaseTopics.humidity, TOPIC_SENSOR_SCD4X_HUMIDITY_BASE);
//
//	//if (topicsObj.containsKey("co2"))
//	//{		
//	//	const char* co2Var = topicsObj["co2"];
//
//	//	//If topic is not a base, do not set it.
//	//	if (co2Var[0] == '/')
//	//	{
//	//		globalUniqueBaseTopics.co2 = co2Var;
//	//	}
//	//}
//
//	//if (topicsObj.containsKey("temperature"))
//	//{
//	//	const char* tempVar = topicsObj["temperature"];
//
//	//	//If topic is not a base, do not set it.
//	//	if (tempVar[0] == '/')
//	//	{
//	//		globalUniqueBaseTopics.temperature = tempVar;
//	//	}
//	//}
//
//	//if (topicsObj.containsKey("humidity"))
//	//{
//	//	const char* humVar = topicsObj["humidity"];
//
//	//	//If topic is not a base, do not set it.
//	//	if (humVar[0] == '/')
//	//	{
//	//		globalUniqueBaseTopics.humidity = humVar;
//	//	}
//	//}
//}



void Scd4xSensor::SetDefaultUniqueSettings(Scd4xConfig_t& uConfig, Scd4xConfigMonitor_bm& uConfigMonitor)
{
	//Internal Monitor
	uConfigMonitor.internal.useDefaults = uConfig.internal.useDefaults != false;

	uConfigMonitor.internal.altitude = uConfig.internal.altitude != SCD4X_ALTITUDE;
	uConfigMonitor.internal.ambientPressure = uConfig.internal.ambientPressure != SCD4X_AMBIENTPRESSURE;
	uConfigMonitor.internal.retainSettings = uConfig.internal.retainSettings != SCD4X_RETAININTERNALLY;
	uConfigMonitor.internal.selfCalibrate = uConfig.internal.selfCalibrate != SCD4X_SELFCALIBRATE;
	uConfigMonitor.internal.tempOffset = uConfig.internal.tempOffset != SCD4X_TEMPOFFSET; //For some reason kernel error happens here while debuggin.


	//Program Monitor
	uConfigMonitor.program.useDefaults = uConfig.program.useDefaults != false;

	uConfigMonitor.program.bootSelfTest = uConfig.program.bootSelfTest != SCD4X_SELFTEST_ON_BOOT;
	uConfigMonitor.program.checkDataReady = uConfig.program.checkDataReady != SCD4X_CHECK_DATA_READY;
	uConfigMonitor.program.maxFailedReads = uConfig.program.maxFailedReads != SCD4X_MAX_FAILED_READS;
	uConfigMonitor.program.periodicMeasure = uConfig.program.periodicMeasure != SCD4X_PERIODICMEASUREMENTS;
	uConfigMonitor.program.powerMode = uConfig.program.powerMode != SCD4X_POWERMODE;
	uConfigMonitor.program.readRateAdditional = uConfig.program.readRateAdditional != SCD4X_READRATE_ADDITIONAL;
	uConfigMonitor.program.i2cPort = uConfig.program.i2cPort != SCD4X_I2C_PORT;
	


	//MQTT Monitor
	uConfigMonitor.mqtt.publishCo2 = uConfig.mqtt.publishCo2 != SCD4X_PUBLISH_CO2;
	uConfigMonitor.mqtt.publishTemperature = uConfig.mqtt.publishTemperature != SCD4X_PUBLISH_TEMPERATURE;
	uConfigMonitor.mqtt.publishHumdiity = uConfig.mqtt.publishHumdiity != SCD4X_PUBLISH_HUMIDITY;

	//Internal Settings
	uConfigMonitor.internal.useDefaults = false;

	uConfig.internal.altitude = SCD4X_ALTITUDE;
	uConfig.internal.ambientPressure = SCD4X_AMBIENTPRESSURE;
	uConfig.internal.retainSettings = SCD4X_RETAININTERNALLY;
	uConfig.internal.selfCalibrate = SCD4X_SELFCALIBRATE;
	uConfig.internal.tempOffset = SCD4X_TEMPOFFSET;


	//Program Settings
	uConfig.program.useDefaults = false;

	uConfig.program.bootSelfTest = SCD4X_SELFTEST_ON_BOOT;
	uConfig.program.checkDataReady = SCD4X_CHECK_DATA_READY;
	uConfig.program.maxFailedReads = SCD4X_MAX_FAILED_READS;
	uConfig.program.periodicMeasure = SCD4X_PERIODICMEASUREMENTS;
	uConfig.program.powerMode = SCD4X_POWERMODE;
	uConfig.program.readRateAdditional = SCD4X_READRATE_ADDITIONAL;
	uConfig.program.i2cPort = SCD4X_I2C_PORT;

	//MQTT
	uConfig.mqtt.publishCo2 = SCD4X_PUBLISH_CO2;
	uConfig.mqtt.publishTemperature = SCD4X_PUBLISH_TEMPERATURE;
	uConfig.mqtt.publishHumdiity = SCD4X_PUBLISH_HUMIDITY;
}

void Scd4xSensor::ReadConfigObjectUnique(JsonVariantConst& rootObj, Scd4xConfig_t& uConfig, Scd4xConfigMonitor_bm& uConfigMonitor)
{

	if (rootObj.containsKey("internal"))
	{
		JsonVariantConst internalObj = rootObj["internal"];

		//Mqtt::ReadKey(uConfig.internal.useDefaults, internalObj, "useDefaults");
		//Mqtt::ReadKey(uConfig.internal.altitude, internalObj, "altitude");
		//Mqtt::ReadKey(uConfig.internal.ambientPressure, internalObj, "ambientPressure");
		//Mqtt::ReadKey(uConfig.internal.retainSettings, internalObj, "retainSettings");
		//Mqtt::ReadKey(uConfig.internal.selfCalibrate, internalObj, "selfCalibrate");
		//Mqtt::ReadKey(uConfig.internal.tempOffset, internalObj, "tempOffset");

		if (internalObj.containsKey("useDefaults"))
			uConfig.internal.useDefaults = internalObj["useDefaults"];

		//if (uConfig.internal.useDefaults)
		//	goto SkipInternal;

		if (internalObj.containsKey("altitude"))
			uConfig.internal.altitude = internalObj["altitude"];

		if (internalObj.containsKey("ambientPressure"))
			uConfig.internal.ambientPressure = internalObj["ambientPressure"];

		if (internalObj.containsKey("retainSettings"))
			uConfig.internal.retainSettings = internalObj["retainSettings"];

		if (internalObj.containsKey("selfCalibrate"))
			uConfig.internal.selfCalibrate = internalObj["selfCalibrate"];

		if (internalObj.containsKey("tempOffset"))
			uConfig.internal.tempOffset = internalObj["tempOffset"];

	}

	SkipInternal:
	if (rootObj.containsKey("program"))
	{
		JsonVariantConst programObj = rootObj["program"];

		//Cannot use ReadKey with bit-fields.
		//Mqtt::ReadKey(uConfig.program.useDefaults, programObj, "useDefaults");
		//Mqtt::ReadKey(uConfig.program.periodicMeasure, programObj, "periodicMeasure");
		//Mqtt::ReadKey(uConfig.program.bootSelfTest, programObj, "bootSelfTest");
		//Mqtt::ReadKey(uConfig.program.readRateAdditional, programObj, "readRateAdditional");
		//Mqtt::ReadKey(uConfig.program.maxFailedReads, programObj, "maxFailedReads");

		if (programObj.containsKey("useDefaults"))
			uConfig.program.useDefaults = programObj["useDefaults"];

		if (programObj.containsKey("checkDataReady"))
			uConfig.program.checkDataReady = programObj["checkDataReady"];

		if (programObj.containsKey("periodicMeasure"))
			uConfig.program.periodicMeasure = programObj["periodicMeasure"];

		if (programObj.containsKey("bootSelfTest"))
			uConfig.program.bootSelfTest = programObj["bootSelfTest"];

		if (programObj.containsKey("readRateAdditional"))
			uConfig.program.readRateAdditional = programObj["readRateAdditional"];

		if (programObj.containsKey("maxFailedReads"))
			uConfig.program.maxFailedReads = programObj["maxFailedReads"];

		if (programObj.containsKey("i2cPort"))
			uConfig.program.i2cPort = programObj["i2cPort"];

		if (programObj.containsKey("powerMode"))
		{
#warning JSON UDF
			uConfig.program.powerMode = programObj["powerMode"].as<SCD4x_PowerMode>();
			//const char* pmchar = programObj["powerMode"];

			//if (strcmp(pmchar, "low") == 0)
			//{
			//	uConfig.program.powerMode = SCD4x_PowerMode::PM_LOW;
			//}
			//else if (strcmp(pmchar, "std") == 0 || strcmp(pmchar, "standard") == 0)
			//{
			//	uConfig.program.powerMode = SCD4x_PowerMode::PM_STANDARD;
			//}
			//else
			//{
			//	uint8_t pmint = programObj["powerMode"];
			//	if (pmint == 0 && pmint == 1)
			//	{
			//		uConfig.program.powerMode = (SCD4x_PowerMode)pmint;
			//	}
			//}
		}

	}

	if (rootObj.containsKey("mqtt"))
	{
		JsonVariantConst mqttObj = rootObj["mqtt"];

		if (mqttObj.containsKey("publishCo2"))
			uConfig.mqtt.publishCo2 = mqttObj["publishCo2"];

		if (mqttObj.containsKey("publishTemperature"))
			uConfig.mqtt.publishTemperature = mqttObj["publishTemperature"];

		if (mqttObj.containsKey("publishHumdiity"))
			uConfig.mqtt.publishHumdiity = mqttObj["publishHumdiity"];

	}
}



#pragma endregion

#pragma region MqttSensor Functions

bool Scd4xSensor::Connect()
{
	DEBUG_LOG_F("Connecting %s(SCD4x)\r\n", name.c_str());
	if (sensorStatus.connected)
		return true;

	IsConnected();

	if (sensorStatus.connected)
	{

		DEBUG_LOG_LN("SCD4x Connected");
		//DEBUG_LOG_LN("SCD4x Connected");
		//client.publish(topic_debug, "SCD4x Connected");

		//Just incase the ESP was reset when periodic measurements were started,
		//we will command it to stop so we can configure it.
		StopPeriodicMeasurements(true);

		//Perform test to see if sensor is working properly.
		if (uniqueConfig.program.bootSelfTest && !deviceStatus.configured)
		{
			//Wait 10 seconds incase periodic measurements were enabled.
			DEBUG_LOG_LN("SCD4x Performing Self-Test...");

			EspSense::YieldWatchdog(10000);

			uniqueStatus.error = sensor.performSelfTest(uniqueStatus.selfTestResults);

			if (uniqueStatus.error)
			{
				DisplayError("Self-Test failed. ");
			}
			else
			{
				char result[46];
				sprintf(result, "Self Test was successful. | Result = 0x%000x\r\n", uniqueStatus.selfTestResults);
				DEBUG_LOG_LN(result);
			}

		}

	}
	else
	{
		//Only display error message on setup, or if an interval has passed.
		if (!status.config.setupComplete || millis() > deviceStatus.publishErrorTimestamp + (MQTT_ERROR_PUBLISH_RATE * 1000))
		{
			DEBUG_LOG_LN("Could not connect to SCD4x");
			//DEBUG_LOG_LN("Could not connect to SCD4x");
			//client.publish(topic_debug, "Could not connect to SCD4x");
			deviceStatus.publishErrorTimestamp = millis();
		}
	}

	return sensorStatus.connected;
}

bool Scd4xSensor::IsConnected()
{
	DEBUG_LOG_F("Checking %s(SCD41) Connection...", name.c_str());
	/*
	As there is no function to check if the device is connected,
	we can try getting the serial number, if they contain data we have connected.
*/
	SCD4xSerialNumber_t serialNumber;

	bool currentStatus = sensorStatus.connected;

	//Don't display error in GetSerialNumber, it will call IsConnected causing an infinite loop.
	bool success = GetSerialNumber(serialNumber, false);

	//Now is the time to display the error if it exists.
	if (!success)
	{
		DisplayError(false);
	}

	sensorStatus.connected = success && (serialNumber.serial0 && serialNumber.serial1 && serialNumber.serial2);

	if (sensorStatus.connected)
		DEBUG_LOG_LN("Connected");

	if ((currentStatus || !status.mqtt.devicesConfigured) && !sensorStatus.connected)
	{
		MarkDisconnected();

		if (deviceStatus.configured)
			DEBUG_LOG_F(MQTT_DMSG_DISCONNECTED, name.c_str());

		ResetStatusPartial(false, false);
	}

	return sensorStatus.connected;
}

bool Scd4xSensor::Read()
{	
	if (!deviceStatus.enabled) 
		return false;

	//Already performing another task.
	if (uniqueStatus.standardOperationDisabled)
	{
		DEBUG_LOG_F(busyMessage, name.c_str(), "Read");
		return false;
	}

	//SCD41
	if (sensorStatus.connected)
	{

		if (uniqueStatus.skipNextMeasurement) return false;

		DEBUG_LOG_F("%s SCD41 Read() : Determining what action to take...", name.c_str());

		//Data ready, go directly to read.
		//dataReady is set in scd4x_dataReady().
		//This is here just incase that function is called elsewhere.
		if (uniqueStatus.dataReady)
			goto ReadSCD4x;

		//Measure single-shot
		if (!uniqueConfig.program.periodicMeasure && !uniqueStatus.measurementStarted)
		{
			DEBUG_LOG("Starting SCD41 Single-shot Measurement...");

			sensorStatus.measurementTimestamp = millis();
			uniqueStatus.error = sensor.measureSingleShot();
			sensorStatus.failedReads = 0;

			if (uniqueStatus.error)
			{
				DisplayError("...Error Starting SCD41 Measurement ");
				return false;
			}
			else
			{
				DEBUG_LOG("SCD41 Measurement Started...");
				uniqueStatus.powerMode = SCD4x_PowerMode::PM_STANDARD;
			}
		}


	CheckDataReady:

		//Check if measurement is ready to read.
		if (uniqueConfig.program.checkDataReady)
			IsDataReady(&uniqueStatus.error);

		//#warning  allowing read ignoring data ready.
		if (uniqueStatus.dataReady ||  (!uniqueConfig.program.checkDataReady && millis() > sensorStatus.measurementTimestamp + GetMinReadInterval()))
			{
			ReadSCD4x:
				DEBUG_LOG("Reading Data...");

				uniqueStatus.error = sensor.readMeasurement(measurementData.co2, measurementData.temperature, measurementData.humidity);

				//Reset flag.
				uniqueStatus.dataReady = false;
				

				if (uniqueStatus.error)
				{
					FailedRead();
					DEBUG_LOG_F("...Error Reading SCD41 Measurement | Code : %d Message : \r\n", uniqueStatus.error);
					DisplayError();
					return false;
				}
				else
				{
#if COMPILE_DEBUG_CODE == true
					successfulReads++;
					DEBUG_LOG("...Successful reads : ");
					DEBUG_LOG_LN(successfulReads);
#endif
					DEBUG_LOG_LN("...SCD4X Data Read.");
					//sensorStatus.measurementTimestamp = millis();
					sensorStatus.failedReads = 0;
					sensorStatus.newData = measurementData.co2 != 0;

					if (!sensorStatus.newData)
					{
						DEBUG_LOG_LN("...SCD4X Measurement Sample Bad!");
					}
					else
					{
						DEBUG_LOG("Success.\r\Co2 : ");
						DEBUG_LOG(measurementData.co2);
						DEBUG_LOG("ppm\r\nTemperature : ");
						DEBUG_LOG(measurementData.temperature);
						DEBUG_LOG("°C\r\nHumidity : ");
						DEBUG_LOG(measurementData.humidity);
						DEBUG_LOG_LN("%");
						DEBUG_NEWLINE();
					}

					return true;
				}
			}
	}

	return false;
}

bool Scd4xSensor::PowerDown()
{
	//Already performing another task.
	if (uniqueStatus.standardOperationDisabled)
	{
		DEBUG_LOG_F(busyMessage, name.c_str(), "PowerDown");
		return false;
	}

	if (deviceStatus.enabled)
		Disable();

	DEBUG_LOG_F("%s (SCD4x) powering down...\r\n", name);

	/*if (!CanSendCommand(0))
	{
		return false;
	}*/

	uniqueStatus.error = sensor.powerDown();

	if (uniqueStatus.error)
	{
		DisplayError("SCD41 Power down Failed", uniqueStatus.error);
		sensorStatus.sleeping = false;
	}
	else
	{
		DEBUG_LOG_LN("SCD41 Power down Complete.");
		sensorStatus.sleeping = true;
	}

	return uniqueStatus.error == 0;
}

bool Scd4xSensor::Wake()
{
	if (sensorStatus.sleeping)
	{
		DEBUG_LOG_F("Warking %s", name.c_str());
		if (uniqueStatus.error = sensor.wakeUp() != 0) //0 = no error
		{
			DisplayError("SCD41 Wake Up Failed", uniqueStatus.error);
			return false;
		}
		DEBUG_LOG_F("%s Is Awake", name.c_str());
	}

	return true;
}

#pragma endregion




#pragma region Private Functions





bool Scd4xSensor::IsDataReady(uint16_t* error)
{
	uint16_t dummy;
	return IsDataReady(error, &dummy);
}

bool Scd4xSensor::IsDataReady(uint16_t* error, uint16_t* result)
{
	//Already performing another task.
	if (uniqueStatus.standardOperationDisabled)
	{
		DEBUG_LOG_F(busyMessage, name.c_str(), "Check Data Ready");
		return false;
	}

	DEBUG_LOG_LN("Checking SCD41 Data Ready...");

	if (uniqueStatus.dataReady)
	{
		DEBUG_LOG_LN("Data Ready.");
		return true;
	}

	uniqueStatus.error = sensor.getDataReadyStatus(*result);

	if (uniqueStatus.error)
	{
		uniqueStatus.dataReady = false;
		DisplayError("Error Getting SCD41 Data Ready");
		FailedRead();
		return false;
	}

	//Check result. If last 11 bits are 0 data is not ready.
	bool ready = uniqueStatus.dataReady = *result & SCD4X_LAST_11_BITS;
	//uniqueStatus.dataReady = ready;

	//Print bits of result.
	DEBUG_LOG("Get Data Ready Result : ");
	for (int i = 0; i < 16; i++)
	{
		DEBUG_LOG(*result >> i & 1);
	}
	DEBUG_LOG_LN(); //Newline

	if (ready)
	{
		DEBUG_LOG_LN("Data Ready.");
		return true;
	}
	else
	{
		DEBUG_LOG_LN("Data Not Ready.");
		FailedRead();
		return false;
	}
}


bool Scd4xSensor::CanSendCommand(Scd4x_Command_Type command, uint16_t elapsed, bool fix)
{	bool triedToConnect = false;
CheckAgain:
	if (!sensorStatus.connected)
	{
		if (fix && !triedToConnect) {
			Connect(); triedToConnect = true;
			goto CheckAgain;
		}
		return false;
	}

	switch (command)
	{
		//These commands can be started when a measurement is taking place.
	case Scd4x_Command_Type::SCD4X_READ_MEASUREMENT:
	case Scd4x_Command_Type::SCD4X_STOP_PERIODIC_MEASUREMENT:
	case Scd4x_Command_Type::SCD4X_SET_AMBIENT_PRESSURE:
	case Scd4x_Command_Type::SCD4X_GET_DATA_READY_STATUS:
		break;
	default:
		if (elapsed && millis() < sensorStatus.measurementTimestamp + elapsed)
		{
			DEBUG_LOG_LN("Cannot Start Command. Not enough time has elapsed since last measurement.");
			if (fix) {
				uniqueStatus.skipNextMeasurement = true;
			} 
			return false;
		}
		else if (uniqueStatus.measurementStarted)
		{
			DEBUG_LOG_LN("Cannot Start Command as a measurement has been started.");
			if (fix) {
				uniqueStatus.skipNextMeasurement = true;
			}
			return false;
		}
		else if (uniqueStatus.periodicallyMeasuring)
		{
			if (fix) {
				StopPeriodicMeasurements();
				goto CheckAgain;
			}

			DEBUG_LOG_LN("Cannot Start Command until periodic measurements are disabled.");
			return false;
		}
	}


	

	if (sensorStatus.sleeping)
	{
		if (fix) {
			Wake();
			goto CheckAgain;
		}

		DEBUG_LOG_LN("Cannot Start Command until device is awake.");

		return false;
	}

	return true;
}

bool Scd4xSensor::StartPeriodicMeasurements()
{
	if (uniqueStatus.periodicallyMeasuring) return true;

	if (!uniqueStatus.periodicallyMeasuring)
	{
		//Already performing another task.
		if (uniqueStatus.standardOperationDisabled)
		{
			DEBUG_LOG_F(busyMessage, name.c_str(), "StartPeriodicMeasurements");
			return false;
		}
	}


	DEBUG_LOG_LN("Starting Periodic Measurements...");

	if (!CanSendCommand(Scd4x_Command_Type::SCD4X_START_PERIODIC_MEASUREMENT, 0, true))
	{
		return false;
	}

#if COMPILE_DEBUG == true
	PULSE_LED_EXT(1);
#endif

	switch (uniqueConfig.program.powerMode)
	{
	default:
	case SCD4x_PowerMode::PM_STANDARD:
		uniqueStatus.error = sensor.startPeriodicMeasurement();
		break;
	case SCD4x_PowerMode::PM_LOW:
		uniqueStatus.error = sensor.startLowPowerPeriodicMeasurement();
		break;
	}

	if (uniqueStatus.error)
	{
		DisplayError("Error Starting Periodic Measurements.");
		return false;
	}
	else
	{
		DEBUG_LOG_LN("Periodic Measurements Started.");
		uniqueStatus.powerMode = uniqueConfig.program.powerMode;
		uniqueStatus.periodicallyMeasuring = true;
		sensorStatus.measurementTimestamp = millis();
		return true;
	}
}

bool Scd4xSensor::StopPeriodicMeasurements(bool force)
{
	//if (!settings.scd4x.program.periodicallyMeasure) return;
	//settings.scd4x.program.periodicallyMeasure = true;

	if (!uniqueStatus.periodicallyMeasuring && !force) return true;

	DEBUG_LOG_LN("Stopping Periodic Measurements...");
	uniqueStatus.error = sensor.stopPeriodicMeasurement();

	if (uniqueStatus.error)
	{
		DisplayError("Error Stopping Periodic Measurements.");
		return false;
	}
	else
	{
		DEBUG_LOG_LN("Periodic Measurements Stopped.");
		uniqueStatus.periodicallyMeasuring = false;
		sensorStatus.measurementTimestamp = millis();
		return true;
	}
}

void Scd4xSensor::FailedRead()
{
	sensorStatus.failedReads++;
	DEBUG_LOG_LN("SCD4x Read Failed");

	DEBUG_LOG_F("Fail count = %d\r\n", sensorStatus.failedReads);

	if (sensorStatus.failedReads >= uniqueConfig.program.maxFailedReads && !uniqueStatus.periodicallyMeasuring)
	{
		sensorStatus.failedReads = 0;
		uniqueStatus.measurementStarted = false;
	}
}

bool Scd4xSensor::GetSerialNumber(SCD4xSerialNumber_t& serialNumber, bool displayError)
{
	DEBUG_LOG_F("Getting SCD4x(%s) Serial Number...", name.c_str());
	uniqueStatus.error = sensor.getSerialNumber(serialNumber.serial0, serialNumber.serial1, serialNumber.serial2);
	
	DEBUG_LOG_F("-SN : 0x%000x%000x%000x\r\n\r\n", serialNumber.serial0, serialNumber.serial1, serialNumber.serial2);

	if (uniqueStatus.error)
	{
		if (displayError)
			DisplayError();

		return false;
	}

	return true;
}

bool Scd4xSensor::PerformSelfTest()
{
	if (!uniqueStatus.performingSelfTest)
	{
		//Already performing another task.
		if (uniqueStatus.standardOperationDisabled)
		{
			DEBUG_LOG_F(busyMessage, name.c_str(), "Perform Self Test");
			return false;
		}

		uniqueStatus.performingSelfTest = true;
		uniqueStatus.standardOperationDisabled = true;
		uniqueStatus.reenable = deviceStatus.enabled;
	}

	DEBUG_LOG_LN("SCD41 Performing Self-Test...");



	if (!CanSendCommand(Scd4x_Command_Type::SCD4X_PERFORM_SELF_TEST, 0, true))
	{
		return false;
	}


	uniqueStatus.error = sensor.performSelfTest(uniqueStatus.selfTestResults);

	if (uniqueStatus.error)
	{
		DisplayError("SCD41 Self-Test Failed", uniqueStatus.error);
	}
	else
	{
		DEBUG_LOG_LN("SCD41 Self-Test Complete.");
		DEBUG_LOG_F("Result = %000X", uniqueStatus.selfTestResults);

//#warning  WIP all special functions

		//client.publish(TOPIC_SENSOR_SCD41_SELFTEST_RESULTS, (char*)&result);
	}

	if (uniqueStatus.reenable)
		Enable();

	uniqueStatus.performingSelfTest = false;
	uniqueStatus.standardOperationDisabled = false;

	return uniqueStatus.error == 0 /*&& !result*/;
}

bool Scd4xSensor::PerformFactoryReset()
{
	if (!uniqueStatus.performingFactoryReset)
	{
		//Already performing another task.
		if (uniqueStatus.standardOperationDisabled)
		{
			DEBUG_LOG_F(busyMessage, name.c_str(), "Perform Factory Reset");
			return false;
		}

		uniqueStatus.performingFactoryReset = true;
		uniqueStatus.standardOperationDisabled = true;
	}

	DEBUG_LOG_LN("SCD41 Performing Factory Reset...");

	if (!CanSendCommand(Scd4x_Command_Type::SCD4X_PERFORM_FACTORY_RESET, 30000, true))
	{
		return false;
	}

	uniqueStatus.error = sensor.performFactoryReset();

	if (uniqueStatus.error)
	{
		DisplayError("SCD41 Factory Reset Failed");
	}
	else
	{
		DEBUG_LOG_LN("SCD41 Factory Reset Complete.");
	}

	ResetStatusPartial(false, true);
	//uniqueStatus.dataReady = 0;
	//uniqueStatus.error = 0;
	//uniqueStatus.failedReads = 0;
	//uniqueStatus.measurementStarted = 0;
	//uniqueStatus.performingFactoryReset = 0;
	//uniqueStatus.performingSelfTest = 0;
	//uniqueStatus.periodicallyMeasuring = 0;
	//uniqueStatus.powerMode = SCD4x_PowerMode::PM_STANDARD;
	//uniqueStatus.reenable = 0;
	//uniqueStatus.selfTestResults = 0;
	//uniqueStatus.usingGlobalConfig = 0;
	//uniqueStatus.standardOperationDisabled = false;

	//sensorStatus.connected = false;
	//sensorStatus.measurementTimestamp = 0;
	//sensorStatus.sleeping = false;
	//sensorStatus.newData = 0;

	//deviceStatus.enabled = false;
	//deviceStatus.publishTimestamp = 0;
	//deviceStatus.publishErrorTimestamp = 0;

	//uniqueConfigMonitor.bitmap = 0xFFFFFFFF;

	Connect();
	Configure();


	return uniqueStatus.error == 0;
}

bool Scd4xSensor::PerformForcedCalibration(uint16_t targetCo2Concentration, uint16_t& frcCorrection)
{
	if (!uniqueStatus.performingCalibration)
	{
		//Already performing another task.
		if (uniqueStatus.standardOperationDisabled)
		{
			DEBUG_LOG_F(busyMessage, name.c_str(), "Perform Forced Calibration");
			return false;
		}

		uniqueStatus.performingCalibration = true;
		uniqueStatus.standardOperationDisabled = true;
	}

	calibrationSettings.targetCo2Concentration = targetCo2Concentration;
	calibrationSettings.frcCorrection = frcCorrection;

	DEBUG_LOG_LN("SCD41 Performing Forced-Calibration...");

	//Need to wait 30 seconds after last measurement.
	if (!CanSendCommand(Scd4x_Command_Type::SCD4X_PERFORM_FORCED_CALIBRATION, 30000, true))
	{
		return false;
	}

	uniqueStatus.error = sensor.performForcedRecalibration(targetCo2Concentration, frcCorrection);

	if (uniqueStatus.error)
	{
		DisplayError("SCD41 Forced-Calibration Failed", uniqueStatus.error);
	}
	else
	{
		DEBUG_LOG_LN("SCD41 Forced-Calibration Complete.");
	}

	uniqueStatus.performingCalibration = false;
	uniqueStatus.standardOperationDisabled = false;

	return !uniqueStatus.error;
}

bool Scd4xSensor::ReInit()
{
	//Already performing another task.
	if (uniqueStatus.standardOperationDisabled)
	{
		DEBUG_LOG_F(busyMessage, name.c_str(), "Re-initialize");
		return false;
	}

	DEBUG_LOG_LN("SCD41 Re-initializing...");

	if (!CanSendCommand(Scd4x_Command_Type::SCD4X_REINIT, 0, true))
	{
		return false;
	}

	uniqueStatus.error = sensor.reinit();

	if (uniqueStatus.error)
	{
		DisplayError("SCD41 Re-Initialization Failed", uniqueStatus.error);
	}
	else
	{
		DEBUG_LOG_LN("SCD41 Re-Initialization Complete.");
	}

	return !uniqueStatus.error;
}


bool Scd4xSensor::SetAltitude(uint16_t altitude, bool force)
{
	DEBUG_LOG_F("Setting %s(SCD4x) Altitude : %d\r\n", name.c_str(), altitude);

	uniqueConfigMonitor.internal.altitude = uniqueConfig.internal.altitude != altitude;
	uniqueConfig.internal.altitude = altitude;

	//Already configured
	if (!uniqueConfigMonitor.internal.altitude && deviceStatus.configured && !force) return true;

	//Invalid value.
	if (!IsValidAltitude(altitude)) return false;

	uniqueStatus.error = sensor.setSensorAltitude(altitude);

	if (uniqueStatus.error)
	{
		DisplayError("Error Setting Altitude", false);
		return false;
	}
	else
	{
		uniqueConfigMonitor.internal.altitude = false;
		return true;
	}
}

bool Scd4xSensor::SetAmbientPressure(uint16_t ambientPressure, bool force)
{
	DEBUG_LOG_F("Setting %s(SCD4x) Ambient Pressure : %d\r\n", name.c_str(), ambientPressure);

	uniqueConfigMonitor.internal.ambientPressure = uniqueConfig.internal.ambientPressure != ambientPressure;
	uniqueConfig.internal.ambientPressure = ambientPressure;

	//Already configured
	if (!uniqueConfigMonitor.internal.ambientPressure && deviceStatus.configured && !force) return true;

	//Invalid value.
	if (!IsValidAmbientPressure(ambientPressure)) return false;

	uniqueStatus.error = sensor.setAmbientPressure(ambientPressure);

	if (uniqueStatus.error)
	{
		DisplayError("Error Setting Ambient Pressure", false);
		return false;
	}
	else
	{
		uniqueConfigMonitor.internal.ambientPressure = false;
		return true;
	}
}

bool Scd4xSensor::SetTemperatureOffset(float tempOffset, bool force)
{
	DEBUG_LOG_F("Setting %s(SCD4x) Temperature Offset : ", name.c_str());
	DEBUG_LOG_LN(tempOffset);

	uniqueConfigMonitor.internal.tempOffset = uniqueConfig.internal.tempOffset != tempOffset;
	uniqueConfig.internal.tempOffset = tempOffset;

	//Already configured
	if (!uniqueConfigMonitor.internal.tempOffset && deviceStatus.configured && !force) return true;

	//Invalid value.
	if (!IsValidTemperatureOffset(tempOffset)) return false;

	uniqueStatus.error = sensor.setTemperatureOffset(tempOffset);

	if (uniqueStatus.error)
	{
		DisplayError("Error Setting Temperature Offset", false);
		return false;
	}
	else
	{
		uniqueConfigMonitor.internal.tempOffset = false;
		return true;
	}
}

bool Scd4xSensor::SetSelfCalibration(bool enable, bool force)
{
	DEBUG_LOG_F("Setting %s(SCD4x) Self-Calibration : %d\r\n", name.c_str(), enable);

	uniqueConfigMonitor.internal.selfCalibrate = uniqueConfig.internal.selfCalibrate != enable;
	uniqueConfig.internal.selfCalibrate = enable;

	//Already configured
	if (!uniqueConfigMonitor.internal.selfCalibrate && deviceStatus.configured && !force) return true;

	uniqueStatus.error = sensor.setAutomaticSelfCalibration(enable);

	if (uniqueStatus.error)
	{
		DisplayError("Error Setting Self-Calibration", false);
		return false;
	}
	else
	{
		uniqueConfigMonitor.internal.tempOffset = false;
		return true;
	}
}

bool Scd4xSensor::SetRetainSettings(bool retain, bool force)
{
	DEBUG_LOG("Setting %s(SCD4x) Retain Settings : ");
	DEBUG_LOG_LN(retain);

	uniqueConfigMonitor.internal.retainSettings = uniqueConfig.internal.retainSettings != retain;
	uniqueConfig.internal.retainSettings = retain;

	//Already configured
	if (!uniqueConfigMonitor.internal.retainSettings && deviceStatus.configured && !force) return true;

	if (retain)
	{
		uniqueStatus.error = sensor.persistSettings();

		if (uniqueStatus.error)
		{
			DisplayError("Error Setting Ambient Pressure", false);
			return false;
		}
		else
		{
			uniqueConfigMonitor.internal.retainSettings = false;
			return true;
		}
	}

	return true;
}



bool Scd4xSensor::IsValidAltitude(uint16_t altitude)
{
	#warning  wip
		return true;
}

bool Scd4xSensor::IsValidAmbientPressure(uint16_t altitude)
{
	#warning  wip
		return true;
}

bool Scd4xSensor::IsValidTemperatureOffset(float tempoff)
{
	#warning  wip
		return true;
}


bool Scd4xSensor::ReadInternalConfig(bool publish)
{
	//Already performing another task.
	if (uniqueStatus.standardOperationDisabled)
	{
		DEBUG_LOG_F(busyMessage, name.c_str(), "Read Internal Config");
		return false;
	}

	return false;
}

bool Scd4xSensor::PublishInternalData()
{
	return false;
}



uint32_t Scd4xSensor::GetMinReadInterval()
{
	switch (uniqueStatus.powerMode)
	{
	default:
	case SCD4x_PowerMode::PM_STANDARD:
		return  5000 + uniqueConfig.program.readRateAdditional;

	case SCD4x_PowerMode::PM_LOW:
		return 30000 + uniqueConfig.program.readRateAdditional;
	}
}


void Scd4xSensor::DisplayError(const char* message, uint16_t error, bool checkConnection)
{
	DEBUG_LOG_LN(message);
	DisplayError(error, checkConnection);
}

void Scd4xSensor::DisplayError(uint16_t error, bool checkConnection)
{
	//Check connection
	if (checkConnection)
		if (!IsConnected())
		{
			DEBUG_LOG_LN("Device not connected.");
			return;
		}
	DEBUG_LOG_F("...\r\n-Code : %d\r\n-Message : ", error);
	errorToString(error, errorMessage, 256);
	DEBUG_LOG_LN(errorMessage);
}

void Scd4xSensor::DisplayError(bool checkConnection)
{
	DisplayError(uniqueStatus.error, checkConnection);
}

void Scd4xSensor::DisplayError(const char* message, bool checkConnection)
{
	DisplayError(message, uniqueStatus.error, checkConnection);
}

void Scd4xSensor::ResetStatusPartial(bool resetEnabled, bool setUnconfigured, bool unique, bool device, bool sensor)
{
	if (unique)
	{
		memset(&uniqueStatus, 0, sizeof(SCD4xStatus_t));
	}

	if (device)
	{
		if (setUnconfigured)
		{
			uniqueConfigMonitor.bitmap = 0xFFFFFFFF;
			deviceStatus.configured = false;
			deviceStatus.subscribed = false;
		}

		if (resetEnabled)
			deviceStatus.enabled = false;

		deviceStatus.publishErrorTimestamp = 0;
		deviceStatus.publishTimestamp = 0;		
	}

	if (sensor)
	{
		sensorStatus.measurementTimestamp = 0;
		sensorStatus.newData = false;
		sensorStatus.sleeping = false;
	}	
}

#pragma endregion


#pragma region JSON UDFs

const char* scd4x_powermode_strings[2] = { "std" , "low"};


bool canConvertFromJson(JsonVariantConst src, const SCD4x_PowerMode&)
{
	return JsonHelper::JsonParseEnum(src, 2, scd4x_powermode_strings, nullptr) != -1;
}

void convertFromJson(JsonVariantConst src, SCD4x_PowerMode& dst)
{
	JsonHelper::UdfHelperConvertFromJsonEnums(src, (EnumClass_t&)dst, 2, "SCD4x_PowerMode", scd4x_powermode_strings, nullptr);
}

bool convertToJson(const SCD4x_PowerMode& src, JsonVariant dst)
{
	JsonHelper::UdfHelperConvertToJsonEnums((EnumClass_t&)src, dst, 2, "SCD4x_PowerMode", scd4x_powermode_strings, nullptr);
}

//void convertFromJson(JsonVariantConst src, SCD4x_PowerMode& dst)
//{
//	bool success;
//	SCD4x_PowerMode parseResult = (SCD4x_PowerMode)JsonHelper::JsonParseEnum(src, 2, scd4x_powermode_strings, nullptr, &success);
//
//	if (success)
//		dst = parseResult;
//	else
//		DEBUG_LOG_LN("SCD4x_PowerMode Parsing Failed");
//}
//
//bool convertToJson(const SCD4x_PowerMode& src, JsonVariant dst)
//{
//#if SERIALIZE_ENUMS_TO_STRING
//	bool set = JsonHelper::EnumValueToJson(dst, (uint8_t)src, scd4x_powermode_strings, 2);
//#else
//	bool set = dst.set((uint8_t)src);
//#endif
//
//	if (set) return true;
//
//	DEBUG_LOG_LN("SCD4x_PowerMode Conversion to JSON failed.");
//	return false;
//}




//bool canConvertFromJson(JsonVariantConst src, const SCD4xStatus_t&)
//{
//	return src.containsKey("periodicallyMeasuring") || src.containsKey("uniqueStatus");
//}

//void convertFromJson(JsonVariantConst src, SCD4xStatus_t& dst)
//{
//	JsonVariantConst uniqueStatusObj = src;
//
//	if (src.containsKey("uniqueStatus"))
//		uniqueStatusObj = src["uniqueStatus"];
//
//
//	if (uniqueStatusObj.containsKey("measurementStarted"))
//		dst.measurementStarted = uniqueStatusObj["measurementStarted"];
//
//	if (uniqueStatusObj.containsKey("periodicallyMeasuring"))
//		dst.periodicallyMeasuring = uniqueStatusObj["periodicallyMeasuring"];
//
//	if (uniqueStatusObj.containsKey("dataReady"))
//		dst.dataReady = uniqueStatusObj["dataReady"];
//
//	if (uniqueStatusObj.containsKey("skipNextMeasurement"))
//		dst.skipNextMeasurement = uniqueStatusObj["skipNextMeasurement"];
//
//	if (uniqueStatusObj.containsKey("performingSelfTest"))
//		dst.performingSelfTest = uniqueStatusObj["performingSelfTest"];
//
//	if (uniqueStatusObj.containsKey("performingFactoryReset"))
//		dst.performingFactoryReset = uniqueStatusObj["performingFactoryReset"];
//
//	if (uniqueStatusObj.containsKey("performingCalibration"))
//		dst.performingCalibration = uniqueStatusObj["performingCalibration"];
//
//	if (uniqueStatusObj.containsKey("reenable"))
//		dst.reenable = uniqueStatusObj["reenable"];
//
//	if (uniqueStatusObj.containsKey("powerMode"))
//		dst.powerMode = uniqueStatusObj["powerMode"].as<SCD4x_PowerMode>();
//
//	if (uniqueStatusObj.containsKey("error"))
//		dst.error = uniqueStatusObj["error"];
//
//	if (uniqueStatusObj.containsKey("selfTestResults"))
//		dst.selfTestResults = uniqueStatusObj["selfTestResults"];
//}

bool convertToJson(const SCD4xStatus_t& src, JsonVariant dst)
{
	dst["measurementStarted"] = src.measurementStarted;
	dst["periodicallyMeasuring"] = src.periodicallyMeasuring;
	dst["dataReady"] = src.dataReady;
	dst["skipNextMeasurement"] = src.skipNextMeasurement;
	dst["performingSelfTest"] = src.performingSelfTest;
	dst["performingFactoryReset"] = src.performingFactoryReset;
	dst["performingCalibration"] = src.performingCalibration;
	dst["reenable"] = src.reenable;
	dst["powerMode"].set(src.powerMode);
	dst["error"] = src.error;
	dst["selfTestResults"] = src.selfTestResults;
}


//bool canConvertFromJson(JsonVariantConst src, const Scd4xConfig_t&)
//{
//	return src.containsKey("internal") || src.containsKey("program");
//}

void convertFromJson(JsonVariantConst src, Scd4xConfig_t& dst)
{
	JsonVariantConst root = src;

	if (src.containsKey("uniqueStatus"))
		root = src["uniqueStatus"];


	if (root.containsKey("internal"))
	{
		JsonVariantConst obj = root["internal"];

		if (obj.containsKey("useDefaults"))
			dst.internal.useDefaults = obj["useDefaults"];

		if (obj.containsKey("altitude"))
			dst.internal.altitude = obj["altitude"];

		if (obj.containsKey("ambientPressure"))
			dst.internal.ambientPressure = obj["ambientPressure"];

		if (obj.containsKey("tempOffset"))
			dst.internal.tempOffset = obj["tempOffset"];

		if (obj.containsKey("retainSettings"))
			dst.internal.retainSettings = obj["retainSettings"];

		if (obj.containsKey("selfCalibrate"))
			dst.internal.selfCalibrate = obj["selfCalibrate"];
	}

	if (root.containsKey("program"))
	{
		JsonVariantConst obj = root["program"];

		if (obj.containsKey("useDefaults"))
			dst.program.useDefaults = obj["useDefaults"];

		if (obj.containsKey("bootSelfTest"))
			dst.program.bootSelfTest = obj["bootSelfTest"];

		if (obj.containsKey("checkDataReady"))
			dst.program.checkDataReady = obj["checkDataReady"];

		if (obj.containsKey("i2cPort"))
			dst.program.i2cPort = obj["i2cPort"];

		if (obj.containsKey("maxFailedReads"))
			dst.program.maxFailedReads = obj["maxFailedReads"];

		if (obj.containsKey("periodicMeasure"))
			dst.program.periodicMeasure = obj["periodicMeasure"];

		if (obj.containsKey("powerMode"))
			dst.program.powerMode = obj["powerMode"].as<SCD4x_PowerMode>();

		if (obj.containsKey("readRateAdditional"))
			dst.program.readRateAdditional = obj["readRateAdditional"];
	}

	if (root.containsKey("mqtt"))
	{
		JsonVariantConst obj = root["mqtt"];

		if (obj.containsKey("publishCo2"))
			dst.mqtt.publishCo2 = obj["publishCo2"];

		if (obj.containsKey("publishHumdiity"))
			dst.mqtt.publishHumdiity = obj["publishHumdiity"];

		if (obj.containsKey("publishTemperature"))
			dst.mqtt.publishTemperature = obj["publishTemperature"];
	}

}

bool convertToJson(const Scd4xConfig_t& src, JsonVariant dst)
{
	JsonObject internalObj = dst.createNestedObject("internal");
	internalObj["useDefaults"] = src.internal.useDefaults;
	internalObj["altitude"] = src.internal.altitude;
	internalObj["ambientPressure"] = src.internal.ambientPressure;
	internalObj["tempOffset"] = src.internal.tempOffset;
	internalObj["retainSettings"] = src.internal.retainSettings;
	internalObj["selfCalibrate"] = src.internal.selfCalibrate;

	JsonObject programObj = dst.createNestedObject("program");
	programObj["useDefaults"] = src.program.useDefaults;
	programObj["bootSelfTest"] = src.program.bootSelfTest;
	programObj["checkDataReady"] = src.program.checkDataReady;
	programObj["i2cPort"] = src.program.i2cPort;
	programObj["maxFailedReads"] = src.program.maxFailedReads;
	programObj["periodicMeasure"] = src.program.periodicMeasure;
	programObj["powerMode"].set<SCD4x_PowerMode>(src.program.powerMode);
	programObj["readRateAdditional"] = src.program.readRateAdditional;

	JsonObject mqttObj = dst.createNestedObject("mqtt");
	mqttObj["publishCo2"] = src.mqtt.publishCo2;
	mqttObj["publishHumdiity"] = src.mqtt.publishHumdiity;
	mqttObj["publishTemperature"] = src.mqtt.publishTemperature;
}

#pragma endregion