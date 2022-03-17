#include "CrcStream.h"

#include "Config/config_master.h"
#include "Config/global_status.h"

extern GlobalStatus_t status;

size_t Crc32Stream::write(uint8_t c)
{
	crc.add(c);
	return 1;
}

size_t Crc32Stream::write(const uint8_t* buffer, size_t length)
{
	crc.add(buffer, length);
	return length;
}



void Crc32FileStream::init(File* _file)
{
	file = _file;
}

size_t Crc32FileStream::write(uint8_t c)
{
	crc.add(c);
	return file->write(c);
}

size_t Crc32FileStream::write(const uint8_t* buffer, size_t length)
{
	crc.add(buffer, length);
	return file->write(buffer, length);
}

void Crc32EepromStream::init(EepromStream* _eepromStream)
{
	eepromStream = _eepromStream;
}

size_t Crc32EepromStream::write(uint8_t c)
{
	if (!status.device.eepromMounted) return 0;

	crc.add(c);
	return eepromStream->write(c);
}

size_t Crc32EepromStream::write(const uint8_t* buffer, size_t length)
{
	if (!status.device.eepromMounted) return 0;

	crc.add(buffer, length);
	return eepromStream->write(buffer, length);
}
