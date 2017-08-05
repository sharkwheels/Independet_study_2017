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


def concentrate_handler(unused_addr,args,con1):
	"""
	Are you concentrating intently on something? 
	"""
	global enableSend
	print("Muse-{0}: Concentrate: {1}".format(currentMacAddress,con1))
	
	if con1 >= 1.0:
		send_command()

def send_command():
	cmd = struct.pack('>B', 2)
	ser.write(cmd)
	print(cmd)
	ser.flush()
	

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
	dispatcher.map("/muse/elements/experimental/concentration",concentrate_handler,"concentration_breath")
	#dispatcher.map("/muse/eeg",eeg_handler,"eeg")
	#dispatcher.map("/muse/acc",accel_handler,"accel")
	

	server = osc_server.ThreadingOSCUDPServer((args.ip, args.port), dispatcher)  ## enter server and port on command line

	print("Serving on {}".format(server.server_address))
	server.serve_forever()
