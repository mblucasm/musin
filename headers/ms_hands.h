#ifndef MSHANDS_H
#define MSHANDS_H
#define MUSIN

#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include "ms_defs.h"
#include "ms_iarray.h"

typedef struct {
    int cards[HANDLEN];
    int times[HANDLEN];
    int len;
} Hand;

// Imprime la mano (forma numerica y normal)
MUSIN void msHandPrint(Hand hand, bool newline);
// Devuelve una mano dada una mano en forma de array
MUSIN Hand msHandFromArray(int handArray[HANDLEN]);
// Devuelve el score de grande de la mano
MUSIN int msHandGetScoreGrande(Hand hand);
// Devuelve el score de chica de la mano
MUSIN int msHandGetScoreChica(Hand hand);
// Devuelve el score de pares de la mano
MUSIN int msHandGetScorePares(Hand hand);
// Devuelve el score de juego de la mano
MUSIN int msHandGetScoreJuego(Hand hand, bool laReal);
// Dada una mano en forma de array la cambia a la siguiente
MUSIN void msHandArrayNext(int handArray[HANDLEN]);
// Rellena scores[LANCES] con los scores de cada lance de la mano
// teniendo en cuenta su posicion en la mesa (mano, semimano, semipostre, postre)
MUSIN void msHandGetScoresPositional(Hand hand, Position pos, bool laReal, float scores[LANCES]);
// Devuelve el numero de amarracos que te llevarias de m(a)s por tener ese score en pares
MUSIN int msParesScoreToAmarr(int score);
// Devuelve el numero de amarracos que te llevarias de m(a)s por tener ese score en juego
MUSIN int msJuegoScoreToAmarr(int score);

// ============= CODIGO DE LAS FUNCIONES =============

void msHandPrint(Hand hand, bool newline) {
    for(int i = 0; i < hand.len; ++i) {
        for(int j = 0; j < hand.times[i]; ++j) {
            printf("%d ", hand.cards[i]);
        }
    } putchar(' ');
    for(int i = 0; i < hand.len; ++i) {
        for(int j = 0; j < hand.times[i]; ++j) {
            printf("%c", CARDSET[hand.cards[i]]);
        }
    } if(newline) putchar('\n');
}

Hand msHandFromArray(int handArray[HANDLEN]) {
    Hand hand = {0};
    size_t startPos = 0;
    while(startPos < HANDLEN) {
        hand.cards[hand.len]   = handArray[startPos];
        hand.times[hand.len++] = iArrayCountConsec(handArray, HANDLEN, &startPos);
    } return hand;
}

int msHandGetScoreGrande(Hand hand) {
    int score = 0;
    for(int i = 0; i < hand.len; ++i) {
        for(int j = 0; j < hand.times[i]; ++j) {
            score *= 10;
            score += hand.cards[i];
        }
    } return score;
}

int msHandGetScoreChica(Hand hand) {
    int score = 0;
    for(int i = hand.len-1; i >= 0; --i) {
        for(int j = 0; j < hand.times[i]; ++j) {
            score *= 10;
            score += hand.cards[i];
        }
    } return score;
}

int msHandGetScorePares(Hand hand) {
    int score = 0, count = 0;
    for(int i = 0; i < hand.len; ++i) {
        int times = hand.times[i];
        int card  = hand.cards[i];
        if     (times == 2) {score = 10  * score + (card + 1); ++count;}
        else if(times == 3)  score = 11  * (card + 1);
        else if(times == 4)  score = 110 * (card + 1);
    } return (count == 2) ? 10 * score : score;
}

int msHandGetScoreJuego(Hand hand, bool laReal) {
    int score = 0;
    for(int i = 0; i < hand.len; ++i) score += hand.times[i] * VALUESET[hand.cards[i]];
    if(score == 32) score += 9;
    else if(score == 31) {
        if(laReal) if(hand.cards[1] == SEVEN && hand.times[1] == 3) ++score;
        score += 11;
    } return score;
}

void msHandArrayNext(int handArray[HANDLEN]) {
    size_t minIdx = iArrayFind(handArray, HANDLEN, handArray[HANDLEN - 1]);
    assert(minIdx != HANDLEN && "ERROR: min not found");
    ++handArray[minIdx];
    for(size_t i = minIdx + 1; i < HANDLEN; ++i) handArray[i] = 0;
}

void msHandGetScoresPositional(Hand hand, Position pos, bool laReal, float scores[LANCES]) {
    scores[GRANDE] = (float)msHandGetScoreGrande(hand)        + OFFSETSET[pos];
    scores[CHICA]  = (float)msHandGetScoreChica(hand)         - OFFSETSET[pos];
    scores[PARES]  = (float)msHandGetScorePares(hand)         + OFFSETSET[pos];
    scores[JUEGO]  = (float)msHandGetScoreJuego(hand, laReal) + OFFSETSET[pos];
}

int msParesScoreToAmarr(int score) {
    if(score >= DUPLEXMIN) return 3;
    if(score >= TRIOMIN)   return 2;
    if(score >= PAIRMIN)   return 1;
    return 0; 
}

// Como acepta el score en (int) trunca el numero, si
// este era 30.X queda en 30. Si es > 30, tiene juego.
int msJuegoScoreToAmarr(int score) {
    if(score >= THIRTYONE) return 3;
    if(score > THIRTY) return 2;
    return 1;
}

#endif // MSHANDS_H