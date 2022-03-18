#include "JsonHelper.h"
#include "HelperFunctions.h"
#include "EspSenseValidation.h"
#include "Macros.h"

extern HardwareSerial* serial;			//Message
extern HardwareSerial* serialDebug;		//Debug

DynamicJsonDocument* JsonHelper::CreateDocument(size_t size)
{
	DEBUG_LOG_LN("Creating JSON Document...");

	//Not enough memory on the heap.
	if (ESP.getFreeHeap() < size + 2048)
	{
		DEBUG_LOG_LN("NOT ENOUGH MEMORY ON THE HEAP!");
		return nullptr;
	}

	DynamicJsonDocument* doc = new DynamicJsonDocument(size);

	return doc;
}



size_t JsonHelper::CreatePackAndSerialize(size_t docSize, String& serializeTo, DynamicJsonDocument** out_doc, PACK_JSON_FUNC packFunction)
{
	if (serializeTo == nullptr && out_doc == nullptr) return 0;

	DynamicJsonDocument* doc = JsonHelper::CreateAndPackDocument(docSize, packFunction);

	if (doc == nullptr) return 0;

	size_t size = serializeJson(*doc, serializeTo);

	if (out_doc != nullptr)
	{
		*out_doc == doc;
	}
	else
	{
		doc->clear();
		free(doc);
	}

	return size;
}

DynamicJsonDocument* JsonHelper::CreateAndPackDocument(size_t docSize, PACK_JSON_FUNC packFunction)
{
	DEBUG_LOG_LN("CreateAndPackDocument");
	DynamicJsonDocument* doc = JsonHelper::CreateDocument(docSize);
	
	if (doc == nullptr) 
		return nullptr;

	JsonObject docObj = doc->as<JsonObject>();
	packFunction(docObj);
	return doc;
}

int JsonHelper::JsonParseEnum(JsonVariantConst& jvar, const uint8_t len, const char** strings, int* values, bool* success)
{
	int e = -1;

if (success != nullptr)
		*success = true;

	if (jvar.is<const char*>() && strings != nullptr)
	{
		String s = jvar.as<String>();

		DEBUG_LOG_LN(s.c_str());

		for (int i = 0; i < len; i++)
		{
			if (s == strings[i])
			{
				e = i;
				goto GetValueOrIndex;
			}
		}

		if (success != nullptr)
			*success = false;

		return -1;
		
	}
	else if (jvar.is<int>())
	{
		int i = jvar.as<int>();

		//Out of range, invalid value.
		if (i >= len || i < 0)
		{
			if (success != nullptr)
				*success = false;

			return -1;
		}

		e = i;			
	}
	else 
	{
		if (success != nullptr)
			*success = false;

		return -1; 
	}

	GetValueOrIndex:

	//If values is not a nullptr that means the enum starts at 0 and increments by 1 for each value.
	if (values == nullptr)
	{
		//Value is an index
		return e;
	}
	else
	{
		//Return value.
		return values[e];
	}
}



bool JsonHelper::EnumValueToJson(JsonVariant& jvar, int value, int* values, const uint8_t len, bool enumContainsNeg1)
{
	bool success;

	int index = EnumValueToIndex(value, values, len, enumContainsNeg1  ? &success : nullptr);

	if (index == -1 || (enumContainsNeg1 && !success)) return false;

	return jvar.set(index);
}

bool JsonHelper::EnumValueToJson(JsonVariant& jvar, int value, const char** strings, const uint8_t len, int* values)
{
	const char* str = EnumTo(value, strings, len, values);

	if (str == nullptr) return false;

	return jvar.set(str);
}

/// <summary>
/// ArduinoJson UDF helper.
/// Shared helper function for converting JSON objects/variants to enums.
/// If enum is not a class, create a dummy enum class that can be used for casting.
/// If parsing fails, the enum will retain its last value.
/// </summary>
/// <param name="src">JsonVariant/Object containing the enum value.</param>
/// <param name="dst">Enum to place the value.</param>
/// <param name="length">How many values does the enum contain.</param>
/// <param name="name">(Optional)Name to print when parsing has failed.</param>
/// <param name="strings">String representation of the enum value.</param>
/// <param name="values">If enum is not incremented from 0 and on, the values must be stored.</param>
void JsonHelper::UdfHelperConvertFromJsonEnums(JsonVariantConst src, EnumClass_t& dst, uint8_t length, const char* name, const char** strings, int* values)
{
	int parseResult = JsonHelper::JsonParseEnum(src, length, strings, values);

	if (parseResult != -1)
		dst = parseResult;
	else
		DEBUG_LOG_F("%s to JSON Parsing Failed", name != nullptr ? name : "Enum");
}

/// <summary>
/// ArduinoJson UDF helper.
/// Shared helper function for converting enums JSON objects/variants.
/// If enum is not a class, create a dummy enum class that can be used for casting.
/// </summary>
/// <param name="src">Enum to convert.</param>
/// <param name="dst">JsonVariant/Object to set.</param>
/// <param name="length">How many values does the enum contain.</param>
/// <param name="name">(Optional)Name to print when parsing has failed.</param>
/// <param name="strings">(Optional)String representation of the enum value.</param>
/// <param name="values">(Optional*)If enum is not incremented from 0 and on, the values must be passed.</param>
bool JsonHelper::UdfHelperConvertToJsonEnums(const EnumClass_t& src, JsonVariant dst, uint8_t length, const char* name, const char** strings, int* values)
{
	bool set = false;

	if (strings != nullptr && SERIALIZE_ENUMS_TO_STRING)
		set = JsonHelper::EnumValueToJson(dst, src, strings, length, values);
	else if (values != nullptr /*&& SERIALIZE_ENUMS_TO_INDEX*/)
	{
		int value = EnumValueToIndex(src, values, length, &set);

		if (set)
			set = dst.set(value);
	}
	else 
		set = dst.set(src);	//Enum already is an index.

	if (set) return true;

	DEBUG_LOG_F("%s Conversion to JSON failed.", name != nullptr ? name : "Enum");
	return false;
}


JsonHelper::JsonSerialStream::JsonSerialStream(JsonDocument& doc)
{
	_doc = &doc;
	_serial = serial;
}

JsonHelper::JsonSerialStream::JsonSerialStream(JsonDocument& doc, HardwareSerial* serial) {
	_doc = &doc;
	_serial = serial;
}


size_t JsonHelper::JsonSerialStream::write(uint8_t c)
{
	return serial->write(c);
}

size_t JsonHelper::JsonSerialStream::write(const uint8_t* buffer, size_t length)
{
	return serial->write(buffer, length);
}

size_t JsonHelper::JsonSerialStream::PrintContents()
{
	_serial->println("Printing Json Doc...");
	JsonSerialStream* stream = this;
	size_t size = serializeJson(*_doc, *stream);
	_serial->println("\r\n...Finished.");
	return size;
}


size_t JsonHelper::JsonSerialStream::PrintContents(JsonDocument& doc)
{
	return PrintContents(doc, serial);
}

size_t JsonHelper::JsonSerialStream::PrintContents(JsonDocument& doc, HardwareSerial* serial)
{
	JsonSerialStream stream = JsonSerialStream(doc, serial);
	return stream.PrintContents();
}



#pragma region User Defined Functions



bool canConvertFromJson(JsonVariantConst src, const ConfigSource&)
{
	return JsonHelper::JsonParseEnum(src, 5, config_source_strings, (int*)config_source_values) != -1;
}


void convertFromJson(JsonVariantConst src, ConfigSource& dst)
{
	JsonHelper::UdfHelperConvertFromJsonEnums(src, (EnumClass_t&)dst, 5, "ConfigSource", config_source_strings, (int*)config_source_values);
	//ConfigSource parseResult = (ConfigSource)JsonHelper::JsonParseEnum(src, 5, config_source_strings, (int*)config_source_values);

	//if ((int)parseResult != -1)
	//	dst = parseResult;
	//else
	//	DEBUG_LOG_LN("ConfigSource Parsing Failed");
}

bool convertToJson(const ConfigSource& src, JsonVariant dst)
{
	JsonHelper::UdfHelperConvertToJsonEnums((EnumClass_t&)src, dst, 5, "ConfigSource", config_source_strings, (int*)config_source_values);
//#if SERIALIZE_ENUMS_TO_STRING
//	bool set = JsonHelper::EnumValueToJson(dst, (int)src, config_source_strings, 5);
//#else
//	bool set = dst.set(src);
//#endif
//
//	if (set) return true;
//
//	DEBUG_LOG_LN("ConfigSource Conversion to JSON failed.");
//	return false;
}

#if DEVELOPER_MODE
bool canConvertFromJson(JsonVariantConst src, const TaskConfig_t&)
{
	return src.containsKey("core") || src.containsKey("useDefaults") || src.containsKey("recurRate") || src.containsKey("stackSize") || src.containsKey("priority");
}


void convertFromJson(JsonVariantConst src, TaskConfig_t& dst)
{
#if DEVELOPER_MODE
	if (!status.misc.developerMode) return;

	if (src.containsKey("useDefaults"))
		dst.useDefaults = src["useDefaults"];

	if (src.containsKey("core"))
		dst.core = src["core"];

	if (src.containsKey("priority"))
		dst.priority = src["priority"];

	if (src.containsKey("recurRate"))
		dst.recurRate = src["recurRate"];

	if (src.containsKey("stackSize"))
		dst.stackSize = src["stackSize"];
#endif
}

bool convertToJson(const TaskConfig_t& src, JsonVariant dst)
{
	dst["useDefaults"] = src.useDefaults;	//Developer mode only
	dst["core"] = src.core;
	dst["priority"] = src.priority;
	dst["recurRate"] = src.recurRate;
	dst["stackSize"] = src.stackSize;

	return true;
}
#endif

bool canConvertFromJson(JsonVariantConst src, const IPAddress&)
{
	if (src.is<const char*>())
	{
		const char* cchar = src.as<const char*>();

		return IPAddress().fromString(cchar);
		//return IsValidIPAddress((char*)cchar); Apprently arduino regex library is terrible...
	}
	else if (src.is<JsonArrayConst>())
	{
		return src.size() == 4;
	}
}


void convertFromJson(JsonVariantConst src, IPAddress& dst)
{
	if (src.is<const char*>())
	{
		dst.fromString((const char*)src);
	}
	else if (src.is<JsonArrayConst>())
	{
		dst[0] = src[0];
		dst[1] = src[1];
		dst[2] = src[2];
		dst[3] = src[3];
	}
	else
		DEBUG_LOG_LN("Cannot Parse IPAddress");
}


bool convertToJson(const IPAddress& src, JsonVariant dst)
{
	//String required for website JS at this time.
//#if SERIALIZE_ENUMS_TO_STRING
	dst.set(src.toString().c_str());
//#else
//	bool set =
//		dst.add(src[0]) &&
//		dst.add(src[1]) &&
//		dst.add(src[2]) &&
//		dst.add(src[3]);
//
//	if (set) return true;
//
//	DEBUG_LOG_LN("IPAddress Conversion to JSON failed.");
//	return false;
//#endif
}



bool canConvertFromJson(JsonVariantConst src, const WifiPower&)
{
	return JsonHelper::JsonParseEnum(src, 12, wifi_power_strings, (int*)wifi_power_values) != -1;
}

void convertFromJson(JsonVariantConst src, WifiPower& dst)
{
	JsonHelper::UdfHelperConvertFromJsonEnums(src, (EnumClass_t&)dst, 12, "WifiPower", wifi_power_strings, (int*)wifi_power_values);
}

bool convertToJson(const WifiPower& src, JsonVariant dst)
{
	JsonHelper::UdfHelperConvertToJsonEnums((EnumClass_t&)src, dst, 12, "WifiPower", wifi_power_strings, (int*)wifi_power_values);
}

//void convertFromJson(JsonVariantConst src, WifiPower& dst)
//{
//	bool success;
//	WifiPower parseResult = (WifiPower)JsonHelper::JsonParseEnum(src, 12, wifi_power_strings, (int*)wifi_power_values, &success);
//
//	if (success)
//		dst = parseResult;
//	else
//		DEBUG_LOG_LN("WifiPower Parsing Failed");
//}
//
//bool convertToJson(const WifiPower& src, JsonVariant dst)
//{
//#if SERIALIZE_ENUMS_TO_STRING
//	bool set = JsonHelper::EnumValueToJson(dst, (int)src, esp_updater_strings, 12);
//	if (set) return true;
//#else
//	bool set;
//	int index = EnumValueToIndex((int)src, (int*)wifi_power_values, 12, &set);
//	if (set) return dst.set(index);
//#endif
//
//
//
//
//
//	DEBUG_LOG_LN("WifiPower Conversion to JSON failed.");
//	return false;
//}




//
//bool canConvertFromJson(JsonVariantConst src, const DeviceUpdaterMode&)
//{
//	return JsonHelper::JsonParseEnum(src, 4, esp_updater_strings) != -1;
//}
//void convertFromJson(JsonVariantConst src, DeviceUpdaterMode& dst)
//{
//	bool success;
//	DeviceUpdaterMode parseResult = (DeviceUpdaterMode)JsonHelper::JsonParseEnum(src, 4, esp_updater_strings, nullptr, &success);
//
//	if (success)
//		dst = parseResult;
//	else
//		DEBUG_LOG_LN("DeviceUpdaterMode Parsing Failed");
//}
//
//bool convertToJson(const DeviceUpdaterMode& src, JsonVariant dst)
//{
//#if SERIALIZE_ENUMS_TO_STRING
//	bool set = JsonHelper::EnumValueToJson(dst, (int)src, esp_updater_strings, 4);
//#else
//	bool set = dst.set((uint8_t)src);
//#endif
//
//	if (set) return true;
//
//	DEBUG_LOG_LN("DeviceUpdaterMode Conversion to JSON failed.");
//	return false;
//}




bool canConvertFromJson(JsonVariantConst src, const ConfigAutobackupMode&)
{
	return JsonHelper::JsonParseEnum(src, 4, esp_autobackup_strings) != -1;
}

void convertFromJson(JsonVariantConst src, ConfigAutobackupMode& dst)
{
	JsonHelper::UdfHelperConvertFromJsonEnums(src, (EnumClass_t&)dst, 4, "ConfigAutobackupMode", esp_autobackup_strings, nullptr);
}

bool convertToJson(const ConfigAutobackupMode& src, JsonVariant dst)
{
	JsonHelper::UdfHelperConvertToJsonEnums((EnumClass_t&)src, dst, 4, "ConfigAutobackupMode", esp_autobackup_strings, nullptr);
}

//void convertFromJson(JsonVariantConst src, ConfigAutobackupMode& dst)
//{
//	bool success;
//	ConfigAutobackupMode parseResult = (ConfigAutobackupMode)JsonHelper::JsonParseEnum(src, 4, esp_autobackup_strings, nullptr, &success);
//
//	if (success)
//		dst = parseResult;
//	else
//		DEBUG_LOG_LN("ConfigAutobackupMode Parsing Failed");
//}
//
//bool convertToJson(const ConfigAutobackupMode& src, JsonVariant dst)
//{
//#if SERIALIZE_ENUMS_TO_STRING
//	bool set = JsonHelper::EnumValueToJson(dst, (int)src, esp_autobackup_strings, 4);
//#else
//	bool set = dst.set((uint8_t)src);
//#endif
//
//	if (set) return true;
//
//	DEBUG_LOG_LN("ConfigAutobackupMode Conversion to JSON failed.");
//	return false;
//}




bool canConvertFromJson(JsonVariantConst src, const WifiMode&)
{
	return JsonHelper::JsonParseEnum(src, 4, wifi_mode_strings) != -1;
}

void convertFromJson(JsonVariantConst src, WifiMode& dst)
{
	JsonHelper::UdfHelperConvertFromJsonEnums(src, (EnumClass_t&)dst, 4, "WifiMode", wifi_mode_strings, nullptr);
}

bool convertToJson(const WifiMode& src, JsonVariant dst)
{
	JsonHelper::UdfHelperConvertToJsonEnums((EnumClass_t&)src, dst, 4, "WifiMode", wifi_mode_strings, nullptr);
}


//void convertFromJson(JsonVariantConst src, WifiMode& dst)
//{
//
//	WifiMode parseResult = (WifiMode)JsonHelper::JsonParseEnum(src, 4, wifi_mode_strings, nullptr);
//
//	if ((int)parseResult != -1)
//		dst = parseResult;
//	else
//		DEBUG_LOG_LN("WifiMode Parsing Failed");
//}
//
//bool convertToJson(const WifiMode& src, JsonVariant dst)
//{
//#if SERIALIZE_ENUMS_TO_STRING
//	bool set = JsonHelper::EnumValueToJson(dst, (int)src, wifi_mode_strings, 4);
//#else
//	bool set = dst.set((uint8_t)src);
//#endif
//
//	if (set) return true;
//
//	DEBUG_LOG_LN("WifiMode Conversion to JSON failed.");
//	return false;
//}



bool canConvertFromJson(JsonVariantConst src, const UART_DATABITS&)
{
	return JsonHelper::JsonParseEnum(src, 4, nullptr, (int*)uart_databits_values) != -1;
}

void convertFromJson(JsonVariantConst src, UART_DATABITS& dst)
{
	JsonHelper::UdfHelperConvertFromJsonEnums(src, (EnumClass_t&)dst, 4, "UART_DATABITS", nullptr, (int*)uart_databits_values);
}

bool convertToJson(const UART_DATABITS& src, JsonVariant dst)
{
	JsonHelper::UdfHelperConvertToJsonEnums((EnumClass_t&)src, dst, 4, "UART_DATABITS", nullptr, (int*)uart_databits_values);
}

//void convertFromJson(JsonVariantConst src, UART_DATABITS& dst)
//{
//
//	UART_DATABITS parseResult = (UART_DATABITS)JsonHelper::JsonParseEnum(src, 4, nullptr, (int*)uart_databits_values);
//
//	if ((int)parseResult != -1)
//		dst = parseResult;
//	else
//		DEBUG_LOG_LN("UART_DATABITS Parsing Failed");
//}
//
//bool convertToJson(const UART_DATABITS& src, JsonVariant dst)
//{
//	bool set = JsonHelper::EnumValueToJson(dst, (int)src, (int*)uart_databits_values, 4);
//
//	if (set) return true;
//
//	DEBUG_LOG_LN("UART_DATABITS Conversion to JSON failed.");
//	return false;
//}




bool canConvertFromJson(JsonVariantConst src, const UART_PARITY&)
{
	return JsonHelper::JsonParseEnum(src, 4, uart_parity_strings, (int*)uart_parity_values) != -1;
}

void convertFromJson(JsonVariantConst src, UART_PARITY& dst)
{
	JsonHelper::UdfHelperConvertFromJsonEnums(src, (EnumClass_t&)dst, 3, "UART_PARITY", uart_parity_strings, (int*)uart_parity_values);
}

bool convertToJson(const UART_PARITY& src, JsonVariant dst)
{
	JsonHelper::UdfHelperConvertToJsonEnums((EnumClass_t&)src, dst, 3, "UART_PARITY", uart_parity_strings, (int*)uart_parity_values);
}

//void convertFromJson(JsonVariantConst src, UART_PARITY& dst)
//{
//
//	UART_PARITY parseResult = (UART_PARITY)JsonHelper::JsonParseEnum(src, 3, uart_parity_strings, (int*)uart_parity_values);
//
//	if ((int)parseResult != -1)
//		dst = parseResult;
//	else
//		DEBUG_LOG_LN("UART_PARITY Parsing Failed");
//}
//
//bool convertToJson(const UART_PARITY& src, JsonVariant dst)
//{
//#if SERIALIZE_ENUMS_TO_STRING
//	bool set = JsonHelper::EnumValueToJson(dst, (int)src, uart_parity_strings, 2, (int*)uart_parity_values);
//#else
//	//return JsonHelper::EnumValueToJson(dst, (int)src, (int*)uart_parity_values, 2);
//	bool set = dst.set(EnumValueToIndex((uint8_t)src, (int*)uart_parity_values, 2));
//#endif
//	if (set) return true;
//
//	DEBUG_LOG_LN("UART_PARITY Conversion to JSON failed.");
//	return false;
//}



bool canConvertFromJson(JsonVariantConst src, const UART_STOPBITS&)
{
	return JsonHelper::JsonParseEnum(src, 4, nullptr, (int*)uart_stopbits_values) != -1;
}

void convertFromJson(JsonVariantConst src, UART_STOPBITS& dst)
{
	JsonHelper::UdfHelperConvertFromJsonEnums(src, (EnumClass_t&)dst, 2, "UART_PARITY", nullptr, (int*)uart_stopbits_values);
}

bool convertToJson(const UART_STOPBITS& src, JsonVariant dst)
{
	JsonHelper::UdfHelperConvertToJsonEnums((EnumClass_t&)src, dst, 2, "UART_PARITY", nullptr, (int*)uart_stopbits_values);
}

//void convertFromJson(JsonVariantConst src, UART_STOPBITS& dst)
//{
//
//	UART_STOPBITS parseResult = (UART_STOPBITS)JsonHelper::JsonParseEnum(src, 2, nullptr, (int*)uart_stopbits_values);
//
//	if ((int)parseResult != -1)
//		dst = parseResult;
//	else
//		DEBUG_LOG_LN("UART_STOPBITS Parsing Failed");
//}
//
//bool convertToJson(const UART_STOPBITS& src, JsonVariant dst)
//{
//	//return JsonHelper::EnumValueToJson(dst, (int)src, (int*)uart_parity_values, 2);
//	bool set = dst.set(EnumValueToIndex((uint8_t)src, (int*)uart_stopbits_values, 2));
//
//	if (set) return true;
//
//	DEBUG_LOG_LN("UART_STOPBITS Conversion to JSON failed.");
//	return false;
//}

#pragma endregion


