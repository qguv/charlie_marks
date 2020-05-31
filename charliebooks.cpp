#include <SPIFFS.h>
#include <WiFi.h>
#include <WiFiSettings.h>
#include <ArduinoOTA.h>
#include <Adafruit_IS31FL3731.h>

WiFiClient wificlient;

Adafruit_IS31FL3731_Wing ledmatrix;

String msg;

//const char *msg =
//include "msg.txt"
//;

void setup_ota()
{
	ArduinoOTA.setHostname(WiFiSettings.hostname.c_str());
	ArduinoOTA.setPassword(WiFiSettings.password.c_str());
	ArduinoOTA.begin();
}

void setup()
{
	pinMode(13, OUTPUT);
	digitalWrite(13, LOW);

	Serial.begin(115200);
	SPIFFS.begin(true);

	ledmatrix = Adafruit_IS31FL3731_Wing();
	ledmatrix.begin();
	ledmatrix.setFrame(0);
	ledmatrix.setRotation(1);
	ledmatrix.clear();

	msg = WiFiSettings.string("message", 1, 65535, "Hello");

	WiFiSettings.onWaitLoop = []() {
		digitalWrite(13, HIGH);
		if (!ledmatrix.begin()) WiFiSettings.portal();
		return 50;
	};

	WiFiSettings.onSuccess = []() {
		digitalWrite(13, LOW);
		setup_ota();
	};

	WiFiSettings.onPortal = []() {
		digitalWrite(13, HIGH);
		if (ledmatrix.begin()) ESP.restart();
		setup_ota();
	};

	WiFiSettings.onPortalWaitLoop = []() {
		digitalWrite(13, HIGH);
		if (ledmatrix.begin()) ESP.restart();
		ArduinoOTA.handle();
	};

	WiFiSettings.connect();
}

void loop()
{
	static uint32_t i = 0;
	static uint8_t brightness = 0x00;

	if (!i) {
		brightness ^= 0x08;
	}

	ledmatrix.drawPixel(i % 15, i / 15, brightness);
	delay(50);
	i = (i + 1) % (15 * 7);
	ArduinoOTA.handle();
}
