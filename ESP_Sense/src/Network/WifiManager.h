#ifndef _WIFIMANAGER_h
#define _WIFIMANAGER_h

#pragma region Includes

#include <Arduino.h>

#include "../Config/config_master.h"
#include "../Config/global_status.h"

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif

#pragma endregion

namespace WifiManager
{

#pragma region Initialization Functions

	/// <summary>
	/// Initialize and connect to Wifi and/or Access Point depending on the mode.
	/// If mode is Default status.wifi.mode will be used.
	/// </summary>
	/// <param name="mode">Station, Access Point, or combined.</param>
	void Initialize();

	void Deinitialize();

	void RegisterEventHandlers();

	void UnregisterEventHandlers();

	bool SetPowerLevel(wifi_power_t power);

	wifi_power_t GetPowerLevel();

	void Loop();

	void ChangeWifiMode(wifi_mode_t mode, bool restart = false, bool saveRetained = false);

#pragma endregion

#pragma region Connection Functions

	/// <summary>
	/// Connect to Wifi and/or access point.
	/// </summary>
	/// <returns>0 = connected, 1 = wifi failed, 2 = Access Point failed, 3 = Both Failed, 4 = Entering Hotspot mode(STA Mode only)</returns>
	int Connect();

	/// <summary>
	/// Connect to Wifi.
	/// </summary>
	/// <returns>0 = connected, 1 = failed, 2 = Entering accessPoint mode, 3 = Incorrect wifi mode</returns>
	int ConnectWifi(bool waitForConnection = true);

	/// <summary>
	/// Reconnect to Wifi.
	/// </summary>
	/// <returns>0 = connected, 1 = failed, 2 = Entering accessPoint mode, 3 = Incorrect wifi mode, 4 = Wifi disabled</returns>
	int Reconnect(bool waitForConnection);

	/// <summary>
	/// Disconnects from both Wifi and Access Point.
	/// </summary>
	/// <returns>Bitmap : 0 = success, bit 0 = Wifi Failed, bit 1 = Access point failed</returns>
	uint8_t Disconnect();

	bool DisconnectWifi();

	/// <summary>
	/// Can a connection attempt be made at this time?
	/// </summary>
	/// <returns></returns>
	bool CanAttemptConnection();

	/// <summary>
	/// Handles reconnecting or disconnecting from Wifi and Access point depending on the enable flag.
	/// </summary>
	void MonitorConnection();

	void SetWifiConnectionLed();

	//const char* GetIPString();

#pragma endregion

	bool DisplayMessageIntervalMet();


	namespace AccessPoint {
		bool StartAdvertising(bool waitForConnection = true);
		bool Disconnect();
		bool CheckHotspotButton();
		uint8_t GetApClientCount();
		void SetAccessPointConnectionLed();
		//const char* GetIPString();
	}




	namespace Tasks
	{
		void StartWifiLoopTask();

		void StopWifiLoopTask();

#if defined(ESP8266)
		void WifiLoopTask();
#elif defined(ESP32)
		void WifiLoopTask(void* pvParameters);
#endif

		bool StartBlinkTask(bool station, bool accessPoint = false, uint16_t blinkRate = 1000, uint16_t stopAfter = 0);
		void StopBlinkTask();
		void BlinkTask(void* pvParameters);

	}

}

#endif

