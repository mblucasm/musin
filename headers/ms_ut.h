// Este .h contiente unicamente
// algunas funciones de utils.h

#ifndef UT_H
#define UT_H

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#define UTIL_ASSERT_FREAD_LEN

typedef enum {
    UTIL_ERROR_NO_ERROR,
    UTIL_ERROR_OPENING_FILE,
    UTIL_ERROR_MALLOC,
    UTIL_ERROR_FSEEK,
    UTIL_ERROR_FWRITE,
    UTIL_ERROR_FILE_WAS_NULL,
    UTIL_ERROR_FREAD,
} UtilErrorType;

// All functions that accept UtilError* as a parameter consider passing 'NULL' as an argument -> ignore error handling
typedef struct {
    UtilErrorType type;
    const char *info[2];
} UtilError;

typedef enum {
    UTIL_COLOR_DEFAULT,
    UTIL_COLOR_RED,
    UTIL_COLOR_GREEN,
    UTIL_COLOR_YELLOW,
    UTIL_COLOR_CYAN,
} UtilColor;

typedef struct {
    const char *ptr;
    size_t len;
} UtilSlice;

// Calls malloc, is assumed caller calls free
#define MEM
// Errors of functions that have this infront of their declaration DO return error->info
#define REI

void utilErrorSet(UtilError *error, UtilErrorType type, const char *filePath, const char *msg);
void utilErrorSetType(UtilError *error, UtilErrorType type);

char *utilArgShift(int *argc, char ***argv);
void utilFgetsCorrect(char *buffer);
void utilConsoleColorSet(UtilColor color);

MEM REI char *utilFileRead   (const char *file_path, UtilError *error);   // OPENING_FILE, FSEEK, MALLOC, FREAD | #ifdef UTIL_ASSERT_FREAD_LEN

#define USNULL ((UtilSlice){0})
#define USFMT "%.*s"
#define USARG(slice) (int)slice.len, slice.ptr

UtilSlice utilSliceNew        (const char *buffer, size_t len);
UtilSlice utilSliceNewLiteral (const char *buffer);
UtilSlice utilSliceTrimRight  (UtilSlice slice);
UtilSlice utilSliceTrimLeft   (UtilSlice slice);
UtilSlice utilSliceTrim       (UtilSlice slice);
UtilSlice utilSliceSlice      (UtilSlice *slice, const char delim, bool skipDelim);
bool utilSliceCompar          (UtilSlice slice1, UtilSlice slice2);

// Implementation ================================================

#define utilErrorNone ((UtilError){0})
#define __utilErrorSetGoto(error, type, label) do {utilErrorSet(error, type, filePath, strerror(errno)); goto label;} while(0)
#define __utilErrorSetTypeGoto(error, type, label) do {utilErrorSetType(error, type); goto label;} while(0)

void utilFgetsCorrect(char *buffer) {
    size_t len = strlen(buffer);
    if(len >= 1) if(buffer[len - 1] == '\n') buffer[len - 1] = '\0';
    if(len >= 2) if(buffer[len - 2] == '\r') buffer[len - 2] = '\0';
}

void utilConsoleColorSet(UtilColor color) {
    switch(color) {
        case UTIL_COLOR_DEFAULT:
        printf("\033[0m");
        break;
        case UTIL_COLOR_RED:
        printf("\033[0;31m");
        break;
        case UTIL_COLOR_GREEN:
        printf("\033[0;32m");
        break;
        case UTIL_COLOR_YELLOW:
        printf("\033[0;33m");
        break;
        case UTIL_COLOR_CYAN:
        printf("\033[0;36m");
        break;
        default:
        fprintf(stderr, "\nCOLOR NOT DEFINED UTILS.H\n");
        exit(1);
        break;
    }
}

void utilErrorSet(UtilError *error, UtilErrorType type, const char *filePath, const char *msg) {
    if(error) {
        error->type = type;
        error->info[0] = filePath;
        error->info[1] = msg;
    }
}

void utilErrorSetType(UtilError *error, UtilErrorType type) {
    if(error) error->type = type;
}

char *utilFileRead(const char *filePath, UtilError *error) {
    if(error) *error = utilErrorNone;
    FILE *f = fopen(filePath, "rb");
    char *buffer = NULL;
    if(f) {
        if(fseek(f, 0, SEEK_END) == 0) {
            size_t len = ftell(f);
            rewind(f);
            buffer = malloc(sizeof(char) * (len + 1));
            if(buffer == NULL) __utilErrorSetTypeGoto(error, UTIL_ERROR_MALLOC, defer1);
            #ifdef UTIL_ASSERT_FREAD_LEN
                size_t readed_len;
                if((readed_len = fread(buffer, sizeof(char), len, f)) == len) {
                    buffer[len] = '\0';
                } else __utilErrorSetGoto(error, UTIL_ERROR_FREAD, defer2);
            #else
                len = fread(buffer, sizeof(char), len, f);
                buffer[len] = '\0';
            #endif
        } else __utilErrorSetGoto(error, UTIL_ERROR_FSEEK, defer1);
        fclose(f);
    } else utilErrorSet(error, UTIL_ERROR_OPENING_FILE, filePath, strerror(errno));
    return buffer;
    defer2:
        free(buffer);
        buffer = NULL;
    defer1:
        fclose(f);
        return buffer;
}

UtilSlice utilSliceNewLiteral(const char *buffer) {
    return (UtilSlice) {
        .ptr = buffer,
        .len = strlen(buffer),
    };
}

UtilSlice utilSliceNew(const char *buffer, size_t len) {
    return (UtilSlice) {
        .ptr = buffer,
        .len = len,
    };
}

UtilSlice utilSliceTrim(UtilSlice slice) {
    return utilSliceTrimLeft(utilSliceTrimRight(slice));
}

UtilSlice utilSliceTrimLeft(UtilSlice slice) {
    size_t i = 0;
    while(i < slice.len && isspace(slice.ptr[i])) ++i;
    return utilSliceNew(slice.ptr + i, slice.len - i);
}

UtilSlice utilSliceTrimRight(UtilSlice slice) {
    size_t i = 0;
    while(i < slice.len && isspace(slice.ptr[slice.len - i - 1])) ++i;
    return utilSliceNew(slice.ptr, slice.len - i);
}

UtilSlice utilSliceSlice(UtilSlice *slice, const char delim, bool skipDelim) {
    size_t i = 0;
    while (i < slice->len && slice->ptr[i] != delim) ++i;
    UtilSlice rem = utilSliceNew(slice->ptr, i);
    slice->ptr += i;
    slice->len -= i;
    if(skipDelim && slice->len != 0) {
        ++slice->ptr;
        --slice->len;
    }
    return rem;
}

char *utilArgShift(int *argc, char ***argv) {

    char *arg = NULL;
    if(*argc > 0) {
        arg = **argv;
        --(*argc); 
        ++(*argv);
    }
    return arg;
}

bool utilSliceCompar(UtilSlice slice1, UtilSlice slice2) {
    if(slice1.len != slice2.len) return false;
    for(size_t i = 0; i < slice1.len; ++i)
        if(slice1.ptr[i] != slice2.ptr[i])
            return false;
    //
    return true;
}

#endif // UT_H