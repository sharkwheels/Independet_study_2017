### IMPORTS ######################################
import argparse
import time
import re
import struct
import serial
from pythonosc import dispatcher
from pythonosc import osc_server

addressList = ['0006666F509F']
currentMacAddress = ''
enableSend = False

################# Serial ##################

def connect_to_port():
	ser = None
	try:
		ser = serial.Serial('/dev/cu.usbmodem1421', 9600)
	except:
		pass
	return ser

ser = connect_to_port()
time.sleep(5)
if ser:
	print("Connection to /dev/cu.usbmodem14211 established succesfully!\n")
else:
	print("Could not make connect to /dev/cu.usbmodem14211")



### MUSE HANDLER ######################################

def muse_handler(unused_addr,args,mConfig):
	
	#print("[{0}] ~ {1}".format(args[0], mConfig))

	global currentMacAddress

	## sends an str that looks like a dict but is malformed, stripping out the Mac Address
	## unique to each muse
	
	x = mConfig.split(',')
	dirtyAddress = x[0] 
	cleanAddress = re.sub(r'[^\w\s]','',dirtyAddress)
	finalAddress = cleanAddress[8:] #its always gonna be mac_addr

	#print("Mac Address:", finalAddress)

	if finalAddress in addressList:
		currentMacAddress = finalAddress

			
### DATA HELPERS ######################################

def blink_handler(unused_addr,args,blinkBoolean):
	"""
	Boolean for Blink 
	"""
	if blinkBoolean == 1:
		if currentMacAddress == addressList[0]:
			print("Muse-{0}: Blink1: ".format(currentMacAddress))
		elif currentMacAddress == addressList[1]:
			print("Muse-{0}: Blink2: ".format(currentMacAddress))
			
		

def eeg_handler(unused_addr, args, ch1, ch2, ch3, ch4):
	"""
	Raw EEG Floats
	"""
	#print("EEG (uV) per channel: ", ch1, ch2, ch3, ch4)
	if ch1 > 850:
		if currentMacAddress == addressList[0]:
			print("Muse-{0}: {1}".format(currentMacAddress,ch1))

def jaw_handler(unused_addr,args,clenchBool):
	"""
	Senses if the person's jaw is clenched or not (boolean).
	"""
	if clenchBool == 1:
		if currentMacAddress == addressList[0]:
			print("Muse-{0}: Clench: ".format(currentMacAddress))

def concentrate_handler(unused_addr,args,con1):
	"""
	Are you concentrating intently on something? 
	"""
	print("Muse-{0}: Concentrate: {1}".format(currentMacAddress,con1))
	global enableSend
	#if con1 >=1.0:
		#cmd = struct.pack('>B', 2)
		#ser.write(cmd)
	while con1 >= 1.0:
		enableSend = True
		if enableSend == False:
			break

		

def mellow_handler(unused_addr,args,mel1):
	"""
	Are you mellow?
	"""
	print("Muse-{0}: Mellow: {1}".format(currentMacAddress,mel1))
	

def accel_handler(unused_addr,args,acc1,acc2,acc3):
	"""
	are you moving?
	"""
	print("Muse-{0}: Accel: X:{1}, Y:{2} Z:{3}".format(currentMacAddress,acc1,acc2,acc3))


### RUN IT #################################################

if __name__ == "__main__":
	parser = argparse.ArgumentParser()
	parser.add_argument("--ip",
						default="127.0.0.1", #127.0.0.1
						help="The ip to listen on")
	parser.add_argument("--port",
						type=int,
						default=5000,
						help="The port to listen on")
	args = parser.parse_args()

	### CALL DATA HANDLERS  ######################################

	dispatcher = dispatcher.Dispatcher()
	dispatcher.map("/debug", print)
	dispatcher.map("/muse/config", muse_handler,"Which Muse")
	
	## bools
	#dispatcher.map("/muse/elements/blink", blink_handler,"Blink")
	#dispatcher.map("/muse/elements/jaw_clench", jaw_handler,"Jaw")
	
	## streams ###
	#dispatcher.map("/muse/elements/experimental/mellow",mellow_handler,"Mellow")
	#dispatcher.map("/muse/elements/experimental/concentration",concentrate_handler,"concentration_breath")
	#dispatcher.map("/muse/eeg",eeg_handler,"eeg")
	#dispatcher.map("/muse/acc",accel_handler,"accel")
	

	server = osc_server.ThreadingOSCUDPServer((args.ip, args.port), dispatcher)  ## enter server and port on command line

	print("Serving on {}".format(server.server_address))
	server.serve_forever()
