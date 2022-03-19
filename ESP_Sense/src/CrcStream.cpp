#include "CrcStream.h"

#include "Config/config_master.h"
#include "Config/global_status.h"

extern GlobalStatus_t status;


size_t Crc32Stream::write(uint8_t c)
{
	crc.add(c);
	writeSerial(c);
	return 1;
}

size_t Crc32Stream::write(const uint8_t* buffer, size_t length)
{
	crc.add(buffer, length);
	writeSerial(buffer, length);
	return length;
}

size_t Crc32Stream::writeSerial(uint8_t c)
{
	if (_serial != nullptr)
		return _serial->write(c);

	return 0;
}

size_t Crc32Stream::writeSerial(const uint8_t* buffer, size_t length)
{
	if (_serial != nullptr)
		return _serial->write(buffer, length);

	return 0;
}



Crc32FileStream::Crc32FileStream(File* _file) /*: Crc32Stream()*/
{
	crc.reset();
	file = _file;
	_serial = nullptr;
}

Crc32FileStream::Crc32FileStream(File* _file, HardwareSerial* _ser) /*: Crc32Stream(_ser)*/
{
	crc.reset();
	file = _file;
	_serial = _ser;
}

size_t Crc32FileStream::write(uint8_t c)
{
	crc.add(c);
	writeSerial(c);
	return file->write(c);
}

size_t Crc32FileStream::write(const uint8_t* buffer, size_t length)
{
	crc.add(buffer, length);
	writeSerial(buffer, length);
	return file->write(buffer, length);
}

Crc32EepromStream::Crc32EepromStream(EepromStream* _eepromStream) /*: Crc32Stream()*/
{
	crc.reset();
	eepromStream = _eepromStream;
	_serial = nullptr;
}

Crc32EepromStream::Crc32EepromStream(EepromStream* _eepromStream, HardwareSerial* _ser) /*: Crc32Stream(_ser)*/
{
	crc.reset();
	eepromStream = _eepromStream;
	_serial = _ser;
}


size_t Crc32EepromStream::write(uint8_t c)
{
	if (!status.device.eepromMounted) return 0;

	crc.add(c);
	writeSerial(c);
	return eepromStream->write(c);
}

size_t Crc32EepromStream::write(const uint8_t* buffer, size_t length)
{
	if (!status.device.eepromMounted) return 0;

	crc.add(buffer, length);
	writeSerial(buffer, length);
	return eepromStream->write(buffer, length);
}

Crc32FileEepromStream::Crc32FileEepromStream(File* _file, EepromStream* _eepromStream)
{
	crc.reset();
	file = _file;
	eepromStream = _eepromStream;
	_serial = nullptr;
}

Crc32FileEepromStream::Crc32FileEepromStream(File* _file, EepromStream* _eepromStream, HardwareSerial* _ser)
{
	crc.reset();
	file = _file;
	eepromStream = _eepromStream;
	_serial = _ser;
}

size_t Crc32FileEepromStream::write(uint8_t c)
{
	crc.add(c);
	writeSerial(c);

	//if (status.device.eepromMounted)
	eepromStream->write(c);

	//if(status.device.fsMounted)
	return file->write(c);
}

size_t Crc32FileEepromStream::write(const uint8_t* buffer, size_t length)
{
	crc.add(buffer, length);
	writeSerial(buffer, length);

	//if (status.device.eepromMounted)
	eepromStream->write(buffer, length);

	//if (status.device.fsMounted)
	return file->write(buffer, length);
}
