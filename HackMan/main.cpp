#pragma GCC optimize("-O3")
#pragma GCC optimize("inline")
#pragma GCC optimize("omit-frame-pointer")
#pragma GCC optimize("unroll-loops")

#include <iostream>
#include <algorithm>
#include <queue>
#include <vector>
#include <fstream>
#include <string>
#include <ctime>
#include "defs.h"
#include "state.h"
#include "fastrand.h"
#include "hash.h"
#include "search.h"
//#include "pvtable.h"
#include "misc.h"
#include "test.h"

using namespace std;

int width;
int height;
int timebank;
int time_per_move;
int time_remaining;
int max_rounds;
int current_round;


int pathLen[HEIGHT + 2][WIDTH + 2][HEIGHT + 2][WIDTH + 2];
BugsList prevBugsList[1];

void getInfluenceValue(int x, int y, int (&path)[HEIGHT + 2][WIDTH + 2], State *state) {
    for (int i = 0; i < HEIGHT + 2; i++) {
        for (int j = 0; j < WIDTH + 2; j++) {
            path[i][j] = 0;
        }
    }

    queue<Point> q;

    q.push(Point(x, y));
    path[x][y] = 1;  
    
    while(!q.empty()) {
        Point top = q.front();
        q.pop();

        for (int i = 0; i < 4; i++) {
            Point cur = Point(top.first + dx[i], top.second + dy[i]);
            if (state->map[cur.first][cur.second] != WALL
                    && path[cur.first][cur.second] == 0) {
                q.push(cur);
                path[cur.first][cur.second] = path[top.first][top.second] + 1;  
            }
        }   
    }
}

void initInfluenceTable(State *state) {

    ofstream myfile;
      myfile.open("lookup-table.txt");

    for (int i = 0; i < HEIGHT + 2; i++) {
        for (int j = 0; j < WIDTH + 2; j++) {
            for (int k = 0; k < HEIGHT + 2; k++) {
                for (int t = 0; t < WIDTH + 2; t++) {

                    if (state->map[i][j] == WALL 
                        || state->map[k][t] == WALL) {
                        myfile << "0,";
                        continue;
                    }

                    if (i == k && j == t) {
                        myfile << "0,";
                        continue;
                    }

                    int pathFirst[HEIGHT + 2][WIDTH + 2];
                    int pathSecond[HEIGHT + 2][WIDTH + 2];

                    getInfluenceValue(i, j, pathFirst, state);
                    getInfluenceValue(k, t, pathSecond, state);

                    int sum = 0;
                    for (int f = 0; f < HEIGHT + 2; f++) {
                        for (int g = 0; g < WIDTH + 2; g++) {
                            if (pathFirst[f][g] < pathSecond[f][g]) {
                                sum++;
                            } else if (pathFirst[f][g] > pathSecond[f][g]) {
                                sum--;
                            }
                        }
                    }

                    printf("(%d %d) (%d %d) influence = %d\n\n",  i, j, k, t, sum);
                    myfile << sum << ",";
                }
            }
        }
    }
    myfile.close();
}


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

void getSpace(int x, int y, int (&path)[HEIGHT + 2][WIDTH + 2], State *state, int len) {
    for (int i = 0; i < HEIGHT + 2; i++) {
        for (int j = 0; j < WIDTH + 2; j++) {
            path[i][j] = 0;
        }
    }

    queue<Point> q;

    q.push(Point(x, y));
    path[x][y] = 1;  
    
    while(!q.empty()) {
        Point top = q.front();
        q.pop();

        for (int i = 0; i < 4; i++) {
            Point cur = Point(top.first + dx[i], top.second + dy[i]);
            if (state->map[cur.first][cur.second] != WALL
                    && path[cur.first][cur.second] == 0) {
                q.push(cur);

                if (path[top.first][top.second] + 1 > len) {
                    return;
                }

                path[cur.first][cur.second] = path[top.first][top.second] + 1;  
            }
        }   
    }
}


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

int getDistance(int x, int y, int x1, int y1, int (&path)[HEIGHT + 2][WIDTH + 2], State *state) {
    for (int i = 0; i < HEIGHT + 2; i++) {
        for (int j = 0; j < WIDTH + 2; j++) {
            path[i][j] = 0;
        }
    }

    queue<Point> q;

    q.push(Point(x, y));
    path[x][y] = 1;  
    
    while(!q.empty()) {
        Point top = q.front();
        q.pop();

        if (top.first == x1 && top.second == y1) {
            return path[top.first][top.second];
        }

        for (int i = 0; i < 4; i++) {
            Point cur = Point(top.first + dx[i], top.second + dy[i]);
            if (state->map[cur.first][cur.second] != WALL
                    && path[cur.first][cur.second] == 0) {
                q.push(cur);
                path[cur.first][cur.second] = path[top.first][top.second] + 1;  
            }
        }   
    }

    ASSERT(false);
    return 0;
}

void initDistanceTable(State *state) {

    ofstream myfile;
    myfile.open("dist-table.txt");

    for (int i = 0; i < HEIGHT + 2; i++) {
        for (int j = 0; j < WIDTH + 2; j++) {
            for (int k = 0; k < HEIGHT + 2; k++) {
                for (int t = 0; t < WIDTH + 2; t++) {

                    if (state->map[i][j] == WALL 
                        || state->map[k][t] == WALL) {
                        myfile << "0,";
                        continue;
                    }

                    if (i == k && j == t) {
                        myfile << "0,";
                        continue;
                    }

                    int path[HEIGHT + 2][WIDTH + 2];

                    int dist = getDistance(i, j, k, t, path, state);
                    myfile << dist << ",";
                }
            }
        }
    }
    myfile.close();
}

void process_next_command(State* state) {
    string command;

    cin >> command;
    if (command == "settings") {
        string type;
        cin >> type;
        if (type == "timebank") {
            cin >> timebank;
        } else if (type == "time_per_move") {
            cin >> time_per_move;
        } else if (type == "player_names") {
            string names;
            cin >> names;
            // player names aren't very useful
        } else if (type == "your_bot") {
            cin >> state->myName;
        } else if (type == "your_botid") {
            cin >> state->myId;
        } else if (type == "field_width") {
            cin >> width;
        } else if (type == "field_height") {
            cin >> height;
        } else if (type == "max_rounds") {
            cin >> max_rounds;
        }
    } else if (command == "update") {
        string player_name, type;
        cin >> player_name >> type;
        if (type == "round") {
            cin >> state->round;
        } else if (type == "field") {
            string s;
            cin >> s;

            state->snippetsList->nSnippets = 0;
            state->weaponsList->nSnippets = 0;
            prevBugsList->nBugs = state->bugsList->nBugs;

            for (int i = 0; i < prevBugsList->nBugs; i++) {
                prevBugsList->bugs[i].x = state->bugsList->bugs[i].x;
                prevBugsList->bugs[i].y = state->bugsList->bugs[i].y;
                prevBugsList->bugs[i].prevX = state->bugsList->bugs[i].prevX;
                prevBugsList->bugs[i].prevY = state->bugsList->bugs[i].prevY;
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
                            state->bugsList->bugs[n].prevX = 0;
                            state->bugsList->bugs[n].prevY = 0;
                            state->bugsList->nBugs++;
                        } else if (c == 'W') {
                            state->map[x][y] = WEAPON;
                            int n = state->weaponsList->nSnippets++;
                            Snippet weapon;
                            weapon.x = x;
                            weapon.y = y;
                            weapon.visited = FALSE;
                            weapon.appearRound = state->round;
                            state->weaponsList->snippets[n] = weapon;
                        } else if (c == 'C') {
                            state->map[x][y] = SNIPPET;
                            int n = state->snippetsList->nSnippets++;
                            Snippet s;
                            s.x = x;
                            s.y = y;
                            s.visited = FALSE;
                            s.appearRound = state->round;
                            state->snippetsList->snippets[n] = s;
                        } else {
                            // played id
                            int id = c - '0';
                            if (id == PLAYER0) {
                                state->first->prevX = state->first->x;
                                state->first->prevY = state->first->y;
                                state->first->x = x;
                                state->first->y = y;
                                state->map[x][y] = FREE_CELL;
                            } else {
                                state->second->prevX = state->second->x;
                                state->second->prevY = state->second->y;
                                state->second->x = x;
                                state->second->y = y;
                                state->map[x][y] = FREE_CELL;
                            }
                        }
                    }
                }
            }
        } else if (type == "snippets") {
            double snippets;
            cin >> snippets;
            if (player_name == state->myName) {
                if (state->myId == PLAYER0) {
                    state->first->snippets = snippets;
                } else {
                    state->second->snippets = snippets;
                }
            } else {
                if (state->myId == PLAYER0) {
                    state->second->snippets = snippets;
                } else {
                    state->first->snippets = snippets;
                }
            }
        } else if (type == "has_weapon") {
            string value;
            cin >> value;
            if (player_name == state->myName) {
                if (state->myId == PLAYER0) {
                    state->first->has_weapon = (value == "true");
                } else {
                    state->second->has_weapon = (value == "true");
                }
            } else {
                if (state->myId == PLAYER0) {
                    state->second->has_weapon = (value == "true");
                } else {
                    state->first->has_weapon = (value == "true");
                }
            }
        } else if (type == "is_paralyzed") {
            string value;
            cin >> value;
            if (player_name == state->myName) {
                if (state->myId == PLAYER0) {
                    state->first->is_paralyzed = (value == "true");
                } else {
                    state->second->is_paralyzed = (value == "true");
                }
            } else {
                if (state->myId == PLAYER0) {
                    state->second->is_paralyzed = (value == "true");
                } else {
                    state->first->is_paralyzed = (value == "true");
                }
            }
        }
    } else if (command == "action") {
        int iterations = 0;

        // Fancy way to predict last bugs positions
        while (!calculateBugsPrevPosition(state, prevBugsList, state->bugsList)) {
            fprintf(stderr, "calculateBugsPrevPosition\n");
            iterations++;

            if(iterations > 5) {
                fprintf(stderr, "ERROR\n");
                break;
            }
        }

        int playersSum  = (int)(state->first->snippets + state->second->snippets);
        if (state->snippetsEaten < playersSum) {
            state->snippetsEaten = playersSum;
        } else if (state->snippetsEaten > playersSum) {
            if (state->snippetsEaten == playersSum + 3) {
                state->snippetsEaten++;
            } else if (state->snippetsEaten == playersSum + 7) {
                state->snippetsEaten++;
            } else if (state->snippetsEaten == playersSum + 6) {
                state->snippetsEaten += 2;
            }
        }


        string useless_move;
        cin >> useless_move;
        cin >> time_remaining;

        SearchInfo info[1];
        info->depth = MAX_SEARCH_DEPTH - 5;
        info->timeset = true;
        info->starttime = getTimeMs();
        info->stoptime = info->starttime + 130;
        info->debug = true;

        state->first->centerBonus = 0;
        state->second->centerBonus = 0;

        if (info->debug) {
            state->printMap();
            cerr << "state eval = " << state->evaluate() << endl;
        }
        
        Move move = searchPosition(state, info, state->myId);

        cout << output[move.direction] << endl;
    }
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int main() {
    fast_srand((int)time(0));
    
    State *state = new State();

    //initHashKeys();
    //initHashTable(state->hashTable, 128);
    std::cerr << "init done" << std::endl;

    //runTests();

    while (true) {
        process_next_command(state);
    }

    return 0;
}

