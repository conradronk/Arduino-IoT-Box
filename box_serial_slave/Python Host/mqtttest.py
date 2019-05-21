import paho.mqtt.client as mqtt #import the client1
import time
import serial

##Language Functions##################################################################
def allOff():
	# arduinoSerial.write(b"channel-a-amplitude:0")
# 	arduinoSerial.flush()
# 	arduinoSerial.write(b"channel-b-amplitude:0")
# 	arduinoSerial.flush()
# 	arduinoSerial.write(b"ac-1-setting:0")
# 	arduinoSerial.flush()
	arduinoSerial.write(b"ac-2-setting:0")
	# arduinoSerial.flush()
# 	arduinoSerial.write(b"ac-3-setting:0")
# 	arduinoSerial.flush()
# 	arduinoSerial.write(b"ac-4-setting:0")

##Callback Functions##################################################################
def on_message(client, userdata, message):
	payload = str(message.payload.decode("utf-8"))
	topic = message.topic
	package = topic[17:] + ":" + payload
	if topic[17:] == "language_processing":
		print("interpretting language command in Python")
		if payload == "all_off":
			allOff()
			
	else:
		arduinoSerial.write(package.encode('utf-8'))	
	print(package)
##End these Functions#################################################################

broker_address="io.adafruit.com" 

arduinoSerial = serial.Serial('/dev/cu.usbmodem1421', 9600)

client = mqtt.Client("P1") #create new instance 
client.username_pw_set(username="conradronk", password="34e3c0e31c5e4c6882d1bae2ece71757")
client.on_message=on_message  


client.connect(broker_address, port=1883, keepalive=60) #connect to broker

client.loop_start()

client.subscribe("conradronk/feeds/channel-a-amplitude", qos=0)
client.subscribe("conradronk/feeds/channel-a-temperature", qos=0)
client.subscribe("conradronk/feeds/ac-1-setting", qos=0)
client.subscribe("conradronk/feeds/ac-2-setting", qos=0)
client.subscribe("conradronk/feeds/ac-3-setting", qos=0)
client.subscribe("conradronk/feeds/ac-4-setting", qos=0)
client.subscribe("conradronk/feeds/sunrise", qos=0)
client.subscribe("conradronk/feeds/language_processing", qos=0)

while True:
	pass
	

	

	

