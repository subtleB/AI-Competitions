#include "stdio.h"
#include "defs.h"
#include "state.h"
#include "pvtable.h"
#include <iostream>
#include <cstdlib>

using namespace std;

int getPvLine(const int depth, State *state) {
    ASSERT(depth < MAX_SEARCH_DEPTH && depth >= 1);

    MoveTuple move = probePvMove(state);
    int count = 0;
    
    while (move.firstDirection != NO_MOVE && count < depth) {
        ASSERT(count < MAX_SEARCH_DEPTH);
        state->pvArray[count++] = move;

        Move firstMove, secondMove;
        firstMove.direction = move.firstDirection;
        secondMove.direction = move.firstDirection;

        // Move players
        state->makeMove(firstMove, state->first);
        state->makeMove(secondMove, state->second);
        state->storePlayersState();

        // Move bugs
        state->performBugMoves();

        // Calculate changes due to collisions
        state->pickUpSnippets();
        state->pickUpWeapons();
        state->collideWithEnemies();
        state->collideWithPlayers();

        state->round++;
        state->hashKey ^= roundHash[state->round - 1];
        state->hashKey ^= roundHash[state->round];

        move = probePvMove(state);
    }

    while (state->ply > 0) {
        state->round--;

        state->restoreBugs();
        state->restoreWeapons();
        state->restoreSnippets();
        state->restorePlayersState();

        state->takeMove(state->second);
        state->takeMove(state->first);
    }
    
    return count;
}

void clearhashTable(HashTable *table) {
  HashEntry *tableEntry;
  
  for (tableEntry = table->pTable; tableEntry < table->pTable + table->numEntries; tableEntry++) {
    tableEntry->posKey = 0ULL;
    MoveTuple nomove;
    nomove.firstDirection = NO_MOVE;
    nomove.secondDirection = NO_MOVE;
    nomove.score = 0;
    tableEntry->move = nomove;
    tableEntry->depth = 0;
    tableEntry->score = 0;
    tableEntry->flags = 0;
  }
  table->newWrite = 0;
}

void initHashTable(HashTable *table, const int MB) {  
    int hashSize = 0x100000 * MB;
    table->numEntries = hashSize / sizeof(HashEntry);
    table->numEntries -= 2;
    
    if (table->pTable != NULL) {
        free(table->pTable);
    }
        
    table->pTable = (HashEntry *) malloc(table->numEntries * sizeof(HashEntry));
    if (table->pTable == NULL) {
        fprintf(stderr, "HashTable allocation failed, trying %dmb...\n",MB/2);
        initHashTable(table, MB / 2);
    } else {
        clearhashTable(table);
    }
}

int probeHashEntry(State *pos, MoveTuple *move, int alpha, int beta, int depth) {
    int index = (pos->hashKey ^ pos->pathDependentKey) % pos->hashTable->numEntries;

    ASSERT(index >= 0 && index <= pos->hashTable->numEntries - 1);
    ASSERT(depth>=1 && depth<MAX_SEARCH_DEPTH);
    ASSERT(alpha<beta);
    ASSERT(alpha>=-INF && alpha<=INF);
    ASSERT(beta>=-INF && beta<=INF);
    ASSERT(pos->ply>=0 && pos->ply<MAX_SEARCH_DEPTH);

    if (pos->hashTable->pTable[index].posKey == (pos->hashKey ^ pos->pathDependentKey)) {
        *move = pos->hashTable->pTable[index].move;
        if (pos->hashTable->pTable[index].depth >= depth){
            pos->hashTable->hit++;
            ASSERT(pos->hashTable->pTable[index].depth>=1&&pos->hashTable->pTable[index].depth<MAX_SEARCH_DEPTH);
            return TRUE;
        }
    }
    return FALSE;
}

void storeHashEntry(State *pos, const MoveTuple move, const int depth) {
    int index = (pos->hashKey ^ pos->pathDependentKey) % pos->hashTable->numEntries;
    
    ASSERT(index >= 0 && index <= pos->hashTable->numEntries - 1);
    ASSERT(depth >= 1 && depth < MAX_SEARCH_DEPTH);
    ASSERT(pos->ply >= 0 && pos->ply < MAX_SEARCH_DEPTH);
    
    if (pos->hashTable->pTable[index].posKey == 0) {
        pos->hashTable->newWrite++;
    } else {
        pos->hashTable->overWrite++;
    }

    pos->hashTable->pTable[index].move = move;
    pos->hashTable->pTable[index].posKey = (pos->hashKey ^ pos->pathDependentKey);
    pos->hashTable->pTable[index].score = move.score;
    pos->hashTable->pTable[index].depth = depth;
}

MoveTuple probePvMove(const State *pos) {
    int index = (pos->hashKey ^ pos->pathDependentKey) % pos->hashTable->numEntries;
    ASSERT(index >= 0 && index <= pos->hashTable->numEntries - 1);
    
    if (pos->hashTable->pTable[index].posKey == (pos->hashKey ^ pos->pathDependentKey)) {
        return pos->hashTable->pTable[index].move;
    }
    
    MoveTuple nomove;
    nomove.firstDirection = NO_MOVE;
    nomove.secondDirection = NO_MOVE;
    nomove.score = 0;
    return nomove;
}



