#ifndef MSIARRAY_H
#define MSIARRAY_H
#define MUSIN

#include <stdio.h>
#include <stdbool.h>

// Intercambia los valores de sus entradas
MUSIN void iSwap(int *a, int *b);
// Ordena un array de 4 elementos de mayor a menor
MUSIN void iArraySort4(int arr[4]);
// Imprime el array
MUSIN void iArrayPrint(const int arr[], size_t len, bool newline);
// Cuenta desde startPos cuantos numeros consecutivos
// iguales hay. Tambien mueve starPos hasta el lugar correspondiente.
// Si startPos es NULL, empieza en la posicion 0 y no la mueve.
MUSIN size_t iArrayCountConsec(int *hayStack, size_t len, size_t *startPos);
// Devuelve la posicion de needle en el, devuelve len si no
// lo encuentra
MUSIN size_t iArrayFind(int *hayStack, size_t len, int needle);

// ============= CODIGO DE LAS FUNCIONES =============

void iSwap(int *a, int *b) {
    int t = *a; *a = *b; *b = t;
}

void iArraySort4(int arr[4]) {
    if(arr[0] < arr[1]) iSwap(arr + 0, arr + 1);
    if(arr[2] < arr[3]) iSwap(arr + 2, arr + 3);
    if(arr[0] < arr[2]) iSwap(arr + 0, arr + 2);
    if(arr[1] < arr[3]) iSwap(arr + 1, arr + 3);
    if(arr[1] < arr[2]) iSwap(arr + 1, arr + 2); 
}

void iArrayPrint(const int arr[], size_t len, bool newline) {
    for(size_t i = 0; i < len; ++i) printf("%d ", arr[i]);
    if(newline) putchar('\n');
}

size_t iArrayCountConsec(int *hayStack, size_t len, size_t *startPos) {
    size_t count = 1;
    size_t i = startPos ? *startPos : 0;
    for(; i < len - 1 && hayStack[i] == hayStack[i + 1]; ++i) ++count;
    if(startPos) *startPos = i + 1;
    return count;
}

size_t iArrayFind(int *hayStack, size_t len, int needle) {
    for(size_t i = 0; i < len; ++i) if(hayStack[i] == needle) return i;
    return len;
}

#endif // MSIARRAY_H