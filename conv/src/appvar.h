#ifndef APPVAR_H
#define APPVAR_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct appvar {
    char   name[8];
    char   comment[42];
    int    archived;
    size_t data_length;
    char*  data;
} appvar_t;

void appvar_init(appvar_t* var);

void appvar_destroy(appvar_t* var);

void appvar_name(appvar_t* var, char* name);

void appvar_comment(appvar_t* var, char* comment);

void appvar_archive(appvar_t* var, int archived);

int appvar_can_append(appvar_t* var, size_t data_length);

int appvar_append(appvar_t* var, char* data, size_t data_length);

int appvar_append_byte(appvar_t* var, uint8_t byte);

size_t appvar_size(appvar_t* var);

void appvar_write_string(appvar_t* var, char* out);

void appvar_write_file(appvar_t* var, FILE* out);

#endif