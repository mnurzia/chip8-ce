from PIL import Image, ImageDraw

# LCD uses RGB1555 which is actually 565 but with the 6th green bit at the 15th
# http://wikiti.brandonw.net/index.php?title=84PCE:Ports:4000

colors = [0] * 256

img = Image.open("palette_raw.png")
img2 = Image.new("RGBA", (720, 212), (0, 0, 0, 0));
pixels = img.load()
draw = ImageDraw.Draw(img2)
fnt_l = ImageDraw.truetype("/cygdrive/c/Windows/Fonts/SauceCodeProNF Medium.ttf", 28)
fnt_m = ImageDraw.truetype("/cygdrive/c/Windows/Fonts/SauceCodeProNF Medium.ttf", 24)
fnt_s = ImageDraw.truetype("/cygdrive/c/Windows/Fonts/SauceCodeProNF Medium.ttf", 16)

def bitextend(n, i):
    if n & (1 << i):
        return n 

for y in range(16):
    for x in range(16):
        pix = pixels[x, y]
        idx = y*16 + x
        r = pix[0]
        g = pix[1]
        b = pix[2]
        r5 = r >> 3
        g6 = g >> 2
        b5 = b >> 3
        r8 = r5 << 3 | (r5 >> 2)
        g8 = g6 << 2 | (g6 >> 4)
        b8 = b5 << 3 | (b5 >> 2)
        rgb1555 = ((r5 << 10) | ((g6 >> 1) << 5) | b5) | ((g6 >> 5) << 15)
        colors[y*16 + x] = pixels[x, y]
        rect = [0, 0, 0, 0]
        tpos = [0, 0]
        tsiz = 0
        if idx < 16:
            rect = [104 + (idx * 32), 0, 135 + (idx * 32), 31]
            tpos = [120 + (idx * 32), 16]
            tsiz = 20
            draw.text(tpos, str(idx), fnt_l)
        elif idx < 232:
            cz = (idx - 16) // 36
            cy = ((idx - 16) % 36) // 6
            cx = ((idx - 16) % 36) % 6
            rect = [
                (cz * 120) + (cx * 20),
                48 + (cy * 20),
                19 + (cz * 120) + (cx * 20),
                67 + (cy * 20)
            ]
        else:
            rect = [
                24 + 28 * (idx - 232),
                184,
                51 + 28 * (idx - 232),
                211
            ]
        draw.rectangle(rect, fill=(r8, g8, b8))
        print("0x%04X, " % rgb1555, end="")
    print()

img.close()
img2.save("palette_1555.png")