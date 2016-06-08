import Image
import sys
import cPickle as pickle
import io
import os
import serial
import struct
import sys
import time, math

background_color = 0, 0, 0
try:
    background_color = tuple([int(v) for v in sys.argv[2:5]])
except:
    pass

def pIpic(infile):
    global background_color
  # Opening the GIF picture.
    try:
        im = Image.open(infile)
    except IOError:
        print "Cant load", infile
        sys.exit(1)

    i = 0
    mypalette = im.getpalette( )
    name = infile.rsplit(".",1)[0]

  # Extracting the data out of the pictures.
    list = []
    try:
        while 1:
            im.putpalette(mypalette)
            imt = im.convert("RGBA")

            new_im = Image.new("RGB", imt.size, background_color)
            rotated = imt.rotate(270)
            new_im.paste(rotated,None, rotated)
            #new_im.save(name+str(i)+'.bmp')

            pixeldata = new_im.load( )
            (w, h) = new_im.size

            #with open(name+str(i)+'.txt', 'w') as fajl:
            for j in range(h):
                for k in range(w):
                    #print pixeldata[j,k]
                    #list.append(" ".join([str(l) for l in pixeldata[j,k]]))
                    list.append(([str(l) for l in pixeldata[j,k]]))

            i += 1
            im.seek(im.tell( ) + 1)


    except EOFError:
        pass # end of sequence

  # Taking a shit on the screen.
    #print "LIST: %i" % len(list)
    #print list[0][1]
    #print "LISTMOD: %i" % (len(list)/256)
  
  # Making the binary data.
    rgb = []
    for i in range(len(list)/256):
        pic = ""
        for j in range(256):
            pixdat = ''.join([chr(int((float(v)/255.0)**math.e*255.0)) for v in list[i*256+j]])
            if len(pixdat) != 3:
                raise Exception("WTF!   %s" % repr(pixdat))
            pic += pixdat 
            # pic += chr(int(list[i*256+j][0]))
            # pic += chr(int(list[i*256+j][1]))
            # pic += chr(int(list[i*256+j][2]))

      # Turn the picture clockvise 90*
        # pikk = ""
        # for c in range(16):
        #     for r in range(16):
        #         pikk += pic[]


        rgb.append(pic[::-1])    

    #print "RGB: %i" % len(rgb)
    #print rgb






  # Sending the data out.    
    cnt = 0
    while(1):
        for p in range(len(rgb)):
            cnt = cnt+1
            print "Skickar nr%i" % cnt
            ser.write(rgb[p])
            ser.flush( )
            time.sleep(0.1)

#0.004096s @ 1.5MBaud
ser = serial.Serial('/dev/ttyUSB0',baudrate=1500000)
pIpic(sys.argv[1])

# def pI(infile):

#     try:
#         im = Image.open(infile)
#     except IOError:
#         print "Cant load", infile
#         sys.exit(1)

#     i = 0
#     mypalette = im.getpalette( )
#     name = infile.rsplit(".",1)[0]

#     try:
#         while 1:
#             im.putpalette(mypalette)
#             new_im = Image.new("RGB", im.size)
#             new_im.paste(im)
#             new_im.save(name+str(i)+'.bmp')

#             pixeldata = new_im.load( )
#             (w, h) = new_im.size

#             with open(name+str(i)+'.txt', 'w') as fajl:
#                 for j in range(h):
#                     for k in range(w):
#                         #print pixeldata[j,k]
#                         fajl.write(" ".join([str(l) for l in pixeldata[j,k]])+'\n')

#             i += 1
#             im.seek(im.tell( ) + 1)


#     except EOFError:
#         pass # end of sequence

################################################################################################################

# def pI2(infile):
#     try:
#         im = Image.open(infile)
#     except IOError:
#         print "Cant load", infile
#         sys.exit(1)
#     i = 0
#     mypalette = im.getpalette()
#     name = infile.rsplit(".",1)[0]

#     try:
#         while 1:
#             im.putpalette(mypalette)
#             new_im = Image.new("RGB", im.size)
#             new_im.paste(im)
#             new_im.save(name+str(i)+'.bmp')

#             pixeldata = new_im.load()
#             (w, h) = new_im.size

#             # image_data = [pixeldata[x, y] for y in range(h) for x in range(w)]

# #            image_data = {(x,y):pixeldata[x, y] for y in range(h) for x in range(w)}


#             with open(name+str(i)+'.pickled', "wb") as fajl:
#                 pickle.dump( (w, h, image_data), fajl, pickle.HIGHEST_PROTOCOL)


#             i += 1
#             im.seek(im.tell() + 1)


#     except EOFError:
#         pass # end of sequence




# for y in range(h):
#     for x in range(w):
#         fajl.write(struct.pack("BBB", *pixeldata[x, y]))