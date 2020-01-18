#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VERSION "0.1.0"

#include "appvar.h"
#include "game.h"
#include "gamepack.h"

char* progname = NULL;

void error(char* msg) {
    printf("%s: %s\n", progname, msg);
    exit(1);
}

int main(int argc, char** argv) {
    appvar_t var;
    gamepack_t gamepack;
    FILE* output_file;
    int optind = 1;
    game_t *current_game = NULL;
    char output_name[] = "CH8Pack00.8xv";
    int i = 0;
    progname = argv[0];
    
    appvar_init(&var);
    gamepack_init(&gamepack);
    
    appvar_name(&var, "CH8Pak00");
    appvar_comment(&var, "CHIP-8 ROM Converter " VERSION " by mnurzia");
    
    if (argc == 0) {
        exit(1);
    }
    
    if (optind == argc) {
        error("expected one or more Chip-8 ROM files");
    }
    
    while (optind < argc) {
        char* optarg = argv[optind];
        char option = '\0';
        
        if (optarg[0] == '-') {
            option = optarg[1];
            if (optarg[1] == '-') {
                char* longopt = &(optarg[2]);
                if (strcmp(longopt, "name") == 0) {
                    option = 'n';
                } else if (strcmp(longopt, "author") == 0) {
                    option = 'a';
                } else if (strcmp(longopt, "year") == 0) {
                    option = 'y';
                } else if (strcmp(longopt, "output") == 0) {
                    option = 'o';
                } else if (strcmp(longopt, "key") == 0) {
                    option = 'k';
                } else if (strcmp(longopt, "fg") == 0) {
                    option = 'f';
                } else if (strcmp(longopt, "bg") == 0) {
                    option = 'b';
                } else if (strcmp(longopt, "archive") == 0) {
                    option = 'z';
                } else if (strcmp(longopt, "speed") == 0) {
                    option = 's';
                } else {
                    char buf[128];
                    sprintf(buf, "\"--%s\" - invalid argument", longopt);
                    error(buf);
                }
            }
        }
        
        if (option) {
            switch (option) {
                case 'n': {
                    optind++;
                    if (optind == argc) {
                        error("-n, --name: option expects an argument");
                    }
                    if (!current_game) {
                        error("-n, --name: no game specified yet");
                    }
                    game_name(current_game, argv[optind]);
                    break;
                }
                case 'a': {
                    optind++;
                    if (optind == argc) {
                        error("-a, --author: option expects an argument");
                    }
                    if (!current_game) {
                        error("-a, --author: no game specified yet");
                    }
                    game_author(current_game, argv[optind]);
                    break;
                }
                case 'y': {
                    uint16_t year = 0;
                    optind++;
                    if (optind == argc) {
                        error("-y, --year: option expects an argument");
                    }
                    if (!current_game) {
                        error("-y, --year: no game specified yet");
                    }
                    if (sscanf(argv[optind], "%" SCNu16, &year) != 1) {
                        error("-y, --year: not a valid number");
                    }
                    game_year(current_game, year);
                    break;
                }
                case 'o': {
                    char buf[9] = { 0 };
                    optind++;
                    if (optind == argc) {
                        error("-o, --output: option expects an argument");
                    }
                    strncpy(buf, argv[optind], 8);
                    buf[8] = '\0';
                    sprintf(output_name, "%s.8xv", buf);
                    appvar_name(&var, buf);
                    break;
                }
                case 'k': {
                    uint32_t key = 0;
                    optind++;
                    if (optind == argc) {
                        error("-k, --key: option expects two arguments");
                    }
                    if (!current_game) {
                        error("-k, --key: no game specified yet");
                    }
                    if (sscanf(argv[optind], "%x", &key) != 1) {
                        error("-k, --key: not a valid number");
                    }
                    if (key > 15) {
                        error("-k, --key: expected key in range 0-F");
                    }
                    optind++;
                    if (optind == argc) {
                        error("-k, --key: option expects two arguments");
                    }
                    if (game_mapkey(current_game, key, argv[optind])) {
                        char buf[128];
                        sprintf(buf, "-k, --key: \"%s\" - invalid key name", argv[optind]);
                        error(buf);
                    }
                    break;
                }
                case 'f': {
                    uint16_t fg = 0;
                    optind++;
                    if (optind == argc) {
                        error("-f, --fg: option expects an argument");
                    }
                    if (!current_game) {
                        error("-f, --fg: no game specified yet");
                    }
                    if (sscanf(argv[optind], "%" SCNu16, &fg) != 1) {
                        error("-f, --fg: not a valid number");
                    }
                    if (fg > 255) {
                        error("-f, --fg: expected color in range 0-255");
                    }
                    game_color_fg(current_game, (uint8_t)fg);
                    break;
                }
                case 'b': {
                    uint16_t bg = 0;
                    optind++;
                    if (optind == argc) {
                        error("-b, --bg: option expects an argument");
                    }
                    if (!current_game) {
                        error("-b, --bg: no game specified yet");
                    }
                    if (sscanf(argv[optind], "%" SCNu16, &bg) != 1) {
                        error("-b, --bg: not a valid number");
                    }
                    if (bg > 255) {
                        error("-b, --bg: expected color in range 0-255");
                    }
                    game_color_bg(current_game, (uint8_t)bg);
                    break;
                }
                case 's': {
                    uint16_t speed = 0;
                    optind++;
                    if (optind == argc) {
                        error("-s, --speed: option expects an argument");
                    }
                    if (!current_game) {
                        error("-s, --speed: no game specified yet");
                    }
                    if (sscanf(argv[optind], "%" SCNu16, &speed) != 1) {
                        error("-s, --speed: not a valid number");
                    }
                    game_speed(current_game, speed);
                    break;
                }
                default: {
                    char buf[128];
                    sprintf(buf, "\"-%c\" - invalid argument", option);
                    error(buf);
                    break;
                }
            }
        } else {
            FILE* current_desc;
            char* current_contents = malloc(sizeof(char) * 32);
            size_t written_total = 0;
            if (!current_contents) {
                perror("Unable to allocate file contents");
                exit(1);
            }
            current_game = gamepack_new_game(&gamepack);
            game_name(current_game, optarg);
            current_desc = fopen(optarg, "rb");
            if (!current_desc) {
                char buf[128];
                sprintf(buf, "\"%s\" - unable to open file", optarg);
                error(buf);
            }
            while (!feof(current_desc)) {
                size_t written = fread(&(current_contents[written_total]), 1, 32, current_desc);
                written_total += written;
                current_contents = realloc(current_contents, sizeof(char) * (written_total + 32));
                if (!current_contents) {
                    perror("Unable to reallocate file contents");
                    exit(1);
                }
            }
            if (game_contents(current_game, written_total, current_contents)) {
                char buf[128];
                sprintf(buf, "\"%s\" - game too large", optarg);
                error(buf);
            }
        }
        
        optind++;
    }
    
    if (gamepack.n_games == 0) {
        error("no games specified");
    }
    
    if ((i = gamepack_write(&gamepack, &var))) {
        char buf[128];
        sprintf(buf, "Error writing game %i", i);
        error(buf);
    }
    
    output_file = fopen(output_name, "wb");
    if (!output_file) {
        char buf[128];
        sprintf(buf, "\"%s\" - unable to open file for writing", output_name);
        error(buf);
    }
    appvar_write_file(&var, output_file);
    fclose(output_file);
    
    gamepack_destroy(&gamepack);
    appvar_destroy(&var);
    
    return 0;
}