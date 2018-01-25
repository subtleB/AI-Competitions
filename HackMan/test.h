#ifndef TEST_H_INCLUDED
#define TEST_H_INCLUDED

#include <iostream>
#include <string>

#include "defs.h"
#include "fastrand.h"
#include "state.h"

typedef struct {
    int id;
    int posRound;
    Player first;
    Player second;
    SnippetsList snippetsList[1];
    SnippetsList weaponsList[1];
    BugsList bugsList[1];
    Direction answer;
} TestCase;

inline bool calculateBugsPrevPosition(State *state, BugsList *prevList, BugsList *list) {
    int done = 0;
    for (int i = 0; i < list->nBugs; i++) {
        int count = 0;
        int index = 0;
        for (int j = 0; j < prevList->nBugs; j++) {
            int bugX = list->bugs[i].x;
            int bugY = list->bugs[i].y;

            int prevBugX = prevList->bugs[j].x;
            int prevBugY = prevList->bugs[j].y;

            for (int k = 0; k < 4; k++) {
                if (bugX == prevBugX + dx[k] && bugY == prevBugY + dy[k]) {

                    if (1 == count && prevList->bugs[index].x == prevBugX 
                        && prevList->bugs[index].y == prevBugY) {
                        continue;
                    }

                    count++;
                    index = j;
                }
            }
        }
        // Bug found
        if (1 == count) {
            list->bugs[i].prevX = prevList->bugs[index].x;
            list->bugs[i].prevY = prevList->bugs[index].y;
            prevList->bugs[index] = prevList->bugs[--prevList->nBugs];
            done++;
        }
    }


    if ((prevList->nBugs > 0 && done == list->nBugs) || prevList->nBugs == 0) {
        return true;
    }

    int count = 0;
    bool visited[list->nBugs];
    for (int j = 0; j < list->nBugs; j++) {
        visited[j] = false;
    }

    for (int j = 0; j < list->nBugs; j++) {
        for (int i = 0; i < prevList->nBugs; i++) {

            int x1 = INF, y1 = INF;
            int mandotory = 0;
            for (int k = 0; k < 4; k++) {
                if (state->map[prevList->bugs[i].x + dx[k]]
                              [prevList->bugs[i].y + dy[k]] != WALL
                        && (prevList->bugs[i].x + dx[k] != prevList->bugs[i].prevX 
                         || prevList->bugs[i].y + dy[k] != prevList->bugs[i].prevY)) {
                    x1 = dx[k];
                    y1 = dy[k];
                    mandotory++;
                }
            }

            if ((mandotory == 1) && (list->bugs[j].x == (prevList->bugs[i].x + x1))
                && (list->bugs[j].y == (prevList->bugs[i].y + y1)) && !visited[i]) {
                list->bugs[j].prevX = prevList->bugs[i].x;
                list->bugs[j].prevY = prevList->bugs[i].y;
                count++;
                visited[i] = true;
                break;
            }
        }
    }

    return prevList->nBugs - count == 0 ? true : false;
}

Bug* bugFound(BugsList *list, int x, int y) {
    for (int i = 0; i < list->nBugs; i++) {
        int bugX = list->bugs[i].x;
        int bugY = list->bugs[i].y;
        for (int j = 0; j < 4; j++) {
            if (bugX == x + dx[j] && bugY == y + dy[j]) {
                return &list->bugs[i];
            }
        }
    }
    return NULL;
}

inline void updateField(State *state, string s) {
    state->snippetsList->nSnippets = 0;
    BugsList bugsList[1];
    bugsList->nBugs = state->bugsList->nBugs;

    for (int i = 0; i < bugsList->nBugs; i++) {
        bugsList->bugs[i].x = state->bugsList->bugs[i].x;
        bugsList->bugs[i].y = state->bugsList->bugs[i].y;
        bugsList->bugs[i].prevX = state->bugsList->bugs[i].prevX;
        bugsList->bugs[i].prevY = state->bugsList->bugs[i].prevY;
    }

    state->bugsList->nBugs = 0;

    int n = s.length();
    int i = 0;
    for (int x = 1; x < HEIGHT + 1; x++) {
        for (int y = 1; y < WIDTH + 1; y++) {
            while (true) {
                if (i == n || s[i] == ',') {
                    i++;
                    break;
                }
                char c = s[i++];
                if (c == 'x') {
                    state->map[x][y] = WALL;
                } else if (c == '.') {
                    state->map[x][y] = FREE_CELL;
                } else if (c == 'E') {
                    if ((x == 8 && (y == 9 || y == 10 || y == 11 || y == 12))
                            || (x == 7 && y == 10) || (x == 7 && y == 11)) {
                        state->map[x][y] = WALL;
                    } else {
                        state->map[x][y] = FREE_CELL;
                    }
                    int n = state->bugsList->nBugs;
                    state->bugsList->bugs[n].x = x;
                    state->bugsList->bugs[n].y = y;

                    Bug *bug = bugFound(bugsList, x, y);

                    if (NULL != bug) {
                        state->bugsList->bugs[n].prevX = bug->x;
                        state->bugsList->bugs[n].prevY = bug->y;
                        ASSERT(bug->x > 0 && bug->x < HEIGHT + 1);
                        ASSERT(bug->y > 0 && bug->y < WIDTH + 1);
                    } else {
                        state->bugsList->bugs[n].prevX = 0;
                        state->bugsList->bugs[n].prevY = 0;
                    }

                    state->bugsList->nBugs++;
                } else if (c == 'W') {
                    state->map[x][y] = WEAPON;
                } else if (c == 'C') {
                    state->map[x][y] = SNIPPET;
                    int n = state->snippetsList->nSnippets++;
                    Snippet s;
                    s.x = x;
                    s.y = y;
                    s.visited = false;
                    s.appearRound = state->round;
                    state->snippetsList->snippets[n] = s;
                } else {
                    // played id
                    int id = c - '0';
                    if (id == PLAYER0) {
                        state->first->x = x;
                        state->first->y = y;
                        state->first->prevX = 0;
                        state->first->prevY = 0;
                        state->map[x][y] = FREE_CELL;
                    } else {
                        state->second->x = x;
                        state->second->y = y;
                        state->second->prevX = 0;
                        state->second->prevY = 0;
                        state->map[x][y] = FREE_CELL;
                    }
                }
            }
        }
    }
}

inline bool checkTestCase(TestCase test) {
    printf("Start test number %d...\n", test.id);
    State *state = new State();
    //initHashTable(state->hashTable, 128);
    updateField(state, ".,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,x,x,x,x,x,.,x,x,x,x,x,x,.,x,x,x,x,x,.,.,x,.,.,.,.,.,x,x,x,x,x,x,.,.,.,.,.,x,.,.,x,.,x,x,x,.,.,.,x,x,.,.,.,x,x,x,.,x,.,.,.,.,.,.,x,x,x,.,x,x,.,x,x,x,.,.,.,.,.,.,x,x,x,.,x,.,.,.,.,.,.,.,.,x,.,x,x,x,.,.,.,.,x,.,x,.,x,x,x,x,x,x,.,x,.,x,.,.,.,x,x,0,x,.,.,.,x,x,x,x,x,x,.,.,.,x,1,x,x,.,.,.,x,x,x,.,x,x,x,x,x,x,.,x,x,x,.,.,.,.,x,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,x,.,.,x,x,x,.,x,x,x,x,x,x,x,x,x,x,.,x,x,x,.,.,x,x,x,.,.,.,.,.,.,.,.,.,.,.,.,x,x,x,.,.,x,x,x,.,x,x,x,.,x,x,.,x,x,x,.,x,x,x,.,.,.,.,.,.,.,.,.,.,x,x,.,.,.,.,.,.,.,.,.");
    state->myId = 0; 

    state->round = test.posRound;
    state->first->x = test.first.x;
    state->first->y = test.first.y;
    state->first->prevX = test.first.prevX;
    state->first->prevY = test.first.prevY;
    state->first->snippets = test.first.snippets;
    state->first->has_weapon = test.first.has_weapon;

    state->second->x = test.second.x;
    state->second->y = test.second.y;
    state->second->prevX = test.second.prevX;
    state->second->prevY = test.second.prevY;
    state->second->snippets = test.second.snippets;
    state->second->has_weapon = test.second.has_weapon;

    for (int i = 0; i < test.snippetsList->nSnippets; i++) {
        Snippet s = test.snippetsList->snippets[i];
        state->map[s.x][s.y] = SNIPPET;
        int n = state->snippetsList->nSnippets++;
        s.visited = false;
        state->snippetsList->snippets[n] = s;  
    }

    for (int i = 0; i < test.weaponsList->nSnippets; i++) {
        Snippet s = test.weaponsList->snippets[i];
        state->map[s.x][s.y] = WEAPON;
        int n = state->weaponsList->nSnippets++;
        s.visited = false;
        state->weaponsList->snippets[n] = s;  
    }

    for (int i = 0; i < test.bugsList->nBugs; i++) {
        Bug bug = test.bugsList->bugs[i];
        int n = state->bugsList->nBugs++;
        state->bugsList->bugs[n] = bug;
    }

    SearchInfo info[1];
    info->timeset = false;
    info->debug = false;
    info->depth = 8;
    Move move0 = searchPosition(state, info, 0);

    if(move0.direction == test.answer) {
        printf("Test number %d passed\n", test.id);
    } else {
        printf("ERROR: Test number %d failed! Expected %s Given %s\n",
                 test.id, output[test.answer].c_str(), output[move0.direction].c_str());
        state->printMap();
    }
}

TestCase bugPrediction(int id) {
    TestCase test;
    test.id = id;
    test.posRound = 37;
    test.answer = LEFT;

    test.first.x = 10;
    test.first.y = 14;
    test.first.prevX = 10;
    test.first.prevY = 15;
    test.first.snippets = 3;
    test.first.has_weapon = false;

    test.second.x = 7;
    test.second.y = 14;
    test.second.prevX = 6;
    test.second.prevY = 14;
    test.second.snippets = 2;
    test.second.has_weapon = false;

    test.bugsList->nBugs = 1;
    test.bugsList->bugs[0].x = 6;
    test.bugsList->bugs[0].y = 13;
    test.bugsList->bugs[0].prevX = 6;
    test.bugsList->bugs[0].prevY = 12;

    test.weaponsList->nSnippets = 0;

    test.snippetsList->nSnippets = 1;
    test.snippetsList->snippets[0].x = 6;
    test.snippetsList->snippets[0].y = 11;

    return test;
}

TestCase doNotGoForHopelessSnip(int id) {
    TestCase test;
    test.id = id;
    test.posRound = 90;
    test.answer = LEFT;

    test.first.x = 12;
    test.first.y = 13;
    test.first.prevX = 12;
    test.first.prevY = 12;
    test.first.snippets = 7;
    test.first.has_weapon = true;

    test.second.x = 8;
    test.second.y = 18;
    test.second.prevX = 7;
    test.second.prevY = 18;
    test.second.snippets = 4;
    test.second.has_weapon = false;

    test.bugsList->nBugs = 2;
    test.bugsList->bugs[0].x = 7;
    test.bugsList->bugs[0].y = 19;
    test.bugsList->bugs[0].prevX = 7;
    test.bugsList->bugs[0].prevY = 20;

    test.bugsList->bugs[1].x = 7;
    test.bugsList->bugs[1].y = 17;
    test.bugsList->bugs[1].prevX = 6;
    test.bugsList->bugs[1].prevY = 17;

    test.weaponsList->nSnippets = 0;

    test.snippetsList->nSnippets = 2;
    test.snippetsList->snippets[0].x = 8;
    test.snippetsList->snippets[0].y = 16;
    test.snippetsList->snippets[1].x = 14;
    test.snippetsList->snippets[1].y = 4;

    return test;
}

inline void runTests() {
    checkTestCase(bugPrediction(0));
    checkTestCase(doNotGoForHopelessSnip(1));
}

#endif //#ifndef TEST_H_INCLUDED

