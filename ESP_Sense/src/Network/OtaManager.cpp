#include "OtaManager.h"

#include <ArduinoOTA.h>
#include <WiFiUdp.h>

#include "../../ESP_Sense.h"
#include "../macros.h"
#include "../FileManager.h"

TaskHandle_t taskOtaLoop;

void Network::OTA::Initialize()
{
#if COMPILE_OTA
	status.server.ota.enabled = true;

	if (status.server.ota.configured) return;

	DEBUG_LOG_LN("\r\nInitializing OTA...");

	ArduinoOTA.onStart([]() {
		String type;
		if (ArduinoOTA.getCommand() == U_FLASH) {
			type = "sketch";
		}
		else { // U_FS
			type = "filesystem";
			FileManager::UnMountFileSystem();
		}

		status.server.ota.updating = true;

		if (status.server.updating != UpdateMode::ESP_UPDATE_NULL)
			EspSense::EnterUpdateMode(UpdateMode::ESP_UPDATE_OTA);

#warning send POST to browser, if browser is open it will redirect to page to watch status. Also disable other modules.

		DEBUG_LOG_LN("OTA Start updating " + type);
	});
	ArduinoOTA.onEnd([]() {
		DEBUG_LOG_LN("\r\n...OTA Service Stopped.");
		status.server.ota.updating = false;
	});
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
		DEBUG_LOG_F("OTA Progress: %u%%\r\n", (progress / (total / 100)));
		status.server.ota.updating = true;
	});
	ArduinoOTA.onError([](ota_error_t error) {

		//Not sure if error will stop update.
		//Added "= true" to onProgress just incase it keeps going.
		status.server.ota.updating = false;


		DEBUG_LOG_F("OTA Error[%u]: ", error);
		if (error == OTA_AUTH_ERROR) {
			DEBUG_LOG_LN("OTA Auth Failed");
		}
		else if (error == OTA_BEGIN_ERROR) {
			DEBUG_LOG_LN("OTA Begin Failed");
		}
		else if (error == OTA_CONNECT_ERROR) {
			DEBUG_LOG_LN("OTA Connect Failed");
		}
		else if (error == OTA_RECEIVE_ERROR) {
			DEBUG_LOG_LN("OTA Receive Failed");
		}
		else if (error == OTA_END_ERROR) {
			DEBUG_LOG_LN("OTA End Failed");
		}
	});

#if defined(ESP8266)
	ArduinoOTA.begin(false);			//Do not use DNS, as we have already created a DNS server.
#elif defined(ESP32)
	ArduinoOTA.setMdnsEnabled(false);	//Do not use DNS, as we have already created a DNS server.
	ArduinoOTA.begin();
#endif

	Tasks::InitializeOtaLoopTask();

	DEBUG_LOG_LN("...OTA Ready.");

#endif
}

void Network::OTA::Deinitialize()
{
#if COMPILE_OTA
	status.server.ota.enabled = false;
	status.server.ota.configured = false;
	status.server.ota.updating = false;
	ArduinoOTA.end();
#endif
}

void Network::OTA::Loop()
{
#if COMPILE_OTA
	if (!status.server.ota.enabled || !status.wifi.connected || status.device.tasks.otaTaskRunning) return;

	if (status.device.tasks.enabled)
	{
		//Reinitialize loop task.
		Tasks::InitializeOtaLoopTask();
		/*return;*/
	}		

	//ArduinoOTA.handle();
	//EspSense::YieldWatchdog(1);
#endif
}

void Network::OTA::Enable()
{
#if COMPILE_OTA
	status.server.ota.enabled = status.server.ota.configured;
	//ArduinoOTA.begin();
#endif
}

void Network::OTA::Disable()
{
#if COMPILE_OTA
	if (!status.server.ota.enabled) return;

	status.server.ota.enabled = false;

	if (status.device.tasks.otaTaskRunning)
		Network::OTA::Tasks::StopLoopTask();
#endif
}

void Network::OTA::Tasks::InitializeOtaLoopTask()
{
	status.server.ota.enabled = true;

	if (status.device.tasks.otaTaskRunning) return;

	status.device.tasks.otaTaskRunning = true;

#if defined(ESP8266)
	schedule_recurrent_function_us([]() {
		bool result = Network::OTA::Tasks::OtaLoopTask();

		if (!result)
		{
			status.device.tasks.otaTaskRunning = false;
		}

		return result;
	}, config.device.taskSettings.recurRate);
#elif defined(ESP32)
	xTaskCreatePinnedToCore(
		Network::OTA::Tasks::OtaLoopTask,							// Task function
		"OTALoop",													// Name
		config.server.ota.taskSettings.stackSize,					// Stack size
		NULL,														// Parameters
		config.server.ota.taskSettings.priority,					// Priority
		&taskOtaLoop,												// Task handle
		CPU_DUALCORE ? 1 : config.server.ota.taskSettings.core);	// Core	*Must be on core 1 as once update has started it does not leave the loop.
#endif

}

void Network::OTA::Tasks::StopLoopTask()
{
	if (!status.device.tasks.otaTaskRunning) return;
	status.device.tasks.otaTaskRunning = false;
	status.server.ota.enabled = false;
	vTaskDelete(taskOtaLoop);
}

#if defined(ESP8266)
bool Network::OTA::FtpLoopTask()
{

}

#elif defined(ESP32)
void Network::OTA::Tasks::OtaLoopTask(void* pvParameters) {
	DEBUG_LOG("Network::OTA::OtaLoopTask() started on core ");
	DEBUG_LOG_LN(xPortGetCoreID());

	status.device.tasks.otaTaskRunning = true;

	unsigned long nextAliveMessage;

	for (;;) {

		//If updating, stay in loop indefinitely.
#if CPU_DUALCORE
		if (status.server.ota.updating)
			EspSense::FeedWatchdog();
		else
			EspSense::YieldWatchdog(config.server.ota.taskSettings.recurRate);
#else
		if (status.server.ota.updating)
		{
			EspSense::FeedWatchdog();
			yield();
		}
		else
			EspSense::YieldWatchdog(config.server.ota.taskSettings.recurRate);
#endif

		if (!status.device.tasks.enabled || !status.server.ota.enabled || !status.wifi.connected)
			break;

			ArduinoOTA.handle();

#if COMPILE_ALIVE_MSG
		if (millis() > nextAliveMessage)
		{
			nextAliveMessage = millis() + 10000;

			DEBUG_LOG_LN("OtaLoopTask Alive");
			EspSense::DisplayMemoryInfo();
			DEBUG_NEWLINE();
		}
#endif
	}

	DEBUG_LOG_LN("Exiting OTALoop Task");

	status.device.tasks.otaTaskRunning = false;
	vTaskDelete(NULL);
}
#endif


