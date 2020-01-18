    .assume adl=1

; core.h
    .def _core_instruction
    .ref _core_memory
    .ref _core_reg_v
    .ref _core_reg_i
    .ref _core_reg_pc
    .ref _core_timer_delay
    .ref _core_timer_sound
    .ref _core_stack
    .ref _core_reg_sp
    .ref _core_flags
    .ref _core_lfsr
    .ref _core_reg_key

; input.h
    .ref _input_keys

; video.h
    .ref _video_memory
    .ref _video_clear
    .ref _video_sprite
    .ref _video_sprite_x;
    .ref _video_sprite_y;
    .ref _video_sprite_h;
    .ref _video_sprite_buffer;
    
    .ref _dbg_Debugger

_core_instruction:
    push de
    ld hl, _core_memory     ; set up the instruction pointer
    ld.s de, (_core_reg_pc)
    add hl, de
    push ix                 ; set up fast register access consts.
    push iy
    ld iy, _core_reg_v
_core_instruction_load:
    push bc
    ld de, $000000
    ld bc, (hl)             ; bc now contains the opcode byte-swapped
    ld d, c                 ; move the first byte of the opcode
    srl d                   ; get the first nibble of the opcode
    srl d
    srl d
    srl d
    jr z, _core_ins_class_0
    dec d
    jr z, _core_ins_jp
    dec d
    jr z, _core_ins_call
    dec d
    jr z, _core_ins_sei
    dec d
    jr z, _core_ins_sne
    dec d
    jr z, _core_ins_ser
    dec d
    jr z, _core_ins_ldi
    dec d
    jr z, _core_ins_addi
    dec d
    jr z, _core_ins_class_8
    dec d
    jr z, _core_ins_sner
    dec d
    jr z, _core_ins_ldii
    dec d
    jr z, _core_ins_jpo
    dec d
    jr z, _core_ins_rnd
    dec d
    jr z, _core_ins_drw
    dec d
    jr z, _core_ins_class_e
    dec d
    jr z, _core_ins_class_f
    jp _core_ins_invalid
_core_ins_class_0:          ; if the instruction is of "0XXX"
    ld a, c                 ; "compare" c with zero              
    jr nz, _core_ins_invalid; if not, jump to the invalid instruction handler (currently no 1802 programs supported)
    ld a, b                 
    cp a, $EE               ; check if instruction is "00EE" - RET
    jr z, _core_ins_ret
    cp a, $E0               ; check if instruction is "00E0" - CLS
    jr z, _core_ins_cls
    cp a, $FD
    jr z, _core_ins_exi
    jp _core_ins_invalid
_core_ins_ret:              ; RET instruction
    ld.s de, (_core_reg_sp) ;   compute new stack pointer
    ld a, e
    or a, a
    jp z, _core_ins_stkunder;   if there is an underflow throw an error
    dec a
    dec a
    ld (_core_reg_sp), a    ;   save stack pointer
    ld e, a
    ld hl, _core_stack
    add hl, de              ;   get effective stack address
    ld de, 0                ;   clear de
    ld e, (hl)              ;   load the return address into de
    inc hl
    ld d, (hl)
    ld hl, _core_memory
    add hl, de              ;   then add it to mem base
    jp _core_ins_end
_core_ins_cls:              ; CLS instruction
    push hl
    call _video_clear       ;   clear the video
    ;ld hl, _core_flags      ;   before we get hl back, set the display flag
    ;set 1, (hl)
    pop hl
    inc hl                  ;   increment program counter
    inc hl
    jp _core_ins_end
_core_ins_exi:
    inc hl
    inc hl
    jp _core_ins_exitf
_core_ins_jp:               ; JP instruction
    ld h, b
    ld l, c
    ld bc, 0
    ld b, h
    ld c, l
    ld a, c                 
    and a, $0F              ;   clip that pesky top nibble off
    ld c, b                 ;       swap the instruction bytes while we're at it
    ld b, a
    ld hl, _core_memory     ;   add bc offset to pc
    add hl, bc
    jp _core_ins_end
_core_ins_call:             ; CALL instruction
    push iy
    ld a, c
    ld e, b
    ld bc, 0
    ld c, e
    and a, $0F              ;   clip top nibble
    ld b, a
    ld de, 0
    ld a, (_core_reg_sp)
    ld e, a
    ld iy, hl               ;   temporarily save pc, we'll need it later  
    ld hl, _core_stack      ;   load up the stack
    ld a, $40               ;   check if there may be an overflow
    cp a, e                 ;       sets zero if e == $40 (32 elements on stack)
    jp z, _core_ins_stkover ;   jump to overflow handler if that's the case
    add hl, de              ;   get a stack pointer
    ld a, e
    inc a
    inc a
    ld (_core_reg_sp), a    ;   save the new sp
    ld de, iy               ;   get back pc
    inc de
    inc de
    push hl
    ex de, hl
    ld de, _core_memory
    xor a
    sbc hl, de
    ld de, hl
    pop hl
    ld (hl), e              ;   save pc on the stack
    inc hl
    ld (hl), d
    ld hl, _core_memory     ;   (finally) load the new address into pc
    add hl, bc
    pop iy
    jp _core_ins_end
_core_ins_sei:              ; SEI instruction
    ld a, c
    and a, $0F              ;   clip top nibble
    ld ix, iy               ;   set reg constant (will get optimized into an lea)
    ld e, a                 
    ld d, 0
    add ix, de              
    ld a, (ix)              ;   load up the register for comparison
    cp a, b                 ;   do the actual test
    jr nz, _core_ins_sei_fail;  quick trick to add two extra to pc incase it passes
    inc hl
    inc hl
_core_ins_sei_fail:
    inc hl
    inc hl
    jp _core_ins_end
_core_ins_sne:              ; SNE instruction
    ld a, c
    and a, $0F              ;   clip top nibble
    ld ix, iy               ;   set reg constant (will get optimized into an lea)
    ld e, a                 
    ld d, 0
    add ix, de              
    ld a, (ix)              ;   load up the register for comparison
    cp a, b                 ;   do the actual test
    jr z, _core_ins_sne_fail
    inc hl
    inc hl
_core_ins_sne_fail:
    inc hl
    inc hl
    jp _core_ins_end
_core_ins_ser:              ; SER instruction
    ld a, c
    and a, $0F              ;   clip top nibble
    ld ix, iy               ;   set reg constant (will get optimized into an lea)
    ld e, a                 
    ld d, 0
    add ix, de              
    ld a, (ix)              ;   load up the register for comparison
    ld c, a                 ;   save a for later
    ld a, b                 ;   get 2nd register no
    srl a
    srl a
    srl a
    srl a
    ld ix, iy               ;   retreive 2nd register value
    ld e, a
    add ix, de
    ld a, (ix)
    cp a, c                 ;   perform the comparison
    jr nz, _core_ins_ser_fail
    inc hl
    inc hl
_core_ins_ser_fail:
    inc hl
    inc hl
    jp _core_ins_end
_core_ins_ldi:              ; LDI instruction
    ld a, c
    and a, $0F              ;   clip top nibble
    ld ix, iy               ;   set reg constant (will get optimized into an lea)
    ld e, a                 
    ld d, 0
    add ix, de              
    ld (ix), b              ;   save it.
    inc hl
    inc hl
    jp _core_ins_end
_core_ins_addi:              ; ADDI instruction
    ld a, c
    and a, $0F              ;   clip top nibble
    ld ix, iy               ;   set reg constant
    ld e, a
    ld d, 0
    add ix, de
    ld a, (ix)              ;   load the register
    add a, b                ;   perform the addition
    ld (ix), a              ;   save the register back
    inc hl
    inc hl
    jp _core_ins_end
_core_ins_class_8:          ; if the instruction is of "8XXX"
    push hl                 ; we won't need hl till much later
    ld a, b
    srl a                   ; retrieve y register index
    srl a
    srl a
    srl a                   ; a now holds opcode[y].
    ld ix, iy
    ld e, a
    ld d, 0
    add ix, de
    ld l, (ix)              ; load vy into l
    ld a, c
    and a, $0F              ; we load opcode[x] last so the pointer to x can be preserved in ix.
    ld ix, iy
    ld e, a
    add ix, de
    ld h, (ix)              ; load vx into h
    ld a, b                 ; setup a for decrement-compare
    and a, $0F              ; now vx and vy are loaded; begin to test each ins variant.
    jr z, _core_ins_ldr
    dec a
    jr z, _core_ins_or
    dec a
    jr z, _core_ins_and
    dec a
    jr z, _core_ins_xor
    dec a
    jr z, _core_ins_add
    dec a
    jr z, _core_ins_sub
    dec a
    jr z, _core_ins_shr
    dec a
    jr z, _core_ins_subn
    sub a, 7
    jr z, _core_ins_shl
    pop hl
    jp _core_ins_invalid
_core_ins_ldr:              ; LDR instruction
    ld (ix), l
    jr _core_ins_class_8_end
_core_ins_or:               ; OR instruction.
    ld a, h
    or a, l
    ld (ix), a
    jr _core_ins_class_8_end
_core_ins_and:              ; AND instruction.
    ld a, h
    and a, l
    ld (ix), a
    jr _core_ins_class_8_end
_core_ins_xor:              ; XOR instruction.
    ld a, h
    xor a, l
    ld (ix), a
    jr _core_ins_class_8_end
_core_ins_add:              ; ADD instruction.
    ld a, h
    add a, l
    ld (ix), a
    jr nc, _core_ins_add_fail
    ld (iy+15), 1           ; set vf to carry
    jp _core_ins_class_8_end
_core_ins_add_fail:
    ld (iy+15), 0
    jp _core_ins_class_8_end
_core_ins_sub:              ; SUB instruction.
    ld a, h
    sub a, l
    ld (ix), a
    jr c, _core_ins_sub_fail;   remember !borrow is stored to vf...
    ld (iy+15), 1
    jp _core_ins_class_8_end
_core_ins_sub_fail:
    ld (iy+15), 0
    jp _core_ins_class_8_end
_core_ins_shr:              ; SHR instruction.
    srl h
    ld (ix), h
    jr nc, _core_ins_shr_fail;
    ld (iy+15), 1
    jp _core_ins_class_8_end
_core_ins_shr_fail:
    ld (iy+15), 0
    jp _core_ins_class_8_end
_core_ins_subn:             ; SUBN instruction.
    ld a, h
    sub a, l
    ld (ix), a
    jr c, _core_ins_subn_fail
    ld (iy+15), 1
    jp _core_ins_class_8_end
_core_ins_subn_fail:
    ld (iy+15), 0
    jp _core_ins_class_8_end
_core_ins_shl:              ; SHL instruction.
    sla h
    ld (ix), h
    jr nc, _core_ins_shl_fail;
    ld (iy+15), 1
    jp _core_ins_class_8_end
_core_ins_shl_fail:
    ld (iy+15), 0
    jp _core_ins_class_8_end
_core_ins_class_8_end:
    pop hl
    inc hl
    inc hl
    jp _core_ins_end
_core_ins_sner:             ; SNER instruction.
    ld a, c
    and a, $0F              ;   clip top nibble
    ld ix, iy               ;   set reg constant (will get optimized into an lea)
    ld e, a                 
    ld d, 0
    add ix, de              
    ld a, (ix)              ;   load up the register for comparison
    ld c, a                 ;   save a for later
    ld a, b                 ;   get 2nd register no
    srl a
    srl a
    srl a
    srl a
    ld ix, iy               ;   retreive 2nd register value
    ld e, a
    add ix, de
    ld a, (ix)
    cp a, c                 ;   perform the comparison
    jr z, _core_ins_sner_fail
    inc hl
    inc hl
_core_ins_sner_fail:
    inc hl
    inc hl
    jp _core_ins_end
_core_ins_ldii:             ; LDII instruction. (Load I Immediate)
    ld e, b
    ld a, c
    and a, $0F
    ld (_core_reg_i+1), a
    ld d, a
    ld a, e
    ld (_core_reg_i), a
    inc hl
    inc hl
    jp _core_ins_end
_core_ins_jpo:              ; JPO instruction. (Jump Immediate + Offset)
    ld hl, 0
    ld l, b
    ld a, c
    and a, $0F
    ld h, a                 ;   DE now holds NNN. Pretty neat, eh?
    ld b, 0
    ld c, (iy)
    add hl, bc              ;   HL doesn't need to be clipped here, that is done later.
    ld bc, _core_memory     ;   get the memory offset back
    add hl, bc
    jp _core_ins_end
_core_ins_rnd:
    push hl
    ld a, c
    and a, $0F
    ld e, a
    ld d, 0
    ld ix, iy
    add ix, de
    ld a, (_core_lfsr+1)    ;   This uses an 16 bit LFSR. Super low quality randomness, but probably good enough.
    srl a
    ld h, a
    ld a, (_core_lfsr)
    rr a
    ld l, a
    jr nc, _core_ins_rnd_noxor
    ld a, l
    xor a, 0
    ld l, a
    ld a, h
    xor a, $B4
    ld h, a
_core_ins_rnd_noxor:
    ld (_core_lfsr), hl     ;   save back the lfsr
    ld a, l
    and a, b                ;   apply the logical and
    ld (ix), a              ;   save to the target register
    pop hl
    inc hl
    inc hl
    jp _core_ins_end
_core_ins_drw:              ; DRW instruction.
    ld a, c
    and a, $0F
    ld e, a
    ld d, 0
    ld ix, iy
    add ix, de
    ld a, (ix)
    ;and a, $3F              ;   clip top 2 bits to wrap
    ld (_video_sprite_x), a
    ld a, b
    srl a
    srl a
    srl a
    srl a
    ld e, a
    ld ix, iy
    add ix, de
    ld a, (ix)
    ;and a, $3F              ;   clip top 3 bits to wrap
    ld (_video_sprite_y), a
    ld a, b
    and a, $0F
    ld (_video_sprite_h), a
    push hl
    push bc
    push af
    ld ix, _video_sprite_buffer
    ld bc, $000000
    ld a, (_core_reg_i)     ;   had to do it to em :)
    ld c, a
    ld a, (_core_reg_i+1)
    ld b, a
    pop af
_core_ins_drw_loop:
    ld hl, _core_memory
    add hl, bc
    ld de, (hl)
    ld (ix), e
    inc ix
    inc bc
    push af
    ld a, b
    and a, $0F
    ld b, a
    pop af
    dec a
    jr nz, _core_ins_drw_loop
    call _video_sprite
    pop bc
    pop hl
    inc hl
    inc hl
    jp _core_ins_end
_core_ins_class_e:          ; if the instruction is of EXCC
    ld a, b
    cp a, $9E
    jr z, _core_ins_skp
    cp a, $A1
    jr z, _core_ins_sknp
    jp _core_ins_invalid
_core_ins_skp:              ; SKP instruction.
    ld a, c
    and a, $0F
    ld e, a
    ld d, 0
    ld ix, iy
    add ix, de
    ld e, (ix)
    ld a, e
    and a, $0F
    ld e, a
    ld ix, _input_keys
    add ix, de
    ld a, 1
    cp a, (ix)
    jr nz, _core_ins_skp_fail
    inc hl
    inc hl
_core_ins_skp_fail:
    inc hl
    inc hl
    jp _core_ins_end
_core_ins_sknp:             ; SKNP instruction.
    ld a, c
    and a, $0F
    ld e, a
    ld d, 0
    ld ix, iy
    add ix, de
    ld e, (ix)
    ld a, e
    and a, $0F
    ld e, a
    ld ix, _input_keys
    add ix, de
    ld a, 1
    cp a, (ix)
    jr z, _core_ins_skp_fail
    inc hl
    inc hl
_core_ins_sknp_fail:
    inc hl
    inc hl
    jp _core_ins_end
_core_ins_class_f:          ; if the instruction is of FNNN
    ld a, b
    cp a, $07
    jr z, _core_ins_lddt
    cp a, $0A
    jr z, _core_ins_wkey
    cp a, $15
    jr z, _core_ins_ldrdt
    cp a, $18
    jr z, _core_ins_ldrst
    cp a, $1E
    jr z, _core_ins_addri
    cp a, $29
    jr z, _core_ins_fnt
    cp a, $33
    jr z, _core_ins_bcd
    cp a, $55
    jr z, _core_ins_sri
    cp a, $65
    jr z, _core_ins_lri
    jp _core_ins_invalid
_core_ins_lddt:             ; LD from delay timer instruction.
    ld a, c
    and a, $0F
    ld e, a
    ld d, 0
    ld ix, iy
    add ix, de
    ld a, (_core_timer_delay)
    ld (ix), a
    inc hl
    inc hl
    jp _core_ins_end
_core_ins_wkey:             ; LD key (wait for key) instruction.
    ld a, c
    and a, $0F
    ld (_core_reg_key), a
    inc hl
    inc hl
    jp _core_ins_wait
_core_ins_ldrdt:            ; LD into delay timer instruction.
    ld a, c
    and a, $0F
    ld e, a
    ld ix, iy
    add ix, de
    ld a, (ix)
    ld (_core_timer_delay), a
    inc hl
    inc hl
    jp _core_ins_end
_core_ins_ldrst:            ; LD into sound timer instruction.
    ld a, c
    and a, $0F
    ld e, a
    ld ix, iy
    add ix, de
    ld a, (ix)
    ld (_core_timer_sound), a
    inc hl
    inc hl
    jp _core_ins_end
_core_ins_addri:            ; ADD register to I instruction.
    ld a, c
    and a, $0F
    ld e, a
    ld d, 0
    ld ix, iy
    add ix, de
    ld a, (ix)
    ld ix, $000000
    ld ixl, a
    ld bc, $000000
    ld a, (_core_reg_i)
    ld c, a
    ld a, (_core_reg_i+1)
    ld b, a
    add ix, bc
    ld a, ixh
    and a, $0F
    ld (_core_reg_i+1), a
    ld a, ixl
    ld (_core_reg_i), a
    inc hl
    inc hl
    jp _core_ins_end
_core_ins_fnt:              ; LD font pointer VX into I instruction.
    ld a, c
    and a, $0F
    ld e, a
    ld d, 0
    ld ix, iy
    add ix, de
    ld a, (ix)
    and a, $0F
    ld e, a
    ld d, 5
    mlt de
    ld a, e
    ld (_core_reg_i), a
    xor a
    ld (_core_reg_i+1), a
    inc hl
    inc hl
    jp _core_ins_end
_core_ins_bcd:              ; LD BCD number into M[I+0..2] instruction.
    ld a, c
    and a, $0F
    ld e, a
    ld d, 0
    ld ix, iy
    add ix, de
    push hl
    ld a, (_core_reg_i)
    ld e, a
    ld a, (_core_reg_i+1)
    ld d, a
    ld hl, _core_memory
    add hl, de
    ld a, (ix)              ; http://wikiti.brandonw.net/index.php?title=Z80_Routines:Other:DispA     
    call _core_ins_bcd_dispa
    jp _core_ins_bcd_end
_core_ins_bcd_dispa:
	ld	c,-100
	call	_core_ins_bcd_dispa_na1
	ld	c,-10
	call	_core_ins_bcd_dispa_na1
	ld	c,-1
_core_ins_bcd_dispa_na1:	ld	b,0-1
_core_ins_bcd_dispa_na2:	inc	b
	add	a,c
	jr	c,_core_ins_bcd_dispa_na2
	sub	c		;works as add 100/10/1
	push af		;safer than ld c,a
    ld (hl), b
    inc de
    ld a, d
    and a, $0F
    ld d, a
    ld hl, _core_memory
    add hl, de
	pop af		;safer than ld a,c
	ret
_core_ins_bcd_end:
    pop hl
    inc hl
    inc hl
    jp _core_ins_end
_core_ins_sri:             ; Store registers at I instruction.
    ld a, c
    and a, $0F
    inc a
    ld bc, $000000
    ld b, a
    ld ix, iy
    ld a, (_core_reg_i)
    ld e, a
    ld a, (_core_reg_i+1)
    ld d, a
    push hl
_core_ins_sri_loop:
    ld hl, _core_memory
    add hl, de
    ld a, (ix)
    ld (hl), a
    inc ix
    inc de
    ld a, d
    and a, $0F
    ld d, a
    inc c
    ld a, c
    cp a, b
    jr nz, _core_ins_sri_loop
    pop hl
    inc hl
    inc hl
    jr _core_ins_end
_core_ins_lri:             ; Load registers at I instruction.
    ld a, c
    and a, $0F
    inc a
    ld bc, $000000
    ld b, a
    ld ix, iy
    ld a, (_core_reg_i)
    ld e, a
    ld a, (_core_reg_i+1)
    ld d, a
    push hl
_core_ins_lri_loop:
    ld hl, _core_memory
    add hl, de
    ld a, (hl)
    ld (ix), a
    inc ix
    inc de
    ld a, d
    and a, $0F
    ld d, a
    inc c
    ld a, c
    cp a, b
    jr nz, _core_ins_lri_loop
    pop hl
    inc hl
    inc hl
    jr _core_ins_end
_core_ins_invalid:
    push hl
    ld hl, _core_flags
    set 0, (hl)             ; set the "invalid" flag
    pop hl
    pop bc
    ld a, b
    or a, 0x80
    ld b, a
    push bc
    jp _core_ins_end
_core_ins_stkunder:
    push hl
    ld hl, _core_flags
    set 3, (hl)             ; set the "underflow" flag
    pop hl
    pop bc
    ld a, b
    or a, 0x80
    ld b, a
    push bc
    jp _core_ins_end
_core_ins_stkover:
    ld hl, iy               ; Recall that iy was pushed because we needed hl for an indirect write.
    pop iy                  ; we didn't forget about you.
    push hl
    ld hl, _core_flags
    set 2, (hl)             ; set the "overflow" flag
    pop hl
    pop bc
    ld a, b
    or a, 0x80
    ld b, a
    push bc
    jp _core_ins_end
_core_ins_wait:
    push hl
    ld hl, _core_flags
    set 4, (hl)             ; set the "wait" flag
    pop hl
    pop bc
    ld a, b
    or a, 0x80
    ld b, a
    push bc
    jp _core_ins_end
_core_ins_exitf:
    push hl
    ld hl, _core_flags
    set 5, (hl)             ; set the "exit" flag
    pop hl
    pop bc
    ld a, b
    or a, 0x80
    ld b, a
    push bc
    jp _core_ins_end
_core_ins_end:
    xor a
    ld bc, _core_memory
    or a                    ;   clear carry
    sbc hl, bc
    ld a, h
    and a, $0F              ; clip to 12 bits
    ld (_core_reg_pc+1), a
    ld h, a
    ld a, l
    ld (_core_reg_pc), a
    add hl, bc
    pop bc
    bit 7, b                ; check if the error / wait flag was set
    jr nz, _core_ins_veryend
    dec bc
    ld a, 0
    cp c
    jp nz, _core_instruction_load
    cp b
    jp nz, _core_instruction_load
_core_ins_veryend:
    ld hl, _core_timer_delay
    cp a, (hl)
    jr z, _core_ins_end_notimer
    dec (hl)
_core_ins_end_notimer:
    ld hl, _core_timer_sound
    cp a, (hl)
    jr z, _core_ins_end_nosound
    dec (hl)
_core_ins_end_nosound:
    pop iy                  ; save back iy and ix
    pop ix
    pop de
    ret
