import pyshark
import collections
import time
import operator
import logging
import struct
import socket

### LOGGING AND SERIAL SETUP ###################################################
logging.basicConfig(level=logging.WARNING,
                    format='%(asctime)s %(levelname)s %(message)s',
                    filename='pysharklive.log',
                    filemode='w')
logging.debug('A debug message')
#logging.info('Some information')
logging.warning('A shot across the bows')

### SOCKET #########################################################################
# this should be better
s = socket.socket()         # Create a socket object
host = socket.gethostname() # Get local machine name
port = 5000                # Reserve a port for your service.

try:
	s.connect((host, port))
	time.sleep(2)
	s.send(b"pyshark says what up!\n")
except socket.error as e:
	print(e,s)

### DATA #########################################################################
capture = pyshark.LiveCapture(interface='en0')
capture.sniff(timeout=20)

def makeDicts():
	"""make a dict out of a live packet capture w/ the data we want"""
	listOfDicts = []
	for pkt in capture.sniff_continuously(packet_count=50):
		listOfDicts.append({"source": pkt["ip"].src, "destination":pkt["ip"].dst, "protocol":pkt.highest_layer})
	return listOfDicts

def findSomeFunStuff():
	"""just digging around"""
	for pkt in capture.sniff_continuously(packet_count=5):
		print(pkt["eth"].addr)


def printUtility(thingsToPrint):
	"""print utility"""
	for i in thingsToPrint:
		print("{0}: {1} > {2}".format(i["protocol"],i["source"],i["destination"]))

def sendToSocket(thingsToSend):
	"""Send the protocols to the socket one by one"""
	for i in thingsToSend:
		proto = "{0}\n".format(i["protocol"])
		s.send(proto.encode())
		print(proto)
		time.sleep(1)

#findSomeFunStuff()
allThePackets = makeDicts()
printUtility(allThePackets)
sendToSocket(allThePackets)
