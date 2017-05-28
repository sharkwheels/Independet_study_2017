import pyshark
import collections
import time
import operator
import logging
import serial
import struct
import socket


### LOGGING AND SERIAL SETUP ###################################################
logging.basicConfig(level=logging.WARNING,
                    format='%(asctime)s %(levelname)s %(message)s',
                    filename='pysharktest.log',
                    filemode='w')
logging.debug('A debug message')
#logging.info('Some information')
logging.warning('A shot across the bows')

### SOCKET #########################################################################

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

capture = pyshark.FileCapture('steam2.pcapng',only_summaries=True)

def makeDicts():
	"""Just mess around and make a custom dictionary out of the data dump"""
	listOfDicts = []
	for pkt in capture:
		listOfDicts.append({"source": pkt.source, "destination":pkt.destination, "protocol":pkt.protocol, "info":pkt.info})
	return listOfDicts


### UTILITY FUNCTIONS #########################################################################

def sortMaxtoMin(allTheStuff,keyToUse,cN):
	""" Sort incoming data from most active to least, return the top X items """
	rawSources = []
	dictSources = {}
	toReturn = []

	for i in allTheStuff:
		rawSources.append(i[keyToUse])
	print(len(rawSources))
	toCount = [item for item, count in collections.Counter(rawSources).items() if count > 0]
	#print(toCount)
	for i in toCount:
		dictSources.update({i:rawSources.count(i)})
	sortedIps = sorted(dictSources.items(),key=operator.itemgetter(1),reverse=True)
	
	if cN <= 0 or cN > len(sortedIps):
		cN = len(sortedIps)
	#print(cN)
	toReturn = sortedIps[:cN]
	#print(toReturn)
	return toReturn

	
def formatPrinter(title,listOfThings):
	"""Just a print utitlity"""
	print(title)
	for i in listOfThings:
		print("{0}: {1}".format(i[0],i[1]))
	print("----------------")
	print(" ")

def sendToSocket(title,listOfThings):
	"""Send some things to a socket"""
	print(title)
	time.sleep(2)
	for i in listOfThings:
		time.sleep(1)
		print("{0}: {1}".format(i[0],i[1]))
		toSend = "{0}: {1}\n".format(i[0],i[1])
		s.send(toSend.encode())


allTheStuff = makeDicts()
protocols = sortMaxtoMin(allTheStuff,"protocol",10)	## sourceList, the key you're looking for, how many reps above?
activeIP = sortMaxtoMin(allTheStuff,"source",5)
infoTest = sortMaxtoMin(allTheStuff,"info",5)

formatPrinter("Active IPs",activeIP)
formatPrinter("Common Protocols",protocols)
formatPrinter("info",infoTest)

sendToSocket("Protocols",protocols)


### THE SHIT I AIN'T USING #########################################################################

def getProtocols():
	"""Look for some common protocols we'd like to note"""
	prots = [
		"SSDP",
		"TCP",
		"TLSv1.2",
		"DNS",
		"QUIC",
		"MDNS",
		"UDP",
		"CDP",
		"ARP",
		"IGMPv2",
		"ICMPv6",
		"DHCPv6",
		"NETBios",
		"NBNS"
	]
	capProts = []
	toSort = {}
	
	for pkt in capture:
		capProts.append(pkt.protocol)
	for i in prots:
		toSort.update({i:capProts.count(i)})
	sortedprots = sorted(toSort.items(),key=operator.itemgetter(1),reverse=True)
	return sortedprots

## trying to make data into a dict
def getData(protocol):
	listOfDicts = []
	for pkt in capture:
		if pkt.protocol == protocol:
			listOfDicts.append({"source": pkt.source, "destination":pkt.destination, "info":pkt.info})
	return listOfDicts

def getTheInfo(infoItem):
	
	options = ["Server Hello",
		"Application Data",
		"Change Cipher Spec",
		"Encrypted Handshake Message",
		"Server Key Exchange",
		"Client Key Exchange",
		"Encrypted Alert",
		"New Session Ticket",
		"Hello Request",
		"Ignored Unknown Record",
		"Certificate",
		"M-SEARCH * HTTP/1.1 ",
		"[TCP Keep-Alive]",
		"[TCP Dup ACK 5230#1]",
		"[TCP segment of a reassembled PDU]",
		"[TCP Window Update]",
		"[TCP Retransmission]",
		"[TCP Keep-Alive ACK] "
		]
	allTheInfo = []
	for x in infoItem:
		print(x["info"])
		allTheInfo.append(x["info"])

	for i in options:
		print(i, ": ", allTheInfo.count(i))

	

"""
cap0 = capture[0]
print(dir(cap0))

print(cap0.info)
print(cap0.length)
print(cap0.protocol)
print(cap0.no)
print(cap0.source)
print(cap0.destination)
print(cap0.summary_line)
print(cap0.time)
"""


## datasets to work with
"""
ipSSDP = getData("SSDP") 	#this is always search
ipTCP = getData("TCP")		# this has a lot of itneresting things
ipTLSV = getData("TLSv1.2")	#weird
ipDNS = getData("DNS")	#hmm
ipQUIC = getData("QUIC")	# kind of boring on the high level
ipMDNS = getData("MDNS")

print("Protocols")
print("----------------")
print("SSDP: ",len(ipSSDP))
print("TCP: ",len(ipTCP))
print("TLSV: ",len(ipTLSV))
print("DNS: ",len(ipDNS))
print("QUIC: ",len(ipQUIC))
print("MDNS: ", len(ipMDNS))
print("----------------")
"""