# ----------------------------
# Set NAME to the program name
# Set ICON to the png icon file name
# Set DESCRIPTION to display within a compatible shell
# Set COMPRESSED to "YES" to create a compressed program
# ----------------------------

NAME        ?= CHIP8
COMPRESSED  ?= YES
ICON        ?= iconc.png
DESCRIPTION ?= "CHIP-8 Emulator"

# ----------------------------

.DEFAULT_GOAL := cdebug

cdebug: clean debug

cmake: clean default

include $(CEDEV)/include/.makefile