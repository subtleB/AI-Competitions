#ifndef SEARCH_H_INCLUDED
#define SEARCH_H_INCLUDED

#include "defs.h"
#include "state.h"

Move searchPosition(State *state, SearchInfo *info, int playerId);

#endif //#ifndef SEARCH_H_INCLUDED

