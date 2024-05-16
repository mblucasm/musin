#ifndef MSDEAL_H
#define MSDEAL_H
#define MUSIN

#include <stdlib.h>
#include "ms_defs.h"
#include "ms_hands.h"
#include "ex_random.h"

// Reparte las cartas a los jugadores como se debe hacer en el mus.
// Utiliza la libreria "ex_random.h" para la generacion de numeros
// aleatorios. Parece no dar problemas
MUSIN void msDeal(Hand hands[PLAYERS], struct xrandom *rnd);
// Reparte las cartas a los jugadores como se debe hacer en el mus.
// Utiliza la libreria <stdlib.h> para la generacion de numeros
// aleatorios (la funcion rand()). Da problemas
MUSIN void msDealRandFuncStdlib(Hand hands[PLAYERS]);
// Reparte las cartas a los jugadores como  NO se debe hacer en el mus.
// Reparte las cartas a los jugadores de uno en uno. 
// Utiliza la libreria <stdlib.h> para la generacion de numeros
// aleatorios (la funcion rand()). Da problemas pero menos (?)
MUSIN void msDealRandFuncStdlibWrongOrder(Hand hands[PLAYERS]);

// ============= CODIGO DE LAS FUNCIONES =============

void msDeal(Hand hands[PLAYERS], struct xrandom *rnd) {
    int deck[] = DECKNEW;
    int matrix[PLAYERS][HANDLEN];
    for(int j = 0; j < HANDLEN; ++j) {
        for(int i = 0; i < PLAYERS; ++i) {
            repeat: {
                int idx  = xrandom_u64(rnd) % DECKLEN;
                if(deck[idx] > 0) {
                    matrix[i][j] = idx;
                    --deck[idx];
                } else goto repeat;
            }
        }
    }
    for(int i = 0; i < PLAYERS; ++i) {
        iArraySort4(matrix[i]);
        hands[i] = msHandFromArray(matrix[i]);
    }
}

void msDealRandFuncStdlib(Hand hands[PLAYERS]) {
    int deck[] = DECKNEW;
    int matrix[PLAYERS][HANDLEN];
    for(int j = 0; j < HANDLEN; ++j) {
        for(int i = 0; i < PLAYERS; ++i) {
            repeat: {
                int idx = rand() % DECKLEN;
                if(deck[idx] > 0) {
                    matrix[i][j] = idx;
                    --deck[idx];
                } else goto repeat;
            }
        }
    }
    for(int i = 0; i < PLAYERS; ++i) {
        iArraySort4(matrix[i]);
        hands[i] = msHandFromArray(matrix[i]);
    }
}

void msDealRandFuncStdlibWrongOrder(Hand hands[PLAYERS]) {
    int deck[] = {8, 4, 4, 4, 4, 4, 4, 8};
    int matrix[PLAYERS][HANDLEN];
    for(int i = 0; i < PLAYERS; ++i) {
        for(int j = 0; j < HANDLEN; ++j) {
            repeat: {
                int idx = rand() % DECKLEN;
                if(deck[idx] > 0) {
                    matrix[i][j] = idx;
                    --deck[idx];
                } else goto repeat;
            }
        }
    }
    for(int i = 0; i < PLAYERS; ++i) {
        iArraySort4(matrix[i]);
        hands[i] = msHandFromArray(matrix[i]);
    }
}

#endif // MSDEAL_H