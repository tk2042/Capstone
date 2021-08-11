# Timothy Kelly
# CS-350 Final Project 1
# February 21, 2021 (originally created)

#!/usr/bin/env python

# Import all proper libraries to used in program
import grovepi
import math
import time
from grove_rgb_lcd import *
import json 

# Connect the Grove Temperature & Humidity Sensor Pro to digital port D7
# Included sensor that came with Raspberry Pi kit
sensor = 7

# temp_humidity_sensor_type
# the color of the physical sensor that was used in this program
blue = 0 # The blue colored sensor.

# Connect the Light Sensor to analog port A0
# Connect the Sound Sensor to analog port A1
# SIG,NC,VCC,GND
# Above letters are the pin sensors used in the Grove Pi kit
light_sensor = 0
sound_sensor = 1

# Connect the LED's to digital ports
# SIG,NC,VCC,GND
# connection of light sensors to Grove Pi
Blue = 2 # D2
Red = 3 # D3
Green = 4 # D4

# Turn on sensor when it exceeds threshold resistance
# I experimented with other integers, but settled with 10 to activate device
threshold = 10

# What we want the light sensors to register as
grovepi.pinMode(light_sensor,"INPUT")
grovepi.pinMode(sound_sensor, "INPUT")
grovepi.pinMode(Blue,"OUTPUT")
grovepi.pinMode(Red,"OUTPUT")
grovepi.pinMode(Green,"OUTPUT")

# Beginning of while loop to initiate main function of program
while True:
    try:
        # Gets and reads the sensor value
        sensor_value = grovepi.analogRead(light_sensor)

	# Read the sound level
	sensor_value = grovepi.analogRead(sound_sensor)
        
        # The first parameter is the port, the second parameter
        # is the type of sensor.
        [temp,humidity] = grovepi.dht(sensor,blue) # Get temp and humidity
        
	# Math function for Fahrenheit conversion
        temp = ((temp * 9) / 5.0) + 32 
        
	# Info to be collected in json file
        data = ([temp,humidity])
        
        # Calculate resistance of sensor in K
        resistance = (float)(1023 - sensor_value) * 10 / sensor_value
        
        # Prints reading of temperature and humidity on rgb screen
        print("temp = %.02f F humidity = %.02f%%"%(temp, humidity))
        t = str(temp)
        h = str(humidity)
        
	# Set lcd screen color to light blue
        setRGB(0,50,50) 
	
	# Displays data to lcd
        setText("Temp:" + t + "F      " + "Humidity:" +h + "%")       
        
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
            grovepi.digitalWrite(Green and Blue, 1)
        
        else:
            # Send LOW to switch off LED
            
        # Collects data every 30 minutes    
        time.sleep(30 * 60) 
         
        # Write collected data to json file
        with open('data_file.json', 'a') as write_file:
            json.dump(data, write_file)
        
    except IOError:
        print ("Error")



