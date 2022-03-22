#ifndef _SENSOR_SCD4X_h
#define _SENSOR_SCD4X_h

#include <Arduino.h>

#include <ArduinoJson.hpp>
#include <ArduinoJson.h>
#include <PubSubClient.h>

#include <Wire.h>               //I2C

#include <SensirionCore.h>
#include <SensirionI2CScd4x.h>

#include "../../../../Config/config_master.h"
#include "../../../../Config/global_status.h"
#include "scd4x_config.h"

#include "../../MqttDevice.h"
#include "../MqttSensor.h"

#include "../../../../HelperFunctions.h"



class Scd4xSensor : public MqttSensor
{
public:

	static const char* deviceName;		//Used for print statements
	static const char* deviceKey;		//Used for keys, and filtering

	const char* DeviceName() override
	{
		return deviceName;
	}
	const char* DeviceKey() override
	{
		return deviceKey;
	}

	/// <summary>
	/// Sensor object which controls the actual sensor.
	/// </summary>
	SensirionI2CScd4x sensor;

#pragma region Global Variables

	/// <summary>
	/// Global MQTT Device Configuration
	/// </summary>
	static MqttDeviceConfig_t globalDeviceConfig;
	static MqttDeviceConfigMonitor_t globalDeviceConfigMonitor;

	/// <summary>
	/// Global MQTT Settings
	/// </summary>
	static MqttDeviceMqttSettings_t globalDeviceMqttSettings;
	static MqttDeviceMqttSettingsMonitor_t globalDeviceMqttSettingsMonitor;



	/// <summary>
	/// SCD4X Configuration
	/// </summary>
	static Scd4xConfig_t globalUniqueConfig;
	static Scd4xConfigMonitor_bm globalUniqueConfigMonitor;

	static MqttDeviceGlobalStatus_t globalDeviceStatus;

	//static SCD4xSpecialBaseTopics_t globalUniqueBaseTopics;

	static char errorMessage[256];

	//static const char* deviceName;

#pragma endregion

#pragma region Variables

	Scd4xConfig_t uniqueConfig;
	Scd4xConfigMonitor_bm uniqueConfigMonitor;

	/// <summary>
	/// SCD4x Status
	/// </summary>
	SCD4xStatus_t uniqueStatus;

	//SCD4xSpecialTopics_t uniqueTopics;

	struct {
		uint16_t co2;
		float temperature;
		float humidity;
	}measurementData;

	struct {
		uint16_t targetCo2Concentration;
		uint16_t frcCorrection;
		float tempOffsetCorrection;
	}calibrationSettings;

#pragma endregion
	
	//SCD4xPreservedStatus_t scd4xPreservedStatus;

	const char* GetConfigFilePath() override {
		return SCD4X_CONFIG_PATH;
	}
	const char* GetConfigFileName() override {
		return SCD4X_CONFIG_NAME;
	}
	const char* GetConfigObjKey() override {
		return SCD4X_CONFIG_OBJECT_KEY;
	}
	const char* GetDefaultBaseTopic() override {
		return TOPIC_SENSOR_SCD4X_BASE;
	}
	String* GetParentBaseTopic() override {
		return &mqttSensorBaseTopic;
	}

	Scd4xSensor(const char* _name, int _index, int _subIndex) : MqttSensor(_name,  _index, _subIndex) 
	{ 
		ResetStatus();
	}

#pragma region MqttDevice Functions


	bool Init() override;

	void ResetStatus() override
	{
		memset(&uniqueStatus, 0, sizeof(SCD4xStatus_t));
		MqttSensor::ResetStatus();
	}

	void Loop() override;

	bool Configure() override;
	
	bool Enable() override;
	bool Disable() override;
	
	bool Subscribe() override;
	bool Unsubscribe() override;
	bool Publish() override;
	//bool PublishAvailability() override;

	void AddStatePayload(JsonVariant& addTo, bool nest = true) override;				//Payload for MQTT state topic
	void AddStatusData(JsonVariant& addTo) override;					//device, binary/sensor/ect.., and unique status
	void AddConfigData(JsonVariant& addTo) override;					//device, binary/sensor/ect.., and unique config

	int ReceiveCommand(char* topic, byte* payload, size_t length) override;

	template<class T>
	int GetSensorObject(T** object)
	{
		*object = &sensor;
		return 0;
	};

//protected:

	//void* GetUniqueConfig() override;
	//void* GetUniqueConfigMonitor() override;
	//void* GetGlobalUniqueConfig() override;
	//void* GetGlobalUniqueConfigMonitor() override;
	//MqttDeviceConfig_t* GetGlobalDeviceConfig()override;
	//MqttDeviceConfigMonitor_t* GetGlobalDeviceConfigMonitor()override;
	//MqttDeviceMqttSettings_t* GetGlobalMqttConfig() override;
	//MqttDeviceMqttSettingsMonitor_t* GetGlobalMqttConfigMonitor() override;
	MqttDeviceGlobalStatus_t* GetGlobalDeviceStatus() override;

	void GenerateTopics() override;

	void SetDefaultSettings() override;
	void SetDefaultGlobalSettings() override;

	void SetDefaultUniqueSettings() override;
	void SetDefaultGlobalUniqueSettings() override;
	void SetDefaultMqttTopics() override;
	//void SetDefaultGlobalMqttTopics() override;

	//bool ReadGlobalConfig() override;
	void ReadConfigObjectUnique(JsonVariantConst& obj) override;
	void ReadGlobalConfigObject(JsonVariantConst& obj) override;
	void ReadGlobalUniqueConfigObject(JsonVariantConst& obj) override;
	//void ReadTopicsObject(JsonObject& topicsObj) override;
	//void ReadGlobalBaseTopicsObject(JsonObject& topicsObj) override;
	

private:
	/// <summary>
	/// Set the settings unique to the SCD4X to default.
	/// Pass either global or non global structures.
	/// </summary>
	static void SetDefaultUniqueSettings(Scd4xConfig_t& uConfig, Scd4xConfigMonitor_bm& uConfigMonitor);

	/// <summary>
	/// Parse the unique objects from either the scd4x.json, or the "config" key from one of the sensors in the array in config_sensors.json.
	/// Pass either global or non global structures.
	/// </summary>
	/// <param name="obj"></param>
	/// <param name="uConfig"></param>
	/// <param name="uConfigMonitor"></param>
	static void ReadConfigObjectUnique(JsonVariantConst& obj, Scd4xConfig_t& uConfig, Scd4xConfigMonitor_bm& uConfigMonitor);

#pragma endregion

#pragma region MqttSensor Functions
	public:
	bool Connect() override;
	bool IsConnected() override;
	bool Read() override;
	bool PowerDown() override;
	bool Wake() override;
#pragma endregion
	
	//bool SetI2cPort(TwoWire* port);


private:

	/// <summary>
	/// Checks to see if the SCD4x's data is ready to read.
	/// *If the last 11 bits are 0, the data is not ready.
	/// </summary>
	/// <param name="error">Out: I2C Error code</param>
	/// <returns>If data is ready.</returns>
	bool IsDataReady(uint16_t* error);

	bool IsDataReady(uint16_t* error, uint16_t* result);

	/// <summary>
	/// Checks to see if a measurement is taking place.
	/// Certain commands can only be sent when the device is "idle."
	/// </summary>
	/// <returns>If the commmand is allowed to be sent.</returns>
	bool CanSendCommand(Scd4x_Command_Type command, uint16_t elapsed = 0, bool fix = false);

	bool StartPeriodicMeasurements();
	bool StopPeriodicMeasurements( bool force = false);

	/// <summary>
	/// Increased failed read counter.
	/// If failed reads reach the configured max, it will allow another measurement to be started.
	/// </summary>
	void FailedRead() override;

	bool GetSerialNumber(SCD4xSerialNumber_t& serialNumber, bool displayError = true);

	bool PerformSelfTest();
	bool PerformFactoryReset();

	bool PerformForcedCalibration(uint16_t targetCo2Concentration, uint16_t& frcCorrection);

	bool ReInit();

#warning add Get functions when browser page is added.
	bool SetAltitude(uint16_t altitude, bool force = false);
	bool SetAmbientPressure(uint16_t ambientPressure, bool force = false);
	bool SetTemperatureOffset(float tempOffset, bool force = false);
	bool SetSelfCalibration(bool enable, bool force);
	bool SetRetainSettings(bool retain, bool force);

	static bool IsValidAltitude(uint16_t altitude);

	static bool IsValidAmbientPressure(uint16_t ambientPressure);

	static bool IsValidTemperatureOffset(float tempoff);

	//static uint16_t AltitudeConversion();

	//static uint16_t AmbientPressureConversion();

	//static uint16_t TemperatureOffsetConversion();

	/// <summary>
	/// Reads the configuration stored on the sensor's EEPROM.
	/// </summary>
	bool ReadInternalConfig(bool publish);

	/// <summary>
	/// Publishes the data read from the internal EEPROM.
	/// </summary>
	/// <returns></returns>
	bool PublishInternalData();

	/// <summary>
	/// Gets the minimum read interval for the SCD4x, depending on its power mode.
	/// </summary>
	/// <returns>Minimum interval in milliseconds.</returns>
	uint32_t GetMinReadInterval();

	/// <summary>
	/// Converts an error code into a message, and displays it with a message prepended.
	/// </summary>
	/// <param name="message">Message to display before error.</param>
	/// <param name="error">Error code</param>
	void DisplayError(const char* message, uint16_t error, bool checkConnection = true);

	/// <summary>
	/// Display an error
	/// </summary>
	/// <param name="error">Error code</param>
	void DisplayError(uint16_t error, bool checkConnection);

	/// <summary>
	/// Display last error.
	/// </summary>
	void DisplayError(bool checkConnection = true);

	/// <summary>
	/// Display last error with a message before.
	/// </summary>
	/// <param name="message">Message to display before error.</param>
	void DisplayError(const char* message, bool checkConnection = true);
	
	void ResetStatusPartial(bool resetEnabled, bool setUnconfigured, bool unique = true, bool device = true, bool sensor = true) ;
};

#pragma region Json UDFs

extern const char* scd4x_powermode_strings[2];
//bool canConvertFromJson(JsonVariantConst src, const SCD4x_PowerMode&);
void convertFromJson(JsonVariantConst src, SCD4x_PowerMode& dst);
bool convertToJson(const SCD4x_PowerMode& src, JsonVariant dst);

//bool canConvertFromJson(JsonVariantConst src, const SCD4xStatus_t&);
//void convertFromJson(JsonVariantConst src, SCD4xStatus_t& dst);
bool convertToJson(const SCD4xStatus_t& src, JsonVariant dst);

//bool canConvertFromJson(JsonVariantConst src, const Scd4xConfig_t&);
void convertFromJson(JsonVariantConst src, Scd4xConfig_t& dst);
bool convertToJson(const Scd4xConfig_t& src, JsonVariant dst);

#pragma endregion


#endif

