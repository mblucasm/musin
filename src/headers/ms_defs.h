#ifndef MSDEFS_H
#define MSDEFS_H

#include <stddef.h>

#define LANCES   4
#define PLAYERS  4
#define HANDLEN  4
#define DECKLEN  8
#define HANDSNUM 330

#define PAIRMIN   1
#define TRIOMIN   10
#define DUPLEXMIN 100

#define THIRTY      30
#define THIRTYONE   42
#define THIRTYTWO   41
#define THIRTYTHREE 33
#define REALGFORLIFE 43     

#define TREINTA      THIRTY
#define TREINTAYUNO  THIRTYONE
#define TREINTAYDOS  THIRTYTWO
#define TREINTAYTRES THIRTYTHREE

#define ACE   0
#define FOUR  1
#define FIVE  2
#define SIX   3
#define SEVEN 4
#define JACK  5
#define HORSE 6
#define KING  7

#define AS      ACE
#define PITO    ACE
#define CUATRO  FOUR
#define CINCO   FIVE
#define SEIS    SIX
#define SIETE   SEVEN
#define SOTA    JACK
#define CABALLO HORSE
#define REY     KING

#define CARDSET   ("A4567SCR")
#define DECKNEW   {8, 4, 4, 4, 4, 4, 4, 8}
#define VALUESET  ((int[]){1, 4, 5, 6, 7, 10, 10, 10})
#define OFFSETSET ((float[]){0.7F, 0.5F, 0.3F, 0.0F})

typedef enum Position {MANO, SEMIMANO, SEMIPOSTRE, POSTRE} Position;
typedef enum Player {P1, P2, P3, P4} Player;
typedef enum Lance {GRANDE, CHICA, PARES, JUEGO} Lance;
enum {TGRANDE, TCHICA, TPARES, TJUEGO, TCOUNT};
enum {COUPLE1, COUPLE2};
enum {JUGADAPASO = '\0', JUGADANONE = 'X', JUGADAORDAGO = 'O', JUGADAQUIERO = 'Q'};

#define ARG_LUCAS ("lucas")
#define ARG_JAIME ("jaime")
#define ARG_LUCAS_LEN ((sizeof(ARG_LUCAS) / sizeof(ARG_LUCAS[0])) - 1)
#define ARG_JAIME_LEN ((sizeof(ARG_JAIME) / sizeof(ARG_JAIME[0])) - 1)

#define POSPC 0
#define TEAMS 2
#define COUPLES 2
#define NOEXISTING -1

#define ORDAGO -1
#define PORQUENO 1
#define AMARR_LIM 40

#define BUFFER_LEN 200
#define ITERS 1000000

#define FLAG_HELP ("help")
#define FLAG_NO_REAL ("noreal")
#define FLAG_DEBUG_MODE ("debug")
#define FLAG_HELP_LEN ((sizeof(FLAG_HELP) / sizeof(FLAG_HELP[0])) - 1) 
#define FLAG_NO_REAL_LEN ((sizeof(FLAG_NO_REAL) / sizeof(FLAG_NO_REAL[0])) - 1)
#define FLAG_DEBUG_MODE_LEN ((sizeof(FLAG_DEBUG_MODE) / sizeof(FLAG_DEBUG_MODE[0])) - 1)

#define FP_WINNINGP "src/files/win40.txt"
#define FP_OPENINGS "src/files/openings.txt"

typedef struct {
    size_t sub[4];
} A4st;

typedef struct {
    double sub[4];
} A4d;

typedef struct {
    size_t occurr, amarrW, amarrL;
} V3stMusin;

typedef struct {
    double occurr, amarrW, amarrL;
} V3dMusin;

typedef struct {
    V3dMusin sub[4];
} A4V3dMusin;

typedef struct {
    V3stMusin sub[4];
} A4V3stMusin;

typedef struct {
    double winSum, lossSum;
} V2dMusin;

typedef struct {
    double lowerWins, lowerLoss;
    double upperWins, upperLoss;
} V4dCiMusin;

typedef struct {
    V4dCiMusin sub[4];
} A4V4dCiMusin;

#endif // MSDEFS_H