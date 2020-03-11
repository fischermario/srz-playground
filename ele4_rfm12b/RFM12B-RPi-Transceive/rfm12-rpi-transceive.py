# TRANSCEIVER

import sys
import signal
import time
import re

import serial

arduino = None

def signal_handler(sig, frame):
	global arduino

	print("Cleaning up...")

	arduino.close()

	sys.exit(0)

def main():
	global arduino

	signal.signal(signal.SIGINT, signal_handler)

	arduino = serial.Serial('/dev/ttyACM0', 57600, timeout=1)

	# First line is empty
	line = arduino.readline().decode('utf-8')

	# Real
	line = arduino.readline().decode('utf-8')

	if line.strip() != "START_TRANSCEIVE":
		print("This is not the transceiver!\n")
		sys.exit(1)

	sendlength = 1

	while True:
		line = arduino.readline().decode('utf-8')
		if len(line) > 0:
			print(line.strip())
			searchObj = re.search( r'^\[(\d+)\]', line.strip(), re.M|re.I)
			if searchObj:
				target = searchObj.group(1)
				arduino.write(target + "\n")
				line = arduino.readline().decode('utf-8')
				if line.strip() == "OK":
					payload = "Jo-Jo-Jo"
					arduino.write((payload + "\n").encode('utf-8'))
					line = arduino.readline().decode('utf-8')
					if line.strip() == "OK":
						print("SUCCESS!")
					else:
						print("FAIL!")
				else:
					print("FAIL!")

		time.sleep(1)

	return 0

if __name__ == '__main__':
	sys.exit(main())
