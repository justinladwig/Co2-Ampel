#include <BearSSLHelpers.h>
#include <CertStoreBearSSL.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiGratuitous.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiType.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiClientSecureAxTLS.h>
#include <WiFiClientSecureBearSSL.h>
#include <WiFiServer.h>
#include <WiFiServerSecure.h>
#include <WiFiServerSecureAxTLS.h>
#include <WiFiServerSecureBearSSL.h>
#include <WiFiUdp.h>

#include <Adafruit_NeoPixel.h>
#include <Adafruit_CCS811.h>

//Definieren der Pins
#define SENSOR_WAKE_PIN D5
#define LED_PIN D6
#define PIEZO_PIN D7

//Definieren der Parameter für die Leds
#define LED_COUNT 2
#define BRIGHTNESS 255

//Definieren der Farben
#define COLOR_GREEN 65536/3
#define COLOR_YELLOW 65536/6
#define COLOR_RED 0
#define COLOR_BLUE 65536 * 2 / 3

//Definieren der CO2 Werte
#define CO2_GOOD 400
#define CO2_WARNING 1000
#define CO2_BAD 2000

#define CO2_ACC_ALERT 1800
#define CO2_VIS_ALERT 1500

//Deklariern des Led-Objekts
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800);

//Deklariern des CO2-Sensor-Objekts
Adafruit_CCS811 ccs;

/*Notizen

  bei großer Abweichung erneut messen und wenn Wert wahr, langsamen Übergang der LEDs mit for-Schleife
  Delays verkürzen
  ersten wert löschen, da immer 0

*/

void setup() {
  WiFi.mode(WIFI_OFF);
  Serial.begin(115200);

  pinMode(PIEZO_PIN, OUTPUT);
  pinMode(SENSOR_WAKE_PIN, OUTPUT);

  //Leds starten
  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.show();


  digitalWrite(SENSOR_WAKE_PIN, LOW);
  //Sensor starten
  if (!ccs.begin()) {
    Serial.println("Failed to start sensor! Please check your wiring.");
    while (1);
  }
}

void loop() {

  if (ccs.available()) {
    if (!ccs.readData()) {
      int eCO2raw = ccs.geteCO2();
      int eCO2 = constrain(eCO2raw, CO2_GOOD, CO2_BAD);
      Serial.print(eCO2raw);
      Serial.println(" ppm");
      strip.fill(getColor(eCO2), 0, LED_COUNT);
      strip.show();

      if (eCO2 >= CO2_ACC_ALERT) {
        delay(50);
        alert();
      } else {
        delay(50);
      }
    } else {
      //Fehler
      Serial.println("ERROR!");
    }
  }

}

void alert() {
  while (1) {
    if (ccs.available()) {
      if (!ccs.readData()) {
        int eCO2raw = ccs.geteCO2();
      int eCO2 = constrain(eCO2raw, CO2_GOOD, CO2_BAD);
        Serial.print(eCO2raw);
        Serial.println(" ppm");
        if (eCO2 < CO2_ACC_ALERT) {
          break;
        }
        uint32_t color = getColor(eCO2);

        strip.fill(getColor(eCO2), 0, LED_COUNT);
        strip.show();
        digitalWrite(PIEZO_PIN, HIGH);

        delay(500);

        strip.clear();
        strip.show();

        delay(500);

        digitalWrite(PIEZO_PIN, LOW);
        strip.fill(getColor(eCO2), 0, LED_COUNT);
        strip.show();

        delay(500);

        strip.clear();
        strip.show();

        delay(500);

        digitalWrite(PIEZO_PIN, HIGH);
        strip.fill(getColor(eCO2), 0, LED_COUNT);
        strip.show();

        delay(500);

        strip.clear();
        strip.show();

        delay(500);

        digitalWrite(PIEZO_PIN, LOW);
        for (int i = 0; i < 1; i++) {
          strip.fill(getColor(eCO2), 0, LED_COUNT);
          strip.show();
          delay(500);
          strip.clear();
          strip.show();
          delay(500);
        }

      } else {
        //Fehler
      }
    }
  }
}


uint32_t getColor(int eCO2) {
  if (eCO2 >= CO2_GOOD && eCO2 <= CO2_WARNING) {
    return strip.ColorHSV(map(eCO2, CO2_GOOD, CO2_WARNING, COLOR_GREEN, COLOR_YELLOW));
  } else if (eCO2 >= CO2_WARNING && eCO2 <= CO2_BAD) {
    return strip.ColorHSV(map(eCO2, CO2_WARNING, CO2_BAD, COLOR_YELLOW, COLOR_RED));
  }
  return COLOR_BLUE;
}
