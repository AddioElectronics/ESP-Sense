#ifndef _ESPSENSEVALIDATION_h
#define _ESPSENSEVALIDATION_h

#include "arduino.h"

#include "GlobalDefs.h"

#if COMPILE_VALIDATION_ASSERT
#define VALIDATE_ASSERT(_e)	assert(_e)
#else
#define VALIDATE_ASSERT(_e)
#endif

enum class GpioPull_t { GPIO_NO_PULL, GPIO_PULL_UP, GPIO_PULL_DOWN };

#define GPIOV_OUTPUT	0x01
#define GPIOV_ANALOG	0x02
#define GPIOV_DAC		0x04
#define GPIOV_PWM		0x08
#define GPIOV_UART		0x10
#define GPIOV_I2C		0x20
#define GPIOV_SPI		0x40
#define GPIOV_PULLUP	0x80
#define GPIOV_PULLDOWN	0x100

typedef union {
	struct {
		bool output : 1;
		bool analog : 1;
		bool dac : 1;
		bool pwm : 1;
		bool uart : 1;
		bool i2c : 1;
		bool spi : 1;
		GpioPull_t pull : 2;
	};
	uint8_t bitmap;
}GpioValidation_bm;
typedef uint8_t GpioValidation_t;

/*
	Checks to see if the baud rate is a standard value.
*/
bool IsBaudrateValid(unsigned long baudrate);

/*
	Checks to see if the frequency is in a valid range.
*/
bool IsValidI2cFreq(uint32_t baudrate);

//bool IsValidIPAddress(char* ipString);

bool IsGpioInUse(int8_t gpio);
bool RegisterGpio(int8_t gpio);
void ReleaseGpio(int8_t gpio);
bool RegisterAndReleaseGpio(int8_t reg, int8_t rel);

bool IsGpioValidPin(int8_t gpio, GpioPull_t pull = GpioPull_t::GPIO_NO_PULL, bool output = false, bool checkInUse = true);

bool IsGpioValidPin(int8_t gpio, GpioValidation_bm& validation, bool checkInUse = true);

bool IsGpioValidPin(int8_t gpio, GpioValidation_t validator, bool checkInUse = true);

bool ValidateSerialPortConfig(uint32_t bitmap);

#endif

