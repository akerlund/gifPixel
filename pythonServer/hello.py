from __future__ import print_function
from flask import Flask
from flask import send_file
###############################################################################
import glob
import Image
import threading
import io
import serial
import struct
import sys
import time, math
import re
import random
#import os
###############################################################################
background_color = 0, 0, 0
delay0 = 0.1
delay1 = 0.1
allaBilder = []
rgb0 = []
rgb1 = []
currentBuff = 1
###############################################################################
app = Flask(__name__)

# The start.
@app.route('/')
def index( ):

	out  = '<h1>FREAK-control-pixlor!</h1>'
	out += '<a href ="/gifs/">List Of GIFs</a><br />'
	out += '<a href ="/user/asd">Test</a><br />'
	return out

# Presents a list of all GIFs.
@app.route('/gifs/')
def gifs( ):	
	return writeHTML(listGifFiles( ))

# Access to the local GIF-files.
@app.route('/get_image/<filename>')
def get_image(filename):    
    return send_file(filename, mimetype='image/gif')

# Calling function for GIFs.
@app.route('/rungif/<gif>/')
def runy(gif):
	unGifIt(gif)
	return writeHTML(listGifFiles( ))

###############################################################################
def listGifFiles( ):	

	gifList  = '<img src="file:///home/akerlund/Dropbox/Projects/Projects/PIXEL_ARRAY/PYTHON_SERVER/2673379647.gif" />'
	gifList += '<a href ="/">Back To Index</a>'
	gifList += '<table style="width:100%">\n'
	gifList += '	<tr>\n'
	gifList += '		<td></td>\n'

	cnt = 1
	
	for file in sorted(glob.glob("*.gif")):
		
		if(cnt==0):
			gifList += '	<tr>\n'
		
		gifList += '		<td><a href ="/rungif/' + str(file) + '" alt="' + str(file) + '" title="' + str(file) + '">'
		gifList += '		<img src="/get_image/' + str(file) + '" /></a></td>\n'
		cnt += 1

		if(cnt==20):
			gifList += '	</tr>\n'
			cnt = 0

	gifList += '</table>\n'

	return gifList

def writeHTML(inner):	
	
	out = '<html>\n'
	out += '<head>\n'
	out += '	<title></title>\n'
	out += '	<style>\n'
	out += '		td a {\n'
	out += '			font-size: 1.5em;\n'
	out += '		}\n'
	out += '	</style>\n'
	out += '</head>\n'
	out += '<body>\n'

	out += inner + '\n'

	out += '</body>\n'
	out += '</html>\n'
	
	return out

###############################################################################
def unGifIt(infile):

	global background_color
	global delay0
	global delay1
	global rgb0
	global rgb1
	global currentBuff

  # Opening the GIF picture.
	try:
		im = Image.open(infile)
	except IOError:
		#print "Cant load"
		sys.exit(1)

	i = 0
	mypalette = im.getpalette( )
	name = infile.rsplit(".",1)[0]

  # Parsing out the background and updatetime. 	
	m = re.match(r"(\w+)\((\w+)\,(\w+)\,(\w+)\,(\w+)\)", infile)
	if(currentBuff == 0):
		if(m is not None):
			background_color = int(m.group(3)), int(m.group(4)), int(m.group(5))
			delay1 = float(m.group(2))/1000
		else:
			background_color = 0,0,0
			delay1 = 0.25
	else:
		if(m is not None):
			background_color = int(m.group(3)), int(m.group(4)), int(m.group(5))
			delay0 = float(m.group(2))/1000
		else:
			background_color = 0,0,0
			delay0 = 0.25

  # Extracting the data out of the pictures.
	list = []
	try:
		while 1:
			im.putpalette(mypalette)
			imt = im.convert("RGBA")

			new_im = Image.new("RGB", imt.size, background_color)
			rotated = imt.rotate(270)
			new_im.paste(rotated,None, rotated)

			pixeldata = new_im.load( )
			(w, h) = new_im.size

			for j in range(h):
				for k in range(w):
					list.append(([str(l) for l in pixeldata[j,k]]))

			i += 1
			im.seek(im.tell( ) + 1)

	except EOFError:
		pass
	
	if(currentBuff == 0):
		rgb1 = []
		for i in range(len(list)/256):
			pic = ""
			for j in range(256):
				pixdat = ''.join([chr(int((float(v)/255.0)**math.e*255.0)) for v in list[i*256+j]])
				if len(pixdat) != 3:
					raise Exception("WTF!   %s" % repr(pixdat))
				pic += pixdat 

			rgb1.append(pic[::-1])
		currentBuff = 1
	else:
		rgb0 = []
		for i in range(len(list)/256):
			pic = ""
			for j in range(256):
				pixdat = ''.join([chr(int((float(v)/255.0)**math.e*255.0)) for v in list[i*256+j]])
				if len(pixdat) != 3:
					raise Exception("WTF!   %s" % repr(pixdat))
				pic += pixdat 

			rgb0.append(pic[::-1])
		currentBuff = 0

def gifSender( ):

	global rgb0
	global rgb1
	global delay0
	global delay1
	global currentBuff

	while(1):

		if(currentBuff == 0):
			for p in range(len(rgb0)):
				ser.write(rgb0[p])
				ser.flush( )
				time.sleep(delay0)
		else:
			for p in range(len(rgb1)):
				ser.write(rgb1[p])
				ser.flush( )
				time.sleep(delay1)
				
def randomRun( ):

	global allaBilder
	runTime = 10.0
	time.sleep(runTime)

	while(1):

		bajs = random.randrange(0,len(allaBilder)-1)
		ranStr = allaBilder[bajs]
		unGifIt(ranStr)
		time.sleep(runTime)

###############################################################################
if __name__ == '__main__':

	global currentBuff
	currentBuff = 0
	ser = serial.Serial('/dev/ttyUSB0',baudrate=1500000)
	
	for file in glob.glob("*.gif"):
	 	allaBilder.append(str(file))

	unGifIt("char00(100,0,0,0).gif")

	t = threading.Thread(target=gifSender)
	t.setDaemon(True)
	t.start()

	# r = threading.Thread(target=randomRun)
	# r.setDaemon(True)
	# r.start()

	app.run(host='0.0.0.0')