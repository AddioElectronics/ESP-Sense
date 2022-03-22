#include "FileManager.h"

#include "Config/config_master.h"
#include "Config/global_status.h"
#include "../ESP_Sense.h"

#include "CrcStream.h"
#include "EspSenseValidation.h"
#include "HelperFunctions.h"
#include "macros.h"

extern GlobalStatus_t status;
extern Config_t config;
extern HardwareSerial* serial;				//Message
extern HardwareSerial* serialDebug;		//Debug

CRC32 crc32;

bool FileManager::MountFileSystem()
{
	DEBUG_LOG_LN("Mounting file system...");

#if defined(ESP8266)
	bool formatted = false;
RetryConnection:
	status.device.fsMounted = ESP_FS.begin();
	//Mounting failed, format and try again.
	if (!status.device.fsMounted && !formatted)
	{
		formatted = true;
		ESP_FS.format();
		delay(500);
		goto RetryConnection;
	}
#elif defined(ESP32)
	status.device.fsMounted = ESP_FS.begin(statusRetained.boot.freshBoot);
#endif

	//Could not mount FS.
	if (!status.device.fsMounted)
	{
		DEBUG_LOG_LN("An error has occured while mounting the file system. EEPROM or Firmware configuration data will be used.");
		return false;
	}

	DEBUG_LOG_LN("File system mounted.");
	return true;
}

bool FileManager::UnMountFileSystem()
{
	DEBUG_LOG_LN("Un-mounting file system...");

	status.device.fsMounted = false;
	ESP_FS.end();

	DEBUG_LOG_LN("File system un-mounted.");
	return false;
}

/// <summary>
/// 
/// </summary>
/// <param name="file"></param>
/// <param name="path"></param>
/// <param name="mode"></param>
/// <param name="createOnNull">If file does not exist while trying to write, create.</param>
/// <returns></returns>
bool FileManager::OpenFile(File* file, const char* path, const char* mode)
{
	file->setTimeout(FILE_STREAM_TIMEOUT);

	DEBUG_LOG_F("Opening %s... Mode : %s.\r\n", path, mode);

	if (!status.device.fsMounted)
	{
		DEBUG_LOG_LN("File cannot be opened as the file system is not mounted.");
		return false;
	}

	bool reading = mode[0] == 'r' || mode[1] == '+';


	if (!ESP_FS.exists(path) && reading)
	{
		DEBUG_LOG_F("The file at path %s does not exist.", path);
		return false;
	}

	*file = ESP_FS.open(path, mode);

	if (!*file)
	{
		file->close();

		DEBUG_LOG_LN("Failed to open!");
		return false;
	}

#if DEVELOPER_MODE
	if (status.misc.developerMode && reading)
	{
		DisplayFileContents(file);
	}
#endif


	DEBUG_LOG_LN("File Opened!");
	return true;
}

bool FileManager::ParseFile(File* file, JsonDocument& doc, const char* filename, bool close)
{
	DEBUG_LOG_F("Parsing JSON File %s...", filename);
	file->setTimeout(FILE_STREAM_TIMEOUT);

	if (FindStartOfJson(file, true) == -1) return -1;	//Not a json file

	DeserializationError derror = deserializeJson(doc, *file);

	if (close)
		file->close();

	if (derror)
	{
		DEBUG_LOG_F("...Failed\r\nError : %s\r\n", derror.c_str());
		return false;
	}

	DEBUG_LOG_LN("...Success.");

	return true;
}

int FileManager::OpenAndParseFile(const char* path, JsonDocument& doc)
{
	File file;
	if (!OpenFile(&file, path, "r")) return 1;	//Could not open file

	int result = ParseFile(&file, doc, path, true);

	if (result == -1) return -1;						//Not a JSON file.
	if (!result) return 2;								//Parsing Error

	file.close();

	return 0;											//Success
}

/// <summary>
/// File may contain unsupported comment at the beginning.
/// This is used to find the actual start of the JSON file.
/// </summary>
/// <returns></returns>
int FileManager::FindStartOfJson(File* file, bool closeOnFail)
{
	DEBUG_LOG("...Finding Start of JSON file...");
	int index = -1;

	while (file->available())
	{
		char c = file->read();
		index++;

		if (c == '{')
		{
			file->seek(index);
			break;
		}		
	}

	if (closeOnFail && index == -1)
		file->close();

	if (index == -1)
		DEBUG_LOG("...Could not find Start...");
	else
		DEBUG_LOG_F("...Start is at character %d...\r\n", index);

	return index;
}

size_t FileManager::GetFileSize(File* file, const char* path, bool isJson)
{
	if (file == nullptr)
	{
		DEBUG_LOG_LN("Cannot GetFileSize() : file is nullptr!");
		return 0;
	}

	//if (!LittleFS.exists(path)) return 0;
	if (!ESP_FS.exists(path)) return 0;

	//File file = LittleFS.open(path, "r");
	*file = ESP_FS.open(path, "r");

	int startJson = isJson ? FindStartOfJson(file) : 0;

	return file->size() - startJson;
}

size_t FileManager::GetFileSize(const char* path, bool isJson)
{
	//File file = LittleFS.open(path, "r");
	File file;
	size_t size = GetFileSize(&file, path, isJson);
	file.close();
	return size;
}


/// <summary>
/// Gets the amount of memory that needs to be allocated to deserialize a json document.
/// Does not include the top comment in the calculation.
/// </summary>
size_t FileManager::GetDocumentFileSize(const char* path)
{
	return GetDocumentFileSize(GetFileSize(path, true));
}

/// <summary>
/// Calculates the amount of memory that needs to be allocated to deserialize a json document.
/// </summary>
size_t FileManager::GetDocumentFileSize(size_t filesize)
{
	if (filesize == 0) return 0;

	//Quick and dirty way to calculate document size.
	int power = GetNextPowerOf2(filesize);

	if (power > filesize * 1.3 && power <= 2048)
		return power;

	int e = power / 8;

	while (power < filesize * 1.3)
	{
		power += e;
	}

	return power;
}


String FileManager::GetFileName(const char* path)
{
	String p = path;
	String name;

	int lastFolder = p.lastIndexOf('/');

	//If path contained /, increase offset to not include in name.
	if (lastFolder >= 0) lastFolder++;

	name = p.substring(lastFolder);
	return name;
}

void FileManager::DisplayFileContents(File* file)
{
	file->seek(0);

	serial->println("File Contents : ");

	while (file->available())
	{
		serial->write(file->read());
	}
	
	file->seek(0);
	DEBUG_NEWLINE();
}

void FileManager::DisplayEepromContents(EepromStream& stream)
{
	serial->println("Printing EEPROM Contents...");

	while (stream.available()) {
		serial->write(stream.read());
	}

	serial->println("\r\n...Finished.");
	DEBUG_NEWLINE();
}

void FileManager::DisplayEepromContents(size_t address, size_t length)
{
	
	EepromStream stream(address, length);
	DisplayEepromContents(stream);
}



uint32_t FileManager::GetFileCRC(const char* path)
{
	File file;
	//DEBUG_LOG_LN("Opening file to calculate CRC...");
	if (!OpenFile(&file, path, "r")) return 0;
	
	return GetFileCRC(&file, true);
}

uint32_t FileManager::GetFileCRC(File* file, bool close)
{
	DEBUG_LOG_LN("Calculating File CRC...");
	crc32.reset();

	while (file->available())
	{
		crc32.add(file->read());
	}

	if (close)
		file->close();
	else
		file->seek(0);

	uint32_t crc =  crc32.getCRC();
	DEBUG_LOG_F("CRC : 0x%0000000X\r\n", crc);
	return crc;
}

/// <summary>
/// Serializes a JSON document into a CRC32 algorithm,
/// and returns the result.
/// </summary>
/// <returns>CRC32 of the serialized document.</returns>
uint32_t FileManager::GetSerializedCRC(JsonDocument& doc, size_t* out_size)
{	
	DEBUG_LOG_LN("Calculating JsonDocument CRC...");
	Crc32Stream crcStream;
	size_t size = serializeJson(doc, crcStream);
	uint32_t crc = crcStream.crc.getCRC();
	DEBUG_LOG_F("CRC : 0x%0000000X\r\n", crc);

	if (out_size != nullptr)
		*out_size = size;

	return crc;
}

uint32_t FileManager::GetEepromCRC(EepromStream& eestream, size_t length)
{
	DEBUG_LOG_LN("Calculating EEPROM CRC...");
	crc32.reset();
	size_t count = 0;
	while (eestream.available() && count < length)
	{
		crc32.add(eestream.read());
		count++;
	}

	uint32_t crc = crc32.getCRC();
	DEBUG_LOG_F("CRC : 0x%0000000X\r\n", crc);
	return crc;
}
