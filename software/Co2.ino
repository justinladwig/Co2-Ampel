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
#define COLOR_GREEN 65536 / 3
#define COLOR_YELLOW 65536 / 6
#define COLOR_RED 0
#define COLOR_BLUE 65536 * 2 / 3

//Definieren der CO2 Werte
#define CO2_GOOD 400
#define CO2_WARNING 1000
#define CO2_BAD 2000

#define CO2_DEFAULT 400

#define CO2_ACC_ALERT 1700
#define CO2_VIS_ALERT 1400

#define CO2_LOOP_DELAY 1000

#define CO2_DIFFERENCE 500
#define CO2_MEASURE_AMOUNT 5
#define CO2_MEASURE_DELAY 1000

#define BEEP_DURATION 500
#define BEEP_AMOUNT 3
#define BLINK_DURATION 500
#define BLINK_AMOUNT 5

//Deklariern des Led-Objekts
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800);

//Deklariern des CO2-Sensor-Objekts
Adafruit_CCS811 ccs;

//Deklarieren des letzten CO2 Werts
int lastECO2 = CO2_DEFAULT;

/*Notizen

  - LEDs vorerst nicht blau.
  - Erstes Measurement 0, vorher nur 0 als Ausgabe
  - Blinken und Piepen gleichzeitig

*/

void setup()
{
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
  if (!ccs.begin())
  {
    Serial.println("Failed to start sensor! Please check your wiring.");
    while (1);
  }
}

void loop()
{
  int eCO2 = checkMeasurementECO2();
  Serial.print("Measurement: ");
  Serial.println(eCO2);
  uint32_t color = getColor(eCO2);
  strip.fill(color, 0, LED_COUNT);
  strip.show();

  if (eCO2 >= CO2_ACC_ALERT)
  {
    for (int i = 0; i < BEEP_AMOUNT; i++)
    {
      digitalWrite(PIEZO_PIN, HIGH);
      delay(BEEP_DURATION);
      digitalWrite(PIEZO_PIN, LOW);
      delay(BEEP_DURATION);
    }
  }

  if (eCO2 >= CO2_VIS_ALERT)
  {
    for (int i = 0; i < BLINK_AMOUNT; i++)
    {
      strip.clear();
      strip.show();
      delay(BLINK_DURATION);
      strip.fill(color, 0, LED_COUNT);
      strip.show();
      delay(BLINK_DURATION);
    }
  } else
  {
    delay(CO2_LOOP_DELAY);
  }
  

}

int checkMeasurementECO2()
{
  int eCO2;
  for (int i = 0; i < CO2_MEASURE_AMOUNT; i++)
  {
    eCO2 = measureECO2();
    if (abs(eCO2 - lastECO2) <= CO2_DIFFERENCE && eCO2 != 0 )
    {
      break;
    }
    delay(CO2_MEASURE_DELAY);
  }

  lastECO2 = eCO2;

  if (eCO2 == 0)
  {
    lastECO2 = CO2_DEFAULT;
    return 0;
  }
  return constrain(eCO2, CO2_GOOD, CO2_BAD);
}

int measureECO2()
{
  digitalWrite(SENSOR_WAKE_PIN, LOW);
  if (ccs.available())
  {
    if (!ccs.readData())
    {
      Serial.println(ccs.geteCO2());
      return ccs.geteCO2();
    }
    else
    {
      Serial.println("ERROR!");
    }
  }
  else
  {
    return 0;
  }
  digitalWrite(SENSOR_WAKE_PIN, HIGH);
}

uint32_t getColor(int eCO2)
{
  if (eCO2 >= CO2_GOOD && eCO2 <= CO2_WARNING)
  {
    return strip.ColorHSV(map(eCO2, CO2_GOOD, CO2_WARNING, COLOR_GREEN, COLOR_YELLOW));
  }
  else if (eCO2 >= CO2_WARNING && eCO2 <= CO2_BAD)
  {
    return strip.ColorHSV(map(eCO2, CO2_WARNING, CO2_BAD, COLOR_YELLOW, COLOR_RED));
  }
  return COLOR_BLUE;
}
