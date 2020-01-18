# chip8-ce

A fast [CHIP-8](https://en.wikipedia.org/wiki/CHIP-8) emulator for the 
[TI-84+ CE](https://en.wikipedia.org/wiki/TI-84_Plus_series#TI-84_Plus_CE_and_TI-84_Plus_CE-T),
written in Ez80 Assembly and C.

## Building

Uses the CE-Programming [toolchain](https://github.com/CE-Programming/toolchain).

`$ make`

## Motivation

I've always been interested in really low level programming, but never quite got into
it simply because it was too weird and difficult. I decided that if I wanted to dip my
toe in the water, so to speak, I should write something relatively simple to start.
I wrote this to familiarize myself with assembly in general, but quickly discovered
that ez80 assembly (or z80 assembly, for that matter) was even more difficult than I 
anticipated. The consequence of the z80 design team using random-logic in their processor
was a more feature rich processor but also a weirdly unorthogonal instruction set,
at least by modern standards.

For instance, it's my impression that the three "general-purpose" registers, BC, DE, and HL are less
"general purpose" than one would think. HL is really only used as a pointer, for
example. BC and DE are used for looping, but BC gains more use because of the 
djnz instruction. However by far the greatest hurdle for me was the lack of some
kind of base+offset addressing mode. Any pointer arithmetic basically has to go
through a bunch of loads, adds, and stores which is only made more frustrating
by the add instruction's limited addressing modes. That's why C is quite slow
on the z80 as it requires a lot of pointer arithmetic for arrays and certain 
stack loads or stores. Amusingly, on the actual CHIP-8 pretty much every register
is general purpose (and there are more of them) except for VF and sometimes V0.
Although the ez80 apparently benefits from a full 24-bit ALU and a pretty good
pipeline, it's still limited by the instruction set but not nearly as badly as
the earlier Ti-8x series.

Another aspect of writing this was that I wanted to write an emulator (or, in this
case, a bytecode interpreter). This isn't anything fancy like a JIT or whatnot, it's
a simple fetch, decode, execute loop type operation not unlike the same operations
that were often found in older CPUS.

This entire experience has been really humbling. I had pretty good faith in my ability to
code and learn new languages, so I decided that a challenge would be fun. The truth is that
debugging this has mostly been a huge nightmare, but learning and initially writing
assembly was weirdly soothing, and extremely rewarding. 

## Implementation Status

### CHIP-8 Instructions

| Opcode | Mnemonic           | Description                                                                  | C Pseudocode                                                 | Status             |
| ------ | ------------------ | ---------------------------------------------------------------------------- | ------------------------------------------------------------ | ------------------ |
| 0NNN   | SYS $*NNN*         | Run RCA1802 program at address *NNN*. Unused in most modern implementations. | N/A                                                          | N/A                |
| 00E0   | CLS                | Clear screen (graphics memory)                                               | gfx_clear();                                                 | :heavy_check_mark: |
| 00EE   | RET                | Pop an address from the stack and set PC to this address.                    | return;                                                      | :heavy_check_mark: |
| 1NNN   | JP $*NNN*          | Set PC to *NNN*.                                                             | PC = 0x*NNN*;                                                | :heavy_check_mark: |
| 2NNN   | CALL $*NNN*        | Push PC to the stack and jump to address *NNN*.                              | \*(0x*NNN*)();                                               | :heavy_check_mark: |
| 3XNN   | SE V*X*, $*NN*     | Skip the next instruction if V*X* equals *NN*.                               | if (V[*X*] == 0x*NN*) { PC += 2; }                           | :heavy_check_mark: |
| 4XNN   | SNE V*X*, $*NN*    | Skip the next instruction if V*X* does not equal *NN*.                       | if (V[*X*] != 0x*NN*) { PC += 2; }                           | :heavy_check_mark: |
| 5XY0   | SE V*X*, V*Y*      | Skip the next instruction if V*X* equals V*Y*.                               | if (V[*X*] == V[*Y*]) { PC += 2; }                           | :heavy_check_mark: |
| 6XNN   | LD V*X*, $*NN*     | Set V*X* to *NN*.                                                            | V[*X*] = 0x*NN*;                                             | :heavy_check_mark: |
| 7XNN   | ADD V*X*, $*NN*    | Set V*X* to V*X* + *NN*                                                      | V[*X*] = V[*X*] + 0x*NN*;                                    | :heavy_check_mark: |
| 8XY0   | LD V*X*, V*Y*      | Set V*X* to V*Y*.                                                            | V[*X*] = V[*Y*];                                             | :heavy_check_mark: |
| 8XY1   | OR V*X*, V*Y*      | Set V*X* to V*X* OR V*Y*.                                                    | V[*X*] = V[*X*] \| V[*Y*];                                   | :heavy_check_mark: |
| 8XY2   | AND V*X*, V*Y*     | Set V*X* to V*X* AND V*Y*.                                                   | V[*X*] = V[*X*] & V[*Y*];                                    | :heavy_check_mark: |
| 8XY3   | XOR V*X*, V*Y*     | Set V*X* to V*X* XOR V*Y*.                                                   | V[*X*] = V[*X*] ^ V[*Y*];                                    | :heavy_check_mark: |
| 8XY4   | ADD V*X*, V*Y*     | Set V*X* to V*X* + V*Y*. Set VF to carry.                                    | V[*X*] = V[*X*] + V[*Y*]; V[*F*] = carry;                    | :heavy_check_mark: |
| 8XY5   | SUB V*X*, V*Y*     | Set V*X* to V*X* - V*Y*. Set VF to not borrow.                               | V[*X*] = V[*X*] - V[*Y*]; V[*F*] = !borrow;                  | :heavy_check_mark: |
| 8X06   | SHR V*X*           | Set VF to V*X*'s least significant bit. Set V*X* to V*X* shifted right 1.    | V[*X*] = V[*X*] >> 1; V[*F*] = lsb;                          | :heavy_check_mark: |
| 8XY7   | SUBN V*X*, V*Y*    | Set V*X* to V*Y* - V*X*. Set VF to not borrow.                               | V[*X*] = V[*Y*] - V[*X*]; V[*F*] = !borrow;                  | :heavy_check_mark: |
| 8X0E   | SHL V*X*           | Set VF to V*X*'s most significant bit. Set V*X* to V*X* shifted left 1.      | V[*X*] = V[*X*] << 1; V[*F*] = msb;                          | :heavy_check_mark: |
| 9XY0   | SNE V*X*, V*Y*     | Skip the next instruction if V*X* does not equal V*Y*.                       | if (V[*X*] != V[*Y*]) { PC += 2; }                           | :heavy_check_mark: |
| ANNN   | LD I, $*NNN*       | Set I to *NNN*.                                                              | I = 0x*NNN*;                                                 | :heavy_check_mark: |
| BNNN   | JP V0, $*NNN*      | Set PC to *NNN* + V0.                                                        | PC = 0x*NNN* + V[0];                                         | :heavy_check_mark: |
| CXNN   | RND V*X*, $*NN*    | Set V*X* to a random value ANDed with *NN*.                                  | V[*X*] = rand() & 0xNN;                                      | :heavy_check_mark: |
| DXYN   | DRW V*X*, V*Y*, $N | Draw a bitmapped sprite at (V*X*, V*Y*) with height *N* starting from M[I].  | gfx_sprite(V[*X*], V[*Y*], 0x*N*, memory[I]);                | :heavy_check_mark: |
| EX9E   | SKP V*X*           | Skip the next instruction if key V*X* is pressed.                            | if (keypad[V[*X*] & 0xF]) { PC += 2; }                       | :heavy_check_mark: |
| EXA1   | SKNP V*X*          | Skip the next instruction if key V*X* is not pressed.                        | if (!keypad[V[*X*] & 0xF]) { PC += 2; }                      | :heavy_check_mark: |
| FX07   | LD V*X*, DT        | Set V*X* to the delay timer.                                                 | V[*X*] = timer_delay;                                        | :heavy_check_mark: |
| FX0A   | LD V*X*, K         | Wait until a key is pressed, and then set V*X* to the key.                   | while (!key_pressed) {} V[*X*] = key;                        | :heavy_check_mark: |
| FX15   | LD DT, V*X*        | Set the delay timer to V*X*.                                                 | timer_delay = V[*X*];                                        | :heavy_check_mark: |
| FX18   | LD ST, V*X*        | Set the sound timer to V*X*.                                                 | timer_sound = V[*X*];                                        | :heavy_check_mark: |
| FX1E   | ADD I, V*X*        | Set I to I + V*X*.                                                           | I = I + V[*X*];                                              | :heavy_check_mark: |
| FX29   | FNT V*X*           | Set I to the location of character V*X*'s font sprite.                       | I = font[V[*X*]];                                            | :heavy_check_mark: |
| FX33   | BCD V*X*           | Set M[I, I+1, I+2] to the decimal digits of the BCD character in V*X*.       | bcd_store(&(memory[I]), V[*X*]);                             | :heavy_check_mark: |
| FX55   | SR (I), V*X*       | Store registers up to V*X* at M[I..I+*X*].                                   | for (int k=0;k<*X*;k++) {memory[I+k] = V[k];}                | :heavy_check_mark: |
| FX65   | LR V*X*, (I)       | Load registers from M[I..I+*X*] into registers up to V*X*.                   | for (int k=0;k<*X*;k++) {V[k] = memory[I+k];}                | :heavy_check_mark: |

### SUPERCHIP Instructions

| Opcode | Mnemonic           | Description                                                                  | C Pseudocode                                                 | Status             |
| ------ | ------------------ | ---------------------------------------------------------------------------- | ------------------------------------------------------------ | ------------------ |
| 00CN   | SCD $*N*           | Scroll the screen down *N* lines.                                            | gfx_scroll_down(0x*N*);                                      | :x:                |
| 00FB   | SCR                | Scroll the screen right four pixels.                                         | gfx_scroll_right(4);                                         | :x:                |
| 00FC   | SCL                | Scroll the screen left four pixels.                                          | gfx_scroll_left(4);                                          | :x:                |
| 00FD   | EXIT               | Exit the program.                                                            | exit(0);                                                     | :x:                |
| 00FE   | LOW                | Set the graphics mode to regular CHIP-8 mode (64x32 pixels)                  | gfx_mode = 0;                                                | :x:                |
| 00FF   | HIGH               | Set the graphics mode to SUPERCHIP mode (128*64 pixels)                      | gfx_mode = 1;                                                | :x:                |
| DXY0   | XDRW V*X*, V*Y*    | If in SUPERCHIP graphics, draw a 16*16 sprite at (V*X*, V*Y*) from M[I].     | if (gfx_mode == 1) {gfx_xsprite(V[*X*], V[*Y*], memory[I]);} | :x:                |
| FX30   | XFNT V*X*          | Set I to the location of character V*X*'s large font sprite.                 | I = xfont[V[*X*]];                                           | :x:                |
| FX75   | SRPL V*X*          | Store V*0*..V*X* in some kind of non-volatile storage.                       | for (int i=0;i<*X*;i++) {write(V[i]);}                       | :x:                |
| FX85   | LRPL V*X*          | Load V*0*..V*X* from some kind of non-volatile storage.                      | for (int i=0;i<*X*;i++) {V[i] = read();}                     | :x:                |

## Special Thanks

While writing this, I used Cowgod's [excellent documentation of the CHIP-8](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM).
This was a huge help. Without it the emulator wouldn't be nearly as accurate.