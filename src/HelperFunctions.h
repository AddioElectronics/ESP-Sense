// HelperFunctions.h

#ifndef _HELPERFUNCTIONS_h
#define _HELPERFUNCTIONS_h

#include <Arduino.h>

#include <Regexp.h>

bool memconstcmp(void* start, const uint8_t value, size_t length);

int GetNextPowerOf2(int x);

int GetPrevPowerOf2(int x);

void CstrToLower(char* s);

void XORArray(uint8_t* str, size_t len, uint8_t key);

size_t MultiLogFormat(HardwareSerial& ser, const char* format, ...);

/// <summary>
/// Converts an enum value to an index.
/// </summary>
/// <param name="value">True value of the enum.</param>
/// <param name="values">Array of the true value for each element of the enum, in order.</param>
/// <param name="len">How many values the enum contains.</param>
/// <param name="success">Out value to get if the conversion was successful. (For using when the enum contains a -1 value)</param>
/// <returns>Index of the enum value.</returns>
int EnumValueToIndex(int value, int* values, const uint8_t len, bool* success = nullptr);

/// <summary>
/// Pass an enum value or index and retrieve a pointer to its string representation.
/// </summary>
/// <param name="value">True value of the enum, must be paired with values. Or index, set value to nullptr.</param>
/// <param name="strings">Array of the values represented as strings.</param>
/// <param name="len">How many values the enum contains.</param>
/// <param name="values">Array of the true value for each element of the enum, in order.</param>
/// <returns>Pointer to the string representation of the enum, or nullptr if something failed.</returns>
const char* EnumTo(int value, const char** strings, const uint8_t len, int* values);

/// <summary>
/// Sets an LED taking push or pull in to account.
/// </summary>
/// <param name="gpio">GPIO of the LED</param>
/// <param name="onLevel">The voltage level on the GPIO which turns the LED on. (Push or pull)</param>
/// <param name="state">Desired state of the LED.</param>
void LedSet(int8_t gpio, bool onLevel, bool state);

/// <summary>
/// Gets the state of an LED, taking Push or Pull in to account.
/// </summary>
/// <param name="gpio">GPIO of the LED</param>
/// <param name="onLevel">The voltage level on the GPIO which turns the LED on. (Push or pull)</param>
/// <returns>True if the LED is on, false if it is off. Returns -1 if the GPIO is invalid.</returns>
int LedGet(int8_t gpio, bool onLevel);

/// <summary>
/// Turns on an LED taking push or pull in to account.
/// </summary>
/// <param name="gpio">GPIO of the LED</param>
/// <param name="onLevel">The voltage level on the GPIO which turns the LED on. (Push or pull)</param>
void LedEnable(int8_t gpio, bool onLevel);

/// <summary>
/// Turns off an LED taking push or pull in to account.
/// </summary>
/// <param name="gpio">GPIO of the LED</param>
/// <param name="onLevel">The voltage level on the GPIO which turns the LED on. (Push or pull)</param>
void LedDisable(int8_t gpio, bool onLevel);

/// <summary>
/// Toggles an LED.
/// </summary>
/// <param name="gpio">GPIO of the LED</param>
void LedToggle(int8_t gpio);

/// <summary>
/// Gets the state of a button taking NO/NC/pull-up in to account.
/// </summary>
/// <param name="gpio">GPIO of the button.</param>
/// <param name="pressedLevel">Which voltage level indicates a button press?</param>
/// <returns>True if button is pressed, false if not.</returns>
bool ButtonGetState(int8_t gpio, bool pressedLevel);


#endif

