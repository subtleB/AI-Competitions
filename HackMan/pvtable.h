#ifndef PVTABLE_H_INCLUDED
#define PVTABLE_H_INCLUDED

#include "defs.h"
#include "fastrand.h"
#include "state.h"

int getPvLine(const int depth, State *pos);
void clearhashTable(HashTable *table);
void initHashTable(HashTable *table, const int mb);
int probeHashEntry(State *pos, MoveTuple *move, int alpha, int beta, int depth);
void storeHashEntry(State *pos, const MoveTuple move, const int depth);
MoveTuple probePvMove(const State *pos);

#endif // #ifndef PVTABLE_H_INCLUDED
