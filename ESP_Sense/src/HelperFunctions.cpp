#include "HelperFunctions.h"

#include <PubSubClient.h>
#include <cstdarg>

#include "Config/config_mqtt.h"

extern PubSubClient mqttClient;


/*
	Compares a block of memory with a constant byte.

	/Returns	True if all bytes match the value. False if any bytes do not match.
*/
bool memconstcmp(void* start, const uint8_t value, size_t length)
{
	for (int i = 0; i < length; i++)
	{
		if (*((uint8_t*)start + i) != value) return false;
	}
	return true;
}


int GetNextPowerOf2(int x)
{
	int power = 1;
	while (power < x)
		power *= 2;

	return power;
}

int GetPrevPowerOf2(int x)
{
	int power = GetNextPowerOf2(x);
	while (power > x)
		power /= 2;

	return power;
}

void CstrToLower(char* s)
{
	size_t len = strlen(s);

	for (size_t i = 0; i < len; i++)
	{
		if (s[i] >= 0x41 && s[i] <= 0x5A)
			s[i] += 0x20;
	}
}


void XORArray(uint8_t* arr, size_t len, uint8_t key)
{
	for (uint16_t i = 0; i < len; i++)
	{
		arr[i] ^= key;
	}
}


size_t MultiLogFormat(HardwareSerial& ser, const char* format, ...)
{
	va_list ap;
	va_start(ap, format);

	char* str;

	int size = vasprintf(&str, format, ap);

	if (size >= 0)
	{
		ser.print(str);

		if (size < MQTT_BUFFER_SIZE)
			mqttClient.publish(TOPIC_DEBUG, str);

		free(str);
	}
	else
	{
		size = 0;
	}

	return size;
}


const char* EnumTo(int value, const char** strings, const uint8_t len, int* values)
{
	if (values != nullptr)
	{
		for (uint8_t i = 0; i <= len; i++)
		{
			if (value == values[i]) return strings[i];
		}
	}
	else
	{
		//Value is an index.
		if (value < len)
			return strings[value];
	}

	return nullptr;
}

//const char* EnumTo(int value, const char** strings, const uint8_t len, int* values = nullptr, bool* success)
//{
//	if (values != nullptr)
//	{
//		int index = EnumValueToIndex(value, values, len);
//
//		if (index == -1)
//			return nullptr;
//
//		return strings[index];
//	}
//
//	if (value < len)
//		return strings[value];
//
//	return nullptr;
//}

int EnumValueToIndex(int trueValue, int* values, uint8_t len, bool* success)
{
	if (success != nullptr) *success = true;

	for (uint8_t i = 0; i < len; i++)
	{
		if (trueValue == values[i]) return i;
	}

	if (success != nullptr) *success = false;
	return -1;
}

void LedSet(int8_t gpio, bool onLevel, bool state)
{
	if (gpio == -1) return;

	digitalWrite(gpio, state == onLevel ? state : !state);
}

int LedGet(int8_t gpio, bool onLevel)
{
	if (gpio == -1) return -1;

	return digitalRead(gpio) == onLevel;
}

void LedEnable(int8_t gpio, bool onLevel)
{
	if (gpio == -1) return;

	digitalWrite(gpio, onLevel);
}

void LedDisable(int8_t gpio, bool onLevel)
{
	if (gpio == -1) return;

	digitalWrite(gpio, !onLevel);
}

void LedToggle(int8_t gpio)
{
	if (gpio == -1) return;

	digitalWrite(gpio, !digitalRead(gpio));
}

bool ButtonGetState(int8_t gpio, bool pressedLevel)
{
	if (gpio == -1) return false;

	return digitalRead(gpio) == pressedLevel;
}


