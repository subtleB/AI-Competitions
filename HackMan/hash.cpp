#include "defs.h"
#include "fastrand.h"
#include "hash.h"
#include <iostream>

U64 pathHash[HEIGHT + 2][WIDTH + 2][2][MAX_ROUNDS];
U64 playerHash[HEIGHT + 2][WIDTH + 2][2];
U64 roundHash[MAX_ROUNDS];
U64 snippetHash[HEIGHT + 2][WIDTH + 2];
U64 weaponsHash[HEIGHT + 2][WIDTH + 2];
U64 bugsHash[HEIGHT + 2][WIDTH + 2];

void initHashKeys() {
    fast_srand(42);
    for (int i = 0; i < HEIGHT + 2; i++) {
        for (int j = 0; j < WIDTH + 2; j++) {
            for (int k = 0; k < MAX_ROUNDS; k++) {
                pathHash[i][j][0][k] = RAND_64;
                pathHash[i][j][1][k] = RAND_64;
            }
            playerHash[i][j][0] = RAND_64;
            playerHash[i][j][1] = RAND_64;
            snippetHash[i][j] = RAND_64;
            weaponsHash[i][j] = RAND_64;
            bugsHash[i][j] = RAND_64;
        }
    }

    for (int i = 0; i < MAX_ROUNDS; i++) {
        roundHash[i] = RAND_64;
    }
}
