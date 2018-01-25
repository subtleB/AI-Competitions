#include "defs.h"
#include "state.h"
//#include "pvtable.h"
#include "misc.h"
#include <string>
#include <iostream>
#include <cmath>

int rootDepth;

static double AlphaBeta(int alpha, int beta, int depth, State *state, SearchInfo *info);

static void checkUp(SearchInfo *info) {
    // Check if time is up
    if(info->timeset == TRUE && getTimeMs() > info->stoptime) {
        info->stopped = TRUE;
    }

}

static void clearForSearch(State *state, SearchInfo *info) {

    int index = 0;
    int index2 = 0;
    state->ply = 0;
    info->stopped = 0;
    info->nodes = 0;
    info->fh = 0;
    info->fhf = 0;
}

static int chooseRow(double scores[MAX_MOVES][MAX_MOVES], int rowMoves, int colMoves) {
    int rowIndex = -1;
    double rowScore = -INF;
    for(int i = 0; i < rowMoves; i++) {
        double rowMin = INF;
        for(int j = 0; j < colMoves; j++) {
            if (scores[i][j] < rowMin) {
                rowMin = scores[i][j];
            }
        }
        if (rowMin > rowScore) {
            rowScore = rowMin;
            rowIndex = i;
        }
        ASSERT(rowMin != INF);
    }
    ASSERT(rowIndex >= 0);
    return rowIndex;
}

static int chooseCol(double scores[MAX_MOVES][MAX_MOVES], int rowMoves, int colMoves) {
    int colIndex = -1;
    double colScore = INF;

    for(int j = 0; j < colMoves; j++) {
        double colMax = -INF;
        for(int i = 0; i < rowMoves; i++) {
            if (scores[i][j] > colMax) {
                colMax = scores[i][j];
            }
        }
        if (colMax < colScore) {
            colScore = colMax;
            colIndex = j;
        }
        ASSERT(colMax != INF);
    }
    ASSERT(colIndex >= 0);
    return colIndex;
}


static MoveTuple minimax(int alpha, int beta, int depth, State *state, SearchInfo *info) {
    MovesList listFirst[1];
    MovesList listSecond[1];
    
    state->getLegalMoves(listFirst, state->first);
    state->getLegalMoves(listSecond, state->second);

    double scores[MAX_MOVES][MAX_MOVES];
    int rowMoves = listFirst->nMoves;
    int colMoves = listSecond->nMoves;

    MoveTuple answer;

    info->nodes++;

    //if(probeHashEntry(state, &answer, alpha, beta, depth) == TRUE) {
    //    state->hashTable->cut++;
    //    return answer;
    //}

    for(int i = 0; i < rowMoves; i++) {
        for(int j = 0; j < colMoves; j++) {
            //U64 stateHashKey = state->hashKey;

            // Move players
            state->makeMove(listFirst->moves[i], state->first);
            state->makeMove(listSecond->moves[j], state->second);
            state->storePlayersState();

            // Move bugs
            state->performBugMoves();

            // Calculate changes due to collisions
            state->pickUpSnippets();
            state->pickUpWeapons();
            state->collideWithEnemies();
            state->collideWithPlayers();

            state->round++;

            //state->hashKey ^= roundHash[state->round - 1];
            //state->hashKey ^= roundHash[state->round];

            // Recursive call
            scores[i][j] = AlphaBeta(-beta, -alpha, depth - 1, state, info);

            state->round--;

            state->restoreBugs();
            state->restoreWeapons();
            state->restoreSnippets();
            state->restorePlayersState();

            state->takeMove(state->second);
            state->takeMove(state->first);

            //state->hashKey = stateHashKey;
        }
    }

    int row = chooseRow(scores, rowMoves, colMoves);
    int col = chooseCol(scores, rowMoves, colMoves);

    if(info->stopped == TRUE) {
        answer.firstDirection = NO_MOVE;
        answer.secondDirection = NO_MOVE;
        answer.score = 0;
    } else {    
        answer.firstDirection = listFirst->moves[row].direction;
        answer.secondDirection = listSecond->moves[col].direction;
        answer.score = scores[row][col];
        //storeHashEntry(state, answer, depth);
    }

    return answer;
}

static double AlphaBeta(int alpha, int beta, int depth, State *state, SearchInfo *info) {
    ASSERT(beta > alpha);
    ASSERT(depth >= 0);

    // TERMINAL STATE
    // Check for 200 rounds
    if (depth <= 0 || state->ply >= MAX_SEARCH_DEPTH || state->round >= 201) {
        return state->evaluate();
    }

    MovesList listFirst[1];
    MovesList listSecond[1];
    
    state->getLegalMoves(listFirst, state->first);
    state->getLegalMoves(listSecond, state->second);

    if ((info->nodes & 15) == 0) {
        checkUp(info);
    }

    info->nodes++;

    MoveTuple answer;
    //if(probeHashEntry(state, &answer, alpha, beta, depth) == TRUE) {
    //    state->hashTable->cut++;
    //    return answer.score;
    //}

    double scores[MAX_MOVES][MAX_MOVES];
    int rowMoves = listFirst->nMoves;
    int colMoves = listSecond->nMoves;

    for(int i = 0; i < rowMoves; i++) {
        for(int j = 0; j < colMoves; j++) {
            //U64 stateHashKey = state->hashKey;

            // Move players
            state->makeMove(listFirst->moves[i], state->first);
            state->makeMove(listSecond->moves[j], state->second);
            state->storePlayersState();

            // Move bugs
            state->performBugMoves();

            // Calculate changes due to collisions
            state->pickUpSnippets();
            state->pickUpWeapons();
            state->collideWithEnemies();
            state->collideWithPlayers();

            if (state->snippetsEaten % ENEMY_APPEAR_RATE != 0) {
                state->enemySpawned = false;
            }

            state->round++;
            //state->hashKey ^= roundHash[state->round - 1];
            //state->hashKey ^= roundHash[state->round];

            // Recursive call
            scores[i][j] = AlphaBeta(-beta, -alpha, depth - 1, state, info);

            state->round--;

            state->restoreBugs();
            state->restoreWeapons();
            state->restoreSnippets();
            state->restorePlayersState();

            state->takeMove(state->second);
            state->takeMove(state->first);

            //state->hashKey = stateHashKey;

            if(info->stopped == TRUE) {
                return 0;
            }
        }
    }

    int row = chooseRow(scores, rowMoves, colMoves);
    int col = chooseCol(scores, rowMoves, colMoves);

    answer.firstDirection = listFirst->moves[row].direction;
    answer.secondDirection = listSecond->moves[col].direction;
    answer.score = scores[row][col];
    //storeHashEntry(state, answer, depth);

    return scores[row][col];
}

Move searchPosition(State *state, SearchInfo *info, int playerId) {
    MoveTuple bestMove;
    int pvMoves = 0;
    clearForSearch(state, info);

    // Iterative deepening
    for(int currentDepth = 1; currentDepth <= info->depth; currentDepth++) {

        MoveTuple currentBest = minimax(-INF, INF, currentDepth, state, info);                              

        if (currentBest.firstDirection != NO_MOVE) {
            if (info->debug == TRUE) {
                fprintf(stderr, "first = %s second = %s\n", 
                        output[currentBest.firstDirection].c_str(),
                        output[currentBest.secondDirection].c_str()
                );
            }
            bestMove = currentBest;
        }

        if(info->stopped == TRUE) {
            break;
        }

        //pvMoves = getPvLine(currentDepth, state);
        //bestMove = state->pvArray[0];
        if (info->debug == TRUE) {
            fprintf(stderr,"score:%f depth:%d nodes:%ld time:%d(ms) \n",
                    bestMove.score, currentDepth, info->nodes, getTimeMs() - info->starttime);
        }
    }

    Move move;
    move.direction = PLAYER0 == playerId ? 
                bestMove.firstDirection :
                bestMove.secondDirection;
    move.score = bestMove.score;
    return move;
}
