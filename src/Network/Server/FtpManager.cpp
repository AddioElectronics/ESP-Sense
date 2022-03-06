#include "FtpManager.h"
#if COMPILE_FTP

#include <StreamUtils.hpp>
#include <StreamUtils.h>	

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <LittleFS.h>	
#elif defined(ESP32)
#include <FS.h>
#include <WiFi.h>
#include <LITTLEFS.h>
#endif

#include <FTPServer.h>
#include <FTPCommon.h>

#include "../../../ESPSense.h"
#include "../../Config/config_mqtt.h""
#include "../../macros.h"

FTPServer ftpServer(ESP_FS);
extern Config_t config;
extern DeviceStatus_t status;

TaskHandle_t taskFtpLoop;

void Network::FTP::Construct()
{
	if (status.server.ftp.enabled) return;	//Already started.

	DEBUG_LOG_LN("Starting FTP Server...");

	if(config.server.ftp.anonymous)
		DEBUG_LOG("-Anonymouse : true");
	else
	{
		DEBUG_LOG("-Username : ");
		DEBUG_LOG_LN(config.server.ftp.user);
		DEBUG_LOG("-Password : ");
		DEBUG_LOG_LN(config.server.ftp.pass);
	}

	if (config.server.ftp.anonymous)
		ftpServer.begin("", "");
	else
		ftpServer.begin(config.server.ftp.user, config.server.ftp.pass);

	status.server.ftp.enabled = true;

	Tasks::InitializeFTPLoopTask();

	//DEBUG_LOG_F("FTP Server Started. \r\nIP : %s\r\nUsername : %s\r\nPassword : %s\r\n", WiFi.localIP(), config.server.ftp.user, config.server.ftp.pass);	//Local IP isn't working with printf.
	DEBUG_LOG_LN("FTP Server Started.");

	if (status.wifi.mode != WIFI_MODE_APSTA)
		DEBUG_LOG("IP : ");

#if defined(ESP8266)
	switch (WiFi.getMode())
	{
	case WIFI_AP:
	case WIFI_AP_STA:
		DEBUG_LOG_LN(WiFi.softAPIP());
	case WIFI_STA:
		DEBUG_LOG_LN(WiFi.localIP());
		break;
	}
#elif defined(ESP32)
	switch (WiFi.getMode())
	{
	case WIFI_MODE_AP:
		DEBUG_LOG_LN(WiFi.softAPIP().toString().c_str());
		break;
	case WIFI_MODE_APSTA:
		DEBUG_LOG_F("Local IP: %s\r\n", status.wifi.station.ip.toString().c_str());
		DEBUG_LOG_F("AP IP : %s\r\n", status.wifi.accessPoint.ip.toString().c_str());
		break;
	case WIFI_MODE_STA:
		DEBUG_LOG_LN(WiFi.localIP().toString().c_str());
		break;
	}
#endif
	DEBUG_LOG("Username : ");
	DEBUG_LOG_LN(config.server.ftp.user);
	DEBUG_LOG("Password : ");
	DEBUG_LOG_LN(config.server.ftp.pass);
}

void Network::FTP::Stop()
{
	if (!status.server.ftp.enabled) return;

	if (status.device.tasks.ftpTaskRunning)
		Tasks::StopLoopTask();

	ftpServer.stop();
	status.server.ftp.enabled = false;
	DEBUG_LOG_LN("FTP Server stopped.");

}

void Network::FTP::SetTimeout(uint32_t timeout)
{
	if(timeout == 0)
		DEBUG_LOG_LN("FTP Timeout being set to default.");
	else
		DEBUG_LOG_F("FTP Timeout being set to %d milliseconds.\r\n", timeout);

	config.server.ftp.timeout = timeout;

	if (timeout == 0)
		ftpServer.setTimeout();
	else
		ftpServer.setTimeout(timeout);

}


void Network::FTP::Loop()
{
	if (status.server.ftp.enabled)
		ftpServer.handleFTP();

	if (!status.server.ftp.enabled || status.device.tasks.ftpTaskRunning || !status.wifi.connected) return;

	if (status.device.tasks.enabled)
	{
		Tasks::InitializeFTPLoopTask();
	}

	//ftpServer.handleFTP();
	//EspSense::YieldWatchdog(1);
}

void Network::FTP::Handle()
{
	if (status.server.ftp.enabled)
		ftpServer.handleFTP();
}

void Network::FTP::Tasks::InitializeFTPLoopTask()
{
	status.server.ftp.enabled = true;

	if (status.device.tasks.ftpTaskRunning) return;

	status.device.tasks.ftpTaskRunning = true;

#if defined(ESP8266)
	schedule_recurrent_function_us([]() {
		bool result = Network::FTP::FtpLoopTask();

		if (!result)
		{
			status.device.tasks.ftpTaskRunning = false;
		}

		return result;
	}, config.device.taskSettings.recurRate);
#elif defined(ESP32)
	xTaskCreatePinnedToCore(
		Network::FTP::Tasks::FtpLoopTask,			// Task function
		"FTPLoop",									// Name
		config.server.ftp.taskSettings.stackSize,	// Stack size
		NULL,										// Parameters
		config.server.ftp.taskSettings.priority,	// Priority
		&taskFtpLoop,								// Task handle
		config.server.ftp.taskSettings.core);		// Core
#endif





}

void Network::FTP::Tasks::StopLoopTask()
{
	if (!status.device.tasks.ftpTaskRunning) return;
	status.device.tasks.ftpTaskRunning = false;
	status.server.ftp.enabled = false;
	vTaskDelete(taskFtpLoop);
}

#if defined(ESP8266)
bool Network::FTP::FtpLoopTask()
{

}

#elif defined(ESP32)
void Network::FTP::Tasks::FtpLoopTask(void* pvParameters) {
	DEBUG_LOG("Network::FTP::FtpLoopTask() started on core ");
	DEBUG_LOG_LN(xPortGetCoreID());
	uint8_t stack;
	uint8_t* stackStart = &stack;
	status.device.tasks.ftpTaskRunning = true;
	//status.server.ftp.enabled = true;

#if COMPILE_ALIVE_MSG
	unsigned long nextAliveMessage;
	unsigned long loopTimestamp;
#endif

	for (;;) {

		//delay(1);
		EspSense::YieldWatchdog(config.server.ftp.taskSettings.recurRate);

		if (!status.server.ftp.enabled || !status.wifi.connected)
			break;

			ftpServer.handleFTP();

#if COMPILE_ALIVE_MSG
		if (millis() > nextAliveMessage)
		{
			nextAliveMessage = millis() + 10000;
			DEBUG_LOG_LN("FtpLoopTask Alive");
			EspSense::DisplayMemoryInfo();
			DEBUG_LOG_F("-Time : %d\r\n", millis() - loopTimestamp);
			DEBUG_NEWLINE();
		}

		loopTimestamp = millis();
#endif
	}

	DEBUG_LOG_LN("Exiting FTPLoop Task");

	status.device.tasks.ftpTaskRunning = false;
	//status.server.ftp.enabled = false;
	vTaskDelete(NULL);
}
#endif


#else
void Network::FTP::Construct() {}
void Network::FTP::Stop() {}
void Network::FTP::SetTimeout(uint32_t timeout) {}
void Network::FTP::Loop() {}
#endif
