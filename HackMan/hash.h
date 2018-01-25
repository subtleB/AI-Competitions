#ifndef HASH_H_INCLUDED
#define HASH_H_INCLUDED

#include "defs.h"
#include "fastrand.h"

extern U64 pathHash[HEIGHT + 2][WIDTH + 2][2][MAX_ROUNDS];
extern U64 playerHash[HEIGHT + 2][WIDTH + 2][2];
extern U64 roundHash[MAX_ROUNDS];
extern U64 snippetHash[HEIGHT + 2][WIDTH + 2];
extern U64 weaponsHash[HEIGHT + 2][WIDTH + 2];
extern U64 bugsHash[HEIGHT + 2][WIDTH + 2];

void initHashKeys();

#endif //#ifndef HASH_H_INCLUDED
