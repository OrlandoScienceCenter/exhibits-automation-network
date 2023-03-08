import os
from os import listdir
import subprocess
from time import sleep
import RPi.GPIO as GPIO
import paho.mqtt.client as mqtt

def on_connect(client, userdata, flags, rc):
    global topic
    print("Connected with result code "+str(rc))
    client.subscribe(topic)
    print('connected to '+str(topic))

def on_message(client, userdata, msg):
    global topic
    global curStat
    print(msg.topic+" "+str(msg.payload))
    if (msg.payload == 'powerOn'):
        if(not curStat):
            startPlayback()
        else:
            client.publish(topic,'Already playing!')
    elif (msg.payload == 'powerOff'):
        if(curStat):
            stopPlayback()
        else:
            client.publish(topic,'Already paused!')
    elif (msg.payload == 'getStatus'):
        if(curStat):
            client.publish(topic,'Currently playing')
        else:
            client.publish(topic,'Currently paused')
    elif (msg.payload == 'powerReset'):
        client.publish(topic,'Soft restart in 5 seconds')
        stopPlayback()
        sleep(5)
        startPlayback()
    elif (msg.payload == 'resetESP'):
        client.publish(topic,'SHUTDOWN IN 2 SECONDS')
        sleep(2)
        os.system('sudo shutdown -r now')

# Announcement functionality under development
#
#    elif (msg.payload == 'custom_action1'):
#        client.publish(topic,'Playing announcement 1')
#        if(curStat):
#            stopPlayback()
#            subprocess.call(['mpg123','-q','/share/Announcements/1.mp3'])
#            stopPlayback()
#            startPlayback()
#        else:
#            subprocess.call(['mpg123','-q','/share/Announcements/1.mp3'])

def stopPlayback():
    global topic
    global curStat
    subprocess.call(['killall', 'mpg123'])
    GPIO.output(24,0)  #If you want to change the LED pin be sure to change it here too
    curStat = 0
    client.publish(topic,'Stopped playback')

def startPlayback():
    global topic
    global curStat
    subprocess.Popen(['mpg123','-q','--loop','-1', mp3_files[0]])
    GPIO.output(24,1)  #If you want to change the LED pin be sure to change it here too
    curStat = 1
    client.publish(topic,'Started playback')

def pausePlayback():
    global topic
    global curStat
    

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect("test.mosquitto.org",1883,60)  #test.mosquitto.org is a free MQTT broker for testing purposes and should be changed for final use

topic = '___________TOPIC-GOES-HERE_______________'


mp3_files = [ f for f in listdir('/share/Loop/') if f[-4:] == '.mp3' ]  #/share/Loop/ can be changed to any path that has an mp3 file in its lowest directory

GPIO.setmode(GPIO.BCM)
GPIO.setup(23, GPIO.IN, pull_up_down=GPIO.PUD_UP)  #Pin 23 can be changed to any GPIO not already in use
GPIO.setup(24, GPIO.OUT)  #Pin 24 can be changed to any GPIO not already in use
curStat = 0

client.loop_start()
while True:
    if (not GPIO.input(23)):  #If you want to change the button pin be sure to change it here too
	if(curStat):
            stopPlayback()
	else:
            startPlayback()
	sleep(1)
    sleep(0.01);
