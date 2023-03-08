from max30102 import MAX30102, MAX30105_PULSE_AMP_MEDIUM
from machine import sleep, SoftI2C, Pin, Timer 
from utime import ticks_diff, ticks_us
from umqtt.robust import MQTTClient
import network
import time
import sys
import urequests

led = Pin(2, Pin.OUT)

i2c = SoftI2C(sda=Pin(22),scl=Pin(21),freq=400000)
sensor = MAX30102(i2c=i2c)


MAX_HISTORY = 32
history = []
bpm_history = []
heartbeats = False
heartrate_value = 0



WIFI_SSID     = #YOUR WIFI NAME
WIFI_PASSWORD = #YOUR WIFI PASSWORD

mqtt_client_id      = bytes('client_'+'12321', 'utf-8') 


ADAFRUIT_IO_URL     = 'io.adafruit.com' 
ADAFRUIT_USERNAME   = #YOUR ADAFRUIT USERNAME
ADAFRUIT_IO_KEY     = #YOUR ADAFRUIT KEY


HEART_RATE_FEED_ID   = 'heartrate'
SP02_RATE_FEED_ID    = 'sp02'
BODY_TEMP_FEED_ID    = 'body-temperature'



if sensor.i2c_address not in i2c.scan():
    print("Sensor not found.")
    
elif not (sensor.check_part_id()):
    print("I2C device ID not corresponding to MAX30102 or MAX30105.")
    
else:
    print("Sensor connected and recognized.")
    
print("Setting up sensor with default configuration.", '\n')
sensor.setup_sensor()


sensor.set_sample_rate(400)
sensor.set_fifo_average(8)
sensor.set_active_leds_amplitude(MAX30105_PULSE_AMP_MEDIUM)
sensor.set_led_mode(2)
sleep(1)

t_start = ticks_us() 


def connect_wifi():
    wifi = network.WLAN(network.STA_IF)
    wifi.active(True)
    wifi.disconnect()
    wifi.connect(WIFI_SSID,WIFI_PASSWORD)
    
    if not wifi.isconnected():
        print('Connecting to WIFI...')
        timeout = 0
        
        while (not wifi.isconnected() and timeout < 10):
            print(10 - timeout)
            timeout = timeout + 1
            time.sleep(1)
            
    if(wifi.isconnected()):
        print('Successfully Connected.')
        print('\n')
    else:
        print('Failed to Connect.')
        print('\n')
        sys.exit()
        

connect_wifi() # Connecting to WiFi Router 


client = MQTTClient(client_id=mqtt_client_id, 
                    server=ADAFRUIT_IO_URL, 
                    user=ADAFRUIT_USERNAME, 
                    password=ADAFRUIT_IO_KEY,
                    ssl=False)


try:            
    client.connect()
except Exception as e:
    print('Could not connect to MQTT server {}{}'.format(type(e).__name__, e))
    sys.exit()


     
heart_rate_feed = bytes('{:s}/feeds/{:s}'.format(ADAFRUIT_USERNAME, HEART_RATE_FEED_ID), 'utf-8') # format - renzjoal/feeds/heartrate
sp02_rate_feed = bytes('{:s}/feeds/{:s}'.format(ADAFRUIT_USERNAME, SP02_RATE_FEED_ID), 'utf-8') # format - renzjoal/feeds/sp02
body_temp_feed = bytes('{:s}/feeds/{:s}'.format(ADAFRUIT_USERNAME, BODY_TEMP_FEED_ID), 'utf-8') # format - renzjoal/feeds/bodytemperature

        

def display_vitalsigns(t):
    
    print('\n')
    print('Heart Rate: ', heartrate_value, 'BPM')
    print('SP02 : ', sp02, '%')
    print("Body Temperature: ", body_temp, '°C')
    

    client.publish(heart_rate_feed,    
                  bytes(str(heartrate_value), 'utf-8'),   # Publishing Heart Rate to adafruit.io
                  qos=0)
    
    client.publish(sp02_rate_feed,    
                  bytes(str(sp02), 'utf-8'),   # Publishing SP02 to adafruit.io
                  qos=0)
    
    client.publish(body_temp_feed,    
                  bytes(str(body_temp), 'utf-8'),   # Publishing Body Temprature to adafruit.io
                  qos=0)
    
    
timer = Timer(0)
timer.init(period=5000, mode=Timer.PERIODIC, callback=display_vitalsigns)


while True:    
    sensor.check()

    if sensor.available():
        
        red_reading = sensor.pop_red_from_storage()
        ir_reading = sensor.pop_ir_from_storage()
        
        hrate = ir_reading
        r = red_reading/ir_reading
        sp02= 130-25 * r #YOU CAN ADJUST THE COMPUTATION FOR THE CALIBRATION OF THE SENSOR, DEFAULT IS 110-25 * r.
  
        history.append(hrate)
       
        history = history[-MAX_HISTORY:]
        minima = 0
        maxima = 0
        threshold_on = 0
        threshold_off = 0

        minima, maxima = min(history), max(history)

        threshold_on = (minima + maxima * 3) // 4   
        threshold_off = (minima + maxima) // 2      
        
        if hrate > 1000:
            
            if not heartbeats and hrate > threshold_on:
                heartbeats = True                    
                led.on()
                t_us = ticks_diff(ticks_us(), t_start)
                t_s = t_us/1000000
                f = 1/t_s
                bpm = f * 60
                
                if bpm < 500:
                    t_start = ticks_us()
                    bpm_history.append(bpm)                    
                    bpm_history = bpm_history[-MAX_HISTORY:] 
                    heartrate_value = round(sum(bpm_history)/len(bpm_history) ,2)
                    body_temp=sensor.read_temperature()
                    
            if heartbeats and hrate < threshold_off:
                heartbeats = False
                led.off()
                
        else:
            led.off()
            heartrate_value=0
            sp02=0
            body_temp=0
            print("FINGER OUT!")
            time.sleep(4)
         
                
                

while True:
    try:
        client.check_msg()                  
    except :
        client.disconnect()
        sys.exit()
