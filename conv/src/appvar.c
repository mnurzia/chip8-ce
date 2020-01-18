#include "appvar.h"

#include <string.h>

#define APPVAR_HEADER_LENGTH 0x37
#define APPVAR_DATAHEADER_LENGTH 0x11

static const char appvar_magic[11] = {
    0x2A, 0x2A, 0x54, 0x49,
    0x38, 0x33, 0x46, 0x2A,
    0x1A, 0x0A, 0x00
};

void appvar_init(appvar_t* var) {
    memset(&(var->name), 0, 8);
    memset(&(var->comment), 0, 42);
    var->archived = 0;
    var->data_length = 0;
    var->data = NULL;
}

void appvar_destroy(appvar_t* var) {
    if (var->data) {
        free(var->data);
    }
}

void appvar_name(appvar_t* var, char name[8]) {
    strncpy(var->name, name, 8); 
}

void appvar_comment(appvar_t* var, char comment[42]) {
    strncpy(var->comment, comment, 42); 
}

void appvar_archive(appvar_t* var, int archived) {
    var->archived = archived;
}

int appvar_can_append(appvar_t* var, size_t data_length) {
    if ((var->data_length + data_length) >= (65536 - APPVAR_DATAHEADER_LENGTH - 5)) {
        return 0;
    }
    return 1;
}

int appvar_append(appvar_t* var, char* data, size_t data_length) {
    if (!appvar_can_append(var, data_length)) {
        return 1;
    }        
    if (var->data) {
        var->data = (char*)realloc(var->data, var->data_length + data_length);
    } else {
        var->data = (char*)malloc(data_length);
    }
    if (!var->data) {
        perror("Unable to allocate appvar data buffer");
        exit(1);
    }
    memcpy(&(var->data[var->data_length]), data, data_length);
    var->data_length += data_length;
    return 0;
}

int appvar_append_byte(appvar_t* var, uint8_t byte) {
    return appvar_append(var, (char*)&byte, 1);
}

size_t appvar_size(appvar_t* var) {
    return APPVAR_HEADER_LENGTH + APPVAR_DATAHEADER_LENGTH + var->data_length + 2 + 2;
}

void appvar_write_string(appvar_t* var, char* out) {
    size_t ptr = 0;
    size_t i = 0;
    size_t data_section_len = APPVAR_DATAHEADER_LENGTH + var->data_length + 2;
    uint16_t checksum = 0;
    size_t data_section_start = 0;
    for (i = 0; i < 11; i++) {
        out[ptr] = appvar_magic[i]; ptr++;
    }
    for (i = 0; i < 42; i++) {
        out[ptr] = var->comment[i]; ptr++;
    }
    out[ptr] = ((data_section_len >> 0) & 0xFF); ptr++;
    out[ptr] = ((data_section_len >> 8) & 0xFF); ptr++;
    data_section_start = ptr;
    out[ptr] = 0x0D; ptr++;
    out[ptr] = 0x00; ptr++;
    out[ptr] = (((var->data_length + 2) >> 0) & 0xFF); ptr++;
    out[ptr] = (((var->data_length + 2) >> 8) & 0xFF); ptr++;
    out[ptr] = 0x15; ptr++;
    for (i = 0; i < 8; i++) {
        out[ptr] = var->name[i]; ptr++;
    }
    out[ptr] = 0x00; ptr++;
    if (var->archived) {
        ((unsigned char*)out)[ptr] = 0x80; ptr++;
    } else {
        out[ptr] = 0x00; ptr++;
    }
    out[ptr] = (((var->data_length + 2) >> 0) & 0xFF); ptr++;
    out[ptr] = (((var->data_length + 2) >> 8) & 0xFF); ptr++;
    out[ptr] = ((var->data_length >> 0) & 0xFF); ptr++;
    out[ptr] = ((var->data_length >> 8) & 0xFF); ptr++;
    for (i = 0; i < var->data_length; i++) {
        out[ptr] = var->data[i]; ptr++;
    }
    for (i = data_section_start; i < ptr; i++) {
        checksum = (checksum + ((unsigned char*)out)[i]) & 0xFFFF;
    }
    out[ptr] = ((checksum >> 0) & 0xFF); ptr++;
    out[ptr] = ((checksum >> 8) & 0xFF); ptr++;
}

void appvar_write_file(appvar_t* var, FILE* out) {
    size_t sz = appvar_size(var);
    char* temp = malloc(sizeof(char) * sz);
    if (!temp) {
        perror("Unable to allocate file temporary buffer");
        exit(1);
    }
    appvar_write_string(var, temp);
    fwrite(temp, sizeof(char), sz, out);
    free(temp);
}