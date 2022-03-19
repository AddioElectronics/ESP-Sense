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

	HardwareSerial* _serial;

	Crc32Stream() 
	{
		crc.reset();
		_serial = nullptr;
	}

	Crc32Stream(HardwareSerial* _ser)
	{
		crc.reset();
		_serial = _ser;
	}

	// Writes one byte, returns the number of bytes written (0 or 1)
	virtual size_t write(uint8_t c);
	// Writes several bytes, returns the number of bytes written
	virtual size_t write(const uint8_t* buffer, size_t length);

	virtual size_t writeSerial(uint8_t c);
	// Writes several bytes, returns the number of bytes written
	virtual size_t writeSerial(const uint8_t* buffer, size_t length);
};

struct Crc32FileStream : Crc32Stream {

	File* file;

	Crc32FileStream(File* _file);
	Crc32FileStream(File* _file, HardwareSerial* _ser);

	size_t write(uint8_t c) override;
	size_t write(const uint8_t* buffer, size_t length) override;
};

struct Crc32EepromStream : Crc32Stream {

	EepromStream* eepromStream;

	Crc32EepromStream(EepromStream* _eepromStream);
	Crc32EepromStream(EepromStream* _eepromStream, HardwareSerial* _ser);

	size_t write(uint8_t c) override;
	size_t write(const uint8_t* buffer, size_t length) override;
};

struct Crc32FileEepromStream : Crc32Stream {

	File* file;
	EepromStream* eepromStream;

	Crc32FileEepromStream(File* _file, EepromStream* _eepromStream);
	Crc32FileEepromStream(File* _file, EepromStream* _eepromStream, HardwareSerial* _ser);

	size_t write(uint8_t c) override;
	size_t write(const uint8_t* buffer, size_t length) override;
};


#endif

