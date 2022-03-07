#include "JsonHelper.h"
#include "HelperFunctions.h"
#include "EspSenseValidation.h"
#include "Macros.h"

bool canConvertFromJson(JsonVariantConst src, const ConfigSource&)
{
	return JsonParseEnum(src, 5, config_source_strings, (int*)config_source_values) != -1;
}

void convertFromJson(JsonVariantConst src, ConfigSource& dst)
{

	ConfigSource parseResult = (ConfigSource)JsonParseEnum(src, 5, config_source_strings, (int*)config_source_values);

	if ((int)parseResult != -1)
		dst = parseResult;
	else
		DEBUG_LOG_LN("ConfigSource Parsing Failed");
}

bool convertToJson(const ConfigSource& src, JsonVariant dst)
{
#if SERIALIZE_ENUMS_TO_STRING
	bool set = EnumValueToJson(dst, (int)src, config_source_strings, 5);
#else
	bool set = dst.set(src);
#endif

	if (set) return true;

	DEBUG_LOG_LN("ConfigSource Conversion to JSON failed.");
	return false;
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
	bool set =
		dst.add(src[0]) &&
		dst.add(src[1]) &&
		dst.add(src[2]) &&
		dst.add(src[3]);

	if (set) return true;

	DEBUG_LOG_LN("IPAddress Conversion to JSON failed.");
	return false;
}



bool canConvertFromJson(JsonVariantConst src, const WifiPower&)
{
	return JsonParseEnum(src, 12, wifi_power_strings, (int*)wifi_power_values) != -1;
}

void convertFromJson(JsonVariantConst src, WifiPower& dst)
{
	bool success;
	WifiPower parseResult = (WifiPower)JsonParseEnum(src, 12, wifi_power_strings, (int*)wifi_power_values, &success);

	if (success)
		dst = parseResult;
	else
		DEBUG_LOG_LN("WifiPower Parsing Failed");
}

bool convertToJson(const WifiPower& src, JsonVariant dst)
{
#if SERIALIZE_ENUMS_TO_STRING
	bool set = EnumValueToJson(dst, (int)src, esp_updater_strings, 12);
	if (set) return true;
#else
	bool set;
	int index = EnumValueToIndex((int)src, (int*)wifi_power_values, 12, &set);
	if (set) return dst.set(index);
#endif

	

	

	DEBUG_LOG_LN("WifiPower Conversion to JSON failed.");
	return false;
}


//
//bool canConvertFromJson(JsonVariantConst src, const DeviceUpdaterMode&)
//{
//	return JsonParseEnum(src, 4, esp_updater_strings) != -1;
//}
//void convertFromJson(JsonVariantConst src, DeviceUpdaterMode& dst)
//{
//	bool success;
//	DeviceUpdaterMode parseResult = (DeviceUpdaterMode)JsonParseEnum(src, 4, esp_updater_strings, nullptr, &success);
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
//	bool set = EnumValueToJson(dst, (int)src, esp_updater_strings, 4);
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
	return JsonParseEnum(src, 4, esp_autobackup_strings) != -1;
}

void convertFromJson(JsonVariantConst src, ConfigAutobackupMode& dst)
{
	bool success;
	ConfigAutobackupMode parseResult = (ConfigAutobackupMode)JsonParseEnum(src, 4, esp_autobackup_strings, nullptr, &success);

	if (success)
		dst = parseResult;
	else
		DEBUG_LOG_LN("ConfigAutobackupMode Parsing Failed");
}

bool convertToJson(const ConfigAutobackupMode& src, JsonVariant dst)
{
#if SERIALIZE_ENUMS_TO_STRING
	bool set = EnumValueToJson(dst, (int)src, esp_autobackup_strings, 4);
#else
	bool set = dst.set((uint8_t)src);
#endif

	if (set) return true;

	DEBUG_LOG_LN("ConfigAutobackupMode Conversion to JSON failed.");
	return false;
}




bool canConvertFromJson(JsonVariantConst src, const WifiMode&)
{
	return JsonParseEnum(src, 4, wifi_mode_strings) != -1;
}

void convertFromJson(JsonVariantConst src, WifiMode& dst)
{

	WifiMode parseResult = (WifiMode)JsonParseEnum(src, 4, wifi_mode_strings, nullptr);

	if ((int)parseResult != -1)
		dst = parseResult;
	else
		DEBUG_LOG_LN("WifiMode Parsing Failed");
}

bool convertToJson(const WifiMode& src, JsonVariant dst)
{
#if SERIALIZE_ENUMS_TO_STRING
	bool set = EnumValueToJson(dst, (int)src, wifi_mode_strings, 4);
#else
	bool set = dst.set((uint8_t)src);
#endif

	if (set) return true;

	DEBUG_LOG_LN("WifiMode Conversion to JSON failed.");
	return false;
}




bool canConvertFromJson(JsonVariantConst src, const UART_DATABITS&)
{
	return JsonParseEnum(src, 4, nullptr, (int*)uart_databits_values) != -1;
}

void convertFromJson(JsonVariantConst src, UART_DATABITS& dst)
{

	UART_DATABITS parseResult = (UART_DATABITS)JsonParseEnum(src, 4, nullptr, (int*)uart_databits_values);

	if ((int)parseResult != -1)
		dst = parseResult;
	else
		DEBUG_LOG_LN("UART_DATABITS Parsing Failed");
}

bool convertToJson(const UART_DATABITS& src, JsonVariant dst)
{
	bool set = EnumValueToJson(dst, (int)src, (int*)uart_databits_values, 4);

	if (set) return true;

	DEBUG_LOG_LN("UART_DATABITS Conversion to JSON failed.");
	return false;
}




bool canConvertFromJson(JsonVariantConst src, const UART_PARITY&)
{
	return JsonParseEnum(src, 4, uart_parity_strings, (int*)uart_parity_values) != -1;
}

void convertFromJson(JsonVariantConst src, UART_PARITY& dst)
{

	UART_PARITY parseResult = (UART_PARITY)JsonParseEnum(src, 3, uart_parity_strings, (int*)uart_parity_values);

	if ((int)parseResult != -1)
		dst = parseResult;
	else
		DEBUG_LOG_LN("UART_PARITY Parsing Failed");
}

bool convertToJson(const UART_PARITY& src, JsonVariant dst)
{
#if SERIALIZE_ENUMS_TO_STRING
	bool set = EnumValueToJson(dst, (int)src, uart_parity_strings, 2, (int*)uart_parity_values);
#else
	//return EnumValueToJson(dst, (int)src, (int*)uart_parity_values, 2);
	bool set = dst.set(EnumValueToIndex((uint8_t)src, (int*)uart_parity_values, 2));
#endif
	if (set) return true;

	DEBUG_LOG_LN("UART_PARITY Conversion to JSON failed.");
	return false;
}



bool canConvertFromJson(JsonVariantConst src, const UART_STOPBITS&)
{
	return JsonParseEnum(src, 4, nullptr, (int*)uart_stopbits_values) != -1;
}

void convertFromJson(JsonVariantConst src, UART_STOPBITS& dst)
{

	UART_STOPBITS parseResult = (UART_STOPBITS)JsonParseEnum(src, 2, nullptr, (int*)uart_stopbits_values);

	if ((int)parseResult != -1)
		dst = parseResult;
	else
		DEBUG_LOG_LN("UART_STOPBITS Parsing Failed");
}

bool convertToJson(const UART_STOPBITS& src, JsonVariant dst)
{
	//return EnumValueToJson(dst, (int)src, (int*)uart_parity_values, 2);
	bool set = dst.set(EnumValueToIndex((uint8_t)src, (int*)uart_stopbits_values, 2));

	if (set) return true;

	DEBUG_LOG_LN("UART_STOPBITS Conversion to JSON failed.");
	return false;
}




int JsonParseEnum(JsonVariantConst& jvar, const uint8_t len, const char** strings, int* values, bool* success)
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



bool EnumValueToJson(JsonVariant& jvar, int value, int* values, const uint8_t len, bool enumContainsNeg1)
{
	bool success;

	int index = EnumValueToIndex(value, values, len, enumContainsNeg1  ? &success : nullptr);

	if (index == -1 || (enumContainsNeg1 && !success)) return false;

	return jvar.set(index);
}

bool EnumValueToJson(JsonVariant& jvar, int value, const char** strings, const uint8_t len, int* values)
{
	const char* str = EnumTo(value, strings, len, values);

	if (str == nullptr) return false;

	return jvar.set(str);
}