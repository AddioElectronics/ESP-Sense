#include "EspSenseValidation.h"

#include <Regexp.h>

#include "../ESPSense.h"
#include "Config/config_master.h"

#include "macros.h"

extern DeviceStatus_t status;
extern Config_t config;

#if defined(ESP8266)
uint64_t gpioUsageBitmap;
#elif defined(ESP32)
uint16_t gpioUsageBitmap;
#endif

bool IsBaudrateValid(unsigned long baudrate)
{
	switch (baudrate)
	{
	case 50:
	case 75:
	case 110:
	case 134:
	case 150:
	case 200:
	case 300:
	case 600:
	case 1200:
	case 1800:
	case 2400:
	case 4800:
	case 9600:
	case 19200:
	case 28800:
	case 38400:
	case 57600:
	case 76800:
	case 115200:
	case 230400:
	case 460800:
	case 576000:
	case 921600:
		return true;
	default:
		DEBUG_LOG_LN("Invalid Baudrate.");
		return false;
	}

}

bool IsValidI2cFreq(uint32_t freq)
{
	//Max I2C Frequency for connected sensors is 100KHz
	return(freq > 10000 && freq < 100000 && freq % 10000 == 0);
}

//Apprently arduino regex library is terrible...
//bool IsValidIPAddress(char* ipString)
//{
//	MatchState ms;
//
//	ms.Target(ipString);
//
//	//Does not check if IP is valid, just if string is an IPV4 format.
//	char result =  ms.Match("^(\d{1,3}[.]\d{1,3}[.]\d{1,3}[.]\d{1,3})$");	
//
//	return result;
//}

bool IsGpioInUse(int8_t gpio)
{
	return (gpioUsageBitmap >> gpio) & 1;
}



/// <summary>
/// Mark the GPIO pin as in use, so it can not be used, and will show a warning when something tries to use it.
/// </summary>
/// <returns>False if the GPIO is already in use. True if the GPIO was free.</returns>
bool RegisterGpio(int8_t gpio)
{
#warning need to keep track if each GPIO pin is registered and to what.
	if (IsGpioInUse(gpio))
		return false;

	gpioUsageBitmap |= (1 << gpio);
	return true;
}

/// <summary>
/// Release the GPIO so it can be used else where.
/// </summary>
void ReleaseGpio(int8_t gpio)
{
	gpioUsageBitmap &= ~(1 << gpio);
}

/// <summary>
/// Register the gpio if not already in use and release another gpio.
/// </summary>
/// <returns>True if the gpio was registered and released. False if the pin was in use.</returns>
bool RegisterAndReleaseGpio(int8_t reg, int8_t rel)
{
	if (RegisterGpio(reg))
	{
		ReleaseGpio(rel);
		return true;
	}
	return false;
}

bool IsGpioValidPin(int8_t gpio, GpioPull_t pull, bool output, bool checkInUse)
{
#if VALIDATE_GPIO
	GpioValidation_bm validation;
	validation.bitmap = 0;
	validation.output = output;
	validation.pull = pull;
	return IsGpioValidPin(gpio, validation, checkInUse);
#else
	return gpio != -1;
#endif
}


bool IsGpioValidPin(int8_t gpio, GpioValidation_bm& validation, bool checkInUse)
{
#if VALIDATE_GPIO
	//#warning Messy, reminder to come and clean this up. Also not taking into account PWM

	if (checkInUse)
		if (IsGpioInUse(gpio))
		{
			DEBUG_LOG_LN("GPIO already in use.");
			return false;
		}

#if defined(ESP8266)
	if ((gpio >= 1 && gpio <= 5) || (gpio >= 12 && gpio <= 15) || gpio == 17)
	{
		if (!validation.i2c && (gpio == config.device.i2c.sclGpio || gpio == config.device.i2c.sdaGpio))
			goto InvalidMessage;

		if (validation.i2c && (gpio == 15))
			goto InvalidMessage;

		if (!validation.uart && (gpio == config.device.serial.port0.rxGpio || gpio == config.device.serial.port0.txGpio || gpio == config.device.serial.port1.rxGpio || gpio == config.device.serial.port1.txGpio || gpio == 1 || gpio == 3))
			goto InvalidMessage;

		if ((validation.output || validation.pull == GpioPull_t::GPIO_PULL_UP) && (gpio == 15 || gpio == 16))
			goto InvalidMessage;

		if ((!validation.output || validation.pull == GpioPull_t::GPIO_PULL_DOWN) && gpio == 2 && gpio != 16)
			goto InvalidMessage;

		if (validation.analog && gpio != 17)
			goto InvalidMessage;

		//if (validation.isSwitch && gpio == 2)
		//	goto InvalidMessage;

		return true;
	}
	goto InvalidMessage;
	//return (gpio >= 1 && gpio <= 5) || (gpio >= 12 && gpio <= 15) || gpio == 17) &&
	//	(validation.i2c || (gpio != config.device.i2c.sclGpio && gpio != config.device.i2c.sdaGpio)) &&
	//	(validation.uart || (gpio != config.device.serial.port0.rxGpio && gpio != config.device.serial.port0.txGpio && gpio != config.device.serial.port1.rxGpio && gpio != config.device.serial.port1.txGpio))) &&
	//	(!validation.analog || gpio == 17);
#elif defined(ESP32)
	if ((gpio >= 1 && gpio <= 5) || (gpio >= 12 && gpio <= 18) || (gpio >= 21 || gpio <= 23) || (gpio == 25 || gpio == 27) || (gpio >= 32 || gpio <= 36) || gpio <= 39)
	{
		if (!validation.i2c && (gpio == config.device.i2c.sclGpio || gpio == config.device.i2c.sdaGpio))
			goto InvalidMessage;

		if (validation.i2c && (gpio == 15))
			goto InvalidMessage;

		if (!validation.uart && (gpio == config.device.serial.port0.rxGpio || gpio == config.device.serial.port0.txGpio || gpio == config.device.serial.port1.rxGpio || gpio == config.device.serial.port1.txGpio || gpio == 1 || gpio == 3))
			goto InvalidMessage;

		if (!validation.output && (gpio == 12 || gpio == 3))
			goto InvalidMessage;

		if (validation.pull == GpioPull_t::GPIO_PULL_DOWN && (gpio >= 34 && gpio <= 36))
			goto InvalidMessage;

		if ((validation.output || validation.pull == GpioPull_t::GPIO_PULL_UP) && ((gpio >= 34 && gpio <= 36) || gpio == 39 || gpio == 3 || gpio == 1 || gpio == 12))
			goto InvalidMessage;

		if (validation.analog && !(gpio == 0 || gpio == 2 || gpio == 4 || (gpio >= 12 && gpio <= 15) || (gpio >= 12 && gpio <= 15) || (gpio == 25 || gpio == 27) || (gpio >= 32 || gpio <= 36) || gpio <= 39))
			goto InvalidMessage;

		if (validation.dac && gpio != 36 && gpio != 39)
			goto InvalidMessage;

		//if (validation.isSwitch)
		//	return false;

		return true;
	}
	//return digitalPinIsValid(gpio) &&
	//	!(gpio >= 6 && gpio <= 11) &&	//Not flash pins
	//	(validation.i2c || (gpio != config.device.i2c.sclGpio && gpio != config.device.i2c.sdaGpio)) &&
	//	(validation.uart || (gpio != config.device.serial.port0.rxGpio && gpio != config.device.serial.port0.txGpio && gpio != config.device.serial.port1.rxGpio && gpio != config.device.serial.port1.txGpio)) &&
	//	(!validation.output || !(gpio >= 34 && gpio <= 36 || gpio == 39));
#endif

InvalidMessage:
	DEBUG_LOG_LN("GPIO Invalid.");
	return false;
#else
	return true;
#endif
}

bool IsGpioValidPin(int8_t gpio, GpioValidation_t validator, bool checkInUse)
{
#if VALIDATE_GPIO
	GpioValidation_bm validation;
	validation.bitmap = validator;
	return IsGpioValidPin(gpio, validation, checkInUse);
#else
	return true;
#endif
}

bool ValidateSerialPortConfig(uint32_t bitmap)
{
	return (bitmap & 0x8000000 != 0 &&
		bitmap & ~0x800003f == 0 &&
		(bitmap & UART_CONFIG_STOPBITS_MASK) >> 4 != 0b10 &&
		bitmap & UART_CONFIG_PARITY_MASK != 0b10);
}
