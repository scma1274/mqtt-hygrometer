
// Bibliotheken einbinden
#include "DHT.h"
#include <ESP8266WiFi.h>
#include <ThingsBoard.h>

// WIFI und MQTT #define
#define WIFI_AP "#nutzeuereigenesinternet"
#define WIFI_PASSWORD "jaegermeister4lifebitches!"

#define TOKEN "BYK018bhUilpJZjQKWfX"

// DHT #define
#define DHTPIN 2            // Signal ist mit GPIO2 verbunden (entspricht PIN 4)
#define DHTTYPE DHT11       // Verwendung des DHT11 Sensors


char thingsboardServer[] = "demo.thingsboard.io";

WiFiClient wifiClient;

// Initialisieren des DHT Sensors
DHT dht(DHTPIN, DHTTYPE);

ThingsBoard tb(wifiClient);

int status = WL_IDLE_STATUS;
unsigned long lastSend;

void setup()
{
  Serial.begin(115200);
  dht.begin();
  delay(10);
  InitWiFi();
  lastSend = 0;
}

void loop()
{
  if ( !tb.connected() ) {
    reconnect();
  }

  if ( millis() - lastSend >1000 ) {                 // Die Temperatur und Luftfeuchtigkeit wird alle 5 Sekunden abgefragt und gesendet               
    getAndSendTemperatureAndHumidityData();
    lastSend = millis();
  }

  tb.loop();
}

void getAndSendTemperatureAndHumidityData()                   // Funktion, die LF und LT ausliest und dann an den MQTT Broker sendet
{
  Serial.println("Collecting temperature data.");

  float humidity = dht.readHumidity();                    // Auslesen von Temperatur (LT) und Luftfeuchtigkeit (LF)
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {            // Validitätsüberprüfung
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.println("Sending data to ThingsBoard:");
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" *C ");

  tb.sendTelemetryFloat("temperature", temperature);
  tb.sendTelemetryFloat("humidity", humidity);
}

void InitWiFi()                           // Funktion zur Herstellung einer WiFi-Verbindung
{
  Serial.println("Connecting to AP ...");
  // attempt to connect to WiFi network

  WiFi.begin(WIFI_AP, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}


void reconnect() {                    // Funktion zur Wiederherstellung der WiFi-Verbindung
  while (!tb.connected()) {
    status = WiFi.status();
    if ( status != WL_CONNECTED) {
      WiFi.begin(WIFI_AP, WIFI_PASSWORD);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println("Connected to AP");
    }
    Serial.print("Connecting to ThingsBoard node ...");     // Verbindungsherstellung zum MQTT Broker
    if ( tb.connect(thingsboardServer, TOKEN) ) {
      Serial.println( "[DONE]" );
    } else {
      Serial.print( "[FAILED]" );
      Serial.println( " : retrying in 5 seconds]" );
      // Wait 5 seconds before retrying
      delay( 5000 );
    }
  }
}
