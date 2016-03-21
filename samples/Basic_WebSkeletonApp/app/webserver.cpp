#include <tytherm.h>


bool serverStarted = false;
HttpServer server;

void onIndex(HttpRequest &request, HttpResponse &response)
{
	response.setCache(86400, true); // It's important to use cache for better performance.
	response.sendFile("index.html");
}

void onConfiguration(HttpRequest &request, HttpResponse &response)
{

	if (request.getRequestMethod() == RequestMethod::POST)
	{
		debugf("Update configuration");

		if (request.getBody() == NULL)
		{
			debugf("NULL bodyBuf");
			return;
		}
		else
		{
			Serial.println(request.getBody());
			DynamicJsonBuffer jsonBuffer;
			JsonObject& root = jsonBuffer.parseObject(request.getBody());
			root.prettyPrintTo(Serial); //Uncomment it for debuging

			String StaSSID = root["StaSSID"].success() ? String((const char *)root["StaSSID"]) : "";
			String StaPassword = root["StaPassword"].success() ? String((const char *)root["StaPassword"]) : "";
			uint8_t StaEnable = root["StaEnable"].success() ? root["StaEnable"] : 255;

			if (StaEnable) // Settings
			{
				if (WifiStation.isEnabled())
				{
					WifiAccessPoint.enable(false);
				}
				else
				{
					WifiStation.enable(true, true);
					WifiAccessPoint.enable(false, true);
				}
				if (WifiStation.getSSID() != StaSSID || (WifiStation.getPassword() != StaPassword && StaPassword.length() >= 8))
				{
					WifiStation.config(StaSSID, StaPassword);
				}
			}
			else
			{
					WifiStation.enable(false, true);
					WifiAccessPoint.enable(true, true);
					WifiAccessPoint.config("WebApp", "ENTERYOURPASSWD", AUTH_WPA2_PSK);
			}
		}
	}
//		saveConfig(ActiveConfig);
	else
	{
		response.setCache(86400, true); // It's important to use cache for better performance.
		response.sendFile("config.html");
	}
}

void onConfiguration_json(HttpRequest &request, HttpResponse &response)
{
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();

	json["StaSSID"] = WifiStation.getSSID();
//	json["StaPassword"] = ActiveConfig.StaPassword;
	json["StaEnable"] = WifiStation.isEnabled() ? 1 : 0;

	response.sendJsonObject(stream);
}
void onFile(HttpRequest &request, HttpResponse &response)
{
	String file = request.getPath();
	if (file[0] == '/')
		file = file.substring(1);

	if (file[0] == '.')
		response.forbidden();
	else
	{
		response.setCache(86400, true); // It's important to use cache for better performance.
		response.sendFile(file);
	}
}

void onAJAXGetState(HttpRequest &request, HttpResponse &response)
{
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();

	json["counter"] = counter;

	response.sendJsonObject(stream);
}


void startWebServer()
{
	if (serverStarted) return;

	server.listen(80);
	server.addPath("/", onIndex);
	server.addPath("/config", onConfiguration);
	server.addPath("/config.json", onConfiguration_json);
	server.addPath("/state", onAJAXGetState);
	server.setDefaultHandler(onFile);
	serverStarted = true;

	if (WifiStation.isEnabled())
		debugf("STA: %s", WifiStation.getIP().toString().c_str());
	if (WifiAccessPoint.isEnabled())
		debugf("AP: %s", WifiAccessPoint.getIP().toString().c_str());
}
