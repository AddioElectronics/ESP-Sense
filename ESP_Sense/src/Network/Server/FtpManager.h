#ifndef _FTPMANAGER_h
#define _FTPMANAGER_h

#include <Arduino.h>

#include "../../Config/config_master.h"
#include "../../Config/global_status.h"

namespace Network 
{
	namespace FTP 
	{
		void Construct();
		void Stop();
		void SetTimeout(uint32_t timeout);

		/// <summary>
		/// Handle FTP server to allow new configuration files to be uploaded.
		/// Device will need to be reset after config is uploaded.
		/// </summary>
		void Loop();

		void Handle();

		namespace Tasks
		{
			void InitializeFTPLoopTask();

			void StopLoopTask();

#if defined(ESP8266)
			bool FtpLoopTask()
#elif defined(ESP32)
			void FtpLoopTask(void* pvParameters);
#endif
		}
	}
}
#endif

