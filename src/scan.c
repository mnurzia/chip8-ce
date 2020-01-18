#include "scan.h"

#include <stdint.h>
#include <string.h>

#include <debug.h>
#include <fileioc.h>

#include "game.h"

uint8_t *scan_search_pos = NULL;

struct game *scan_games = NULL;
uint16_t scan_games_count = 0;
uint16_t scan_games_alloc = 0;

void scan_reset() {
    scan_search_pos = NULL;
    if (scan_games) {
        free(scan_games);
    }
    scan_games_count = 0;
    scan_games_alloc = 0;
}

int scan_step() {
    char* var_name;
    char rdbuf[32];
    uint8_t ngames = 0;
    uint8_t i = 0;
    ti_var_t var = 0;
    
    var_name = ti_Detect(&scan_search_pos, "CH8\0");
    if (var_name == NULL) {
        goto done_scanning; 
    }
    
    var = ti_Open(var_name, "r");
    if (!var) { 
        goto os_error; 
    }
    
    if (ti_Read(rdbuf, 1, 6, var) != 6) { 
        goto early_eof; 
    }
    
    if (rdbuf[0] != 'C' || rdbuf[1] != 'H' || rdbuf[2] != '8' || rdbuf[3] != '\0') {
        goto syntax_error;
    }
    if (rdbuf[4] != 0x01) { 
        goto bad_version; 
    }
    
    ngames = rdbuf[5];
    if (ngames == 0) { 
        goto syntax_error; 
    }
    
    for (i = 0; i < ngames; i++) {
        struct game* current_game = 0;
        if (!scan_games) {
            scan_games_alloc = 4;
            scan_games = (struct game*)malloc(sizeof(struct game) * scan_games_alloc);
            if (!scan_games) {
                exit(1);
            }
        } else if (scan_games_alloc == scan_games_count) {
            scan_games_alloc += 4;
            scan_games = (struct game*)realloc(scan_games, sizeof(struct game) * scan_games_alloc);
            if (!scan_games) {
                exit(1);
            }
        }
        current_game = scan_games + scan_games_count;
        game_reset(current_game);
        strncpy(current_game->var, var_name, 8);
        if (ti_Read(current_game->name, 1, 24, var) != 24) { 
            goto early_eof;
        }
        if (ti_Read(current_game->auth, 1, 24, var) != 24) { 
            goto early_eof; 
        }
        if (ti_Read(&(current_game->year), 1, 2, var) != 2) { 
            goto early_eof; 
        }
        if (ti_Read(current_game->mapping, 1, 16, var) != 16) {
            goto early_eof;
        }
        if (ti_Read(&(current_game->fg), 1, 1, var) != 1) {
            goto early_eof;
        }
        if (ti_Read(&(current_game->bg), 1, 1, var) != 1) {
            goto early_eof;
        }
        if (ti_Read(&(current_game->speed), 1, 2, var) != 2) { 
            goto early_eof; 
        }
        if (ti_Read(&(current_game->size), 1, 2, var) != 2) { 
            goto early_eof; 
        }
        current_game->var_off = ti_Tell(var);
        if (ti_Seek(current_game->size, SEEK_CUR, var) == EOF) {
            goto early_eof;
        }
        scan_games_count++;
    }
    
    if (!ti_Close(var)) {
        var = 0;
        goto os_error;
    }
    return 0;
done_scanning:
    return 1;
os_error:
    if (var) {
        ti_Close(var);
    }
    return 2;
syntax_error:
    if (var) {
        ti_Close(var);
    }
    return 3;
early_eof:
    if (var) {
        ti_Close(var);
    }
    return 4;
bad_version:
    if (var) {
        ti_Close(var);
    }
    return 5;
}