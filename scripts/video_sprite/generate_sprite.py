import sys

base = 0
label_base = ""
palette_marker_base = 0
palette_markers_low = ["_video_clear_pal"]
palette_markers_high = []
output = []

def label(s):
    output.append(s+":")

def sublabel(s1):
    return label_base + "_" + s1

def palette_marker(a):
    global palette_marker_base
    sub = sublabel("pal_%i" % palette_marker_base)
    label(sub)
    if a == "low":
        palette_markers_low.append(sub)
    elif a == "high":
        palette_markers_high.append(sub)
    palette_marker_base += 1

def asm(s):
    output.append("    "+s)

def pal(s):
    output.append(s)

for i in range(0, 8):
    base = i
    label_base = "_video_sprite_bit_%i" % base
    palette_marker_base = 0
    
    label(label_base)
    asm("bit %i, (iy)" % (7 - i))
    asm("jr nz, " + sublabel("s1"))
    label(sublabel("s0"))
    asm(".db $3E")
    palette_marker("low")
    asm(".db $00")
    asm("cp a, (hl)")
    asm("jr nz, " + sublabel("s0_d1"))
    label(sublabel("s0_d0"))
    asm(".db $36")
    palette_marker("low")
    asm(".db $00")
    asm("jr " + sublabel("end"))
    label(sublabel("s0_d1"))
    asm(".db $36")
    palette_marker("high")
    asm(".db $FF")
    asm("jr " + sublabel("end"))
    label(sublabel("s1"))
    asm(".db $3E")
    palette_marker("low")
    asm(".db $00")
    asm("cp a, (hl)")
    asm("jr nz, " + sublabel("s1_d1"))
    label(sublabel("s1_d0"))
    asm(".db $36")
    palette_marker("high")
    asm(".db $FF")
    asm("jr " + sublabel("end"))
    label(sublabel("s1_d1"))
    asm(".db $36")
    palette_marker("low")
    asm(".db $00")
    asm("ld a, 1")
    asm("ld (ix), a")
    label(sublabel("end"))
    asm("ld a, 64")
    asm("inc c")
    asm("cp a, c")
    asm("jr nz, " + sublabel("continue"))
    asm("xor a")
    for j in range(8-i):
        asm("inc hl")
    asm("jp _video_sprite_xend")
    label(sublabel("continue"))
    asm("xor a")
    asm("inc hl")

output_temp = output.copy()
output = []

label("_video_palette_markers_low")
for w in palette_markers_low:
    asm(".dw %s-_video_sprite_bit_0" % w)
asm(".dw 0")

asm("")

label("_video_palette_markers_high")
for w in palette_markers_high:
    asm(".dw %s-_video_sprite_bit_0" % w)
asm(".dw 0")

if sys.argv[1] == "code":  
    for out in output_temp:
        print(out)
elif sys.argv[1] == "markers":
    for out in output:
        print(out)