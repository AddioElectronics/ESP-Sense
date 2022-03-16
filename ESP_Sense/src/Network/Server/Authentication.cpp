#include "Authentication.h"

#include <ArduinoJson.hpp>
#include <ArduinoJson.h>

#include <WiFi.h>

#include "../WifiManager.h"

#include "../../../ESP_Sense.h"
#include "../Website/WebStrings.h"
#include "../../GlobalDefs.h"
#include "../../macros.h"
#include "../Website/WebStrings.h"
#include "../Website/WebpageServer.h"

extern Config_t config;
extern DeviceStatus_t status;

#if COMPILE_SERVER
extern AsyncWebServer server;
#endif	/*COMPILE_SERVER*/


//Private function definitions
namespace Network
{
	namespace Server
	{
		namespace Authentication
		{
			void SendStatus(AsyncWebServerRequest* request);
			void ParseAuthenticationData(AsyncWebServerRequest* request);
			void LogoutRequest(AsyncWebServerRequest* request);
		}
	}
}

void Network::Server::Authentication::Initialize()
{
	if (status.server.authConfigured) return;

	DEBUG_LOG_LN("Starting Authenticator...");

	//If authentication is not required, pre-set authenticated flag.
	status.server.authenticated = !config.server.authenticate;

	//Send the webpage if authentication has passed.
	server.on(Website::Strings::Urls::requestAuth, HTTP_GET, SendStatus);
	status.server.authConfigured = true;

	if (!config.server.authenticate)
	{
		DEBUG_LOG_LN("...No authentication required.");
		return;	//No authentication required
	}

	//AJAX POST request from webpage, parse the data and verify.
	server.on(Website::Strings::Urls::requestAuth, HTTP_POST, ParseAuthenticationData);
	server.on(Website::Strings::Urls::requestLogout, HTTP_POST, LogoutRequest);

	DEBUG_LOG_F("-Username : %s\r\n", config.server.user.c_str());
	DEBUG_LOG_F("-Password : %s\r\n", config.server.pass.c_str());

	DEBUG_LOG_LN("...Authenticator Started.\r\n");
}

bool Network::Server::Authentication::IsAuthenticated(IPAddress& remoteIp)
{
	return (status.server.clientIP == remoteIp && status.server.authenticated) || !config.server.authenticate;
}

bool Network::Server::Authentication::IsAuthenticated(AsyncWebServerRequest* request)
{
	IPAddress remoteIP = request->client()->remoteIP();
	return IsAuthenticated(remoteIP);
}

void Network::Server::Authentication::Logout()
{
	DEBUG_LOG_LN("Server Logging Out.");
	status.server.clientIP == IPAddress();
	status.server.authenticated = false;
}

#pragma region Private Functions

void Network::Server::Authentication::SendStatus(AsyncWebServerRequest* request)
{
	DEBUG_LOG_F("Sending Authentication status to browser : %d\r\n", status.server.authenticated);

	if (IsAuthenticated(request))
		request->send_P(200, "text/plain", "Authentication Successful.");
	else
		request->send_P(401, "text/plain", "401 : Invalid username or password.");
}

void Network::Server::Authentication::ParseAuthenticationData(AsyncWebServerRequest* request)
{
	DEBUG_LOG_LN("Authentication data received. Parsing...");

	StaticJsonDocument<512> doc;

	String body = request->getParam(0)->value();

	DEBUG_LOG("-Data : ");
	DEBUG_LOG_LN(body.c_str());

	DeserializationError derror = deserializeJson(doc, body);

	if (derror)
	{
		DEBUG_LOG("Error parsing Authentication Data : ");
		DEBUG_LOG_LN(derror.c_str());
		return SendStatus(request);
	}

	char* wwwUsername = nullptr;
	char* wwwPassword = nullptr;

	if (doc.containsKey("username"))
	{
		JsonArrayConst userarray = doc["username"].as<JsonArrayConst>();

		int size = userarray.size();

		//Length is not the same, username will not match.
		//Consider not authenticated.
		if (size != config.server.user.length())
			goto Label_NotAuthenticated;

		wwwUsername = (char*)malloc(userarray.size() + 1);
		wwwUsername[size] = 0;	//Set null terminator

		DEBUG_LOG("-Encrypted Username : ");
		DEBUG_LOG_LN(wwwUsername);
		for (uint8_t i = 0; i < size; i++)
		{
			uint8_t c = userarray[i];
			wwwUsername[i] = c ^ 0xAA;
		}

		//XORArray((uint8_t*)wwwUsername, size, 0xAA);

		DEBUG_LOG("-Username : ");
		DEBUG_LOG_LN(wwwUsername);
	}

	if (doc.containsKey("password"))
	{
		JsonArrayConst passarray = doc["password"].as<JsonArrayConst>();

		int size = passarray.size();

		//Length is not the same, password will not match.
		//Consider not authenticated.
		if (size != config.server.pass.length())
			goto Label_NotAuthenticated;

		wwwPassword = (char*)malloc(passarray.size() + 1);
		wwwPassword[size] = 0;	//Set null terminator

		DEBUG_LOG("-Encrypted Password : ");
		DEBUG_LOG_LN(wwwPassword);
		for (uint8_t i = 0; i < size; i++)
		{
			uint8_t c = passarray[i];
			wwwPassword[i] = c ^ 0xAA;
		}

		//XORArray((uint8_t*)wwwPassword, size, 0xAA);

		DEBUG_LOG("-Password : ");
		DEBUG_LOG_LN(wwwPassword);
	}

	/*
	ArduinoJSON changes the bits while deserializing,
	must use string array.
	*/

	//if (doc.containsKey("username"))
	//{
	//	www_username = (const char*)doc["username"];
	//	DEBUG_LOG_F("Encrypted Username : %s\r\n", www_username);
	//	test(www_username.c_str(), 7);
	//	XORArray((uint8_t*)www_username.c_str(), www_username.length(), 0xAA);
	//	test(www_username.c_str(), 7);
	//	DEBUG_LOG_F("Username : %s\r\n", www_username.c_str());
	//}

	//if (doc.containsKey("password"))
	//{
	//	www_password = doc["password"].as<String>();

	//	DEBUG_LOG_F("Encrypted Password : %s\r\n", www_password.c_str());
	//	XORArray((uint8_t*)www_password.c_str(), www_password.length(), 0xAA);
	//	DEBUG_LOG_F("Password : %s\r\n", www_password.c_str());
	//}


	if (config.server.user == wwwUsername && config.server.pass == wwwPassword)
	{
		status.server.authenticated = true;
		status.server.clientIP = request->client()->remoteIP();
		status.server.sessionEnd = config.server.sessionTimeout ? millis() + (config.server.sessionTimeout * 60000) : 0;
	}

Label_NotAuthenticated:

	if (wwwUsername != nullptr)
		free(wwwUsername);

	if (wwwPassword != nullptr)
		free(wwwPassword);

	SendStatus(request);
}

void Network::Server::Authentication::LogoutRequest(AsyncWebServerRequest* request)
{
	if (status.server.clientIP == request->client()->remoteIP())
	{
		Logout();
		request->send_P(200, "text/plain", "Logout Successful.");
		return;
	}

	request->send(401);
}

#pragma endregion