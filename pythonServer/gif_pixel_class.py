#!/usr/bin/env python2

from __future__ import print_function
from flask import Flask
from flask import send_file

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

# # The start.
# @app.route('/')
# def index( ):

# 	out  = '<h1>FREAK-control-pixlor!</h1>'
# 	out += '<a href ="/gifs/">List Of GIFs</a><br />'
# 	out += '<a href ="/user/asd">Test</a><br />'
# 	return out

# # Presents a list of all GIFs.
# @app.route('/gifs/')
# def gifs( ):	
# 	return writeHTML(get_html_list_of_all_gifs( ))

# # Access to the local GIF-files.
# @app.route('/get_image/<filename>')
# def get_image(filename):    
#     return send_file(filename, mimetype='image/gif')

# # Calling function for GIFs.
# @app.route('/rungif/<gif>/')
# def runy(gif):
# 	unGifIt(gif)
# 	return writeHTML(get_html_list_of_all_gifs( ))

# ###############################################################################
		
# def get_html_list_of_all_gifs( ):	

# 	gif_list  = '<img src="file:///home/akerlund/Dropbox/Projects/Projects/PIXEL_ARRAY/PYTHON_SERVER/2673379647.gif" />'
# 	gif_list += '<a href ="/">Back To Index</a>'
# 	gif_list += '<table style="width:100%">\n'
# 	gif_list += '	<tr>\n'
# 	gif_list += '		<td></td>\n'

# 	cnt = 1
	
# 	for file in sorted(glob.glob("*.gif")):
		
# 		if(cnt==0):
# 			gif_list += '	<tr>\n'
		
# 		gif_list += '		<td><a href ="/rungif/' + str(file) + '" alt="' + str(file) + '" title="' + str(file) + '">'
# 		gif_list += '		<img src="/get_image/' + str(file) + '" /></a></td>\n'
# 		cnt += 1

# 		if(cnt==20):
# 			gif_list += '	</tr>\n'
# 			cnt = 0

# 	gif_list += '</table>\n'

# 	return gif_list

# def writeHTML(inner):	
	
# 	out = '<html>\n'
# 	out += '<head>\n'
# 	out += '	<title></title>\n'
# 	out += '	<style>\n'
# 	out += '		td a {\n'
# 	out += '			font-size: 1.5em;\n'
# 	out += '		}\n'
# 	out += '	</style>\n'
# 	out += '</head>\n'
# 	out += '<body>\n'

# 	out += inner + '\n'

# 	out += '</body>\n'
# 	out += '</html>\n'
	
# 	return out

class gif_pixel( ):

    def __init__(self, animation_time = 3.0):

        self.ser               = None
        self.all_gif_files     = []


        self.gif_frame_delay   = 0
        self.rgb_data_to_pixel = []

        self.animation_time = animation_time
        if self.animation_time < 2.0:
            self.animation_time = 2.0

        self.gif_pixel_lock = threading.Lock( )

        self.app               = Flask(__name__)
        self.send_gif_data = True

    def start_gif_pixel(self):

        print("Welcome to the gifPixel\nStarting up.")

        # Opening a serial port to the gif_pixel.
        self.ser = serial.Serial('/dev/ttyUSB1', baudrate = 1500000)

        # Appending a list with all gif file names.
        for file in glob.glob("*.gif"):
            self.all_gif_files.append(str(file))

        #print(len(self.all_gif_files))
        print("Found %i gif pictures in folder." % len(self.all_gif_files))

        # Initializing with gif.
        (self.gif_frame_delay, self.rgb_data_to_pixel) =\
            self.gif_to_rgb_list("char00(100,0,0,0).gif")

        gif_sender_thread = threading.Thread(target = self.gif_sender)
        gif_sender_thread.setDaemon(True)
        gif_sender_thread.start()

        """ This loop will change current gif
        """
        try:
            print("Starting animation loop.")

            while 1:
                # Sleeping until it will chose a new picture.
                time.sleep(self.animation_time)

                # Randomly selecting new picture.
                random_gif_index = random.randrange(0,len(self.all_gif_files)-1)
                random_gif = self.all_gif_files[random_gif_index]
                
                print("New gif: %s" % random_gif)

                # Extracting its data.
                (delay, rgb) = self.gif_to_rgb_list(random_gif)

                # Updating the buffer and the frame delay.
                self.gif_pixel_lock.acquire( )
                self.gif_frame_delay   = delay
                self.rgb_data_to_pixel = rgb
                self.gif_pixel_lock.release( )

                #print("Buffer updated")


        except KeyboardInterrupt:
            print("Exiting the gifPixel")

        self.send_gif_data = False
        gif_sender_thread.join( )


    """ Sends a list of RGB data over the serial interface.
    Shows a whole animation of one picture.
    """
    def gif_sender(self):

        print("Starting gif_sender.")

        while(self.send_gif_data):

            self.gif_pixel_lock.acquire( )

            for p in range(len(self.rgb_data_to_pixel)):
                #self.gif_pixel_lock.acquire( )
                self.ser.write(self.rgb_data_to_pixel[p])
                self.ser.flush( )
                #self.gif_pixel_lock.release( )
                time.sleep(self.gif_frame_delay)

            self.gif_pixel_lock.release( )
            time.sleep(0.01)
            #print("Gif animation complete.")

    """ Will extract the data from pixels in a gif.
    """
    def gif_to_rgb_list(self, infile):

        # Opening the GIF picture.
        try:
            im = Image.open(infile)
        except IOError:
            print("Cant load file: %s", infile)
            sys.exit(1)

        i = 0
        mypalette = im.getpalette( )
        name = infile.rsplit(".",1)[0]

        # Parsing out the background and updatetime. 	
        m = re.match(r"(\w+)\((\w+)\,(\w+)\,(\w+)\,(\w+)\)", infile)

        if(m is not None):
            background_color = int(m.group(3)), int(m.group(4)), int(m.group(5))
            delay = float(m.group(2))/1000
        else:
            background_color = 0,0,0
            delay = 0.25

        # Extracting the data out of the pictures.
        extracted_picture = []
        try:
            while 1:
                im.putpalette(mypalette)
                imt = im.convert("RGBA")

                new_im = Image.new("RGB", imt.size, background_color)
                rotated = imt.rotate(270)
                new_im.paste(rotated, None, rotated)

                pixeldata = new_im.load( )
                (w, h) = new_im.size

                for j in range(h):
                    for k in range(w):
                        extracted_picture.append(([str(l) for l in pixeldata[j,k]]))

                i += 1
                im.seek(im.tell( ) + 1)

        except EOFError:
            pass

        rgb = []
        for i in range(len(extracted_picture)/256):
            pic = ""
            for j in range(256):
                pixdat = ''.join([chr(int((float(v)/255.0)**math.e*255.0)) for v in extracted_picture[i*256+j]])
                if len(pixdat) != 3:
                    raise Exception("WTF! %s" % repr(pixdat))
                pic += pixdat 

            rgb.append(pic[::-1])

        return (delay, rgb)

if __name__ == '__main__':

    gif_pixler = gif_pixel( )
    gif_pixler.start_gif_pixel( )

#	app.run(host='0.0.0.0')