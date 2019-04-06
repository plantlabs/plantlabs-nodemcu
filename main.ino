#include "DHT.h"
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

#define DHTTYPE DHT11   // DHT 11
#define dht_dpin 0

ESP8266WiFiMulti WiFiMulti;

DHT dht(dht_dpin, DHTTYPE);
float totH = 0;
float totT = 0;
int SECONDS = 60;

String SERVER_ADDRESS = "http://35.189.55.162";
String API_ENDPOINT = "/api/update";
String macString;

void setup(void)
{
  dht.begin();
  Serial.begin(115200);
  Serial.println("Humidity and temperature sensor for Vsprout\n\n");

  WiFi.mode(WIFI_STA);
  WiFi.begin("OpenWrt", "PASSWORD");

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  String mac = WiFi.macAddress();

  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.println("IP address: " + WiFi.localIP());
  Serial.println("MAC address: " + WiFi.macAddress());

  for (int i = 0; i < mac.length(); i++) {

    if ((String)mac[i] != ":") {
      macString += mac[i];
    }
  }

  Serial.println(macString);


  delay(700);
}
void loop() {

  int dataSamples = 0;

  // Averages for the last seconds
  for (int i = 0; i < SECONDS; i++) {
    float currH = dht.readHumidity();
    float currT = dht.readTemperature();

    Serial.print("Current temperature = ");
    Serial.print(currT);
    Serial.print("C  ");
    Serial.print("humidity = ");
    Serial.print(currH);
    Serial.println("%  ");

    if ((!isnan(currH)) && (!isnan(currT))) {
      totH = totH + currH;
      totT = totT + currT;
      dataSamples++;
    } 

    delay(1000);
  }

  Serial.println("*********************");
  Serial.print("Average humidity = ");
  Serial.print(totH / dataSamples);
  Serial.print("%  ");
  Serial.print("temperature = ");
  Serial.print(totT / dataSamples);
  Serial.println("C  ");

  if ((WiFiMulti.run() == WL_CONNECTED)) {
    HTTPClient client;

    String averageTemp = String(totT / dataSamples);
    String averageHumidity = String(totH / dataSamples);

    dataSamples = 0;

    String json = "{\"mac\":\"" + macString + "\", \"temperature\":\"" + averageTemp + "\", \"humidity\":\"" + averageHumidity + "\"}";

    Serial.println("About to send: " + json + " to " + SERVER_ADDRESS + API_ENDPOINT);

    HTTPClient http;
    http.begin(SERVER_ADDRESS + API_ENDPOINT);
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(json);
    if (httpCode > 0) {
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);
      String payload = http.getString();
      Serial.println("Got back: " + payload);

    } else {
      Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }

  totH = 0.0;
  totT = 0.0;
  
}
