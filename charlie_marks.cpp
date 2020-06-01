#include <SPIFFS.h>
#include <WiFi.h>
#include <WiFiSettings.h>
#include <ArduinoOTA.h>
#include <Adafruit_IS31FL3731.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

WiFiClient wificlient;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

Adafruit_IS31FL3731_Wing ledmatrix;

String msg;
String msg_chunk;
uint16_t chunk_width;
int zone_sec;
bool ota_running;

const uint8_t number_widths[] = { 3, 1, 3, 3, 3, 3, 4, 4, 4, 4 };

const uint8_t numbers[] = {
	B00000010,
	B00000101,
	B00000101,
	B00000101,
	B00000101,
	B00000101,
	B00000010,

	B00000001,
	B00000001,
	B00000001,
	B00000001,
	B00000001,
	B00000001,
	B00000001,

	B00000010,
	B00000101,
	B00000001,
	B00000001,
	B00000010,
	B00000100,
	B00000111,

	B00000110,
	B00000001,
	B00000001,
	B00000110,
	B00000001,
	B00000001,
	B00000110,

	B00000001,
	B00000101,
	B00000101,
	B00000111,
	B00000001,
	B00000001,
	B00000001,

	B00000111,
	B00000100,
	B00000110,
	B00000001,
	B00000001,
	B00000101,
	B00000010,

	B00000011,
	B00000100,
	B00001000,
	B00001110,
	B00001001,
	B00001001,
	B00000110,

	B00001111,
	B00000001,
	B00000001,
	B00000010,
	B00000010,
	B00000100,
	B00001000,

	B00000110,
	B00001001,
	B00001001,
	B00000110,
	B00001001,
	B00001001,
	B00000110,

	B00000110,
	B00001001,
	B00001001,
	B00000111,
	B00000001,
	B00001001,
	B00000110,
};

void setup_ota()
{
	ArduinoOTA.setHostname(WiFiSettings.hostname.c_str());
	ArduinoOTA.setPassword(WiFiSettings.password.c_str());
	ArduinoOTA.onStart([]() {
			ledmatrix.clear();
			timeClient.end();
			ota_running = true;
	});
	ArduinoOTA.begin();
}

void next_string()
{
	int msg_pos = msg.indexOf(".");
	if (msg_pos == -1) {
		msg_chunk = msg;
		msg = String("");
	} else {
		msg_chunk = msg.substring(0, msg_pos + 1);
		msg.remove(0, msg_pos + 1);
	}

	static int16_t  x1 = 0, y1 = 0;
	static uint16_t h = 0;
	ledmatrix.getTextBounds(msg_chunk.c_str(), 0, 0, &x1, &y1, &chunk_width, &h);
	chunk_width += 15;
}

void show_digit(int n, int *pos)
{
	int w = number_widths[n];
	for (int row = 0; row < 7; row++) {
		for (int col = 0; col < w; col++) {
			ledmatrix.drawPixel(*pos + col, row, ((numbers[7 * n + row] >> (w - col - 1)) & 1) * 0x08);
		}
	}
	*pos += w;
}

void show_time()
{
	while (!timeClient.update())
		timeClient.forceUpdate();

	String fmt_t = timeClient.getFormattedTime().c_str();
	const char *t = fmt_t.c_str();

	ledmatrix.clear();

	int width = 0;
	if (t[0] == '1') {
		width += number_widths[1] + 1;
	}
	width += number_widths[t[1] - '0'] + 1;
	width += number_widths[t[3] - '0'] + 1;
	width += number_widths[t[4] - '0'];

	int pos = (15 - width + 1) / 2;

	if (t[0] == '1') {
		show_digit(1, &pos);
		pos += 1;
	}
	show_digit(t[1] - '0', &pos);
	ledmatrix.drawPixel(pos, 2, 0x02);
	ledmatrix.drawPixel(pos, 4, 0x02);
	pos += 1;
	show_digit(t[3] - '0', &pos);
	pos += 1;
	show_digit(t[4] - '0', &pos);
}

void setup()
{
	pinMode(13, OUTPUT);
	digitalWrite(13, LOW);

	SPIFFS.begin(true);

	ledmatrix = Adafruit_IS31FL3731_Wing();
	ledmatrix.begin();
	ledmatrix.setRotation(2);
	ledmatrix.clear();

	chunk_width = 0;
	ota_running = false;

	msg = WiFiSettings.string("message", 1, 65535, "Hello");
	msg.trim();

	zone_sec = WiFiSettings.integer("zone_sec", 7200);

	WiFiSettings.onWaitLoop = []() {
		digitalWrite(13, HIGH);
		if (!ledmatrix.begin()) WiFiSettings.portal();
		return 50;
	};

	WiFiSettings.onSuccess = []() {
		digitalWrite(13, LOW);
		timeClient.begin();
		timeClient.setTimeOffset(zone_sec);
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

	ledmatrix.setTextSize(1);
	ledmatrix.setTextWrap(false);
	ledmatrix.setTextColor(0x08);
	WiFiSettings.connect();
}

void loop()
{
	static int x = 0;
	static uint8_t frame = 0;
	static int clock_frames = 0;

	if (!ota_running) {
		if (x == chunk_width) {
			x = 0;
			next_string();

			show_time();

			clock_frames = 600;
		}


		if (clock_frames) {
			clock_frames--;
			delay(25);
		} else {
			ledmatrix.setFrame(frame);
			ledmatrix.clear();
			ledmatrix.setCursor(15 - x++, 0);
			ledmatrix.print(msg_chunk.c_str());
			ledmatrix.displayFrame(frame);
		}

		delay(25);

		frame ^= 1;
	}

	ArduinoOTA.handle();
}
