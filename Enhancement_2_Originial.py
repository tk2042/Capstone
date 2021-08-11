# Timothy Kelly
# CS-350 Final Project 1
# February 21, 2021

#!/usr/bin/env python

import grovepi
import math
import time
from grove_rgb_lcd import *
import json # Imports json library

# Connect the Grove Temperature & Humidity Sensor Pro to digital port D7
sensor = 7

# temp_humidity_sensor_type
blue = 0 # The blue colored sensor.

# Connect the Light Sensor to analog port A0
# SIG,NC,VCC,GND
light_sensor = 0

# Connect the LED's to digital ports
# SIG,NC,VCC,GND
Blue = 2 # D2
Red = 3 # D3
Green = 4 # D4

# Turn on sensor when it exceeds threshold resistance
threshold = 10

grovepi.pinMode(light_sensor,"INPUT")
grovepi.pinMode(Blue,"OUTPUT")
grovepi.pinMode(Red,"OUTPUT")
grovepi.pinMode(Green,"OUTPUT")

while True:
    try:
        # Get sensor value
        sensor_value = grovepi.analogRead(light_sensor)
        
        # The first parameter is the port, the second parameter
        # is the type of sensor.
        [temp,humidity] = grovepi.dht(sensor,blue) # Get temp and humidity
        
        temp = ((temp * 9) / 5.0) + 32 # Math function for Fahrenheit temp
        
        data = ([temp,humidity]) # info for json file
        
        # Calculate resistance of sensor in K
        resistance = (float)(1023 - sensor_value) * 10 / sensor_value
        
        # Prints infor to display
        print("temp = %.02f F humidity = %.02f%%"%(temp, humidity))
        t = str(temp)
        h = str(humidity)
        
        setRGB(0,50,50) # Set lcd color to light blue. 
        setText("Temp:" + t + "F      " + "Humidity:" +h + "%") # Displays data to lcd.      
        
        # Sets conditions for activating sensors during desired specs
        if temp > 60 or temp < 85 and humidity < 80:
            # Send HIGH to switch on LED
            grovepi.digitalWrite(Green,1)
        
        elif temp > 85 or temp < 95 and humidity < 80:
            # Send HIGH to switch on LED
            grovepi.digitalWrite(Blue,1)
        
        elif temp > 95:
            # Send HIGH to switch on LED
            grovepi.digitalWrite(Red,1)
        
        elif humidity > 80:
            # Send HIGH to switch on LED
            digitalWrite(Green and Blue, 1)
        
        else:
            # Send LOW to switch off LED
            # (Unsure if I need all these statements)
            grovepi.digitalWrite(Green,0)
            grovepi.digitalWrite(Blue,0)
            grovepi.digitalWrite(Red,0)
            grovepi.digitalWrite(Green and Blue, 0)
            
        time.sleep(30 * 60) # records data every 30 minutes
         
        # Write to json file
        with open('data_file.json', 'a') as write_file:
            json.dump(data, write_file)
        
    except IOError:
        print ("Error")



