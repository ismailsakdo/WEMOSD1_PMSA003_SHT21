#include <ESP8266WiFi.h>
#include <SHT21.h>
#include <Wire.h>
#include "Adafruit_PM25AQI.h"
#include <SoftwareSerial.h>
#include<LiquidCrystal_I2C.h>

Adafruit_PM25AQI aqi = Adafruit_PM25AQI();
SoftwareSerial pmSerial(2, 3);
SHT21 sht;
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Wi-Fi Settings
const char* ssid = "YOUR_SSID"; // your wireless network name (SSID)
const char* password = "YOUR_PASSWORD"; // your Wi-Fi network password

WiFiClient client;

// ThingSpeak Settings
const int channelID = 1526988;
String writeAPIKey = "YOUR_API_THINGSPEAK"; // write API key for your ThingSpeak Channel
const char* apiKey = "api.thingspeak.com";
const int postingInterval = 15000; // 60 will post data for 20 sec in thingspeak

void setup() {
  lcd.init();
  lcd.backlight();
  Wire.begin();
  Serial.begin(115200);
  pmSerial.begin(9600);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  if (! aqi.begin_UART(&pmSerial)) {
    Serial.println("Could not find PM 2.5 sensor!");
    while (1) delay(10);
  }

  Serial.println("PM25 found!");
  lcd.print("Particulate Sense");
  lcd.setCursor(0,1);
  lcd.print("Temp & Humid");
  delay(5000);
}

void loop() {
  
  PM25_AQI_Data data;

  if (! aqi.read(&data)) {
    Serial.println("Could not read from AQI");
    delay(500);  // try again in a bit!
    return;
  }
  
  if (client.connect(apiKey, 80)) {

    float t = sht.getTemperature();
    float h = sht.getHumidity();

    String body = "GET /update?api_key=";
    body += apiKey;
    body += "&field1=";
    body += String(t);
    body += apiKey;
    body += "&field2=";
    body += String(h);
    body += apiKey;
    body += "&field3=";
    body += String(data.pm10_standard);
    body += apiKey;
    body += "&field4=";
    body += String(data.pm10_standard);
    body += apiKey;
    body += "&field5=";
    body += String(data.pm25_standard);
    body += apiKey;
    body += "&field6=";
    body += String(data.pm100_standard);
    body += apiKey;
    body += "&field7=";
    body += String(data.pm10_env);
    body += apiKey;
    body += "&field8=";
    body += String(data.pm100_env);
    body += "\r\n\r\n";


    client.println("POST /update HTTP/1.1");
    client.println("Host: api.thingspeak.com");
    client.println("User-Agent: ESP8266 (nothans)/1.0");
    client.println("Connection: close");
    client.println("X-THINGSPEAKAPIKEY: " + writeAPIKey);
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println("Content-Length: " + String(body.length()));
    client.println("");
    client.print(body);
    
    Serial.println(F("---------------------------------------"));
    Serial.println(F("Concentration Units (standard)"));
    Serial.println(F("---------------------------------------"));
    Serial.print(F("PM 1.0: ")); Serial.print(data.pm10_standard);
    Serial.print(F("\t\tPM 2.5: ")); Serial.print(data.pm25_standard);
    Serial.print(F("\t\tPM 10: ")); Serial.println(data.pm100_standard);
    Serial.println(F("Concentration Units (environmental)"));
    Serial.println(F("---------------------------------------"));
    Serial.print(F("PM 1.0: ")); Serial.print(data.pm10_env);
    Serial.print(F("\t\tPM 2.5: ")); Serial.print(data.pm25_env);
    Serial.print(F("\t\tPM 10: ")); Serial.println(data.pm100_env);
    Serial.println(F("---------------------------------------"));

    lcd.clear();
    lcd.setCursor(0,0); //set coursor Column (1-16) and row (1-2)
    lcd.print("T:");
    lcd.setCursor(2,0);
    lcd.print(t);

    lcd.setCursor(9,0);
    lcd.print("H:");
    lcd.setCursor(11,0);
    lcd.print(h);

    lcd.setCursor(0,1);
    lcd.print("p2.5:");
    lcd.setCursor(5,1);
    lcd.print(data.pm25_standard); // PM2.5 standard

    lcd.setCursor(9,1);
    lcd.print("p10:");
    lcd.setCursor(13,1);
    lcd.print(data.pm100_standard); // PM10 standard
  }
  client.stop();
  delay(postingInterval); // wait and post again
}
