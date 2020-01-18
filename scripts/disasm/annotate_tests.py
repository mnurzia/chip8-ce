import sys

import disasm

f = open(sys.argv[1])
lines = f.readlines()
f.close()

ctr = 0x200

for line in lines:
    if line.startswith("    ") and line[4:7] == "INS":
        asm = disasm.disasm(int(line[10:14], 16), ctr, "sparse")
        print(line.rstrip("\n") + " /* " + asm + " */ ")
        ctr += 2
    elif line.startswith("    ") and line[4:7] == "DAT":
        insb = disasm.bins(int(line[10:14], 16))
        print(line.rstrip("\n") + " /* " + insb + " */ ")
        ctr += 2
    elif line.startswith("    RUN()"):
        ctr = 0x200
        print(line,end="")
    else:
        print(line,end="")