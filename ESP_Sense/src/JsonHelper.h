#ifndef _JSONHELPER_h
#define _JSONHELPER_h

#include <arduino.h>

#include <functional>

#include <ArduinoJson.hpp>
#include <ArduinoJson.h>
#include <IPAddress.h>

#include "GlobalDefs.h"

namespace JsonHelper
{

	DynamicJsonDocument* CreateDocument(size_t size);

	typedef std::function<int(JsonObject& document)> PACK_JSON_FUNC;
	//typedef int (*PACK_JSON_FUNC)(JsonObject& document);
	size_t CreatePackAndSerialize(size_t docSize, String& serializeTo, DynamicJsonDocument** out_doc, PACK_JSON_FUNC packFunction);
	DynamicJsonDocument* CreateAndPackDocument(size_t docSize, PACK_JSON_FUNC packFunction);

	/// <summary>
	/// Parses an enum from a JsonVariant.
	/// JsonVariant can be a string, value or index.
	/// </summary>
	/// <param name="jvar">JsonVariant containing a string representation, value or index.</param>
	/// <param name="len">How many values the enum contains.</param>
	/// <param name="strings">Array of the values represented as strings.</param>
	/// <param name="values">Array of the true value for each element of the enum, in order. *Only required if enum values do not start incrementing from 0.</param>
	/// <param name="success">Out value to get if the parsing was successful. (For using when the enum contains a -1 value)</param>
	/// <returns>True value of the enum, or -1 if something failed.</returns>
	int JsonParseEnum(JsonVariantConst& jvar, const uint8_t len, const char** strings = nullptr, int* values = nullptr, bool* success = nullptr);


	/// <summary>
	/// Finds the index of an enums value, and sets a JsonVariant to that value. 
	/// </summary>
	/// <param name="jvar"></param>
	/// <param name="value"></param>
	/// <param name="values"></param>
	/// <param name="len"></param>
	/// <param name="enumContainsNeg1">If enum contains negative one the function will have to do a special check to see if it failed.</param>
	/// <returns></returns>
	bool EnumValueToJson(JsonVariant& jvar, int value, int* values, const uint8_t len, bool enumContainsNeg1 = false);

	/// <summary>
	/// Sets a JsonVariant to a string representation of an enum.
	/// </summary>
	/// <param name="jvar"></param>
	/// <param name="value"></param>
	/// <param name="strings"></param>
	/// <param name="len"></param>
	/// <returns></returns>
	bool EnumValueToJson(JsonVariant& jvar, int value, const char** strings, const uint8_t len, int* values = nullptr);

}

#pragma region User Defined Functions
#warning move UDFs to the files which contain the type.

//Dummy definition so ArduinoJson can get function by type.
enum class WifiPower {};

//ConfigSource
bool canConvertFromJson(JsonVariantConst src, const ConfigSource&);
void convertFromJson(JsonVariantConst src, ConfigSource& dst);
bool convertToJson(const ConfigSource& src, JsonVariant dst);

//TaskConfig_t
bool canConvertFromJson(JsonVariantConst src, const TaskConfig_t&);
void convertFromJson(JsonVariantConst src, TaskConfig_t& dst);
bool convertToJson(const TaskConfig_t& src, JsonVariant dst);

//IPAddress
bool canConvertFromJson(JsonVariantConst src, const IPAddress&);
void convertFromJson(JsonVariantConst src, IPAddress& dst);
bool convertToJson(const IPAddress& src, JsonVariant dst);

//wifi_power_t
bool canConvertFromJson(JsonVariantConst src, const WifiPower&);
void convertFromJson(JsonVariantConst src, WifiPower& dst);
bool convertToJson(const WifiPower& src, JsonVariant dst);

////DeviceUpdaterMode_t
//bool canConvertFromJson(JsonVariantConst src, const DeviceUpdaterMode&);
//void convertFromJson(JsonVariantConst src, DeviceUpdaterMode& dst);
//bool convertToJson(const DeviceUpdaterMode& src, JsonVariant dst);

//ConfigAutobackupMode_t
bool canConvertFromJson(JsonVariantConst src, const ConfigAutobackupMode&);
void convertFromJson(JsonVariantConst src, ConfigAutobackupMode& dst);
bool convertToJson(const ConfigAutobackupMode& src, JsonVariant dst);

//WifiMode_t
bool canConvertFromJson(JsonVariantConst src, const WifiMode&);
void convertFromJson(JsonVariantConst src, WifiMode& dst);
bool convertToJson(const WifiMode& src, JsonVariant dst);

//UART_DATABITS
bool canConvertFromJson(JsonVariantConst src, const UART_DATABITS&);
void convertFromJson(JsonVariantConst src, UART_DATABITS& dst);
bool convertToJson(const UART_DATABITS& src, JsonVariant dst);

//UART_PARITY
bool canConvertFromJson(JsonVariantConst src, const UART_PARITY&);
void convertFromJson(JsonVariantConst src, UART_PARITY& dst);
bool convertToJson(const UART_PARITY& src, JsonVariant dst);

//UART_STOPBITS
bool canConvertFromJson(JsonVariantConst src, const UART_STOPBITS&);
void convertFromJson(JsonVariantConst src, UART_STOPBITS& dst);
bool convertToJson(const UART_STOPBITS& src, JsonVariant dst);

#pragma endregion

#endif

