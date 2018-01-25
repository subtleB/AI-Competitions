#ifndef STATE_H_INCLUDED
#define STATE_H_INCLUDED

#include "defs.h"
#include "fastrand.h"
#include "hash.h"
#include <string>
#include <iostream>
#include <set> 
#include <vector> 

using namespace std;

class State {
public:
    int map[HEIGHT + 2][WIDTH + 2];
    Undo history[MAX_GAME_MOVES];
    SnippetsList snippetsList[1];
    SnippetsList weaponsList[1];
    //HashTable hashTable[1];
    //MoveTuple pvArray[MAX_SEARCH_DEPTH];
    BugsList bugsList[1];
    Player* first;
    Player* second;
    int round;
    int snippetsEaten;
    int collectedSnippets;
    int hisPly;
    int ply;
    //U64 hashKey;
    bool enemySpawned;
    bool weaponSpawned;

    string myName;
    int myId;

    State();
    void printMap();
    void getLegalMoves(MovesList* list, Player *player) const;
    void getLegalMoves2(MovesList* list, Player *player) const;
    void makeMove(Move &move, Player *player);
    void takeMove(Player *player);
    U64  positionKey() const;
    double evaluate();
    double evaluate2();
    int distToNearestUnit(Player *player, int unitId) const;
    int sumDistanceToSnippsAndWeapons();
    int numbSnippetsCloserToPlayer();
    int minDistToSnippets(int x, int y);
    double nextDestinationValue(Player *p1, Player *p2);

    // Engine
    void makeEngineMove(Move &move, Player *player);
    double getPChase();
    int updateWinner();

    inline void performBugMoves() {
        ASSERT(this->bugsList->nBugs >= 0);
        this->history[this->ply].bugs->nBugs = this->bugsList->nBugs;

        for (int i = 0; i < this->bugsList->nBugs; i++) {
            this->history[this->ply].bugs->bugs[i].prevX = this->bugsList->bugs[i].prevX;
            this->history[this->ply].bugs->bugs[i].prevY = this->bugsList->bugs[i].prevY;
            this->history[this->ply].bugs->bugs[i].x = this->bugsList->bugs[i].x;
            this->history[this->ply].bugs->bugs[i].y = this->bugsList->bugs[i].y;

            int prevX = this->bugsList->bugs[i].prevX;
            int prevY = this->bugsList->bugs[i].prevY;

            this->bugsList->bugs[i].prevX = this->bugsList->bugs[i].x;
            this->bugsList->bugs[i].prevY = this->bugsList->bugs[i].y;

            int x1[5];
            int y1[5];
            int n = 0;

            int x = this->bugsList->bugs[i].x;
            int y = this->bugsList->bugs[i].y;

            // Check if there is mandatory move
            for (int j = 0; j < 4; j++) {
                if (this->map[x + dx[j]][y + dy[j]] != WALL
                        && (x + dx[j] != prevX || y + dy[j] != prevY)) {
                    x1[n  ] = dx[j];
                    y1[n++] = dy[j];
                }
            }

            // Bug spawn
            if (0 == n) {
                ASSERT(8 == this->bugsList->bugs[i].x);
                if (9 == this->bugsList->bugs[i].y) {
                    this->bugsList->bugs[i].y +=  1;
                } else if (12 == this->bugsList->bugs[i].y) {
                    this->bugsList->bugs[i].y -=  1;
                } else {
                    this->bugsList->bugs[i].x -=  1;
                }

            // Mandotory direction found
            } else if (1 == n) {
                this->bugsList->bugs[i].x += x1[0];
                this->bugsList->bugs[i].y += y1[0];
            } else {
                ASSERT(2 == n);

                // Do chase
                int firstDist  = distances[first->x][first->y][x][y];
                int secondDist = distances[second->x][second->y][x][y];

                if ((x == first->x) && (y == first->y)) {
                    if ((x + x1[0] == first->prevX) && (y + y1[0] == first->prevY)) {
                        this->bugsList->bugs[i].x += x1[0];
                        this->bugsList->bugs[i].y += y1[0];
                        //this->hashKey ^= bugsHash[this->bugsList->bugs[i].prevX][this->bugsList->bugs[i].prevY];
                        //this->hashKey ^= bugsHash[this->bugsList->bugs[i].x][this->bugsList->bugs[i].y];
                        continue;
                    } else if ((x + x1[1] == first->prevX) && (y + y1[1] == first->prevY)) {
                        this->bugsList->bugs[i].x += x1[1];
                        this->bugsList->bugs[i].y += y1[1];
                        //this->hashKey ^= bugsHash[this->bugsList->bugs[i].prevX][this->bugsList->bugs[i].prevY];
                        //this->hashKey ^= bugsHash[this->bugsList->bugs[i].x][this->bugsList->bugs[i].y];
                        continue; 
                    }
                }

                if ((x == second->x) && (y == second->y)) {
                    if ((x + x1[0] == second->prevX) && (y + y1[0] == second->prevY)) {
                        this->bugsList->bugs[i].x += x1[0];
                        this->bugsList->bugs[i].y += y1[0];
                        //this->hashKey ^= bugsHash[this->bugsList->bugs[i].prevX][this->bugsList->bugs[i].prevY];
                        //this->hashKey ^= bugsHash[this->bugsList->bugs[i].x][this->bugsList->bugs[i].y];
                        continue;
                    } else if ((x + x1[1] == second->prevX) && (y + y1[1] == second->prevY)) {
                        this->bugsList->bugs[i].x += x1[1];
                        this->bugsList->bugs[i].y += y1[1];
                        //this->hashKey ^= bugsHash[this->bugsList->bugs[i].prevX][this->bugsList->bugs[i].prevY];
                        //this->hashKey ^= bugsHash[this->bugsList->bugs[i].x][this->bugsList->bugs[i].y];
                        continue; 
                    }
                }

                // Chase for nearest player
                if (firstDist < secondDist) {
                    if (distances[first->x][first->y][x + x1[0]][y + y1[0]] 
                            < distances[first->x][first->y][x + x1[1]][y + y1[1]]) {
                        this->bugsList->bugs[i].x += x1[0];
                        this->bugsList->bugs[i].y += y1[0];
                    } else {
                        this->bugsList->bugs[i].x += x1[1];
                        this->bugsList->bugs[i].y += y1[1];
                    }
                } else if (firstDist > secondDist){
                    if (distances[second->x][second->y][x + x1[0]][y + y1[0]] 
                            < distances[second->x][second->y][x + x1[1]][y + y1[1]]) {
                        this->bugsList->bugs[i].x += x1[0];
                        this->bugsList->bugs[i].y += y1[0];
                    } else {
                        this->bugsList->bugs[i].x += x1[1];
                        this->bugsList->bugs[i].y += y1[1];
                    }

                // Chase for random player
                } else {
                    if ((distances[first->x][first->y][x + x1[0]][y + y1[0]] 
                            < distances[first->x][first->y][x + x1[1]][y + y1[1]])
                            || (distances[second->x][second->y][x + x1[0]][y + y1[0]] 
                                < distances[second->x][second->y][x + x1[1]][y + y1[1]])) {
                        this->bugsList->bugs[i].x += x1[0];
                        this->bugsList->bugs[i].y += y1[0];
                    } else {
                        this->bugsList->bugs[i].x += x1[1];
                        this->bugsList->bugs[i].y += y1[1];
                    }
                }
                
            }

            // Incrementally updating the hash
            //this->hashKey ^= bugsHash[this->bugsList->bugs[i].prevX]
            //                         [this->bugsList->bugs[i].prevY];
            //this->hashKey ^= bugsHash[this->bugsList->bugs[i].x]
            //                         [this->bugsList->bugs[i].y];

            ASSERT(this->bugsList->bugs[i].prevX > 0);
            ASSERT(this->bugsList->bugs[i].prevY > 0);
            ASSERT(this->bugsList->bugs[i].prevX != this->bugsList->bugs[i].x
                   || this->bugsList->bugs[i].prevY != this->bugsList->bugs[i].y);
        }
    }

    inline void performEngineBugMoves() {
        ASSERT(this->bugsList->nBugs >= 0);
        for (int i = 0; i < this->bugsList->nBugs; i++) {
            int prevX = this->bugsList->bugs[i].prevX;
            int prevY = this->bugsList->bugs[i].prevY;
            this->bugsList->bugs[i].prevX = this->bugsList->bugs[i].x;
            this->bugsList->bugs[i].prevY = this->bugsList->bugs[i].y;

            int x1[5];
            int y1[5];
            int n = 0;

            int x = this->bugsList->bugs[i].x;
            int y = this->bugsList->bugs[i].y;

            // Check if there is mandatory move
            for (int j = 0; j < 4; j++) {
                if (this->map[x + dx[j]][y + dy[j]] != WALL && (x + dx[j] != prevX
                        || y + dy[j] != prevY)) {
                    x1[n  ] = dx[j];
                    y1[n++] = dy[j];
                }
            }

            // Bug spawn
            if (0 == n) {
                if (8 == this->bugsList->bugs[i].x) {
                    if (9 == this->bugsList->bugs[i].y) {
                        this->bugsList->bugs[i].y +=  1;
                    } else if (12 == this->bugsList->bugs[i].y) {
                        this->bugsList->bugs[i].y -=  1;
                    } else {
                        this->bugsList->bugs[i].x -=  1;
                    }
                }

            // Mandotory direction found
            } else if (1 == n) {
                this->bugsList->bugs[i].x += x1[0];
                this->bugsList->bugs[i].y += y1[0];
            } else {
                ASSERT(2 == n);
                if (fastRandDouble() <= getPChase()) {

                    // Do chase
                    int firstDist  = distances[first->x][first->y][x][y];
                    int secondDist = distances[second->x][second->y][x][y];

                    if ((x == first->x) && (y == first->y)) {
                        if ((x + x1[0] == first->prevX) && (y + y1[0] == first->prevY)) {
                            this->bugsList->bugs[i].x += x1[0];
                            this->bugsList->bugs[i].y += y1[0];
                            //this->hashKey ^= bugsHash[this->bugsList->bugs[i].prevX][this->bugsList->bugs[i].prevY];
                            //this->hashKey ^= bugsHash[this->bugsList->bugs[i].x][this->bugsList->bugs[i].y];
                            continue;
                        } else if ((x + x1[1] == first->prevX) && (y + y1[1] == first->prevY)) {
                            this->bugsList->bugs[i].x += x1[1];
                            this->bugsList->bugs[i].y += y1[1];
                            //this->hashKey ^= bugsHash[this->bugsList->bugs[i].prevX][this->bugsList->bugs[i].prevY];
                            //this->hashKey ^= bugsHash[this->bugsList->bugs[i].x][this->bugsList->bugs[i].y];
                            continue; 
                        }
                    }

                    if ((x == second->x) && (y == second->y)) {
                        if ((x + x1[0] == second->prevX) && (y + y1[0] == second->prevY)) {
                            this->bugsList->bugs[i].x += x1[0];
                            this->bugsList->bugs[i].y += y1[0];
                            //this->hashKey ^= bugsHash[this->bugsList->bugs[i].prevX][this->bugsList->bugs[i].prevY];
                            //this->hashKey ^= bugsHash[this->bugsList->bugs[i].x][this->bugsList->bugs[i].y];
                            continue;
                        } else if ((x + x1[1] == second->prevX) && (y + y1[1] == second->prevY)) {
                            this->bugsList->bugs[i].x += x1[1];
                            this->bugsList->bugs[i].y += y1[1];
                            //this->hashKey ^= bugsHash[this->bugsList->bugs[i].prevX][this->bugsList->bugs[i].prevY];
                            //this->hashKey ^= bugsHash[this->bugsList->bugs[i].x][this->bugsList->bugs[i].y];
                            continue; 
                        }
                    }

                    // Chase for nearest player
                    if (firstDist < secondDist) {
                        if (distances[first->x][first->y][x + x1[0]][y + y1[0]] 
                                < distances[first->x][first->y][x + x1[1]][y + y1[1]]) {
                            this->bugsList->bugs[i].x += x1[0];
                            this->bugsList->bugs[i].y += y1[0];
                        } else {
                            this->bugsList->bugs[i].x += x1[1];
                            this->bugsList->bugs[i].y += y1[1];
                        }
                    } else if (firstDist > secondDist){
                        if (distances[second->x][second->y][x + x1[0]][y + y1[0]] 
                                < distances[second->x][second->y][x + x1[1]][y + y1[1]]) {
                            this->bugsList->bugs[i].x += x1[0];
                            this->bugsList->bugs[i].y += y1[0];
                        } else {
                            this->bugsList->bugs[i].x += x1[1];
                            this->bugsList->bugs[i].y += y1[1];
                        }

                    // Chase for random player
                    } else {
                        if ((distances[first->x][first->y][x + x1[0]][y + y1[0]] 
                                < distances[first->x][first->y][x + x1[1]][y + y1[1]])
                                || (distances[second->x][second->y][x + x1[0]][y + y1[0]] 
                                    < distances[second->x][second->y][x + x1[1]][y + y1[1]])) {
                            this->bugsList->bugs[i].x += x1[0];
                            this->bugsList->bugs[i].y += y1[0];
                        } else {
                            this->bugsList->bugs[i].x += x1[1];
                            this->bugsList->bugs[i].y += y1[1];
                        }
                    }
                // Do not chase
                } else {
                    int r = fastrand() % n;
                    this->bugsList->bugs[i].x += x1[r];
                    this->bugsList->bugs[i].y += y1[r];
                }
                
            }
            ASSERT(this->bugsList->bugs[i].prevX > 0);
            ASSERT(this->bugsList->bugs[i].prevY > 0);
            ASSERT(this->bugsList->bugs[i].prevX != this->bugsList->bugs[i].x
                   || this->bugsList->bugs[i].prevY != this->bugsList->bugs[i].y);
        }
    }



    inline void restoreBugs() {
        ASSERT(this->bugsList->nBugs >= 0);
        ASSERT(this->history[this->ply].bugs->nBugs >= 0);
        this->bugsList->nBugs = this->history[this->ply].bugs->nBugs;

        for (int i = 0; i < this->bugsList->nBugs; i++) {
            this->bugsList->bugs[i].prevX = this->history[this->ply].bugs->bugs[i].prevX;
            this->bugsList->bugs[i].prevY = this->history[this->ply].bugs->bugs[i].prevY;
            this->bugsList->bugs[i].x = this->history[this->ply].bugs->bugs[i].x;
            this->bugsList->bugs[i].y = this->history[this->ply].bugs->bugs[i].y;
        }
    }

    inline void pickUpSnippets() {
        ASSERT(ply <= MAX_SEARCH_DEPTH);
        history[ply].snippetsList->nSnippets = 0;

        if (first->x == second->x && first->y == second->y 
                && map[first->x][first->y] == SNIPPET) {
            history[ply].snippetsList->snippets[history[ply].snippetsList->nSnippets  ].x = first->x;
            history[ply].snippetsList->snippets[history[ply].snippetsList->nSnippets++].y = first->y;
            first->snippets += 0.5 + ((MAX_ROUNDS - round) * 0.001);
            second->snippets += 0.5 + ((MAX_ROUNDS - round) * 0.001);
            map[first->x][first->y] = FREE_CELL;
            //hashKey ^= snippetHash[first->x][first->y];
            eraseFromSnippetsList(first->x, first->y);
        } else {

            if (map[first->x][first->y] == SNIPPET) {
                history[ply].snippetsList->snippets[history[ply].snippetsList->nSnippets].x = first->x;
                history[ply].snippetsList->snippets[history[ply].snippetsList->nSnippets++].y = first->y;
                first->snippets += 1 + ((MAX_ROUNDS - round) * 0.001);
                map[first->x][first->y] = FREE_CELL;
                //hashKey ^= snippetHash[first->x][first->y];
                eraseFromSnippetsList(first->x, first->y);
            }

            if (map[second->x][second->y] == SNIPPET) {
                history[ply].snippetsList->snippets[history[ply].snippetsList->nSnippets  ].x = second->x;
                history[ply].snippetsList->snippets[history[ply].snippetsList->nSnippets++].y = second->y;
                second->snippets += 1 + ((MAX_ROUNDS - round) * 0.001);
                map[second->x][second->y] = FREE_CELL;
                //hashKey ^= snippetHash[second->x][second->y];
                eraseFromSnippetsList(second->x, second->y);
            }
        }
    }

    inline void pickUpSnippets2() {
        ASSERT(ply <= MAX_SEARCH_DEPTH);
        history[ply].snippetsList->nSnippets = 0;

        if (first->x == second->x && first->y == second->y 
                && map[first->x][first->y] == SNIPPET) {
            history[ply].snippetsList->snippets[history[ply].snippetsList->nSnippets  ].x = first->x;
            history[ply].snippetsList->snippets[history[ply].snippetsList->nSnippets++].y = first->y;
            first->snippets += 0.5;
            second->snippets += 0.5;
            map[first->x][first->y] = FREE_CELL;
            //hashKey ^= snippetHash[first->x][first->y];
            eraseFromSnippetsList(first->x, first->y);
        } else {

            if (map[first->x][first->y] == SNIPPET) {
                history[ply].snippetsList->snippets[history[ply].snippetsList->nSnippets].x = first->x;
                history[ply].snippetsList->snippets[history[ply].snippetsList->nSnippets++].y = first->y;
                first->snippets += 1 + ((MAX_SEARCH_DEPTH - ply) * 0.2 / MAX_SEARCH_DEPTH);
                map[first->x][first->y] = FREE_CELL;
                //hashKey ^= snippetHash[first->x][first->y];
                eraseFromSnippetsList(first->x, first->y);
            }

            if (map[second->x][second->y] == SNIPPET) {
                history[ply].snippetsList->snippets[history[ply].snippetsList->nSnippets  ].x = second->x;
                history[ply].snippetsList->snippets[history[ply].snippetsList->nSnippets++].y = second->y;
                second->snippets += 1 + ((MAX_SEARCH_DEPTH - ply) * 0.2 / MAX_SEARCH_DEPTH);
                map[second->x][second->y] = FREE_CELL;
                //hashKey ^= snippetHash[second->x][second->y];
                eraseFromSnippetsList(second->x, second->y);
            }
        }
    }

    inline void eraseFromSnippetsList(int x, int y) {
        for (int i = 0; i < snippetsList->nSnippets; i++) {
            if (snippetsList->snippets[i].x == x
                    && snippetsList->snippets[i].y == y) {
                snippetsList->snippets[i] = snippetsList->snippets[--snippetsList->nSnippets];
                return;
            }
        }
        ASSERT(false);
    }

    inline void eraseFromWeaponsList(int x, int y) {
        for (int i = 0; i < weaponsList->nSnippets; i++) {
            if (weaponsList->snippets[i].x == x
                    && weaponsList->snippets[i].y == y) {
                weaponsList->snippets[i] = weaponsList->snippets[--weaponsList->nSnippets];
                return;
            }
        }
        ASSERT(false);
    }

    inline void enginePickUpSnippets() {
        ASSERT(ply <= MAX_SEARCH_DEPTH);
        if (first->x == second->x && first->y == second->y 
                && map[first->x][first->y] == SNIPPET) {
            first->snippets += 0.5;
            second->snippets += 0.5;
            snippetsEaten++;
            map[first->x][first->y] = FREE_CELL;
            eraseFromSnippetsList(first->x, first->y);
        } else {

            if (map[first->x][first->y] == SNIPPET) {
                first->snippets += 1;
                snippetsEaten++;
                map[first->x][first->y] = FREE_CELL;
                eraseFromSnippetsList(first->x, first->y);
            }

            if (map[second->x][second->y] == SNIPPET) {
                second->snippets += 1;
                snippetsEaten++;
                map[second->x][second->y] = FREE_CELL;
                eraseFromSnippetsList(second->x, second->y);
            }
        }
    }

    inline void storePlayersState() {
        history[ply].snippetsFirst = first->snippets;
        history[ply].snippetsSecond = second->snippets;
        history[ply].weaponFirst = first->has_weapon;
        history[ply].weaponSecond = second->has_weapon;
        history[ply].paralizeFirst = first->is_paralyzed;
        history[ply].paralizeSecond = second->is_paralyzed;
        history[ply].killedBugsFirst = first->killedBugs;
        history[ply].killedBugsSecond = second->killedBugs;

        if (first->is_paralyzed) {
            first->is_paralyzed = false;
        }

        if (second->is_paralyzed) {
            second->is_paralyzed = false;
        }
    }

    inline void restorePlayersState() {
        first->snippets = history[ply].snippetsFirst;
        second->snippets = history[ply].snippetsSecond;
        first->has_weapon = history[ply].weaponFirst;
        second->has_weapon = history[ply].weaponSecond;
        first->is_paralyzed = history[ply].paralizeFirst;
        second->is_paralyzed = history[ply].paralizeSecond;
        first->killedBugs = history[ply].killedBugsFirst;
        second->killedBugs = history[ply].killedBugsSecond;
    }

    inline void restoreSnippets() {
        ASSERT(history[ply].snippetsList->nSnippets >= 0);
        for (int i = 0; i < history[ply].snippetsList->nSnippets; i++) {
            ASSERT(history[ply].snippetsList->snippets[i].x > 0 && history[ply].snippetsList->snippets[i].x < HEIGHT + 1);
            ASSERT(history[ply].snippetsList->snippets[i].y > 0 && history[ply].snippetsList->snippets[i].y < WIDTH + 1);
            map[history[ply].snippetsList->snippets[i].x]
               [history[ply].snippetsList->snippets[i].y] = SNIPPET;
            history[ply].snippetsList->snippets[i].visited = false;
            snippetsList->snippets[snippetsList->nSnippets++] = 
                    history[ply].snippetsList->snippets[i];
        } 
    }

    inline void restoreWeapons() {
        ASSERT(history[ply].weaponsList->nSnippets >= 0);
        for (int i = 0; i < history[ply].weaponsList->nSnippets; i++) {
            ASSERT(history[ply].weaponsList->snippets[i].x > 0 && history[ply].weaponsList->snippets[i].x < HEIGHT + 1);
            ASSERT(history[ply].weaponsList->snippets[i].y > 0 && history[ply].weaponsList->snippets[i].y < WIDTH + 1);
            map[history[ply].weaponsList->snippets[i].x]
               [history[ply].weaponsList->snippets[i].y] = WEAPON;
            history[ply].weaponsList->snippets[i].visited = false;
            weaponsList->snippets[weaponsList->nSnippets++] = 
                    history[ply].weaponsList->snippets[i];
        } 
    }


    inline void pickUpWeapons() {
        history[ply].weaponsList->nSnippets = 0;

        if (first->x == second->x && first->y == second->y
                    && map[first->x][first->y] == WEAPON) {
            // nothing to do
            history[ply].weaponsList->snippets[history[ply].weaponsList->nSnippets  ].x = first->x;
            history[ply].weaponsList->snippets[history[ply].weaponsList->nSnippets++].y = first->y;
            map[first->x][first->y] = FREE_CELL;
            //hashKey ^= weaponsHash[first->x][first->y];
            eraseFromWeaponsList(first->x, first->y);
        } else {
            if (map[first->x][first->y] == WEAPON) {
                first->has_weapon = true;
                map[first->x][first->y] = FREE_CELL;
                history[ply].weaponsList->snippets[history[ply].weaponsList->nSnippets  ].x = first->x;
                history[ply].weaponsList->snippets[history[ply].weaponsList->nSnippets++].y = first->y;
                //hashKey ^= weaponsHash[first->x][first->y];
                eraseFromWeaponsList(first->x, first->y);
            }

            if (map[second->x][second->y] == WEAPON) {
                second->has_weapon = true;
                map[second->x][second->y] = FREE_CELL;
                history[ply].weaponsList->snippets[history[ply].weaponsList->nSnippets  ].x = second->x;
                history[ply].weaponsList->snippets[history[ply].weaponsList->nSnippets++].y = second->y;
               // hashKey ^= weaponsHash[second->x][second->y];
                eraseFromWeaponsList(second->x, second->y);
            }
        }
    }

    inline void enginePickUpWeapons() {
        if (first->x == second->x && first->y == second->y
                    && map[first->x][first->y] == WEAPON) {
            if ((rand() % 2) == 0) {
                first->has_weapon = true;
            } else {
                second->has_weapon = true;
            }
            map[first->x][first->y] = FREE_CELL;
            eraseFromWeaponsList(first->x, first->y);
        } else {
            if (map[first->x][first->y] == WEAPON) {
                first->has_weapon = true;
                map[first->x][first->y] = FREE_CELL;
                eraseFromWeaponsList(first->x, first->y);
            }
            if (map[second->x][second->y] == WEAPON) {
                second->has_weapon = true;
                map[second->x][second->y] = FREE_CELL;
                eraseFromWeaponsList(second->x, second->y);
            }
        }
    }

    inline void collideWithEnemies() {
        int killed[10];
        int nKilled = 0;

        for (int i = 0; i < this->bugsList->nBugs; i++) {
            int bugX = this->bugsList->bugs[i].x;
            int bugY = this->bugsList->bugs[i].y;
            int prevBugX = this->bugsList->bugs[i].prevX;
            int prevBugY = this->bugsList->bugs[i].prevY;

            if (prevBugX == first->x && prevBugY == first->y
                    && bugX == first->prevX && bugY == first->prevY) {
                hitByBug(first, i);
                int add = TRUE;
                for (int j = nKilled - 1; j >= 0; j--) {
                    if (i == killed[j]) {
                        add = FALSE;
                        break;
                    }
                }

                if (add == TRUE) {
                    killed[nKilled++] = i;
                }
            }

            if (prevBugX == second->x && prevBugY == second->y
                    && bugX == second->prevX && bugY == second->prevY) {
                hitByBug(second, i);
                int add = TRUE;
                for (int j = nKilled - 1; j >= 0; j--) {
                    if (i == killed[j]) {
                        add = FALSE;
                        break;
                    }
                }

                if (add == TRUE) {
                    killed[nKilled++] = i;
                }
            }
        }

        killBugs(killed, nKilled);

        // Collide by same position
        nKilled = 0;
        for (int i = 0; i < this->bugsList->nBugs; i++) {
            int bugX = this->bugsList->bugs[i].x;
            int bugY = this->bugsList->bugs[i].y;

            if (bugX == first->x && bugY == first->y) {
                hitByBug(first, i);
            }

            if (bugX == second->x && bugY == second->y) {
                hitByBug(second, i);
            }
        }
        killBugs(killed, nKilled);
    }

    inline void killBugs(int killed[10], int nKilled) {
        for (int i = nKilled - 1; i >= 0; i--) {
            for (int j = nKilled - 1; j >= 0; j--) {
                if (killed[i] > killed[j]) {
                    int tmp = killed[i];
                    killed[i] = killed[j];
                    killed[j] = tmp;
                }
            }
        }

        ASSERT(nKilled <= this->bugsList->nBugs);
        for (int i = nKilled - 1; i >= 0; i--) {
            this->bugsList->bugs[killed[i]] = this->bugsList->bugs[--this->bugsList->nBugs];
        }
    }

    inline void hitByBug(Player *player, int bugIndex) {
        if (player->has_weapon) { // player kills enemy
            player->killedBugs++;
            player->has_weapon = false;
        } else { // player loses snippets
            player->snippets -= HIT_SNIPPETS_LOSS;
        }
    }

    inline void spawnEnemy() {
        int spawnX = 8;
        int spawnY[] = {9, 10, 11, 12};
        int r = fastrand() % 4;
        int n = bugsList->nBugs++;
        bugsList->bugs[n].x = spawnX;
        bugsList->bugs[n].y = spawnY[r];
        bugsList->bugs[n].prevX = 0;
        bugsList->bugs[n].prevY = 0;
    }

    inline void spawnEnemySearch(int *y) {
        int spawnX = 8;
        int spawnY[] = {9, 10, 11, 12};
        int r = fastrand() % 4;
        int n = bugsList->nBugs++;
        bugsList->bugs[n].x = spawnX;
        bugsList->bugs[n].y = spawnY[r];
        bugsList->bugs[n].prevX = 0;
        bugsList->bugs[n].prevY = 0;
        *y = spawnY[r];
    }

    inline void removeEnemySearch(int y) {
        int x = 8;
        for (int i = 0; i < bugsList->nBugs; i++) {
            if (bugsList->bugs[i].x == x 
                && bugsList->bugs[i].y == y) {
                bugsList->bugs[i] = bugsList->bugs[--bugsList->nBugs];
                return;
            }
        }
        ASSERT(false);
    }

    inline void spawnSnippet() {
        int x[300];
        int y[300];
        int n = 0;

        for (int i = 0; i < HEIGHT + 2; i++) {
            for (int j = 0; j < WIDTH + 2; j++) {
                if (map[i][j] == FREE_CELL) {
                    x[n  ] = i;
                    y[n++] = j;
                } 
            }
        }

        ASSERT(n <= 300);

        int r = fastrand() % n;
        map[x[r]][y[r]] = SNIPPET;
        int k = snippetsList->nSnippets++;
        Snippet snippet;
        snippet.x = x[r];
        snippet.y = y[r];
        snippet.visited = false;
        snippet.appearRound = round;
        snippetsList->snippets[k] = snippet;
    }

    inline void spawnSnippetSearch(int *spawnX, int *spawnY) {
        int x;
        int y;
        int n = 0;

        int maxDist = 0;
        int minDiff = INF;
        for (int i = 0; i < HEIGHT + 2; i++) {
            for (int j = 0; j < WIDTH + 2; j++) {
                if (map[i][j] == FREE_CELL) {
                    if (distances[i][j][first->x][first->y] 
                            + distances[i][j][second->x][second->y] > maxDist) {
                        x = i;
                        y = j;
                        maxDist = distances[i][j][first->x][first->y] 
                                    + distances[i][j][second->x][second->y];
                        minDiff = abs(distances[i][j][first->x][first->y]
                                    - distances[i][j][second->x][second->y]);
                    } else if (distances[i][j][first->x][first->y] 
                            + distances[i][j][second->x][second->y] == maxDist
                            && abs(distances[i][j][first->x][first->y]
                            - distances[i][j][second->x][second->y]) 
                            < minDiff) {
                        x = i;
                        y = j;
                        maxDist = distances[i][j][first->x][first->y] 
                                    + distances[i][j][second->x][second->y];
                        minDiff = abs(distances[i][j][first->x][first->y]
                                    - distances[i][j][second->x][second->y]);
                    }
                } 
            }
        }

        map[x][y] = SNIPPET;
        int k = snippetsList->nSnippets++;
        Snippet snippet;
        snippet.x = x;
        snippet.y = y;
        snippet.visited = false;
        snippet.appearRound = round;
        snippetsList->snippets[k] = snippet;
        *spawnX = x;
        *spawnY = y;
    }

    inline void eraseSearchSnippet(int x, int y) {
        map[x][y] = FREE_CELL;
        for (int i = 0; i < snippetsList->nSnippets; i++) {
            if (snippetsList->snippets[i].x == x
                    && snippetsList->snippets[i].y == y) {
                snippetsList->snippets[i] = snippetsList->snippets[--snippetsList->nSnippets];
                return;
            }
        }

        for (int i = 0; i < history[ply].snippetsList->nSnippets; i++) {
            if (history[ply].snippetsList->snippets[i].x == x
                    && history[ply].snippetsList->snippets[i].y == y) {
                history[ply].snippetsList->snippets[i] = 
                        history[ply].snippetsList->snippets[--history[ply].snippetsList->nSnippets];
                return;
            }
        }

        ASSERT(false);

    }

    inline void spawnWeapon() {
        int x[300];
        int y[300];
        int n = 0;

        for (int i = 0; i < HEIGHT + 2; i++) {
            for (int j = 0; j < WIDTH + 2; j++) {
                if (map[i][j] == FREE_CELL) {
                    x[n  ] = i;
                    y[n++] = j;
                } 
            }
        }

        ASSERT(n <= 300);

        int r = fastrand() % n;
        map[x[r]][y[r]] = WEAPON;
        int k = weaponsList->nSnippets++;
        Snippet weapon;
        weapon.x = x[r];
        weapon.y = y[r];
        weapon.visited = false;
        weapon.appearRound = round;
        weaponsList->snippets[k] = weapon;
    }

    inline void collideWithPlayers() {
        // Collide by swap ans same pos
        if ((first->x == second->x && first->y == second->y)
                || (first->x == second->prevX && first->y == second->prevY
                        && second->x == first->prevX && second->y == first->prevY)) {
            if (first->has_weapon && second->has_weapon) {
                first->has_weapon = false;
                second->has_weapon = false;
                first->snippets -= HIT_SNIPPETS_LOSS;
                second->snippets -= HIT_SNIPPETS_LOSS;
                first->is_paralyzed = true;
                second->is_paralyzed = true;
            }

            if (first->has_weapon) {
                first->has_weapon = false;
                first->killedBugs++;
                second->snippets -= HIT_SNIPPETS_LOSS;
                second->is_paralyzed = true;
            }

            if (second->has_weapon) {
                second->has_weapon = false;
                second->killedBugs++;
                first->snippets -= HIT_SNIPPETS_LOSS;
                first->is_paralyzed = true;
            }
        }
    }
};

#endif // #ifndef STATE_H_INCLUDED
