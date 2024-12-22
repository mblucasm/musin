#ifndef MSTABLES_H
#define MSTABLES_H
#define MUSIN

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "ms_defs.h" 
#include "ms_deal.h"
#include "ms_hands.h"
#include "ms_hashbad.h"
#include "ex_random.h"

typedef A4st* TableOccurrences;
typedef A4V3stMusin** TableCategories;
typedef A4V3dMusin** PtableCategories;
typedef V2dMusin** PtableSums;
typedef A4V4dCiMusin CiTable[HANDSNUM][LANCES]; // Pendiente de cambio (heap)

typedef struct {
    TableOccurrences occurrences;
    TableCategories categories;
} Table;

typedef struct {
    PtableCategories categories;
    PtableSums expectedSums;
} Ptable;

MUSIN Table msTableGenerate(size_t iters, bool laReal, struct xrandom *rnd);
MUSIN void msTablePrint(Table table);
MUSIN void msTableDelete(Table table);
MUSIN void __msTableFill(Table table, size_t iters, bool laReal, struct xrandom *rnd);
MUSIN void __msTableHandleGrandeChica(TableCategories tCategories, int tablePos[PLAYERS], int bestCouples[LANCES], Lance lance);
MUSIN void __msTableHandlePares(TableCategories tCategories, float handsScores[PLAYERS][LANCES], int tablePos[PLAYERS], int bestCouples[LANCES]);
MUSIN void __msTableHandleJuego(TableCategories tCategories, float handsScores[PLAYERS][LANCES], int tablePos[PLAYERS], int bestCouples[LANCES]);
MUSIN Ptable msPtableGenerate(Table table);
MUSIN void msPtablePrint(Ptable pTable);
MUSIN void msPtableDelete(Ptable pTable);
MUSIN void __msPtableFill(Ptable pTable, Table table);

// Pendientes de cambios
MUSIN void msCiTableFillT(CiTable ciTable, Ptable pTable);
MUSIN void msCiTablePrint(CiTable ciTable);
MUSIN void msCiTablePrintEP(CiTable ciTable);

// ============= CODIGO DE LAS FUNCIONES =============

Table msTableGenerate(size_t iters, bool laReal, struct xrandom *rnd) {
    Table table;
    table.occurrences = malloc(sizeof(A4st) * HANDSNUM); assert(table.occurrences && "ERROR: not enough RAM");
    memset(table.occurrences, 0, sizeof(A4st) * HANDSNUM);
    table.categories = malloc(sizeof(A4V3stMusin*) * HANDSNUM); assert(table.categories && "ERROR: not enough RAM");
    for(size_t i = 0; i < HANDSNUM; ++i) {
        table.categories[i] = malloc(sizeof(A4V3stMusin) * LANCES); assert(table.categories[i] && "ERROR: not enough RAM");
        memset(table.categories[i], 0, sizeof(A4V3stMusin) * LANCES);
    } __msTableFill(table, iters, laReal, rnd);
    return table;
}

void msTableDelete(Table table) {
    for(size_t i = 0; i < HANDSNUM; ++i) free(table.categories[i]);
    free(table.categories); free(table.occurrences);
}

void __msTableFill(Table table, size_t iters, bool laReal, struct xrandom *rnd) {
    Hand hands[PLAYERS];
    for(size_t it = 0; it < iters; ++it) {
        float handsScores[PLAYERS][LANCES];
        msDeal(hands, rnd);
        msHandGetScoresPositional(hands[MANO], MANO, laReal, handsScores[MANO]);
        float bestScores[PLAYERS] = {
            [GRANDE] = handsScores[MANO][GRANDE],
            [CHICA]  = handsScores[MANO][CHICA],
            [PARES]  = handsScores[MANO][PARES],
            [JUEGO]  = handsScores[MANO][JUEGO],
        };
        int tablePos[PLAYERS] = {
            [MANO] = msHashBad(handsScores[MANO][GRANDE]),
        };
        int bestIndices[PLAYERS] = {
            [GRANDE] = MANO, 
            [CHICA]  = MANO,
            [PARES]  = MANO,
            [JUEGO]  = MANO,
        };
        for(size_t i = 1; i < PLAYERS; ++i) {
            msHandGetScoresPositional(hands[i], i, laReal, handsScores[i]);
            if(bestScores[GRANDE] < handsScores[i][GRANDE]) {bestScores[GRANDE] = handsScores[i][GRANDE]; bestIndices[GRANDE] = i;}
            if(bestScores[CHICA]  > handsScores[i][CHICA])  {bestScores[CHICA]  = handsScores[i][CHICA];  bestIndices[CHICA]  = i;}
            if(bestScores[PARES]  < handsScores[i][PARES])  {bestScores[PARES]  = handsScores[i][PARES];  bestIndices[PARES]  = i;}
            if(bestScores[JUEGO]  < handsScores[i][JUEGO])  {bestScores[JUEGO]  = handsScores[i][JUEGO];  bestIndices[JUEGO]  = i;}
            tablePos[i] = msHashBad(handsScores[i][GRANDE]);
        }
        int bestCouples[LANCES] =  {
            [GRANDE] = (bestIndices[GRANDE] == P1 || bestIndices[GRANDE] == P3) ? COUPLE1 : COUPLE2,
            [CHICA] =  (bestIndices[CHICA]  == P1 || bestIndices[CHICA]  == P3) ? COUPLE1 : COUPLE2,
            [PARES] =  (bestIndices[PARES]  == P1 || bestIndices[PARES]  == P3) ? COUPLE1 : COUPLE2,
            [JUEGO] =  (bestIndices[JUEGO]  == P1 || bestIndices[JUEGO]  == P3) ? COUPLE1 : COUPLE2,
        };
        for(size_t i = 0; i < PLAYERS; ++i) {
            switch(i) {
                case GRANDE: case CHICA:
                    __msTableHandleGrandeChica(table.categories, tablePos, bestCouples, i);
                break;
                case PARES:
                    if(bestScores[PARES] >= 1) __msTableHandlePares(table.categories, handsScores, tablePos, bestCouples);
                break;
                case JUEGO:
                    __msTableHandleJuego(table.categories, handsScores, tablePos, bestCouples);
                break;
            } ++table.occurrences[tablePos[i]].sub[i];
        }
    }
}

void __msTableHandleGrandeChica(TableCategories tCategories, int tablePos[PLAYERS], int bestCouples[LANCES], Lance lance) {
    Player w[2], l[2];
    if(bestCouples[lance] == COUPLE1) {
        w[0] = P1; l[0] = P2;
    } else {
        w[0] = P2; l[0] = P1;
    } w[1] = w[0] + 2; l[1] = l[0] + 2;
    ++tCategories[tablePos[w[0]]][lance].sub[w[0]].occurr;  ++tCategories[tablePos[w[0]]][lance].sub[w[0]].amarrW;
    ++tCategories[tablePos[w[1]]][lance].sub[w[1]].occurr;  ++tCategories[tablePos[w[1]]][lance].sub[w[1]].amarrW;
    ++tCategories[tablePos[l[0]]][lance].sub[l[0]].amarrL; ++tCategories[tablePos[l[1]]][lance].sub[l[1]].amarrL;
}

void __msTableHandlePares(TableCategories tCategories, float handsScores[PLAYERS][LANCES], int tablePos[PLAYERS], int bestCouples[LANCES]) {
    Player w[2], l[2];
    if(bestCouples[PARES] == COUPLE1) {
        w[0] = P1; l[0] = P2;
    } else {
        w[0] = P2; l[0] = P1;
    } w[1] = w[0] + 2; l[1] = l[0] + 2;
    int amarr = msParesScoreToAmarr(handsScores[w[0]][PARES]) + msParesScoreToAmarr(handsScores[w[1]][PARES]);
    tCategories[tablePos[w[0]]][PARES].sub[w[0]].occurr++; tCategories[tablePos[w[0]]][PARES].sub[w[0]].amarrW += amarr;
    tCategories[tablePos[w[1]]][PARES].sub[w[1]].occurr++; tCategories[tablePos[w[1]]][PARES].sub[w[1]].amarrW += amarr;
    tCategories[tablePos[l[0]]][PARES].sub[l[0]].amarrL += amarr; tCategories[tablePos[l[1]]][PARES].sub[l[1]].amarrL += amarr;
}

void __msTableHandleJuego(TableCategories tCategories, float handsScores[PLAYERS][LANCES], int tablePos[PLAYERS], int bestCouples[LANCES]) {
    Player w[2], l[2];
    if(bestCouples[JUEGO] == COUPLE1) {
        w[0] = P1; l[0] = P2;
    } else {
        w[0] = P2; l[0] = P1;
    } w[1] = w[0] + 2; l[1] = l[0] + 2;
    int amarr1 = msJuegoScoreToAmarr(handsScores[w[0]][JUEGO]); int amarr2 = msJuegoScoreToAmarr(handsScores[w[1]][JUEGO]);
    int amarr = amarr1 + amarr2 - (amarr1 == 1 || amarr2 == 1);
    tCategories[tablePos[w[0]]][JUEGO].sub[w[0]].occurr++; tCategories[tablePos[w[0]]][JUEGO].sub[w[0]].amarrW += amarr;
    tCategories[tablePos[w[1]]][JUEGO].sub[w[1]].occurr++; tCategories[tablePos[w[1]]][JUEGO].sub[w[1]].amarrW += amarr;
    tCategories[tablePos[l[0]]][JUEGO].sub[l[0]].amarrL += amarr; tCategories[tablePos[l[1]]][JUEGO].sub[l[1]].amarrL += amarr;
}

void msTablePrint(Table table) {
    size_t i = 0;
    int it[HANDLEN] = {0};
    while(it[0] <= KING) {

        printf("%3d. ", (int)i + 1);
        Hand hand = msHandFromArray(it);
        msHandPrint(hand, false); putchar(' ');

        for(size_t j = 0; j < PLAYERS; ++j) {
            printf("%6d ", (int)table.occurrences[i].sub[j]);
        } putchar(' ');

        for(size_t j = 0; j < LANCES; ++j) {
            for(size_t k = 0; k < PLAYERS; ++k) {
                printf("%6d ", (int)table.categories[i][j].sub[k].occurr);
            } putchar('|');
        }

        printf("\n     ");
        printf("         WINS ");
        printf("                             ");
        for(size_t j = 0; j < LANCES; ++j) {
            for(size_t k = 0; k < PLAYERS; ++k) {
                printf("%6d ", (int)table.categories[i][j].sub[k].amarrW);
            } putchar('|');
        }

        printf("\n     ");
        printf("         LOSS ");
        printf("                             ");
        for(size_t j = 0; j < LANCES; ++j) {
            for(size_t k = 0; k < PLAYERS; ++k) {
                printf("%6d ", (int)table.categories[i][j].sub[k].amarrL);
            } putchar('|');
        }

        ++i; putchar('\n'); putchar('\n');
        msHandArrayNext(it);
    }
}

Ptable msPtableGenerate(Table table) {
    Ptable pTable;
    pTable.categories = malloc(sizeof(A4V3dMusin*) * HANDSNUM); assert(pTable.categories && "ERROR: not enough RAM");
    pTable.expectedSums = malloc(sizeof(V2dMusin*) * HANDSNUM); assert(pTable.expectedSums && "ERROR: not enough RAM");
    for(size_t i = 0; i < HANDSNUM; ++i) {
        pTable.categories[i] = malloc(sizeof(A4V3dMusin) * LANCES); assert(pTable.categories[i] && "ERROR: not enough RAM");
        pTable.expectedSums[i] = malloc(sizeof(V2dMusin) * LANCES); assert(pTable.expectedSums[i] && "ERROR: not enough RAM");
        memset(pTable.expectedSums[i], 0, sizeof(V2dMusin) * LANCES);
    } __msPtableFill(pTable, table);
    return pTable;
}

void __msPtableFill(Ptable pTable, Table table) {
    for(size_t i = 0; i < HANDSNUM; ++i) {
        for(size_t j = 0; j < LANCES; ++j) {
            for(size_t k = 0; k < PLAYERS; ++k) {
                pTable.categories[i][j].sub[k].occurr = (table.occurrences[i].sub[k] == 0) ? 0 : (double)table.categories[i][j].sub[k].occurr / table.occurrences[i].sub[k];
                pTable.categories[i][j].sub[k].amarrW = (table.occurrences[i].sub[k] == 0) ? 0 : (double)table.categories[i][j].sub[k].amarrW / table.occurrences[i].sub[k];
                pTable.categories[i][j].sub[k].amarrL = (table.occurrences[i].sub[k] == 0) ? 0 : (double)table.categories[i][j].sub[k].amarrL / table.occurrences[i].sub[k];
                pTable.expectedSums[i][k].winSum  += pTable.categories[i][j].sub[k].amarrW;
                pTable.expectedSums[i][k].lossSum += pTable.categories[i][j].sub[k].amarrL;
            }
        }
    }
}

void msPtableDelete(Ptable pTable) {
    for(size_t i = 0; i < HANDSNUM; ++i) {
        free(pTable.categories[i]);
        free(pTable.expectedSums[i]);
    } free(pTable.categories); free(pTable.expectedSums);
}

void msPtablePrint(Ptable pTable) {
    size_t i = 0;
    int it[HANDLEN] = {0};
    while(it[0] <= KING) {

        printf("%3d. ", (int)i + 1);
        Hand hand = msHandFromArray(it);
        msHandPrint(hand, false); putchar(' ');

        for(size_t j = 0; j < LANCES; ++j) {
            for(size_t k = 0; k < PLAYERS; ++k) {
                printf("%.4f ", pTable.categories[i][j].sub[k].occurr);
            } putchar('|');
        }

        printf("\n     ");
        printf("         WINS ");
        for(size_t j = 0; j < LANCES; ++j) {
            for(size_t k = 0; k < PLAYERS; ++k) {
                printf("%.4f ", pTable.categories[i][j].sub[k].amarrW);
            } putchar('|');
        } 
        for(size_t j = 0; j < PLAYERS; ++j) printf("%.4f ", pTable.expectedSums[i][j].winSum);

        printf("\n     ");
        printf("         LOSS ");
        for(size_t j = 0; j < LANCES; ++j) {
            for(size_t k = 0; k < PLAYERS; ++k) {
                printf("%.4f ", pTable.categories[i][j].sub[k].amarrL);
            } putchar('|');
        }
        for(size_t j = 0; j < PLAYERS; ++j) printf("%.4f ", pTable.expectedSums[i][j].lossSum);

        ++i; putchar('\n'); putchar('\n');
        msHandArrayNext(it);
    }
}

void msCiTableFillT(CiTable ciTable, Ptable pTable) {
    for(size_t i = 0; i < HANDSNUM; ++i) {
        for(size_t j = 0; j < LANCES; ++j) {
            for(size_t k = 0; k < PLAYERS; ++k) {
                ciTable[i][j].sub[k].lowerWins = pTable.categories[i][j].sub[k].amarrW;
                ciTable[i][j].sub[k].upperWins = pTable.categories[i][j].sub[k].amarrW;
                ciTable[i][j].sub[k].lowerLoss = pTable.categories[i][j].sub[k].amarrL;
                ciTable[i][j].sub[k].upperLoss = pTable.categories[i][j].sub[k].amarrL;
            }
        }
    }
}

void msCiTablePrintEP(CiTable ciTable) {
    for(size_t i = 0; i < HANDSNUM; ++i) {
        for(size_t j = 0; j < LANCES; ++j) {
            for(size_t k = 0; k < PLAYERS; ++k) {
                printf("%.4f ", ciTable[i][j].sub[k].lowerWins);
            }
        }
        printf("\n");
        for(size_t j = 0; j < LANCES; ++j) {
            for(size_t k = 0; k < PLAYERS; ++k) {
                printf("%.4f ", ciTable[i][j].sub[k].upperWins);
            }
        }
        printf("\n");
        for(size_t j = 0; j < LANCES; ++j) {
            for(size_t k = 0; k < PLAYERS; ++k) {
                printf("%.4f ", ciTable[i][j].sub[k].lowerLoss);
            }
        }
        printf("\n");
        for(size_t j = 0; j < LANCES; ++j) {
            for(size_t k = 0; k < PLAYERS; ++k) {
                printf("%.4f ", ciTable[i][j].sub[k].upperLoss);
            }
        }
        printf("|\n");
    }
}

void msCiTablePrint(CiTable ciTable) {
    size_t i = 0;
    int it[HANDLEN] = {0};
    while(it[0] <= KING) {
        printf("%3d. ", (int)i + 1);
        Hand hand = msHandFromArray(it);
        msHandPrint(hand, false); putchar(' ');
        printf("\n     ");
        printf("         WINS ");
        for(size_t j = 0; j < LANCES; ++j) {
            for(size_t k = 0; k < PLAYERS; ++k) {
                printf("%.4f ", ciTable[i][j].sub[k].lowerWins);
            } putchar('|');
        }
        printf("\n     ");
        printf("              ");
        for(size_t j = 0; j < LANCES; ++j) {
            for(size_t k = 0; k < PLAYERS; ++k) {
                printf("%.4f ", ciTable[i][j].sub[k].upperWins);
            } putchar('|');
        }
        printf("\n     ");
        printf("         LOSS ");
        for(size_t j = 0; j < LANCES; ++j) {
            for(size_t k = 0; k < PLAYERS; ++k) {
                printf("%.4f ", ciTable[i][j].sub[k].lowerLoss);
            } putchar('|');
        }
        printf("\n     ");
        printf("              ");
        for(size_t j = 0; j < LANCES; ++j) {
            for(size_t k = 0; k < PLAYERS; ++k) {
                printf("%.4f ", ciTable[i][j].sub[k].upperLoss);
            } putchar('|');
        }
        ++i; putchar('\n'); putchar('\n');
        msHandArrayNext(it);
    }
}

#endif // MSTABLES_H