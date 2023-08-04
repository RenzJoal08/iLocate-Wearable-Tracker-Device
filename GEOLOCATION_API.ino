#include <Arduino.h>
#include <ArduinoJson.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "WiFiClientSecure.h"

#define WLAN_SSID "Reiner"
#define WLAN_PASS "Sabado100700"

const char* host = "www.googleapis.com";
const char* apiEndpoint = "/geolocation/v1/geolocate?key=";
const char* apiKey = "AIzaSyATMwqK8Iyk3zcD3aucZpB2C10PjD8St2Y";


const char* iftttWebhookURL_Google_Map = "https://maker.ifttt.com/trigger/google_link/with/key/rqu-o7J-60NewMKBybS5S";

const char* googleMap = "https://www.google.com/maps/dir/";

float latitude;
float longitude;

float currentLatitudeStart ;
float currentLongitudeStart;

float currentLatitude;
float currentLongitude;


const int trailCount = 5;



float latitudeTrail[trailCount];
float longitudeTrail[trailCount];

float startingLatitude = 14.58730;
float startingLongitude = 120.983940; 

 int count = 0; 


/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER "io.adafruit.com"

// Using port 8883 for MQTTS
#define AIO_SERVERPORT  8883

// Adafruit IO Account Configuration
// (to obtain these values, visit https://io.adafruit.com and click on Active Key)
#define AIO_USERNAME  "renzjoal"
#define AIO_KEY       "aio_CjJn55CBadH09mqnwwF5PclhtT5C"



/************ Global State (you don't need to change this!) ******************/

// WiFiFlientSecure for SSL/TLS support
WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// io.adafruit.com root CA
const char* adafruitio_root_ca = \
    "-----BEGIN CERTIFICATE-----\n" \
    "MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n" \
    "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
    "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n" \
    "QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n" \
    "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n" \
    "b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n" \
    "9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\n" \
    "CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\n" \
    "nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\n" \
    "43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\n" \
    "T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\n" \
    "gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\n" \
    "BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\n" \
    "TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\n" \
    "DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\n" \
    "hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\n" \
    "06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\n" \
    "PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\n" \
    "YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\n" \
    "CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\n" \
    "-----END CERTIFICATE-----\n";



/****************************** Feeds ***************************************/

// Setup a feed called 'test' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish geo_latitude = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/geo_latitude");
Adafruit_MQTT_Publish geo_longitude = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/geo_longitude");
Adafruit_MQTT_Publish geolocation = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/geolocation");



void setup() {

  Serial.begin(115200);

  delay(1000);
  
  Serial.println();
  Serial.println("Setting up device...");

  delay(1000);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.print(WLAN_SSID);
  Serial.println("");
 

  WiFi.mode(WIFI_STA);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  delay(1000);


  while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("..");
      delay(1000);

  }

      Serial.println();
      Serial.println("WiFi connected successfully!");


      Serial.print("IP Address: "); 
      Serial.print(WiFi.localIP()); 
      Serial.println();
      delay(2000);


  // Set Adafruit IO's root CA
  client.setCACert(adafruitio_root_ca);

  
  MQTT_connect();

  
  if (WiFi.status() == WL_CONNECTED) {

    WiFiClientSecure client;
    client.setInsecure(); // Bypass SSL certificate verification
    HTTPClient http;


    currentLatitudeStart =  startingLatitude ;
    currentLongitudeStart = startingLongitude;

      
    Serial.println(" ");
    Serial.println("ELDER'S STARTING LOCATION:");
    Serial.print("LATITUDE: ");
    Serial.println(currentLatitudeStart, 6);
    Serial.print("LONGITUDE: ");
    Serial.println(currentLongitudeStart, 6);


    //Declare string variable for coordinates
    char latitudeStr[10];
    char longitudeStr[20];


    // Format the values with six decimal places
    dtostrf(currentLatitudeStart, 8, 6, latitudeStr);
    dtostrf(currentLongitudeStart, 8, 6, longitudeStr);


    // Construct the JSON payload for map feed
    StaticJsonDocument<256> mapPayload;
    mapPayload["lat"] = latitudeStr;
    mapPayload["lon"] = longitudeStr;
              

    // Serialize the JSON payload
    String mapPayloadString;
    serializeJson(mapPayload, mapPayloadString);


      // Publish the payload to the map feed
      if (!geolocation.publish(mapPayloadString.c_str())) {
            Serial.println("FAILED TO PUBLISH ELDER'S LOCATION TO IOT!");
            Serial.println("");   
      }


      if (!geo_latitude.publish(latitudeStr)) {
            Serial.println("ELDER'S LATITUDE FAILED TO BE SENT TO IOT!");
            Serial.println("");                       
      }


      if (!geo_longitude.publish(longitudeStr))  {
            Serial.println("ELDER'S LONGITUDE FAILED TO BE SENT TO IOT!");
            Serial.println("");                        
      }

      else {
            Serial.println("ELDER'S STARTING POINT COORDINATES SUCCESSFULLY SENT TO IOT MAP SUCCESSFULLY.");
      }  
   
        String googleMap = "https://www.google.com/maps/dir/?api=1&destination=";
        googleMap += String(currentLatitudeStart, 6);
        googleMap += ",";
        googleMap += String(currentLongitudeStart, 6);
      
        Serial.println(googleMap);
        Serial.println(""); 

    }

}



void loop() {

  //Automatically reconnected to the MQTT server when disconnected
  MQTT_connect();

  if (WiFi.status() == WL_CONNECTED) {

    WiFiClientSecure client;
    client.setInsecure(); // Bypass SSL certificate verification
    HTTPClient http;


    // Construct the JSON payload
    DynamicJsonDocument jsonDoc(1024);
    JsonObject payload = jsonDoc.to<JsonObject>();
    payload["homeMobileCountryCode"] = 515;
    payload["homeMobileNetworkCode"] = 03;
    payload["radioType"] = "gsm";
    payload["carrier"] = "SMART";
    JsonArray wifiAccessPoints = payload.createNestedArray("wifiAccessPoints");


   // Scan for Wi-Fi networks and add them to the payload
    int networkCount = WiFi.scanNetworks();
      for (int i = 0; i < networkCount; i++) {
        JsonObject wifi = wifiAccessPoints.createNestedObject();
        wifi["macAddress"] = WiFi.BSSIDstr(i);
        wifi["signalStrength"] = WiFi.RSSI(i);
      }


    // Serialize the JSON payload
    String payloadString;
    serializeJson(payload, payloadString);


    String url = "https://" + String(host) + String(apiEndpoint) + String(apiKey);
      if (http.begin(client, url)) {
         http.addHeader("Content-Type", "application/json");
         int httpCode = http.POST(payloadString);

        if (httpCode == HTTP_CODE_OK) {
            DynamicJsonDocument jsonDoc(1024);
            DeserializationError error = deserializeJson(jsonDoc, http.getString());

          if (error) {
            Serial.println("Error parsing JSON response.");
          } 

          else {

            latitude = jsonDoc["location"]["lat"];
            longitude = jsonDoc["location"]["lng"];

                Serial.println(" ");
                Serial.println("ELDER'S CURRENT LOCATION:");
                Serial.print("LATITUDE: ");
                Serial.println(latitude, 6);
                Serial.print("LONGITUDE: ");
                Serial.println(longitude, 6);


                if (abs(latitude - 14.660403) < 0.000001 && abs(longitude - 120.999115) < 0.000001) {
                       Serial.println("ELDER'S LOCATION COORDINATES ARE INVALID.");
                       Serial.println("");
                }

                else {

                  count++;


                  // Update the trail with the current location
                  for (int i = 0; i < trailCount - 1; i++) {
                      latitudeTrail[i] = latitudeTrail[i + 1];
                      longitudeTrail[i] = longitudeTrail[i + 1];
                   }
                        latitudeTrail[trailCount - 1] = currentLatitude;
                        longitudeTrail[trailCount - 1] = currentLongitude;

                        // Update the current latitude and longitude with the new values
                        currentLatitude = latitude;
                        currentLongitude = longitude;

                        // Display the current and last trail locations on the map
                        displayLocationOnMap(currentLatitude, currentLongitude);
                        displayLocationOnMap(latitudeTrail[trailCount - 1], longitudeTrail[trailCount - 1]);


                        //declare string variable for coordinates
                        char latitudeStr[10];
                        char longitudeStr[10];


                        // Format the values with six decimal places
                        dtostrf(latitude, 8, 6, latitudeStr);
                        dtostrf(longitude, 8, 6, longitudeStr);
                        
                        
                        // Construct the JSON payload for map feed
                        StaticJsonDocument<256> mapPayload;
                        mapPayload["lat"] = latitudeStr;
                        mapPayload["lon"] = longitudeStr;

                      
                        // Serialize the JSON payload
                        String mapPayloadString;
                        serializeJson(mapPayload, mapPayloadString);

                        // Publish the payload to the map feed
                        if (!geolocation.publish(mapPayloadString.c_str())) {
                            Serial.println("FAILED TO PUBLISH ELDER'S LOCATION TO IOT!");
                            Serial.println("");   
                        }


                        if (!geo_latitude.publish(latitudeStr)) {
                            Serial.println("ELDER'S LATITUDE FAILED TO BE SENT TO IOT!");
                            Serial.println("");                       
                        }


                        if (!geo_longitude.publish(longitudeStr))  {
                            Serial.println("ELDER'S LONGITUDE FAILED TO BE SENT TO IOT!");
                            Serial.println("");                        
                        }


                        else {
                            Serial.println("ELDER'S LATITUDE IS SENT TO IOT SUCCESSFULLY.");
                            Serial.println("ELDER'S LONGITUDE IS SENT TO IOT SUCCESSFULLY.");
                            Serial.println("ELDER'S LOCATION IS SENT TO IOT MAP SUCCESSFULLY.");
                        }
                }

             } 
        }

        
    else {
          Serial.print("FAILED TO GET ELDER'S LOCATION. HTTP REQUEST FAILED WITH ERROR CODE: ");
          Serial.println(httpCode);
    }
    http.end();

      }

    }

    delay(3000);
}



// Function for sending IFTTT FOR GOOGLE MAP LINK
void IFTTT_google_map_link(const String& googleMapURL) {

  WiFiClientSecure client;
  client.setInsecure(); // Bypass SSL certificate verification
  HTTPClient http;

  if (http.begin(client, iftttWebhookURL_Google_Map)) {
    http.addHeader("Content-Type", "application/json");

      // Construct the JSON payload for IFTTT
      String Payload = "{\"value1\":\"" + googleMapURL + "\"}";
    
      int httpResponseCode = http.POST(Payload);
    
      if (httpResponseCode > 0) {
          Serial.print("IFTTT Notification Sent Successfully. Response code: ");
          Serial.print(httpResponseCode);
          Serial.println("");
      } 
      
      else {
          Serial.print("Error sending IFTTT Notification. Error code: ");
          Serial.print(httpResponseCode);
          Serial.println("");
      }

    http.end();
  } 
  
  else {
    Serial.println("Unable to connect to IFTTT.");
    Serial.println("");
  }

  delay(1000); // Wait for 1 second before sending the next request
}



void displayLocationOnMap(float latitude, float longitude) {
  // Construct the Google Maps URL with the current and last trail locations

  String url = String(googleMap);

  // Add current location to the URL
  url += String(latitude, 6) + "," + String(longitude, 6) + "/";

  // Add the last trail coordinates to the URL
  url += String(latitudeTrail[trailCount - 1], 6) + "," + String(longitudeTrail[trailCount - 1], 6) + "/";

  // Add the remaining trail coordinates to the URL
  for (int i = 0; i < trailCount - 1; i++) {
    url += String(latitudeTrail[i], 6) + "," + String(longitudeTrail[i], 6);
    if (i < trailCount - 1) {
      url += "/";
    }
  }
  
  // Add the new latitude and longitude to the URL
  url += String(currentLatitude, 6) + "," + String(currentLongitude, 6) + "/";

  
    if (count==6) {
      Serial.println("Google Maps URL: " + url);
      IFTTT_google_map_link(url);
      count=0;
    }

}



// Function to connect and reconnect as necessary to the MQTT server.
void MQTT_connect() {

  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  uint8_t retries = 3;

  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
      Serial.println ("");
      Serial.println(mqtt.connectErrorString(ret));
      Serial.println("Retrying to connect in 3 seconds...");

      mqtt.disconnect();

      delay(3000);  // wait 3 seconds

      retries--;

       if (retries == 0) {

            MQTT_connect();

         while (1);
       }

  }

      Serial.println ("");
      Serial.println("Connected to IOT successfully!");
      Serial.println ("");
}
