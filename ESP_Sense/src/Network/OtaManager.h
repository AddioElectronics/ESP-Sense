#ifndef _OTAMANAGER_h
#define _OTAMANAGER_h

#include <Arduino.h>

#include "../Config/config_master.h"
#include "../Config/global_status.h"

namespace Network 
{
	namespace OTA 
	{
		void Initialize();
		void Deinitialize();
		void Loop();
		void Enable();
		void Disable();

		namespace Tasks
		{
			void InitializeOtaLoopTask();
			void StopLoopTask();

#if defined(ESP8266)
			bool OtaLoopTask()
#elif defined(ESP32)
			void OtaLoopTask(void* pvParameters);
#endif

			void ChangeCurrentRecurRate();
		}
	}
}


#endif

