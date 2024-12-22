#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>

#include "headers/ms_defs.h"
#include "headers/ms_ut.h"
#include "headers/ms_darray.h"
#include "headers/ms_tables.h"
#include "headers/ex_random.h"

#define MUSIN

#ifdef _WIN32
#define CLEAR "cls"
#else
#define CLEAR "clear"
#endif

typedef struct {
    Hand hand;
    UtilSlice ophrase;
    UtilSlice wphrase;
    int amarracos[COUPLES];
    int envites[LANCES];
    int posMano;
    int tablePos;
    int paresScore;
    int juegoScore;
    bool debug;
    bool laReal;
    bool someoneHasJuego;
    bool wePlayPares;
    char *buffer;
    size_t bufferLen;
    Ptable *pt;
} Mus;

typedef enum {MMERROR, MMINFO, MMDEBUG} MsgMode;
typedef enum {STATE_NOACCEPTED, STATE_PASS, STATE_QUIERO} State;
typedef enum {DESITION_PASO, DESITION_QUIERO, DESITION_ENVIDO, DESITION_ENVIDOF, DESITION_ORDAGO, DESITION_NONE} Desition;

typedef struct {
    UtilSlice *elems;
    size_t len;
    size_t cap;
} USDA;

MUSIN Mus musInit(bool laReal, bool debug, Ptable *pt, USDA winningList);
MUSIN void layout(void);
MUSIN void layoutAndInfo(const Mus * const mus, Lance lance);
MUSIN void userHold(void);
MUSIN void userInputWinners(Mus * const mus);
MUSIN void userInput(char *buffer, size_t maxLen);
MUSIN Hand userInputMusHand(void);
MUSIN void userInputMusHandPos(char *buffer, size_t maxLen);
MUSIN void userInputYesNo(char *buffer, size_t maxLen);
MUSIN void userInputPositiveInt(char *buffer, size_t maxLen);
MUSIN void userInputDesition(char *buffer, size_t maxLen, int bet);
MUSIN void userAskJuego(Mus * const mus);
MUSIN bool bufferIsAllNum(char *buffer, size_t maxLen);
MUSIN int msCardToInt(const char c);
MUSIN const char *lanceToString(Lance lance);
MUSIN void musWinCheck(int amarracos[COUPLES], UtilSlice wphrase);
MUSIN void envitesLogic(Mus * const mus, Lance lance);
MUSIN void playLance(Mus * const mus, Lance lance);
MUSIN void debugLogEnvites(const Mus * const mus);
MUSIN void musWelcome(const Mus * const mus);
MUSIN void musLogMsg(MsgMode mode, const char *msg, ...);
MUSIN void botLog(Player player, const char *msg, ...);
MUSIN void botLogic(Mus * const mus, int bet, Lance lance);
MUSIN void botLogParesJuegoInfo(const Mus * const mus, Lance lance);
MUSIN void playerLog(Player player, const char *msg, ...);
MUSIN void conteo(int amarracos[COUPLES], int winnerCouple, Lance lance, UtilSlice wphrase);
MUSIN void whoWins(Mus * const mus);
MUSIN char whoWinsLance(Mus * const mus, Lance lance);
MUSIN void pasoLogic(Player *player, int bet, int *avaPasses, int *passesLim, State *accepted);
MUSIN void quieroLogic(Player player, int bet, int envites[LANCES], Lance lance, State *accepted);
MUSIN void suboLogic(Player *player, int *bet, int *pbet, int *avaPasses, const char *buffer, const char c);
MUSIN void userAskPares(Mus *const mus);

char *msFileParseLines(const char *filePath, USDA *usda) {
    UtilError error;
    char *contents = utilFileRead(filePath, &error); 
    if(contents == NULL) musLogMsg(MMERROR, "%s : %s", error.info[0], error.info[1]);
    UtilSlice text = utilSliceNewLiteral(contents);
    while(text.len > 0) {
        UtilSlice line = utilSliceTrim(utilSliceSlice(&text, '\n', true));
        if(line.len > 0) da_append(*usda, line);
    } return contents;
}

void usage(const char * const program) {
    printf("usage: ./musin [args]\n\n");
    printf("args:\n");
    printf("\t%s          Ayuda\n", FLAG_HELP);
    printf("\t%s        Desactiva la Real\n", FLAG_NO_REAL);
    printf("\t%s         Activa el modo debug\n\n", FLAG_DEBUG_MODE);
    printf("location: %s\n", program);
}

void musParseArguments(int *argc, char ***argv, bool *laReal, bool *debugMode, const char * const program) {
    while(*argc > 0) {
        const char *arg = utilArgShift(argc, argv);
        if     (strncmp(arg, FLAG_NO_REAL, FLAG_NO_REAL_LEN) == 0) *laReal = false;
        else if(strncmp(arg, FLAG_DEBUG_MODE, FLAG_DEBUG_MODE_LEN) == 0) *debugMode = true;
        else if(strncmp(arg, ARG_LUCAS, ARG_LUCAS_LEN) == 0) {botLog(P1, "lucas? el mejor\n"); exit(0);}
        else if(strncmp(arg, ARG_JAIME, ARG_JAIME_LEN) == 0) {botLog(P1, "jaime aprende a jugar\n"); exit(0);}
        else if(strncmp(arg, FLAG_HELP, FLAG_HELP_LEN) == 0) {usage(program); exit(0);}
        else musLogMsg(MMERROR, "Argumento invalido, para ver que argumentos son validos: ./musin %s\n", FLAG_HELP);
    }
}

int main(int argc, char **argv) {

    const char * const program = utilArgShift(&argc, &argv);
    utilConsoleColorSet(UTIL_COLOR_DEFAULT); 

    bool laReal = true;
    bool debugMode = false;
    musParseArguments(&argc, &argv, &laReal, &debugMode, program);

    USDA openingsList = {0};
    USDA winningList = {0};

    char *toFree1 = msFileParseLines(FP_OPENINGS, &openingsList);
    char *toFree2 = msFileParseLines(FP_WINNINGP, &winningList);

    srand(time(0));
    struct xrandom rnd = xrandom(time(0));
    Table table = msTableGenerate(ITERS, laReal, &rnd);
    Ptable pTable = msPtableGenerate(table);

    Mus mus = musInit(laReal, debugMode, &pTable, winningList); 
    musWelcome(&mus);
    userInputMusHandPos(mus.buffer, mus.bufferLen);
    mus.posMano = mus.buffer[0] - '1';

    while(mus.amarracos[COUPLE1] < 40 && mus.amarracos[COUPLE2] < 40) {

        mus.hand = userInputMusHand();
        mus.paresScore = msHandGetScorePares(mus.hand);
        mus.juegoScore = msHandGetScoreJuego(mus.hand, mus.laReal);
        mus.ophrase = openingsList.elems[rand() % openingsList.len];

        memset(mus.envites, 0, sizeof(int) * LANCES);
        mus.tablePos = msHashBad(msHandGetScoreGrande(mus.hand));

        playLance(&mus, GRANDE);
        playLance(&mus, CHICA);
        playLance(&mus, PARES);
        playLance(&mus, JUEGO);

        whoWins(&mus);
        mus.posMano = (mus.posMano + 1) % PLAYERS;
    } 

    // Habria que cambiar todo para que el programa
    // pueda llegar aqui, TODO!
    assert(false && "How");
    msTableDelete(table);
    msPtableDelete(pTable);
    free(toFree1); free(toFree2);
    da_del(winningList); da_del(openingsList);

    return 0;
}

void layout(void) {
    system(CLEAR);
    printf("                            MUSIN\n");
    printf("Diego Estevez, Juan Pablo Morente, Jaime Mateo y Lucas Martin\n\n");
}

bool bufferIsAllNum(char* buffer, size_t maxLen) {
    size_t len = strnlen(buffer, maxLen);
    for(size_t i = 0; i < len; ++i) if(!isdigit(buffer[i])) return false;
    return true;
}

void userInput(char *buffer, size_t maxLen) {
    fflush(stdin);
    fgets(buffer, maxLen, stdin);
    utilFgetsCorrect(buffer);
}

void userInputPositiveInt(char *buffer, size_t maxLen) {
    repeat:
    userInput(buffer, maxLen);
    if(!bufferIsAllNum(buffer, maxLen)) goto repeat;
}

void userInputYesNo(char *buffer, size_t maxLen) {
    repeat:
    userInput(buffer, maxLen);
    buffer[0] = toupper(buffer[0]);
    if(buffer[0] != '\0' && buffer[0] != 'N') goto repeat;
}

void userInputMusHandPos(char *buffer, size_t maxLen) {
    repeat:
    layout();
    musLogMsg(MMINFO, "El ordenador es considerado J1\n");
    printf("Quien es mano? (1-2-3-4): ");
    userInput(buffer, maxLen);
    if(strnlen(buffer, maxLen) != 1 || !('1' <= buffer[0] && buffer[0] <= '4')) goto repeat;
}

Hand userInputMusHand(void) {
    char localBuffer[BUFFER_LEN];
    int handArray[HANDLEN];
    repeat:
    layout();
    printf("Introduce la mano recibida: ");
    userInput(localBuffer, BUFFER_LEN);
    if(strnlen(localBuffer, BUFFER_LEN) != 4) goto repeat;
    for(size_t i = 0; i < HANDLEN; ++i) {
        int card = msCardToInt(localBuffer[i]);
        if(card == NOEXISTING) goto repeat;
        handArray[i] = card;
    } iArraySort4(handArray);
    return msHandFromArray(handArray);
}

int msCardToInt(const char c) {
    switch(toupper(c)) {
        case 'C': return HORSE;
        case 'S': return JACK;
        case '7': return SEVEN;
        case '6': return SIX;
        case '5': return FIVE;
        case '4': return FOUR;
        case 'R': case '3': return KING;
        case 'A': case '1': case '0': return ACE;
        default:  return NOEXISTING;
    }
}

void musLogMsg(MsgMode mode, const char *msg, ...) {
    va_list va; va_start(va, msg);
    switch(mode) {
        case MMERROR:
            utilConsoleColorSet(UTIL_COLOR_RED);
            fprintf(stdout, "[ERROR]: ");
        break;
        case MMINFO:
            fprintf(stdout, "[INFO]: ");
        break;
        case MMDEBUG:
            utilConsoleColorSet(UTIL_COLOR_YELLOW);
            fprintf(stdout, "[DEBUG]: ");
        break;
    } utilConsoleColorSet(UTIL_COLOR_DEFAULT); 
    vfprintf(stdout, msg, va);
    va_end(va);
    if(mode == MMERROR) exit(1);
}

void userHold(void) {
    fflush(stdin);
    (void)getchar();
}

void musWelcome(const Mus * const mus) {
    layout();
    if(mus->debug) musLogMsg(MMDEBUG, "Modo debug activado\n");
    musLogMsg(MMINFO, " La Real %s estA activada\n\n", mus->laReal ? "SI" : "NO");
    printf("Presiona ENTER para empezar a jugar... ");
    userHold();
}

void userInputDesition(char *buffer, size_t maxLen, int bet) {
    repeat:
    userInput(buffer, maxLen); 
    buffer[0] = toupper(buffer[0]);
    if((buffer[0] != JUGADAPASO && buffer[0] != JUGADAQUIERO && buffer[0] != JUGADAORDAGO) && !bufferIsAllNum(buffer, maxLen)) goto repeat;
    if(buffer[0] == JUGADAQUIERO && !bet) goto repeat;
}

Desition botLanceGrandeChicaLogic(double myP, int myPos, int bet) {
    if(myP >= 0.90f) {
        if(bet == ORDAGO) return DESITION_QUIERO; 
        if(bet <= 10)     return DESITION_ENVIDOF;
        if(myPos == MANO) return DESITION_ORDAGO;
        return DESITION_QUIERO;
    }
    if(myP >= 0.80f) {
        if(bet == ORDAGO) return DESITION_QUIERO; 
        if(bet <= 5)      return DESITION_ENVIDO;
        if(myPos == MANO) return DESITION_ORDAGO;
        return DESITION_QUIERO;
    }
    return DESITION_PASO;
}   

Desition botBestParesLogic(double myP, int myPos, int bet) {
    if(bet == ORDAGO) return DESITION_QUIERO;
    if(myP >= 0.95f) {
        if(bet <= 15) return DESITION_ENVIDO;
        if(myPos == MANO) return DESITION_ORDAGO;
        return DESITION_QUIERO;
    } 
    if(bet <= 10) return DESITION_ENVIDO;
    if(myPos == MANO) return DESITION_ORDAGO;
    return DESITION_QUIERO; 
}

Desition botMidParesLogic(double myP, int myPos, int bet) {
    if(bet == ORDAGO) return DESITION_QUIERO;
    if(myP >= 0.87f) {
        if(bet <= 3) return DESITION_ENVIDOF;
        if(myPos == MANO) return DESITION_ORDAGO;
        return DESITION_QUIERO;
    }
    if(bet <= 3) return DESITION_ENVIDO;
    return DESITION_QUIERO;
}

Desition botWorstParesLogic(int myPos, int bet) {
    if(bet == ORDAGO && myPos == MANO) return DESITION_QUIERO;
    if(bet == 0) return DESITION_ENVIDO;
    if(bet <= 3) return DESITION_QUIERO;
    if(myPos == MANO) return DESITION_QUIERO;
    return DESITION_PASO;
}

Desition botLanceParesLogic(double myP, int myPos, int bet, int paresScore) {
    if(!paresScore)  return DESITION_NONE;
    if(myP >= 0.90f) return botBestParesLogic(myP, myPos, bet); 
    if(myP >= 0.83f) return botMidParesLogic(myP, myPos, bet);
    if(myP >= 0.70f) return botWorstParesLogic(myPos, bet);
    return DESITION_PASO;
}

Desition botJuegoLogic(int myPos, int bet, int juegoScore) {
    if(juegoScore <= THIRTY) return DESITION_NONE;
    if(juegoScore == REALGFORLIFE) {
        if(bet == ORDAGO) return DESITION_QUIERO;
        if(bet == 0) return DESITION_ENVIDOF;
        return DESITION_ORDAGO;
    }
    if(juegoScore == THIRTYONE || juegoScore == THIRTYTWO) {
        if(bet == ORDAGO && myPos == MANO) return DESITION_QUIERO; 
        if(bet <= 7) {
            if(myPos == MANO) return DESITION_ENVIDO;
            if(bet) return DESITION_QUIERO;
            return DESITION_ENVIDO;
        }
        if(myPos == MANO) return DESITION_ORDAGO;
        return DESITION_PASO;
    }
    if(myPos == MANO) return DESITION_ENVIDO;
    return DESITION_PASO;
}

Desition botPuntoLogic(int myPos, int bet, int juegoScore) {
    if(juegoScore > THIRTY) return DESITION_NONE;
    if(juegoScore >= 27) {
        if(bet == ORDAGO) return DESITION_QUIERO;
        if(bet <= 10) return DESITION_ENVIDO;
        if(myPos == MANO) return DESITION_ORDAGO;
        return DESITION_QUIERO;
    } return DESITION_PASO;
}

Desition botLanceJuegoLogic(int myPos, int bet, int juegoScore, bool someoneHasJuego) {
    if(someoneHasJuego) return botJuegoLogic(myPos, bet, juegoScore);
    return botPuntoLogic(myPos, bet, juegoScore);
}

void botTakeDesition(char *buffer, size_t maxLen, Desition des) {
    assert(maxLen >= 2 && "ERROR: maxLex must be >= 2");
    switch(des) {
        case DESITION_ENVIDOF: buffer[0] = '5'; buffer[1] = '\0'; break;
        case DESITION_ENVIDO:  buffer[0] = '2'; buffer[1] = '\0'; break;
        case DESITION_QUIERO:  buffer[0] = 'Q';  break;
        case DESITION_ORDAGO:  buffer[0] = 'O';  break;
        case DESITION_NONE:    buffer[0] = 'X';  break;
        case DESITION_PASO:    buffer[0] = '\0'; break;
        default: assert(false && "UNREACHABLE");
    }
}

void botLogic(Mus * const mus, int bet, Lance lance) {
    Desition des;
    int myPos = (PLAYERS - mus->posMano) % 4;
    double myP = mus->pt->categories[mus->tablePos][lance].sub[myPos].occurr;

    switch(lance) {
        case GRANDE:
        case CHICA: des = botLanceGrandeChicaLogic(myP, myPos, bet); break;
        case PARES: des = botLanceParesLogic(myP, myPos, bet, mus->paresScore); break;
        case JUEGO: des = botLanceJuegoLogic(myPos, bet, mus->juegoScore, mus->someoneHasJuego); break;
        default: assert(false && "UNREACHABLE");
    } botTakeDesition(mus->buffer, mus->bufferLen, des);
}

void botLog(Player player, const char *msg, ...) {
    va_list va;
    va_start(va, msg);
    if(player == P1) {
        utilConsoleColorSet(UTIL_COLOR_GREEN);
        fprintf(stdout, "[ORDENADOR]: ");  
        vfprintf(stdout, msg, va);
        utilConsoleColorSet(UTIL_COLOR_DEFAULT);
    }
    va_end(va);
}

void playerLog(Player player, const char *msg, ...) {
    va_list va;
    va_start(va, msg);
    if(player == P1) {utilConsoleColorSet(UTIL_COLOR_GREEN); fprintf(stdout, "[ORDENADOR]: ");}
    else {utilConsoleColorSet(UTIL_COLOR_CYAN); fprintf(stdout, "[JUGADOR_%d]: ", player + 1);}
    vfprintf(stdout, msg, va);
    utilConsoleColorSet(UTIL_COLOR_DEFAULT);
    va_end(va);
}

void pasoLogic(Player *player, int bet, int *avaPasses, int *passesLim, State *accepted) {
    botLog(*player, "Paso\n");
    if(!bet) *player = (*player + 1) % 4;
    else {
        *player = (*player + 2) % 4;
        *passesLim = 2;
    } 
    ++(*avaPasses);
    if(*avaPasses == *passesLim) *accepted = STATE_PASS;
}

void noneLogic(Player *player, int bet, int *avaPasses, int *passesLim, State *accepted) {
    if(!bet) *player = (*player + 1) % 4;
    else {
        *player = (*player + 2) % 4;
        *passesLim = 2;
    } 
    ++(*avaPasses);
    if(*avaPasses == *passesLim) *accepted = STATE_PASS;
}

void quieroLogic(Player player, int bet, int envites[LANCES], Lance lance, State *accepted) {
    if(bet == ORDAGO) {layout(); playerLog(player, "Quiero el ordago\n"); exit(0);}
    botLog(player, "Quiero\n");
    envites[lance] = bet;
    *accepted = STATE_QUIERO;
}

void suboLogic(Player *player, int *bet, int *pbet, int *avaPasses, const char *buffer, const char c) {
    if(*bet) *pbet = *bet;
    if(*bet == ORDAGO) {
        layout();
        playerLog(*player, "Quiero el ordago\n");
        exit(0);
    } else {
        if(c == JUGADAORDAGO) *bet = ORDAGO;
        else *bet += (int)strtol(buffer, NULL, 10);
        if(*bet == ORDAGO) botLog(*player, "Ordago!\n");
        else playerLog(*player, "Van %d\n", *bet);
        *player = (*player + 1) % 4;
        *avaPasses = 0;
    }
}

void envitesLogic(Mus * const mus, Lance lance) {
    
    int bet = 0, pbet = PORQUENO, avaPasses = 0, passesLim = PLAYERS;
    Player player = mus->posMano;
    State accepted = STATE_NOACCEPTED;

    while(!accepted) {
        if(player == P1) botLogic(mus, bet, lance);
        else {
            printf("Que hace J%d?: ", player + 1);
            userInputDesition(mus->buffer, mus->bufferLen, bet);
        } const char c = mus->buffer[0];
        switch(c) {
            case JUGADAQUIERO: quieroLogic(player, bet, mus->envites, lance, &accepted);    break;
            case JUGADAPASO:   pasoLogic(&player, bet, &avaPasses, &passesLim, &accepted);  break;
            case JUGADANONE:   noneLogic(&player, bet, &avaPasses, &passesLim, &accepted);  break;
            default:           suboLogic(&player, &bet, &pbet, &avaPasses, mus->buffer, c); break;
        }
    }

    if(accepted == STATE_PASS) {
        if(passesLim == 2) {
            int winnerMember = (player + 1) % 4;
            if(winnerMember == P1 || winnerMember == P3) mus->amarracos[COUPLE1] += pbet;
            else mus->amarracos[COUPLE2] += pbet;
            if(mus->debug) {putchar('\n'); musLogMsg(MMDEBUG, "Se dan %d amarracos a la pareja de J%d y J%d\n", pbet, winnerMember + 1, (winnerMember + 2) % 4 + 1);}    
        } else mus->envites[lance] = 1;
    } else assert(accepted == STATE_QUIERO && "NOT POSSIBLE");

    if(lance == PARES || lance == JUEGO) mus->envites[lance] -= mus->envites[lance] == 1;
}

void musWinCheck(int amarracos[COUPLES], UtilSlice wphrase) {
    if(amarracos[COUPLE1] >= 40)      {layout(); utilConsoleColorSet(UTIL_COLOR_GREEN); printf("[ORDENADOR]: "); printf(USFMT"\n", USARG(wphrase)); utilConsoleColorSet(UTIL_COLOR_DEFAULT); exit(0);}
    else if(amarracos[COUPLE2] >= 40) {layout(); utilConsoleColorSet(UTIL_COLOR_GREEN); printf("[ORDENADOR]: "); printf("Hemos perdido, voy con diego?\n"); utilConsoleColorSet(UTIL_COLOR_DEFAULT); exit(0);}
}

void botLogParesJuegoInfo(const Mus * const mus, Lance lance) {
    if(lance == PARES) if(!mus->paresScore)          botLog(P1, "NO TENGO PARES\n");
    if(lance == JUEGO) if(mus->juegoScore <= THIRTY) botLog(P1, "NO TENGO JUEGO\n");
}

const char *lanceToString(Lance lance) {
    switch(lance) {
        case GRANDE: return "GRANDE";
        case CHICA:  return "CHICA";
        case PARES:  return "PARES";
        case JUEGO:  return "JUEGO";
        default:     return NULL;
    }
}

void botSpeakOpening(const Mus * const mus, Lance lance) {
    if(lance == GRANDE) {
        utilConsoleColorSet(UTIL_COLOR_GREEN);
        printf("[ORDENADOR]: ");
        if(utilSliceCompar(mus->ophrase, utilSliceNewLiteral("HARDCODED"))) printf("Llevo un ca%con!", 164);
        else printf(USFMT, USARG(mus->ophrase));
        printf("\n\n");
        utilConsoleColorSet(UTIL_COLOR_DEFAULT);
    }
}

void layoutAndInfo(const Mus * const mus, Lance lance) {
    layout();
    botSpeakOpening(mus, lance);
    musLogMsg(MMINFO, "%d-%d\n", mus->amarracos[COUPLE1], mus->amarracos[COUPLE2]);
    musLogMsg(MMINFO, "LANCE DE %s\n", lanceToString(lance)); 
    musLogMsg(MMINFO, "'': paso, 'q': quiero, 'o': ordago, 'numero': sube\n\n");
    if(mus->debug) {
        musLogMsg(MMDEBUG, "J%d es mano\n", mus->posMano + 1);
        musLogMsg(MMDEBUG, ""); msHandPrint(mus->hand, true);
        musLogMsg(MMDEBUG, "Probabilidad de la mano en %s: %g%%\n", lanceToString(lance), 100*mus->pt->categories[mus->tablePos][lance].sub[MANO].occurr);
        putchar('\n');
    } botLogParesJuegoInfo(mus, lance);
}

void userAskPares(Mus *const mus) {
    printf("Se juegan los pares? '': SI, 'N': NO\n");
    userInputYesNo(mus->buffer, mus->bufferLen);
    mus->wePlayPares = mus->buffer[0] == '\0';
}

void userAskJuego(Mus * const mus) {
    printf("Alguien tiene juego (que no sea el bot)? '': SI, 'N': NO\n");
    userInputYesNo(mus->buffer, mus->bufferLen);
    mus->someoneHasJuego = mus->buffer[0] == '\0';
}

void debugLogEnvites(const Mus * const mus) {
    if(mus->debug) {
        putchar('\n'); musLogMsg(MMDEBUG, "");
        iArrayPrint(mus->envites, LANCES, true);
    } printf("\n..."); 
    userHold();
}

void playLance(Mus * const mus, Lance lance) {
    layoutAndInfo(mus, lance);
    if(lance == PARES) userAskPares(mus);
    if(lance == JUEGO) userAskJuego(mus);
    if(lance == JUEGO) {if(!(!mus->someoneHasJuego && mus->juegoScore > THIRTY)) envitesLogic(mus, lance);}
    else if(lance == PARES) {if(mus->wePlayPares) envitesLogic(mus, lance);}
    else envitesLogic(mus, lance);    
    musWinCheck(mus->amarracos, mus->wphrase);
    debugLogEnvites(mus);
}

void userInputWinners(Mus * const mus) {
    repeat:
    userInput(mus->buffer, mus->bufferLen); mus->buffer[0] = toupper(mus->buffer[0]);
    if(mus->buffer[0] != '\0' && mus->buffer[0] != 'C') goto repeat;
}

char whoWinsLance(Mus * const mus, Lance lance) {
    layout(); printf("'': Nosotros, 'C': Contrarios\n");
    printf("Quien ha ganado %s?: ", lanceToString(lance));
    userInputWinners(mus);
    if(mus->buffer[0] == '\0') mus->amarracos[COUPLE1] += mus->envites[lance];
    else mus->amarracos[COUPLE2] += mus->envites[lance];
    musWinCheck(mus->amarracos, mus->wphrase);
    return mus->buffer[0];
}

void conteo(int amarracos[COUPLES], int winnerCouple, Lance lance, UtilSlice wphrase) {
    char buffer[BUFFER_LEN];
    layout(); printf("Conteo de %s: ", lanceToString((int)lance));
    userInputPositiveInt(buffer, BUFFER_LEN);
    amarracos[winnerCouple] += (int)strtol(buffer, NULL, 10);
    musWinCheck(amarracos, wphrase);
}

void whoWins(Mus * const mus) {
    (void)whoWinsLance(mus, GRANDE);
    (void)whoWinsLance(mus, CHICA);
    int paresWinner = whoWinsLance(mus, PARES) == '\0' ? COUPLE1 : COUPLE2;
    int juegoWinner = whoWinsLance(mus, JUEGO) == '\0' ? COUPLE1 : COUPLE2;
    conteo(mus->amarracos, paresWinner, PARES, mus->wphrase);
    conteo(mus->amarracos, juegoWinner, JUEGO, mus->wphrase);
}

Mus musInit(bool laReal, bool debug, Ptable *pt, USDA winningList) {
    static char buffer[BUFFER_LEN];
    return (Mus) {
        .laReal = laReal,
        .debug = debug,
        .amarracos = {0},
        .pt = pt,
        .buffer = buffer,
        .bufferLen = BUFFER_LEN,
        .wphrase = winningList.elems[rand() % winningList.len],
    };
}