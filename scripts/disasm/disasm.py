import sys

def bins(a):
    return "".join(["1" if (a & (1 << i)) else "0" for i in range(15, -1, -1)])

def disasm(fins, addr, mode="full"):
    ostr = []
    
    def out(text):
        ostr.append(text)

    ins = fins >> 8
    ins2 = fins & 0xFF

    cls0 = ins >> 4
    cls1 = ins & 15
    cls2 = ins2 >> 4
    cls3 = ins2 & 15
    insb = bins(fins)
    
    if mode == "full":
        out("%04X %02X%02X %s " % (addr, ins, ins2, insb))
    elif mode == "sparse":
        out("%04X " % (addr))
    elif mode == "none":
        pass
    else:
        raise ValueError("invalid mode")
    
    if cls0 == 0:
        if cls1 == 0:
            if cls2 == 0xE:
                if cls3 == 0x0:
                    out("CLS")
                elif cls3 == 0xE:
                    out("RET")
                else:
                    out("INVALID")
            else:
                out("INVALID")
        else:
            out("SYS $%03X" % (fins & 0x0FFF))
    elif cls0 == 1:
        out("JP $%03X" % (fins & 0x0FFF))
    elif cls0 == 2:
        out("CALL $%03X" % (fins & 0x0FFF))
    elif cls0 == 3:
        out("SE V%X, $%02X" % (cls1, fins & 0xFF))
    elif cls0 == 4:
        out("SNE V%X, $%02X" % (cls1, fins & 0xFF))
    elif cls0 == 5:
        out("SE V%X, V%X" % (cls1, cls2))
    elif cls0 == 6:
        out("LD V%X, $%02X" % (cls1, fins & 0xFF))
    elif cls0 == 7:
        out("ADD V%X, $%02X" % (cls1, fins & 0xFF))
    elif cls0 == 8:
        if cls3 == 0:
            out("LD V%X, V%X" % (cls1, cls2))
        elif cls3 == 1:
            out("OR V%X, V%X" % (cls1, cls2))
        elif cls3 == 2:
            out("AND V%X, V%X" % (cls1, cls2))
        elif cls3 == 3:
            out("XOR V%X, V%X" % (cls1, cls2))
        elif cls3 == 4:
            out("ADD V%X, V%X" % (cls1, cls2))
        elif cls3 == 5:
            out("SUB V%X, V%X" % (cls1, cls2))
        elif cls3 == 6:
            out("SHR V%X" % cls2)
        elif cls3 == 7:
            out("SUBN V%X, V%X" % (cls1, cls2))
        elif cls3 == 0xE:
            out("SUBN V%X" % cls2)
        else:
            out("INVALID")
    elif cls0 == 9:
        out("SNE V%X, V%X" % (cls1, cls2)) 
    elif cls0 == 0xA:
        out("LD I, $%03X" % (fins & 0x0FFF))
    elif cls0 == 0xB:
        out("JP V0, $%03X" % (fins & 0x0FFF)) 
    elif cls0 == 0xC:
        out("RND V%X, $%02X" % (cls1, fins & 0xFF))
    elif cls0 == 0xD:
        out("DRW V%X, V%X, $%X" % (cls1, cls2, cls3))
    elif cls0 == 0xE:
        if ins2 == 0x9E:
            out("SKP V%X" % cls1)
        elif ins2 == 0xA1:
            out("SKNP V%X" % cls1)
        else:
            out("INVALID")
    elif cls0 == 0xF:
        if ins2 == 0x07:
            out("LD V%X, DT" % cls1)
        elif ins2 == 0x0A:
            out("LD V%X, K" % cls1)
        elif ins2 == 0x15:
            out("LD DT, V%X" % cls1)
        elif ins2 == 0x18:
            out("LD ST, V%X" % cls1)
        elif ins2 == 0x1E:
            out("ADD I, V%X" % cls1)
        elif ins2 == 0x29:
            out("FNT V%X" % cls1)
        elif ins2 == 0x33:
            out("BCD V%X" % cls1)
        elif ins2 == 0x55:
            out("SR (I), V%X" % cls1)
        elif ins2 == 0x65:
            out("LR V%X, (I)" % cls1)
        else:
            out("INVALID")
    else:
        out("INVALID")
    
    return "".join(ostr)

if __name__ == "__main__":
    f = open(sys.argv[1], 'rb')
    dat = f.read()
    f.close()

    ctr = 0
    while ctr < len(dat):
        ins = dat[ctr]
        ctr += 1
        ins2 = dat[ctr]
        ctr += 1
        
        fins = (ins << 8) + ins2
        
        print(disasm(fins, ctr - 2 + 0x200, mode="sparse"))