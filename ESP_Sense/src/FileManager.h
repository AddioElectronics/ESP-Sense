#ifndef _FILEMANAGER_h
#define _FILEMANAGER_h

#include "arduino.h"

#include <ArduinoJson.hpp>				//Used for deserializing config files, and de/serializing data for/from Home Assistant
#include <ArduinoJson.h>

#include <StreamUtils.hpp>
#include <StreamUtils.h>

#include <FS.h>

#if defined(ESP8266)
#include <LittleFS.h>	
#elif defined(ESP32)
#include <LITTLEFS.h>
#endif

#include <CRC32.h>

namespace FileManager {

	bool MountFileSystem();
	bool UnMountFileSystem();

	bool OpenFile(File* file, const char* path, const char mode, bool closeOnFail = false);

	bool ParseFile(File* file, JsonDocument& doc, const char* filename, bool close = true);

	int OpenAndParseFile(const char* path, JsonDocument& doc);

	int FindStartOfJson(File* file, bool closeOnFail = false);

	size_t GetFileSize(File* file, const char* path, bool isJson = false);
	size_t GetFileSize(const char* path, bool isJson = false);

	size_t GetDocumentFileSize(const char* path);
	size_t GetDocumentFileSize(size_t filesize);

	String GetFileName(const char* path);

	void DisplayFileContents(File* file);

	uint32_t GetFileCRC(const char* path);
	uint32_t GetFileCRC(File* file, bool close = true);

	uint32_t GetSerializedCRC(JsonDocument& doc);

	uint32_t GetEepromCRC(EepromStream& eestream, size_t length);
}


#endif

