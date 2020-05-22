#include <FastLED.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <WiFiSettings.h>
#include <ArduinoOTA.h>
#include <HTTPClient.h>

const int ledpin  = 27;
const int numleds = 25;
const int buttonpin = 39;
const int debounce_threshhold = 0;

enum button_state_t {
    NOT_PRESSED = 0,
    JUST_PRESSED,
    HELD,
    JUST_RELEASED,
};

bool button_state;
int debounce_rounds;

String url;

CRGB leds[numleds];
WiFiClient wificlient;

void setup_ota()
{
    ArduinoOTA.setHostname(WiFiSettings.hostname.c_str());
    ArduinoOTA.setPassword(WiFiSettings.password.c_str());
    ArduinoOTA.begin();
}

void setup()
{
    button_state = false;
    debounce_rounds = 0;

    FastLED.addLeds<WS2812B, ledpin, GRB>(leds, numleds);
    FastLED.setBrightness(20);

    Serial.begin(115200);
    SPIFFS.begin(true);
    pinMode(buttonpin, INPUT);

    url = WiFiSettings.string("url", 8, 512, "http://10.10.10.151/cm?cmnd=power1%202");

    WiFiSettings.onWaitLoop = []() {
        static CHSV color(0, 255, 255);
        color.hue += 10;
        FastLED.showColor(color);
        if (!digitalRead(buttonpin)) WiFiSettings.portal();
        return 50;
    };

    WiFiSettings.onSuccess = []() {
        FastLED.showColor(CRGB::Green);
        setup_ota();
        delay(200);
    };

    WiFiSettings.onPortal = []() {
        setup_ota();
    };

    WiFiSettings.onPortalWaitLoop = []() {
        static CHSV color(0, 255, 255);
        color.saturation--;
        FastLED.showColor(color);
        ArduinoOTA.handle();
    };

    WiFiSettings.connect();
}

enum button_state_t check_button()
{
    bool inst_button_state = !digitalRead(buttonpin);
    if (inst_button_state != button_state) {
        debounce_rounds++;
    } else if (debounce_rounds > 0) {
        debounce_rounds--;
    }

    if (debounce_rounds > debounce_threshhold) {
        debounce_rounds = 0;
        button_state = inst_button_state;
        return button_state ? JUST_PRESSED : JUST_RELEASED;
    }

    return button_state ? HELD : NOT_PRESSED;
}

void loop()
{
    static CHSV color(0, 255, 0);

    button_state_t btn = check_button();
    if (btn == JUST_PRESSED) {
        color.value = 255;
        color.hue += 10;
    }

    FastLED.showColor(color);

    if (btn == JUST_PRESSED) {
        HTTPClient http;
        http.begin(url.c_str());
        http.GET();
        http.end();
    }

    if (btn == NOT_PRESSED && color.value > 0) {
        color.value -= 1;
    }

    ArduinoOTA.handle();
}
