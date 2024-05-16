#ifndef DARRAY_H
#define DARRAY_H

#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#define DA_CHUNK_LEN 30

// da_append(da, elem) // append elem to dinamic array
// da_del(da) // delete full dinamic array
// da_last(da) // expands to a pointer to the last element of the dinamic array

// append elem to dinamic array
#define da_append(da, elem)                                                   \
    do {                                                                      \
        if((da).cap == 0) {                                                   \
            (da).cap = DA_CHUNK_LEN;                                          \
            (da).elems = malloc(sizeof(elem) * (da).cap);                     \
            if((da).elems == NULL) assert(false && "ERROR: not enough RAM");  \
        }                                                                     \
        if((da).cap == (da).len) {                                            \
            (da).cap += DA_CHUNK_LEN;                                         \
            (da).elems = realloc((da).elems, sizeof(elem) * (da).cap);        \
            if((da).elems == NULL) assert(false && "ERROR: not enough RAM");  \
        }                                                                     \
        (da).elems[(da).len++] = elem;                                        \
    } while(0)
//

// delete full dinamic array
#define da_del(da)        \
    do {                  \
        (da).cap = 0;     \
        (da).len = 0;     \
        free((da).elems); \
    } while (0)
//

// expands to a pointer to the last element of the dinamic array
#define da_last(da) ((da).elems + (da).len - 1)

#endif // DARRAY_H