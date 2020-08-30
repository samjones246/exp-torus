from PIL import Image
import PIL.ImageOps
import pytesseract
import cv2
import os
import sys

import torus

pytesseract.pytesseract.tesseract_cmd = "C:\\Program Files\\Tesseract-OCR\\tesseract.exe"

os.system("adb push inp_a      /data/local/tmp/inp")
os.system("adb shell chmod +x /data/local/tmp/inp")

#os.system("adb shell /data/local/tmp/inp tap 500 1520");
#os.system("adb shell /data/local/tmp/inp swipe 140 750 140 1540 0.1");
#sys.exit()
while True:

    #os.system("adb exec-out screencap -p > scr.png")
    os.system("adb shell screencap -p /sdcard/scr.png")
    os.system("adb pull /sdcard/scr.png")
    os.system("adb shell rm /sdcard/scr.png")

    i = cv2.imread('scr.png')
    i = cv2.cvtColor(i, cv2.COLOR_BGR2GRAY)
    #threshold = 127 # to be determined
    #_, i_binarized = cv2.threshold(i, threshold, 255, cv2.THRESH_BINARY)
    #i = Image.fromarray(i_binarized)
    i = Image.fromarray(i)
    i = PIL.ImageOps.invert(i)

    x0 = 45 #66
    y0 = 470 #666
    x1 = 675 #1011
    y1 = 1103 #1626
    dx = (x1 - x0) / 6
    dy = (y1 - y0) / 6
    dd = 28

    def convert(val, axis):
        displayWidth = 720
        displayHeight = 1480
        minX = 0
        maxX = 4000
        minY = 0
        maxY = 4000
        if axis == "x":
            return (val * (maxX - minX + 1) / displayWidth) + minX
        else:
            return (val * (maxY - minY + 1) / displayHeight) + minY

    M = [ [ 0 for x in range(6) ] for y in range(6) ]

    for x in range(0, 6):
        for y in range(0, 6):
            i.save("full.png")
            nu = i.crop((x0 + x * dx + dd, y0 + y * dy + dd, x0 + (x + 1) * dx - dd, y0 + (y + 1) * dy - dd))
            nu.save("%d-%d.png" % (x,y))
            M[y][x] = int(pytesseract.image_to_string(nu,
                    config='--dpi 300 --psm 13 --tessdata-dir ./tess -l digits'))
            if M[y][x] == 924:
                M[y][x] = 24

    print(M)
    
    # check that all numbers are present
    Mall = [item for sublist in M for item in sublist]
    Mall.sort()
    
    all_good = True
    for k in range(0, 36):
        if Mall[k] != k + 1:
            print("\n\n\nERROR\n\n\n")
            all_good = False

    if all_good:
        steps = torus.f(M)
        zip_steps = []
        zip_steps_num = []
        for st in range(len(steps)):
            if st > 0 and steps[st] == zip_steps[-1]:
                zip_steps_num[-1] = zip_steps_num[-1] + 1
            else:
                zip_steps.append(steps[st])
                zip_steps_num.append(1)

        #print(steps)
        #print(zip_steps)
        #print(zip_steps_num)
        print("executing %d swipes" % len(zip_steps))

        commands = "cd /data/local/tmp/;\n"

        for st in range(len(zip_steps)):
            s = zip_steps[st]
            n = zip_steps_num[st]
            di = s[0]
            rc = int(s[1:])
            if di == "R":
                XX0 = x0 + dd
                YY0 = y0 + rc * dy + dd
                XX1 = XX0 + n * dx
                YY1 = YY0
            if di == "L":
                XX0 = x0 + 5 * dx + dd
                YY0 = y0 + rc * dy + dd
                XX1 = XX0 - n * dx
                YY1 = YY0
            if di == "U":
                XX0 = x0 + rc * dx + dd
                YY0 = y0 + 5 * dy + dd
                XX1 = XX0
                YY1 = YY0 - n * dy
            if di == "D":
                XX0 = x0 + rc * dx + dd
                YY0 = y0 + dd
                XX1 = XX0
                YY1 = YY0 + n * dy
            commands += "./inp swipe %d %d %d %d 0.0001;\n" % (convert(XX0, "x"), convert(YY0, "y"), convert(XX1, "x"), convert(YY1, "y"))

        #print("adb shell \"" + commands + "\"")
        #for command in commands:
        with open("commands.sh", "w") as f:
            f.write(commands)
        os.system("adb push commands.sh /data/local/tmp/commands.sh")
        os.system("adb shell \"cd /data/local/tmp;chmod +x commands.sh;./commands.sh\"")
        print("done\n")
        #sys.exit()

        os.system("sleep 0.5")
        commands = "cd /data/local/tmp/;"
        # Great! Claim * Stars
        commands += "./inp tap 2000 3310;"
        commands += "sleep 0.5;"
        # Play Torus Puzzle
        commands += "./inp tap 2000 2865;"
        commands += "sleep 0.5;"
        # Hard - * Stars   ---- using slow input so phone stays awake (?)
        commands += "input tap 380 860;"
        commands += "sleep 1;"
        os.system("adb shell \"" + commands + "\"")
    
    else: # not all_good
        print(Mall)
        sys.exit()

