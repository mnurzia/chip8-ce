    .assume adl=1

; ui.h
    .def _ui_sprite
    .def _ui_sprites
    
; colors.h
    .ref _color_ui_fg_1

    .ref _dbg_Debugger
    
_ui_sprite:
    push bc
    push de
    push ix
    push hl
    ld ix, 0
    add ix, sp
    ld bc, 0
    ld b, (ix+18)
    ld h, b
    ld l, 64
    mlt hl
    ccf
    add hl, bc
    ld bc, 0
    ld c, (ix+15)
    add hl, bc
    ld bc, ($E30014)
    add hl, bc
    ld de, (ix+21)
    ld c, 8
    ld a, (_color_ui_fg_1)
    ld (_ui_sprite_marker), a
_ui_sprite_yloop:
    ld b, 8
    ld a, (de)
_ui_sprite_xloop:
    srl a
    jr nc, _ui_sprite_xloop_nowrite
    .db $36
_ui_sprite_marker:
    .db $0F
_ui_sprite_xloop_nowrite:
    inc hl
    dec b
    jr nz, _ui_sprite_xloop
    inc de
    push bc
    ld bc, 312
    add hl, bc
    pop bc
    dec c
    jr nz, _ui_sprite_yloop
    pop hl
    pop ix
    pop de
    pop bc
    ret

_ui_sprites:
_ui_sprite_0_author:
    .db $18, $3C, $3C, $18, $00, $7E, $FF, $FF

_ui_sprite_1_date:
    .db $42, $FF, $B1, $B7, $B1, $B7, $B1, $FF

_ui_sprite_2_size:
    .db $3F, $6F, $EF, $FF, $81, $FF, $81, $FF

_ui_sprite_3_speed:
    .db $10, $30, $70, $FF, $FF, $70, $30, $10

_ui_sprite_4_pack:
    .db $FF, $81, $FF, $81, $FF, $81, $FF, $FF