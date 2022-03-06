#ifndef _CRCSTREAM_h
#define _CRCSTREAM_h

#include <arduino.h>
#include <CRC32.h>

#include <StreamUtils.hpp>
#include <StreamUtils.h>

#if defined(ESP8266)
#include <LittleFS.h>	
#elif defined(ESP32)
#include <FS.h>
#include <LITTLEFS.h>
#endif

struct Crc32Stream {

	CRC32 crc;

	// Writes one byte, returns the number of bytes written (0 or 1)
	size_t write(uint8_t c);
	// Writes several bytes, returns the number of bytes written
	size_t write(const uint8_t* buffer, size_t length);
};

struct Crc32FileStream {

	File* file;
	CRC32 crc;

	void init(File* _file);

	// Writes one byte, returns the number of bytes written (0 or 1)
	size_t write(uint8_t c);
	// Writes several bytes, returns the number of bytes written
	size_t write(const uint8_t* buffer, size_t length);
};

struct Crc32EepromStream {

	EepromStream* eepromStream;
	CRC32 crc;

	void init(EepromStream* _eepromStream);

	// Writes one byte, returns the number of bytes written (0 or 1)
	size_t write(uint8_t c);
	// Writes several bytes, returns the number of bytes written
	size_t write(const uint8_t* buffer, size_t length);
};

#endif

