# iLocate: An IoT Wearable Tracker Device with Health Analytics for Elders. My capstone (thesis) project.

Programming Language: C++ (Arduino)

How the program works:

1. The project is composed of 2 modules:
   * MAX30102 Sensor module components:
      - 1 Node MCU ESP 32 Module
      - 1 MAX30102 sensor
      - 1 2600 mAh Powerbank (External Power Supply)
      - 1 SSD1306 OLED Display
      - 1 Tactile button
        
   * Geolocation/GPS module components:
      - 1 Node MCU ESP 32 Module
      - 1 2600 mAh Powerbank (External Power Supply)
        
2. Then, you need to download Arduino IDE in this link https://www.arduino.cc/en/software
   
3. Next, you need to have an account in Adafruit through this link https://io.adafruit.com/

4. Also, you need to have a Google Geolocation API account through this link https://developers.google.com/maps/documentation/geolocation/overview
   
5. The following are the wiring of the 2 modules:

 -----------------------------
    MAX30102 Sensor module components:
    
    MAX30102 Sensor                  
    ESP 32 Module
    
      SDA                        
      Pin 21
      
      SCL                       
      Pin 22
      
      GND                        
      GND
      
      VCC                         
      3V
-----------------------------
    SSD1306 OLED Display                  
    ESP 32 Module

      SDA                        
      Pin 21
      
      SCL                       
      Pin 22
      
      GND                        
      GND
      
      VCC                         
      3V
-----------------------------
    Tactile Button                  
    ESP 32 Module

      Pin 17                                           
      GND
-----------------------------                 
    Powerbank (External Power Supply)
    ESP 32 Module

      USB Port
      Micro USB Port


**************************************************    
    Geolocation/GPS module components:

    Powerbank (External Power Supply)
    ESP 32 Module

      USB Port
      Micro USB Port
         
6. Execute and run the program in Arduino IDE.
   
7. Then put your index finger in the MAX30102, and it will detect your heart rate (BPM), Oxygen Saturation (Sp02), and Body Temperature. Then, data will be sent to Adafruit IoT dashboard.
   
8. Download the IFTTT application in Play Store (for Android) or App Store (for iOS).
   
9. Open IFTTT application and activate all the applets for push notifications of irregularities
of vital signs of elders, and push notification of elder's real time location through Google
Map link.
