#include <Arduino.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include <Wire.h>
#include <WiFi.h>
#include "WiFiClientSecure.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HTTPClient.h>


#define MAX_BRIGHTNESS 255

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define RED 0xF800


//IFTTT Webhook URL
const char* iftttWebhookURL_button = "https://maker.ifttt.com/trigger/button_alert/with/key/YOUR IFTTT KEY";
const char* iftttWebhookURL_heartRate_High = "https://maker.ifttt.com/trigger/heart_rate_high/with/key/YOUR IFTTT KEY";
const char* iftttWebhookURL_heartRate_Low = "https://maker.ifttt.com/trigger/heart_rate_low/with/key/YOUR IFTTT KEY";
const char* iftttWebhookURL_spo2_Not_Normal = "https://maker.ifttt.com/trigger/spo2_not_normal/with/key/YOUR IFTTT KEY";
const char* iftttWebhookURL_body_temp_High = "https://maker.ifttt.com/trigger/body_temp_high/with/key/YOUR IFTTT KEY";
const char* iftttWebhookURL_body_temp_Low = "https://maker.ifttt.com/trigger/body_temp_low/with/key/YOUR IFTTT KEY";



// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


MAX30105 particleSensor;


#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)

uint16_t irBuffer[100];   //Infrared LED sensor data
uint16_t redBuffer[100];  //Red LED sensor data
#else
uint32_t irBuffer[100]; 
uint32_t redBuffer[100]; 
#endif


int32_t bufferLength; 
int32_t spo2;
int8_t validSPO2; 
int32_t heartRate; 
int8_t validHeartRate; 

float body_temp,final_body_temp;

int32_t heartRateLcd=0; 
int32_t spo2Lcd=0;
float final_body_tempLcd=0;


const int buttonPin = 12;
int previousButtonState = HIGH;
int currentButtonState;

int count=0;
int count1=0;

/************************* WiFi Access Point *********************************/

#define WLAN_SSID "YOUR WIFI SSID"
#define WLAN_PASS "YOUR WIFI PASSWORD"


/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER "io.adafruit.com"

// Using port 8883 for MQTTS
#define AIO_SERVERPORT  8883

// Adafruit IO Account Configuration
// (to obtain these values, visit https://io.adafruit.com and click on Active Key)
#define AIO_USERNAME  "YOUR ADAFRUIT USERNAME"
#define AIO_KEY       "YOUR ADAFRUIT KEY"



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
Adafruit_MQTT_Publish heart_rate = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/heart_rate");
Adafruit_MQTT_Publish body_temperature = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/body_temperature");
Adafruit_MQTT_Publish spO2 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/spO2");



void setup()
{
  Serial.begin(115200); 
   
  //WELCOME LCD
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

    delay(2000);
    display.clearDisplay();

    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(20, 20);
    display.println("Welcome!");
    
    display.display(); 

    delay(2000);
  

    // Connect to WiFi access point.
    Serial.println();
    Serial.println();
    Serial.println("Setting up device...");

    //Setting Up Device LCD
    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(5, 25);
    display.println(" Setting up device");
  
    display.display(); 

    delay(2000);


    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(WLAN_SSID);

    //Connecting to Wifi LCD
    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(5, 25);
    display.println(" Connecting to WiFi");

    display.display(); 

    delay(2000);

    WiFi.mode(WIFI_STA);
    WiFi.begin(WLAN_SSID, WLAN_PASS);
    delay(2000);


    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("..");
      delay(2000);

    }

      Serial.println();
      Serial.println("WiFi connected successfully!");
      
      //WiFi Connected Successfully LCD
      display.clearDisplay();

      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(25, 20);
      display.println("WiFi connected");
      display.println("     successfully!");

      display.display(); 


      Serial.print("IP Address: "); 
      Serial.print(WiFi.localIP()); 
      Serial.println();
      delay(3000);


  pinMode(buttonPin, INPUT_PULLUP);  


  // Set Adafruit IO's root CA
  client.setCACert(adafruitio_root_ca);

 
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST))
  {
    Serial.println();
    Serial.println(F("MAX30102 sensor was not found. Please check wiring/power."));
    while (1);
  }

  Serial.println();
  Serial.println("Place your index finger on the sensor with steady pressure within 3 seconds.");

  Serial.println("");
  delay (5000);

  byte ledBrightness = 60;  //Options: 0=Off to 255=50mA
  byte sampleAverage = 4;   //Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 2;         //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  byte sampleRate = 100;    //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 411;     //Options: 69, 118, 215, 411
  int adcRange = 4096;      //Options: 2048, 4096, 8192, 16384

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);

}


void loop()
{
  bufferLength = 100; //buffer length of 100 stores 4 seconds of samples running at 25sps
  body_temp=particleSensor.readTemperature();
  final_body_temp = body_temp;


  //Automatically reconnected to the MQTT server when disconnected
  MQTT_connect();

  if (count==0){
      //Place your index finger
        display.clearDisplay();

        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0, 15);
        // Display static text
        display.println("Place your index");
        display.println("finger on the sensor");
        display.println("with steady pressure");
        display.println("within 3 seconds.");
        display.display(); 
        delay (4000);

        count++;
  }

  //Function for button alert for elder
  button_Alert();


  //read the first 100 samples, and determine the signal range
  for (byte i = 0 ; i < bufferLength ; i++)
    {
      while (particleSensor.available() == false)
        particleSensor.check(); 

          redBuffer[i] = particleSensor.getRed();
          irBuffer[i] = particleSensor.getIR();
          particleSensor.nextSample(); 

          Serial.print(F("Red Light = "));
          Serial.print(redBuffer[i], DEC);

          Serial.print(F(", IR Light = "));
          Serial.print(irBuffer[i], DEC);

          Serial.print(F(", Heart Rate = "));
          Serial.print(heartRate, DEC);
          Serial.print(" BPM");

          Serial.print(F(", HR Valid = "));
          Serial.print(validHeartRate, DEC);

          Serial.print(F(", SpO2 = "));
          Serial.print(spo2, DEC);
          Serial.print(" %");

          Serial.print(F(", SpO2 Valid = "));
          Serial.print(validSPO2, DEC);

          Serial.print(F(", Body Temperature = "));
          Serial.print(final_body_temp,2);
          Serial.print(" °C");

          Serial.println("");

          button_Alert();

    }

  //calculate Heart Rate and Spo2 after first 100 samples (first 4 seconds of samples)
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
   

  //dumping the first 25 sets of samples in the memory and shift the last 75 sets of samples to the top
  for (byte i = 25; i < 100; i++) 
  {
      redBuffer[i - 25] = redBuffer[i];
      irBuffer[i - 25] = irBuffer[i];
  }
    

  //take 1 set of samples before calculating the Heart Rate and Spo2
  for (byte i = 99; i < 100; i++)
    {
      while (particleSensor.available() == false) 
        particleSensor.check(); 

          redBuffer[i] = particleSensor.getRed();
          irBuffer[i] = particleSensor.getIR();
          particleSensor.nextSample(); 


              if (irBuffer[i] > 50000) {
                
                //READING VITAL SIGNS

                display.clearDisplay();
                display.setTextSize(1);
                display.setTextColor(WHITE);
                display.setCursor(5, 25);
                display.println("Reading Vital Signs");
                display.display();
                delay (2000);

                  Serial.println("");
                  Serial.print(F("Heart Rate = "));
                  Serial.print(heartRate, DEC);
                  Serial.print(" BPM");

                  Serial.print(F(", SpO2 = "));
                  Serial.print(spo2, DEC);
                  Serial.print("%");

                  Serial.print(F(", Body Temperature = "));
                  Serial.print(final_body_temp,2);
                  Serial.print(" °C");

                  Serial.print(F(", HR Valid = "));
                  Serial.print(validHeartRate, DEC);

                  Serial.print(F(", SpO2 Valid = "));
                  Serial.print(validSPO2, DEC);

                  Serial.println("");

                  button_Alert();

 
/* ------------------------------------- CONDITIONALS STATEMENTS OF VITAL SIGNS OF THE PROGRAM ------------------------------------------------ */     

                  // conditional statements for elder's Heart Rate
                  if (heartRate >= 131 && heartRate <= 150) {
                        Serial.println("ELDER'S HEART RATE IS HIGH.");
                        sendIFTTTRequest_heartRate_high (heartRate);
                        button_Alert();
                    
                        if (heart_rate.publish (heartRate)) {
                            Serial.println("ELDER'S HEART RATE IS SENT TO IOT SUCCESSFULLY.");
                            Serial.println("");
                            button_Alert();
                            heartRateLcd = heartRate;
                        }

                        else {
                            Serial.println("ELDER'S HEART RATE FAILED TO SENT TO IOT!");
                            Serial.println("");
                            delay(1000);
                        }
                  }


                else if (heartRate >= 60 && heartRate <= 130) {
                        Serial.println("ELDER'S HEART RATE IS NORMAL.");
                        button_Alert();

                        if (heart_rate.publish (heartRate)) {
                            Serial.println("ELDER'S HEART RATE IS SENT TO IOT SUCCESSFULLY.");
                            Serial.println("");
                            button_Alert();
                            heartRateLcd = heartRate;
                        }

                        else {
                            Serial.println("ELDER'S HEART RATE FAILED TO SENT TO IOT!");
                            Serial.println("");
                            delay(1000);
                        }
                  }

              
                else if (heartRate >= 40 && heartRate <= 59) {
                        Serial.println("ELDER'S HEART RATE IS LOW.");
                        sendIFTTTRequest_heartRate_low (heartRate);
                        button_Alert();
                        
                        if (heart_rate.publish (heartRate)) { 
                            Serial.println("ELDER'S HEART RATE IS SENT TO IOT SUCCESSFULLY.");
                            Serial.println("");
                            button_Alert();
                            heartRateLcd = heartRate;
                        }

                        else {
                            Serial.println("ELDER'S HEART RATE FAILED TO SENT TO IOT!");
                            Serial.println("");
                            delay(1000);
                        }
                  }


                else {
                        Serial.println("ELDER'S HEART RATE IS INVALID.");
                        Serial.println("");
                        button_Alert();     
                  }



                  // conditional statements for elder's SpO2
                  if (spo2 >= 95 && spo2 <= 100) {
                        Serial.println("ELDER'S SPO2 IS NORMAL.");
                        button_Alert();

                      if (spO2.publish (spo2)) {
                            Serial.println("ELDER'S SPO2 IS SENT TO IOT SUCCESSFULLY.");
                            Serial.println("");
                            button_Alert();
                            spo2Lcd=spo2;
                      }

                      else {
                          Serial.println("ELDER'S SPO2 FAILED TO SENT TO IOT!");
                          delay(1000);
                      }
                  }


                  else if (spo2 >= 90 && spo2 <= 94) {
                        Serial.println("ELDER'S SPO2 IS NOT NORMAL.");
                        sendIFTTTRequest_spo2_not_normal (spo2);
                        button_Alert();

                      if (spO2.publish (spo2)) {
                            Serial.println("ELDER'S SPO2 IS SENT TO IOT SUCCESSFULLY.");
                            Serial.println("");
                            button_Alert();
                            spo2Lcd=spo2;
                      }

                      else {
                          Serial.println("ELDER'S SPO2 FAILED TO SENT TO IOT!");
                          delay(1000);
                      }
                    }


                  else {
                        Serial.println("ELDER'S SPO2 IS INVALID.");
                        Serial.println("");
                        button_Alert();
                  }



                  // conditional statements for elder's Body Temperature
                  if (final_body_temp > 37.5) {
                        Serial.println("ELDER'S BODY TEMPERATURE IS HIGH.");
                        sendIFTTTRequest_body_temp_High (final_body_temp);
                        button_Alert();
                
                  }

                  else if (final_body_temp >= 32 && final_body_temp <= 37.5) {
                        Serial.println("ELDER'S BODY TEMPERATURE IS NORMAL.");
                        button_Alert();       
                  }

                  else {
                        Serial.println("ELDER'S BODY TEMPERATURE IS LOW.");
                        sendIFTTTRequest_body_temp_low (final_body_temp);
                        button_Alert();
                      
                  }



                  //Body Temperature IOT publish data
                  if (body_temperature.publish (final_body_temp)) {
                        Serial.println("ELDER'S BODY TEMPERATURE IS SENT TO IOT SUCCESSFULLY.");
                        Serial.println("");
                        button_Alert();
                        final_body_tempLcd=final_body_temp;
                  }


                  else {
                        Serial.println("ELDER'S BODY TEMPERATURE FAILED TO SENT TO IOT!");
                        Serial.println("");
                        delay(1000);
                  }


                  //LCD Display   
                  display.clearDisplay();

                  display.setTextSize(1);
                  display.setTextColor(WHITE);
                  display.setCursor(0, 20);
                          

                  // Display static text
                  display.print("Heart Rate: " );
                  display.print(heartRateLcd,DEC);
                  display.print(" BPM");

                  display.println("" );

                  display.print("SpO2: " );
                  display.print(spo2Lcd,DEC);
                  display.print(" %");

                  display.println("" );

                  display.print("Body Temp: " );
                  display.print(final_body_tempLcd,2);
                  display.print(" " );
                  display.cp437(true);
                  display.write(167);
                  display.print("C" );
                  display.display();
                  delay(2000);
          
              }


            else {

              heartRate=0;
              spo2=0;
              final_body_temp = 0;

                  Serial.println("");
                  Serial.print(F("Heart Rate = "));
                  Serial.print(heartRate, DEC);
                  Serial.print(" BPM");

                  Serial.print(F(", SpO2 = "));
                  Serial.print(spo2, DEC);
                  Serial.print("%");

                  Serial.print(F(", Body Temperature = "));
                  Serial.print(final_body_temp,2);
                  Serial.print(" °C");

                  Serial.println();
                  Serial.println("INDEX FINGER OUT! PLACE YOUR FINGER ON THE SENSOR.");

                  button_Alert();

                if(count==1){
                  count++;
                }
              
                else{
                  //DISPLAY DATA IN LCD
                  display.clearDisplay();

                  display.setTextSize(1);
                  display.setTextColor(WHITE);
                  display.setCursor(0, 20);

                  display.println("INDEX FINGER OUT!" );
                  display.println("PLACE YOUR FINGER ON THE SENSOR." );

                  display.display();
                  delay(1000);
                }
                  
                 heartRateLcd=0; 
                 spo2Lcd=0;
                 final_body_tempLcd=0;

        
                  if (!heart_rate.publish (heartRate)) {
                          Serial.println("ELDER'S HEART RATE FAILED TO SENT TO IOT!");
                          Serial.println("");
                          
                  }

                  if (!spO2.publish (spo2)) {
                          Serial.println("ELDER'S SPO2 FAILED TO SENT TO IOT!");
                          Serial.println("");
                        
                  }

                  if (!body_temperature.publish (final_body_temp)) {
                          Serial.println("ELDER'S BODY TEMPERATURE FAILED TO SENT TO IOT!");
                          Serial.println("");
                          
                  }

                  else {
             
                          Serial.println("ELDER'S HEART RATE IS SENT TO IOT SUCCESSFULLY.");
                          Serial.println("ELDER'S SPO2 IS SENT TO IOT SUCCESSFULLY.");
                          Serial.println("ELDER'S BODY TEMPERATURE IS SENT TO IOT SUCCESSFULLY.");
                
                          Serial.println("");   

                          button_Alert();           
                  }

            }
    }

    //After gathering 1 new sample recalculate Heart Rate and SpO2
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);  
}



/* ------------------------------------- FUNCTIONS PART OF THE PROGRAM ------------------------------------------------ */


//Function for BUTTON PRESS alert for elder
void button_Alert() {

  currentButtonState = digitalRead(buttonPin);

  if (currentButtonState == LOW && previousButtonState == HIGH) { 

        Serial.println(" ");
        Serial.println("ELDER PRESSED THE ASSISTIVE BUTTON.");
        Serial.println("ELDER NEEDS ASSISTANCE!");
        sendIFTTTRequest_button();
        Serial.println(" ");

  }

  previousButtonState = currentButtonState;

}



//Function for BUTTON PRESS IFTTT alert for elder
void sendIFTTTRequest_button() {

  WiFiClientSecure client;
  client.setInsecure(); // Bypass SSL certificate verification
  HTTPClient http;
  

  if (http.begin(client, iftttWebhookURL_button)) {
    
    int httpResponseCode = http.POST("{}");
    
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
      Serial.println("");
      Serial.println("Unable to connect to IFTTT.");
      Serial.println("");
  }
  
  delay(1000); // Wait for 1 second before sending the next request
}



//Function for IFTTT if elder's HEART RATE IS HIGH
void sendIFTTTRequest_heartRate_high(int notif_heart_Rate) {

  WiFiClientSecure client;
  client.setInsecure(); // Bypass SSL certificate verification
  HTTPClient http;
 

  if (http.begin(client, iftttWebhookURL_heartRate_High)) {
    
   http.addHeader("Content-Type", "application/json");

    // Prepare the payload data
    String payload = "{\"value1\":\"" + String(notif_heart_Rate) + "\"}";

    int httpResponseCode = http.POST(payload);
    
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



//Function for IFTTT if elder's HEART RATE IS LOW
void sendIFTTTRequest_heartRate_low(int notif_heart_Rate) {

  WiFiClientSecure client;
  client.setInsecure(); // Bypass SSL certificate verification
  HTTPClient http;
 

  if (http.begin(client, iftttWebhookURL_heartRate_Low)) {
    
   http.addHeader("Content-Type", "application/json");

    // Prepare the payload data
    String payload = "{\"value1\":\"" + String(notif_heart_Rate) + "\"}";

    int httpResponseCode = http.POST(payload);
    
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



//Function for IFTTT if elder's SPO2 IS NOT NORMAL
void sendIFTTTRequest_spo2_not_normal(int notif_spo2) {

  WiFiClientSecure client;
  client.setInsecure(); // Bypass SSL certificate verification
  HTTPClient http;


  if (http.begin(client, iftttWebhookURL_spo2_Not_Normal)) {
    
   http.addHeader("Content-Type", "application/json");

    // Prepare the payload data
    String payload = "{\"value1\":\"" + String(notif_spo2) + "\"}";
   
    int httpResponseCode = http.POST(payload);
    
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




//Function for IFTTT if elder's BODY TEMP HIGH
void sendIFTTTRequest_body_temp_High(float notif_body_Temp) {

  WiFiClientSecure client;
  client.setInsecure(); // Bypass SSL certificate verification
  HTTPClient http;
 

  if (http.begin(client, iftttWebhookURL_body_temp_High)) {
    
   http.addHeader("Content-Type", "application/json");

    // Prepare the payload data
    String payload = "{\"value1\":\"" + String(notif_body_Temp) + "\"}";

    int httpResponseCode = http.POST(payload);
    
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




//Function for IFTTT if elder's BODY TEMP LOW
void sendIFTTTRequest_body_temp_low(float notif_body_Temp) {

  WiFiClientSecure client;
  client.setInsecure(); // Bypass SSL certificate verification
  HTTPClient http;
 

  if (http.begin(client, iftttWebhookURL_body_temp_Low)) {
    
   http.addHeader("Content-Type", "application/json");

    // Prepare the payload data
    String payload = "{\"value1\":\"" + String(notif_body_Temp) + "\"}";

    int httpResponseCode = http.POST(payload);
    
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




// Function to connect and reconnect as necessary to the MQTT server.
void MQTT_connect() {

   if(count1==0){

    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(20, 20);

    display.println(" Connecting to " );
    display.print("     Adafruit IoT" );
    // display.println("PLACE YOUR FINGER ON THE SENSOR." );
    display.display();
    delay(2000);
    count1++;
  }



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

       
      //LCD Display
      display.clearDisplay();

      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0, 20);

      display.println("    Reconnecting in");
      display.println("     3 seconds.");

      display.display();
      delay(2000);

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
      
      
      //LCD Display
      display.clearDisplay();

      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(20, 20);

      display.println("Connected to IOT " );
      display.println("     successfully!" );
     
      display.display();

      delay(2000);

      Serial.println ("");
}
