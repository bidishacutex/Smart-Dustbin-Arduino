#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

#define SSID "iPhone"
#define PASSWORD "@1234567890@"
#define API_ENDPOINT "https://smartdustbin.awish.site/api/add_reading_get"

#define TRIG_PIN1 5 //D1
#define ECHO_PIN1 4 //D2
#define TRIG_PIN2 12 //D6
#define ECHO_PIN2 13 //D7

//Get dustbin uid from Panel
#define DEVICE_UID_1 "cDaffuhVX8iH"
#define DEVICE_UID_2 "v7UOBgBVUfTy"

float distance1, distance2;
long duration1, duration2;

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN1, OUTPUT);
  pinMode(ECHO_PIN1, INPUT);
  pinMode(TRIG_PIN2, OUTPUT);
  pinMode(ECHO_PIN2, INPUT);

  connectToWiFi();
}

void loop() {
  //Sensor 1 Program
  //Get CM Reading of Sensor 1
  getDistance(TRIG_PIN1, ECHO_PIN1, distance1, duration1);
  float cm_reading1 = distance1;
  long raw_reading1 = duration1;
  //send data to webpanel for Sensor 1
  callAPI(raw_reading1, cm_reading1, DEVICE_UID_1);

  //Sensor 2 Program
  //Get CM Reading of Sensor 2
  getDistance(TRIG_PIN2, ECHO_PIN2, distance2, duration2);
  float cm_reading2 = distance2;
  long raw_reading2 = duration2;
  //Send data to webpanel for Sensor 2
  callAPI(raw_reading2, cm_reading2, DEVICE_UID_2);

  delay(60000);
}

void getDistance(int trigPin, int echoPin, float& distance, long& duration) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  //Ultrasonic Sensor CM Distance formula = duration_in_microseconds * 0.034 / 2
  distance = duration * 0.034 / 2; 
  
}

void connectToWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void callAPI(long raw_reading, float cm_reading, String device_uid) {
  // make the HTTP request
  String url = String(API_ENDPOINT) + "/" + device_uid + "/" + String(raw_reading) + "/" + String(cm_reading);
  Serial.println("URL: " + url);

  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

    // Ignore SSL certificate validation
    client->setInsecure();
    
    //create an HTTPClient instance
    HTTPClient https;
    https.setTimeout(30000);
    //Initializing an HTTPS communication using the secure client
    Serial.print("[HTTPS] begin...\n");
    if (https.begin(*client, url)) {  // HTTPS
      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      int httpCode = https.GET();
      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = https.getString();
          Serial.println(payload);
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }
