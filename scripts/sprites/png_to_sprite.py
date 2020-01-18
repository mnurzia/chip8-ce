from PIL import Image
import os
import sys

for arg in sys.argv[1:]:
    print("_ui_sprite_%s:" % os.path.basename(arg)[:-4])
    img = Image.open(arg)
    img2 = img.convert("RGBA")
    pix = img.load()
    for y in range(0,8):
        curbyte = 0
        for x in range(0,8):
            if pix[x,y][3] > 0:
                curbyte |= 1 << x
        if y != 0:
            print(", $%02X" % curbyte,end="")
        else:
            print("    .db $%02X" % curbyte,end="")
    print()
    print()
    img.close()
    img2.close()