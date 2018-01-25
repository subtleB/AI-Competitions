#pragma GCC optimize("-O3")
#pragma GCC optimize("inline")
#pragma GCC optimize("omit-frame-pointer")
#pragma GCC optimize("unroll-loops")

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <string>
#include <map>
#include <queue>

// Iterative deepeing will stop by time limit or not
#define SEARCH_USE_TIME

// Use hash table to speed up search
#define SEARCH_USE_HASH_TABLE

// Use aspiration window
//#define SEARCH_USE_ASPIRATION_WINDOW

// For assert
//#define DEBUG

// For slow assert
//#define FULL_DEBUG

#define PRODUCTION

int const MAX_SIZE = 9;
int const FIRST = 0;
int const SECOND = 1;
int const PLAYERS = 2;
int const SEARCH_TIME_CHECK_RATE = 31;
int const MAX_SEARCH_DEPTH  = 4;
int const MAX_MOVES = 256;
int const TIME_FOR_MOVE_MS = 200;
int const HASH_TABLE_MEMORY_MB = 128;
int const INF = 30000000;
int const WIN_SCORE = 300000;
int const WALL = -1;
int const FREE = 0;
int const TOP = 3;
int const FOR_ONE = 1;
int const FOR_BOTH = 2;

int const DIRECTIONS = 8;
int const dx[] = {1, 1,  1, -1, -1, -1, 0,  0};
int const dy[] = {1, 0, -1,  1,  0, -1, 1, -1};

int const dirX[8][3] = {{1, 1, 0}, {1, 1,  1},  { 1, 1,  0}, {-1, 0, -1},
                      {-1, -1, -1}, {-1, -1,  0}, {0, 1, -1}, { 0, -1,  1}};
int const dirY[8][3] = {{1, 0, 1}, {0, 1, -1},  {-1, 0, -1}, { 1, 1,  0},
                      { 0, -1,  1}, {-1,  0, -1}, {1, 1,  1}, {-1, -1, -1}};


std::string const out[] = {"SE", "E", "NE",  "SW",  "W", "NW", "S", "N"};
std::string const outType[] = {"MOVE&BUILD", "PUSH&BUILD"};

typedef unsigned long long U64;
typedef std::pair<int,int> Point;

enum {FALSE = 0, TRUE = 1};
enum {HFNONE, HFALPHA, HFBETA, HFEXACT};
enum {MOVE_BUILD = 0, PUSH_BUILD = 1};

using namespace std;

map<string, Point> dir;
map<string, string> dirOut; 

U64 zorbistHashMap[MAX_SIZE + 2][MAX_SIZE + 2][6];
U64 zorbistHashPlayer[MAX_SIZE + 2][MAX_SIZE + 2][2][2];
U64 zorbistHashTurn;

static unsigned int g_seed;
inline void fast_srand(int seed) {
    //Seed the generator
    g_seed = seed;
}
inline int fastrand() {
    //fastrand routine returns one integer, similar output value range as C lib.
    g_seed = (214013*g_seed+2531011);
    return (g_seed>>16)&0x7FFF;
}
inline int fastRandInt(int maxSize) {
    return fastrand() % maxSize;
}
inline int fastRandInt(int a, int b) {
    return(a + fastRandInt(b - a));
}
inline double fastRandDouble() {
    return static_cast<double>(fastrand()) / 0x7FFF;
}
inline double fastRandDouble(double a, double b) {
    return a + (static_cast<double>(fastrand()) / 0x7FFF)*(b-a);
}

#define RAND_64 ((unsigned long long)fastrand() | \
                (unsigned long long)fastrand() << 15 | \
                (unsigned long long)fastrand() << 30 | \
                (unsigned long long)fastrand() << 45 | \
                ((unsigned long long)fastrand() & 0xf) << 60 )

#ifdef WIN32
#include "windows.h"
#else
#include "sys/time.h"
#endif

inline int getTimeMs() { 
#ifdef WIN32
  return GetTickCount();
#else
  struct timeval t;
  gettimeofday(&t, NULL);
  return t.tv_sec*1000 + t.tv_usec/1000;
#endif
}

#ifndef DEBUG
#define ASSERT(n)
#else
#define ASSERT(n) \
if(!(n)) { \
printf("%s - Failed",#n); \
printf("On %s ",__DATE__); \
printf("At %s ",__TIME__); \
printf("In File %s ",__FILE__); \
printf("At Line %d\n",__LINE__); \
exit(1);}
#endif

#ifndef FULL_DEBUG
#define SLOW_ASSERT(n)
#else
#define SLOW_ASSERT(n) \
if(!(n)) { \
printf("%s - Failed",#n); \
printf("On %s ",__DATE__); \
printf("At %s ",__TIME__); \
printf("In File %s ",__FILE__); \
printf("At Line %d\n",__LINE__); \
exit(1);}
#endif

#ifndef PRODUCTION
#include <Windows.h>
inline void setColor(int text, int background) {
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hStdOut, (WORD)((background << 4) | text));
}
#else
inline void setColor(int text, int background) {
}
#endif

/*******************************************************************************************/
/*******************************************************************************************/
/*******************************************************************************************/
/*******************************************************************************************/
class Player {
    public:
    int x;
    int y;
    int proven;

    Player() {
        x = 0;
        y = 0;
        proven = 0;
    }

    Player(int x, int y) {
        this->x = x;
        this->y = y;
        proven = 0;
    }
};

class Move {
    public:
    int score;
    int x1;
    int y1;
    int x2;
    int y2;
    int type;
    int playerId;

    string toString() {
        return "Error";
    }

    string dir1() {
        if (x1 == 1 && y1 == 1) return "SE";
        if (x1 == 0 && y1 == 1) return "E";
        if (x1 == -1 && y1 == 1) return "NE";
        if (x1 == 1 && y1 == -1) return "SW";
        if (x1 == 0 && y1 == -1) return "W";
        if (x1 == -1 && y1 == -1) return "NW";
        if (x1 == 1 && y1 == 0) return "S";
        if (x1 == -1 && y1 == 0) return "N";
        return "ERROR";
    }

    string dir2() {
        if (x2 == 1 && y2 == 1) return "SE";
        if (x2 == 0 && y2 == 1) return "E";
        if (x2 == -1 && y2 == 1) return "NE";
        if (x2 == 1 && y2 == -1) return "SW";
        if (x2 == 0 && y2 == -1) return "W";
        if (x2 == -1 && y2 == -1) return "NW";
        if (x2 == 1 && y2 == 0) return "S";
        if (x2 == -1 && y2 == 0) return "N";
        return "ERROR";
    }
};

class MovesList {
    public:

    Move moves[MAX_MOVES];
    int count;

    MovesList() {
        this->count = 0;
    }
};

class Settings {
    public:

    int depth;
    int nodes;
    int stopped;
    int debug;
    int timeset;
    int stoptime;
    int nullCut;

    Settings() {
        depth = MAX_SEARCH_DEPTH;
        nodes = 0;
        stopped = 0;
        debug = 0;
        timeset = 0;
        stoptime = 0;
        nullCut = 0;
    }

    void refresh() {
        stopped = FALSE;
        nodes = 0;
    }
};
/*******************************************************************************************/
/*******************************************************************************************/
/*******************************************************************************************/
/*******************************************************************************************/

class HashEntry {
    public:
    U64 stateKey;
    Move move;
    int score;
    int depth;
    int flags;
};

class HashTable {
    private:
    HashEntry *hashEntry;
    int numEntries;
    int newWrite;
    int overWrite;
    int hit;

    public:
    int cut;

    HashTable(const int mb) {
        initHashTable(mb);
    };

    void clearHashTable() {
      for (HashEntry *e = this->hashEntry; e < this->hashEntry + this->numEntries; e++) {
        e->stateKey = 0ULL;
        Move nomove;
        e->move = nomove;
        e->depth = 0;
        e->score = 0;
        e->flags = 0;
      }
      this->newWrite = 0;
    };

    void initHashTable(const int mb) {  
        int hashSize = 0x100000 * mb;
        this->numEntries = hashSize / sizeof(HashEntry);
        this->numEntries -= 2;
        
        if (this->hashEntry != NULL) {
            free(this->hashEntry);
        }
            
        this->hashEntry = (HashEntry*) malloc(this->numEntries * sizeof(HashEntry));

        if (this->hashEntry == NULL) {
            initHashTable(mb / 2);
        } else {
            clearHashTable();
        }
    };

    int probeHashEntry(Move *move, U64 stateHash, int ply, int *score, int alpha, int beta, int depth) {
        int index = stateHash % ((U64)this->numEntries);

        ASSERT(index >= 0 && index <= this->numEntries - 1);
        ASSERT(depth >= 1 && depth <= MAX_SEARCH_DEPTH * 2);
        ASSERT(alpha < beta);
        ASSERT(alpha >= -INF && alpha <= INF);
        ASSERT(beta >= -INF && beta <= INF);
        ASSERT(ply >= 0 && ply <= MAX_SEARCH_DEPTH * 2);

        if (this->hashEntry[index].stateKey == stateHash) {
            *move = this->hashEntry[index].move;

            if (this->hashEntry[index].depth >= depth) {
                this->hit++;
            
                ASSERT(this->hashEntry[index].depth >=1 
                       && this->hashEntry[index].depth <= MAX_SEARCH_DEPTH * 2);
                ASSERT(this->hashEntry[index].flags >= HFALPHA 
                       && this->hashEntry[index].flags <= HFEXACT);
                
                *score = this->hashEntry[index].score;

                if (*score > WIN_SCORE) {
                    *score -= ply;
                } else if (*score < -WIN_SCORE) {
                    *score += ply;
                }
                
                switch(this->hashEntry[index].flags) {
                    ASSERT(*score >= -INF && *score <= INF);
                    case HFALPHA: 
                        if (*score <= alpha) {
                            *score = alpha;
                            return TRUE;
                        }
                        break;
                    case HFBETA: 
                        if(*score >= beta) {
                            *score = beta;
                            return TRUE;
                        }
                        break;
                    case HFEXACT:
                        return TRUE;
                        break;
                    default: ASSERT(FALSE); break;
                }
            }
        }
        return FALSE;
    };


    void storeHashEntry(const Move move, const U64 stateHash, const int ply,
                int score, const int flags, const int depth) {
        int index = stateHash % ((U64)this->numEntries);

        ASSERT(index >= 0 && index <= this->numEntries - 1);
        ASSERT(depth >= 1 && depth <= MAX_SEARCH_DEPTH * 2);
        ASSERT(flags >= HFALPHA && flags <= HFEXACT);
        ASSERT(score >= -INF && score <= INF);
        ASSERT(ply >= 0 && ply <= MAX_SEARCH_DEPTH * 2);
        
        if (0 == this->hashEntry[index].stateKey) {
            this->newWrite++;
        } else {
            this->overWrite++;
        }
        
        if (score > WIN_SCORE) {
            score += ply;
        } else if (score < -WIN_SCORE) {
            score -= ply;   
        }

        this->hashEntry[index].move = move;
        this->hashEntry[index].stateKey = stateHash;
        this->hashEntry[index].flags = flags;
        this->hashEntry[index].score = score;
        this->hashEntry[index].depth = depth;
    };

    Move getBestMove(const U64 stateHash) {
        int index = stateHash % ((U64) this->numEntries);
        ASSERT(index >= 0 && index <= this->numEntries - 1);
        
        if(this->hashEntry[index].stateKey == stateHash) {
            return this->hashEntry[index].move;
        }
        
        Move nomove;
        nomove.x1 = 0;
        nomove.y1 = 0;
        return nomove;
    }
};


/*******************************************************************************************/
/*******************************************************************************************/
/*******************************************************************************************/
/*******************************************************************************************/


class State {
    public:

    int **map;

    int firstScore;
    int secondScore;

    Player *first[PLAYERS];
    Player *second[PLAYERS];

    int version;
    int oppSetByGuess;

    int playerToMove;
    int ply;
    int gameRound;
    int myId;
    int size;
    U64 hashKey;
    HashTable* hashTable;

    State(int size) {
        this->size = size;
        playerToMove = FIRST;
        ply = 0;
        gameRound = 0;
        oppSetByGuess = -1;
        firstScore = 0;
        secondScore = 0;
        hashTable = new HashTable(HASH_TABLE_MEMORY_MB);

        map = new int*[size + 2];
        for (int i = 0; i < size + 2; i++) {
            map[i] = new int[size + 2];
        }

        for (int i = 0; i < size + 2; i++) {
            for (int j = 0; j < size + 2; j++) {
                map[i][j] = WALL;
            }
        }

        initHash();
    }

    void getLegalMoves(MovesList *list) {
        Player **current = playerToMove == FIRST ? first : second;
        Player **opp = playerToMove == FIRST ? second : first;

        list->count = 0;

        for (int i = 0; i < PLAYERS; i++) {
            int x = current[i]->x;
            int y = current[i]->y;

            // Seems the player in the shadow
            if (x == 0 || y == 0) {
                continue;
            }

            int level = map[x][y];

            ASSERT(level >= 0 && level <= 3)

            // MOVE && BUILD
            for (int j = 0; j < DIRECTIONS; j++) {
                Move move;
                move.playerId = i;

                if (map[x + dx[j]][y + dy[j]] == WALL) continue;
                if (x + dx[j] == second[0]->x && y + dy[j] == second[0]->y) continue;
                if (x + dx[j] == second[1]->x && y + dy[j] == second[1]->y) continue;
                if (x + dx[j] == first[0]->x && y + dy[j] == first[0]->y) continue;
                if (x + dx[j] == first[1]->x && y + dy[j] == first[1]->y) continue;

                if (map[x + dx[j]][y + dy[j]] - 1 <= level) {
                    move.type = MOVE_BUILD;
                    move.x1 = dx[j];
                    move.y1 = dy[j];

                    current[i]->x += dx[j];
                    current[i]->y += dy[j];

                    int nx = x + dx[j];
                    int ny = y + dy[j];

                    // BUILD
                    for (int k = 0; k < DIRECTIONS; k++) {
                        if (map[nx + dx[k]][ny + dy[k]] == WALL) continue;
                        if (nx + dx[k] == second[0]->x && ny + dy[k] == second[0]->y) continue;
                        if (nx + dx[k] == second[1]->x && ny + dy[k] == second[1]->y) continue;
                        if (nx + dx[k] == first[0]->x && ny + dy[k] == first[0]->y) continue;
                        if (nx + dx[k] == first[1]->x && ny + dy[k] == first[1]->y) continue;

                        move.x2 = dx[k];
                        move.y2 = dy[k];
                        move.score = map[nx][ny] 
                                + (map[nx + dx[k]][ny + dy[k]] + 1) % 4;

                        list->moves[list->count++] = move;
                    }

                    current[i]->x -= dx[j];
                    current[i]->y -= dy[j];
                }
            }

            // PUSH & BUILD
            for (int j = 0; j < DIRECTIONS; j++) {
                Move move;
                move.playerId = i;

                if (map[x + dx[j]][y + dy[j]] == WALL) continue;
                if ((opp[0]->x != x + dx[j] || opp[0]->y != y + dy[j]) 
                    && (opp[1]->x != x + dx[j] || opp[1]->y != y + dy[j])) {
                    continue;
                }
               
                move.type = PUSH_BUILD;
                move.x1 = dx[j];
                move.y1 = dy[j];

                int tx = x + dx[j];
                int ty = y + dy[j];

                // BUILD
                for (int k = 0; k < 3; k++) {
                    if (map[tx + dirX[j][k]][ty + dirY[j][k]] == WALL) continue;
                    if (tx + dirX[j][k] == second[0]->x && ty + dirY[j][k] == second[0]->y) continue;
                    if (tx + dirX[j][k] == second[1]->x && ty + dirY[j][k] == second[1]->y) continue;
                    if (tx + dirX[j][k] == first[0]->x && ty + dirY[j][k] == first[0]->y) continue;
                    if (tx + dirX[j][k] == first[1]->x && ty + dirY[j][k] == first[1]->y) continue;

                    if (map[tx + dirX[j][k]][ty + dirY[j][k]] - 1 <= map[tx][ty]) {
                        move.x2 = dirX[j][k];
                        move.y2 = dirY[j][k];
                        move.score = (map[tx][ty] * map[tx][ty])
                                    - map[tx + dirX[j][k]][ty + dirY[j][k]];
                        list->moves[list->count++] = move;
                    }
                } 
            }
        }

        return;
    };


    int activityValue(Player *player) const {
        int activity = 0;
        for (int i = 0; i < DIRECTIONS; i++) {
            if (WALL == map[player->x + dx[i]][player->y + dy[i]]) {
                continue;
            }

            if (map[player->x + dx[i]][player->y + dy[i]] - 1 
                        > map[player->x][player->y]) {
                continue;
            }

            for (int j = 0; j < DIRECTIONS; j++) {
                if (WALL == map[player->x + dx[i] + dx[j]]
                               [player->y + dy[i] + dy[j]]) {
                    continue;
                }

                if (map[player->x + dx[i] + dx[j]]
                       [player->y + dy[i] + dy[j]] - 1 
                            > map[player->x + dx[i]][player->y + dy[i]]) {
                    continue;
                }

                activity++;
            }
            activity += map[player->x + dx[i]][player->y + dy[i]];
        } 
        activity += map[player->x][player->y] * 2;
        return activity;  
    }

    int getDistances() const {
        int tempMap[MAX_SIZE][MAX_SIZE];
        memset(tempMap, FREE, sizeof(tempMap[0][0]) * MAX_SIZE * MAX_SIZE);

        queue<Point> q;
        q.push(Point(first[0]->x, first[0]->y));
        q.push(Point(first[1]->x, first[1]->y));
        tempMap[first[0]->x][first[0]->y] = 1; 
        tempMap[first[1]->x][first[1]->y] = 1;
        int ans = 0;

        while (!q.empty()) {
            Point top = q.front();
            q.pop();

            for (int i = 0; i < DIRECTIONS; i++) {
                Point cur = Point(top.first + dx[i], top.second + dy[i]);
                if (map[cur.first][cur.second] != WALL
                        && tempMap[cur.first][cur.second] == FREE
                        && (map[top.first][top.second] >= map[cur.first][cur.second] - 1)
                        && !(cur.first == second[0]->x && cur.second == second[0]->y)
                        && !(cur.first == second[1]->x && cur.second == second[1]->y)) {
                    q.push(cur);
                    tempMap[cur.first][cur.second] = 1 + tempMap[top.first][top.second];
                    ans++;
                }
            }
        }

        return  ans;
    }

    int getDistancesOpp() const {
        int tempMap[MAX_SIZE][MAX_SIZE];
        memset(tempMap, FREE, sizeof(tempMap[0][0]) * MAX_SIZE * MAX_SIZE);

        queue<Point> q;

        if (second[0]->x != 0) {
            q.push(Point(second[0]->x, second[0]->y));
            tempMap[second[0]->x][second[0]->y] = 1; 
        }

        if (second[1]->x != 0) {
            q.push(Point(second[1]->x, second[1]->y));
            tempMap[second[1]->x][second[1]->y] = 1; 
        }

        int ans = 0;

        while (!q.empty()) {
            Point top = q.front();
            q.pop();

            for (int i = 0; i < DIRECTIONS; i++) {
                Point cur = Point(top.first + dx[i], top.second + dy[i]);
                if (map[cur.first][cur.second] != WALL
                        && tempMap[cur.first][cur.second] == FREE
                        && (map[top.first][top.second] >= map[cur.first][cur.second] - 1)
                        && !(cur.first == first[0]->x && cur.second == first[0]->y)
                        && !(cur.first == first[1]->x && cur.second == first[1]->y)) {
                    q.push(cur);
                    tempMap[cur.first][cur.second] = 1 + tempMap[top.first][top.second];
                    ans++;
                }
            }
        }

        return  ans;
    }

    int evaluate(int mode) const {
        int score = 0;

        int firstActivity1 = activityValue(first[0]);
        int firstActivity2 = activityValue(first[1]);
        

        int secondActivity1 = 0;
        int secondActivity2 = 0;

        if (second[0]->x != 0) {
            secondActivity1 = activityValue(second[0]);
        }

        if (second[1]->x != 0) {
            secondActivity2 = activityValue(second[1]);
        }

        score += firstActivity1 + firstActivity2;
        score -= secondActivity1 + secondActivity2;
        score += firstScore * 2;
        score -= secondScore * 2;

        score += getDistances() * 10 - getDistancesOpp() * 10;

        if (FOR_ONE == mode) {
            return score;
        } else {
            return playerToMove == FIRST ? score : -score;
        }
    }

    void initHash() {
        zorbistHashTurn = RAND_64;
        for (int i = 1; i <= size; i++) {
            for (int j = 1; j <= size; j++) {
                for (int k = 0; k < 5; k++) {
                    zorbistHashMap[i][j][k] = RAND_64;
                }
                zorbistHashPlayer[i][j][0][0] = RAND_64;
                zorbistHashPlayer[i][j][0][1] = RAND_64;
                zorbistHashPlayer[i][j][1][0] = RAND_64;
                zorbistHashPlayer[i][j][1][1] = RAND_64;
            }
        }
    }

    U64 hash() const {
        U64 hash = 213;

        hash ^= firstScore;
        hash ^= secondScore;

        if (playerToMove == FIRST) {
            hash ^= zorbistHashTurn;
        }

        if (first[0]->x > 0) {
            hash ^= zorbistHashPlayer[first[0]->x][first[0]->y][0][0];
        }

        if (first[1]->x > 0) {
            hash ^= zorbistHashPlayer[first[1]->x][first[1]->y][0][1];
        }

        if (second[0]->x > 0) {
            hash ^= zorbistHashPlayer[second[0]->x][second[0]->y][1][0];
        }

        if (second[1]->x > 0) {
            hash ^= zorbistHashPlayer[second[1]->x][second[1]->y][1][1];
        }

        for (int i = 1; i <= size; i++) {
            for (int j = 1; j <= size; j++) {
                hash ^= zorbistHashMap[i][j][map[i][j] + 1];
            }
        }
        
        return hash;
    }

    void makeMove(Move move, int mode) {
        Player **current = playerToMove == FIRST ? first : second;
        Player **opp = playerToMove == FIRST ? second : first;
        Player *player = current[move.playerId];

        ASSERT(player->x >= 1 && player->y >= 1 && player->x <= size && player->y <= size);
        ASSERT(player->x + move.x1 >= 1 && player->y + move.y1  >= 1);
        ASSERT(player->x + move.x1 <= size && player->y + move.y1  <= size);
        ASSERT(map[player->x + move.x1 + move.x2][player->y + move.y1 + move.y2] != WALL);

        if (MOVE_BUILD == move.type) {

            // Update player hash
            hashKey ^= zorbistHashPlayer[player->x][player->y][playerToMove][move.playerId];
            player->x += move.x1;
            player->y += move.y1;
            hashKey ^= zorbistHashPlayer[player->x][player->y][playerToMove][move.playerId];

            // Update map hashKey
            hashKey ^= zorbistHashMap[player->x + move.x2][player->y + move.y2]
                    [map[player->x + move.x2][player->y + move.y2] + 1];

            map[player->x + move.x2][player->y + move.y2]++;
            if (4 == map[player->x + move.x2][player->y + move.y2]) {
                map[player->x + move.x2][player->y + move.y2] = WALL;
            }

            hashKey ^= zorbistHashMap[player->x + move.x2][player->y + move.y2]
                    [map[player->x + move.x2][player->y + move.y2] + 1];


            if (TOP == map[player->x][player->y]) {
                if (FIRST == playerToMove) {
                    hashKey ^= firstScore;
                    firstScore++;
                    hashKey ^= firstScore;
                } else {
                    hashKey ^= secondScore;
                    secondScore++;
                    hashKey ^= secondScore;
                }
            }

        } else {
            ASSERT(opp[0]->x == player->x + move.x1 && opp[0]->y == player->y + move.y1
                || opp[1]->x == player->x + move.x1 && opp[1]->y == player->y + move.y1);

            int index = (opp[1]->x == player->x + move.x1 
                         && opp[1]->y == player->y + move.y1);

            ASSERT(index == 0 || index == 1);
            ASSERT(opp[index]->x == player->x + move.x1 && opp[index]->y == player->y + move.y1)

            hashKey ^= zorbistHashMap[opp[index]->x][opp[index]->y]
                    [map[opp[index]->x][opp[index]->y] + 1];

            map[opp[index]->x][opp[index]->y]++;
            if (4 == map[opp[index]->x][opp[index]->y]) {
                map[opp[index]->x][opp[index]->y] = WALL;
            }

            hashKey ^= zorbistHashMap[opp[index]->x][opp[index]->y]
                    [map[opp[index]->x][opp[index]->y] + 1];

            hashKey ^= zorbistHashPlayer[opp[index]->x][opp[index]->y][playerToMove ^ 1][index];
            opp[index]->x += move.x2;
            opp[index]->y += move.y2;
            hashKey ^= zorbistHashPlayer[opp[index]->x][opp[index]->y][playerToMove ^ 1][index];
        }

        if (FOR_BOTH == mode) {
            playerToMove ^= 1;
            hashKey ^= zorbistHashTurn;
        }

        SLOW_ASSERT(hashKey == hash());
        return;
    }

    void takeMove(Move move, int mode) {
        if (FOR_BOTH == mode) {
            playerToMove ^= 1;
            hashKey ^= zorbistHashTurn;
        }

        Player **current = playerToMove == FIRST ? first : second;
        Player **opp = playerToMove == FIRST ? second : first;
        Player *player = current[move.playerId];

        ASSERT(player->x >= 1 && player->y >= 1 && player->x <= size && player->y <= size);

        if (MOVE_BUILD == move.type) {

            if (TOP == map[player->x][player->y]) {
                if (FIRST == playerToMove) {
                    hashKey ^= firstScore;
                    firstScore--;
                    hashKey ^= firstScore;
                } else {
                    hashKey ^= secondScore;
                    secondScore--;
                    hashKey ^= secondScore;
                }
            }

            ASSERT(player->x - move.x1 >= 1 && player->y - move.y1 >= 1);
            ASSERT(player->x - move.x1 <= size && player->y - move.y1  <= size);

            hashKey ^= zorbistHashMap[player->x + move.x2][player->y + move.y2]
                            [map[player->x + move.x2][player->y + move.y2] + 1];

            if (WALL == map[player->x + move.x2][player->y + move.y2]) {
                map[player->x + move.x2][player->y + move.y2] = 4;
            }
            map[player->x + move.x2][player->y + move.y2]--;

            hashKey ^= zorbistHashMap[player->x + move.x2][player->y + move.y2]
                            [map[player->x + move.x2][player->y + move.y2] + 1];

            hashKey ^= zorbistHashPlayer[player->x][player->y][playerToMove][move.playerId];
            player->x -= move.x1;
            player->y -= move.y1;
            hashKey ^= zorbistHashPlayer[player->x][player->y][playerToMove][move.playerId];

        } else {
            int index = (opp[1]->x == player->x + move.x1 + move.x2
                         && opp[1]->y == player->y + move.y1 + move.y2);

            hashKey ^= zorbistHashPlayer[opp[index]->x][opp[index]->y][playerToMove ^ 1][index];
            opp[index]->x -= move.x2;
            opp[index]->y -= move.y2;
            hashKey ^= zorbistHashPlayer[opp[index]->x][opp[index]->y][playerToMove ^ 1][index];

            ASSERT(index == 0 || index == 1);
            ASSERT(opp[index]->x == player->x + move.x1
                && opp[index]->y == player->y + move.y1)

            hashKey ^= zorbistHashMap[opp[index]->x][opp[index]->y]
                    [map[opp[index]->x][opp[index]->y] + 1];

            if (WALL == map[opp[index]->x][opp[index]->y]) {
                map[opp[index]->x][opp[index]->y] = 4;
            }
            map[opp[index]->x][opp[index]->y]--;

            hashKey ^= zorbistHashMap[opp[index]->x][opp[index]->y]
                    [map[opp[index]->x][opp[index]->y] + 1];

            ASSERT(opp[0]->x == player->x + move.x1 && opp[0]->y == player->y + move.y1
                || opp[1]->x == player->x + move.x1 && opp[1]->y == player->y + move.y1);
        }

        SLOW_ASSERT(hashKey == hash());
        return;
    }

    int oppIsVisible() {
        return second[0]->x != 0 || second[1]->x != 0;
    }

    void setValue(int i, int j, int value) {
        map[i][j] = value;
    }

    void predictOppPosition(int (&prevMap)[MAX_SIZE][MAX_SIZE], 
                                    Player tFirst[2], Player tSecond[2]) {

        // Nothing to do, can see both
        if (second[0]->proven && second[1]->proven) {
            return;
        }

        // If it was PUSH from opp
        if (tFirst[0].x != first[0]->x || tFirst[0].y != first[0]->y
                || tFirst[1].x != first[1]->x || tFirst[1].y != first[1]->y) {

            int px = (tFirst[0].x - first[0]->x) + (tFirst[1].x - first[1]->x);
            int py = (tFirst[0].y - first[0]->y) + (tFirst[1].y - first[1]->y);
            int d = 0;

            for (int i = 0; i < DIRECTIONS; i++) {
                if (dx[i] == px && dy[i] == py) {
                    d = i;
                    break;
                }
            }

            // Check if someone has been staying from the prev turn
            int second1 = abs(tSecond[0].x + tSecond[0].y) > 0;
            int second2 = abs(tSecond[1].x + tSecond[1].y) > 0;
            int x1 = 0;
            int y1 = 0;

            if (tFirst[0].x != first[0]->x || tFirst[0].y - first[0]->y) {
                x1 = tFirst[0].x;
                y1 = tFirst[0].y;
            } else {
                x1 = tFirst[1].x;
                y1 = tFirst[1].y;
            }

            if (tSecond[0].x == x1 && tSecond[0].y == y1) {
                second1 = FALSE;
            }

            if (tSecond[1].x == x1 && tSecond[1].y == y1) {
                second2 = FALSE;
            }

            // If we see prev pos than there is can't be enemy
            for (int i = 0; i < DIRECTIONS; i++) {
                if (tSecond[0].x + dx[i] == first[0]->x 
                        && tSecond[0].y + dy[i] == first[0]->y) {
                    second1 = FALSE;
                }

                if (tSecond[0].x + dx[i] == first[1]->x 
                        && tSecond[0].y + dy[i] == first[1]->y) {
                    second1 = FALSE;
                }

                if (tSecond[1].x + dx[i] == first[0]->x 
                        && tSecond[1].y + dy[i] == first[0]->y) {
                    second2 = FALSE;
                }

                if (tSecond[1].x + dx[i] == first[1]->x 
                        && tSecond[1].y + dy[i] == first[1]->y) {
                    second2 = FALSE;
                }
            }

            for (int i = 0; i < DIRECTIONS; i++) {

                if (WALL == map[x1 + dx[i]][y1 + dy[i]]) {
                    continue;
                }

                if (x1 + dx[i] == tSecond[0].x && y1 + dy[i] == tSecond[0].y) {
                    second1 = FALSE;
                }

                if (x1 + dx[i] == tSecond[1].x && y1 + dy[i] == tSecond[1].y) {
                    second2 = FALSE;
                }

                for (int j = 0; j < DIRECTIONS; j++) {
                    int nx = x1 + dx[i] + dx[j];
                    int ny = x1 + dy[i] + dy[j];

                    if (WALL == map[nx][ny]) {
                        continue;
                    }

                    if (nx == tSecond[0].x && ny == tSecond[0].y) {
                        second1 = FALSE;
                    }

                    if (nx == tSecond[1].x && ny == tSecond[1].y) {
                        second2 = FALSE;
                    }
                }
            }

            if (TRUE == second1) {
                // Skip if already here
                if (!(second[0]->x == tSecond[0].x && second[0]->y == tSecond[0].y 
                        || second[1]->x == tSecond[0].x && second[1]->y == tSecond[0].y)) {
                    if (!second[0]->proven) {
                        second[0]->x = tSecond[0].x;
                        second[0]->y = tSecond[0].y;
                    } else {
                        second[1]->x = tSecond[0].x;
                        second[1]->y = tSecond[0].y; 
                    } 
                }


            } else if (TRUE == second2) {
                // Skip if already here
                if (!(second[0]->x == tSecond[1].x && second[0]->y == tSecond[1].y 
                        || second[1]->x == tSecond[1].x && second[1]->y == tSecond[1].y)) {
                    if (!second[0]->proven) {
                        second[0]->x = tSecond[1].x;
                        second[0]->y = tSecond[1].y;
                    } else {
                        second[1]->x = tSecond[1].x;
                        second[1]->y = tSecond[1].y; 
                    }
                }
            }

            for (int i = 0; i < 3; i++) {
                int x = tFirst[0].x != first[0]->x 
                            ? tFirst[0].x + dirX[d][i] 
                            : tFirst[1].x + dirX[d][i];
                int y = tFirst[0].y != first[0]->y 
                            ? tFirst[0].y + dirY[d][i] 
                            : tFirst[1].y + dirY[d][i];

                if (WALL == map[x][y]) {
                    continue;
                }

                if (abs(first[0]->x - x) <= 1 && abs(first[0]->y - y) <= 1) {
                    continue;
                }

                if (abs(first[1]->x - x) <= 1 && abs(first[1]->y - y) <= 1) {
                    continue;
                }

                if (!second[0]->proven) {
                    second[0]->x = x;
                    second[0]->y = y;
                } else {
                    second[1]->x = x;
                    second[1]->y = y; 
                }

                break;
            }

        // It was MOVE & BUILD or unsuccessful PUSH
        } else {
            int x = 0;
            int y = 0;
            for (int i = 1; i <= size; i++) {
                for (int j = 1; j <= size; j++) {
                    if (map[i][j] != prevMap[i][j]) {
                        x = i;
                        y = j;
                        break;
                    }
                }
            }

            if (x + y == 0) {
                fprintf(stderr, "*** something went wrong! ****\n");
                return;
            }

            // Check if someone has been staying from the prev turn
            int second1 = abs(tSecond[0].x + tSecond[0].y) > 0;
            int second2 = abs(tSecond[1].x + tSecond[1].y) > 0;

            if (tSecond[0].x == x && tSecond[0].y == y) {
                second1 = FALSE;
            }

            if (tSecond[1].x == x && tSecond[1].y == y) {
                second2 = FALSE;
            }

            // If we see prev pos than there is can't be enemy
            for (int i = 0; i < DIRECTIONS; i++) {
                if (tSecond[0].x + dx[i] == first[0]->x 
                        && tSecond[0].y + dy[i] == first[0]->y) {
                    second1 = FALSE;
                }

                if (tSecond[0].x + dx[i] == first[1]->x 
                        && tSecond[0].y + dy[i] == first[1]->y) {
                    second1 = FALSE;
                }

                if (tSecond[1].x + dx[i] == first[0]->x 
                        && tSecond[1].y + dy[i] == first[0]->y) {
                    second2 = FALSE;
                }

                if (tSecond[1].x + dx[i] == first[1]->x 
                        && tSecond[1].y + dy[i] == first[1]->y) {
                    second2 = FALSE;
                }
            }

            for (int i = 0; i < DIRECTIONS; i++) {

                if (WALL == map[x + dx[i]][y + dy[i]]) {
                    continue;
                }

                if (x + dx[i] == tSecond[0].x && y + dy[i] == tSecond[0].y) {
                    second1 = FALSE;
                }

                if (x + dx[i] == tSecond[1].x && y + dy[i] == tSecond[1].y) {
                    second2 = FALSE;
                }

                for (int j = 0; j < DIRECTIONS; j++) {
                    int nx = x + dx[i] + dx[j];
                    int ny = y + dy[i] + dy[j];

                    if (WALL == map[nx][ny]) {
                        continue;
                    }

                    if (nx == tSecond[0].x && ny == tSecond[0].y) {
                        second1 = FALSE;
                    }

                    if (nx == tSecond[1].x && ny == tSecond[1].y) {
                        second2 = FALSE;
                    }
                }
            }

            if (TRUE == second1) {
                // Skip if already here
                if (!(second[0]->x == tSecond[0].x && second[0]->y == tSecond[0].y 
                        || second[1]->x == tSecond[0].x && second[1]->y == tSecond[0].y)) {
                    if (!second[0]->proven) {
                        second[0]->x = tSecond[0].x;
                        second[0]->y = tSecond[0].y;
                    } else {
                        second[1]->x = tSecond[0].x;
                        second[1]->y = tSecond[0].y; 
                    } 
                }


            } else if (TRUE == second2) {
                // Skip if already here
                if (!(second[0]->x == tSecond[1].x && second[0]->y == tSecond[1].y 
                        || second[1]->x == tSecond[1].x && second[1]->y == tSecond[1].y)) {
                    if (!second[0]->proven) {
                        second[0]->x = tSecond[1].x;
                        second[0]->y = tSecond[1].y;
                    } else {
                        second[1]->x = tSecond[1].x;
                        second[1]->y = tSecond[1].y; 
                    }
                }
            }

            int minDist = INF;
            int minDir = -1;
            for (int k = 0; k < DIRECTIONS; k++) {
                if (WALL != map[x + dx[k]][y + dy[k]]
                        && !(first[0]->x == x + dx[k] && first[0]->y == y + dy[k])
                        && !(first[1]->x == x + dx[k] && first[1]->y == y + dy[k])
                        &&  ((abs(first[0]->x - (x + dx[k])) > 1) 
                            || (abs(first[0]->y - (y + dy[k])) > 1))
                        && ((abs(first[1]->x - (x + dx[k])) > 1) 
                            || (abs(first[1]->y - (y + dy[k])) > 1))) {

                    if (minDist > abs(first[0]->x - (x + dx[k])) 
                                   + abs(first[0]->y - (y + dy[k]))) {
                        minDist = abs(first[0]->x - (x + dx[k])) 
                                        + abs(first[0]->y - (y + dy[k]));
                        minDir = k;
                    }

                    if (minDist > abs(first[1]->x - (x + dx[k])) 
                                   + abs(first[1]->y - (y + dy[k]))) {
                        minDist = abs(first[1]->x - (x + dx[k])) 
                                        + abs(first[1]->y - (y + dy[k]));
                        minDir = k;
                    }

                }
            }

            if (minDist < INF && 0 == second[0]->x) {
                fprintf(stderr, "guess opp is here %d %d\n", x + dx[minDir], y + dy[minDir]);
                second[0]->x = x + dx[minDir];
                second[0]->y = y + dy[minDir];
            } else if (minDist < INF && 0 == second[1]->x) {
                fprintf(stderr, "guess opp is here %d %d\n", x + dx[minDir], y + dy[minDir]);
                second[1]->x = x + dx[minDir];
                second[1]->y = y + dy[minDir];
            }
        }
    }

    void setPlayer(int x, int y, int teamId, int playerId) {
        ASSERT(teamId == FIRST || teamId == SECOND);
        ASSERT(playerId == FIRST || playerId == SECOND);
        if (teamId == FIRST) {
            first[playerId] = new Player(x, y);
        } else {
            second[playerId] = new Player(x, y);
            if (x + y != 0) {
                second[playerId]->proven = TRUE;
            }
        }
    }

    int isTerminal() const {
        return 0;
    }

    void print() const {
        cerr << "hash(inc) = " << hashKey << endl;
        cerr << "hash = " << hash() << endl;
        cerr << first[0]->y - 1 << " " << first[0]->x - 1 << endl;
        cerr << first[1]->y - 1 << " " << first[1]->x - 1 << endl;
        cerr << second[0]->y - 1 << " " << second[0]->x - 1 << endl;
        cerr << second[1]->y - 1 << " " << second[1]->x - 1 << endl;
        for (int i = 1; i <= size; i++) {
            for (int j = 1; j <= size; j++) {
                if (map[i][j] == WALL) {
                    cerr << "#";
                } else{
                    cerr << map[i][j];
                }
            }
            cerr << "\n";
        }
        cerr << "\n\n";
    }
};

/*******************************************************************************************/
/*******************************************************************************************/
/*******************************************************************************************/
/*******************************************************************************************/


inline void checkUp(Settings *settings) {
    if (settings->timeset && getTimeMs() > settings->stoptime) {
        settings->stopped = TRUE;
    }
}

inline void pickNextMove(int index, MovesList *list) {
    Move temp;
    int bestScore = 0;
    int bestNum = index;

    for (int i = index; i < list->count; i++) {
        if (list->moves[i].score > bestScore) {
            bestScore = list->moves[i].score;
            bestNum = i;
        }
    }

    ASSERT(index >= 0 && index < list->count);
    ASSERT(bestNum >= 0 && bestNum < list->count);
    ASSERT(bestNum >= index);

    temp = list->moves[index];
    list->moves[index] = list->moves[bestNum];
    list->moves[bestNum] = temp;
}

int alphaBeta(int alpha, int beta, int depth, State *state,
                             Settings *settings, int nullMovePruning/*, PVLine *pvLine*/) {
    ASSERT(beta > alpha);
    ASSERT(depth >= 0);

    settings->nodes++;

    if (depth <= 0 || state->isTerminal()) {
        return state->evaluate(FOR_BOTH);
    }

    #ifdef SEARCH_USE_TIME
    if ((settings->nodes & SEARCH_TIME_CHECK_RATE) == 0) {
        checkUp(settings);
    }
    #endif

    int score = -INF;
    
    Move pvMove;
    pvMove.x1 = 0;
    pvMove.y1 = 0;

    #ifdef SEARCH_USE_HASH_TABLE
    if (state->hashTable->probeHashEntry(&pvMove, state->hashKey,
                        state->ply, &score, alpha, beta, depth)) {
        state->hashTable->cut++;
        return score;
    }
    #endif

    MovesList list[1];
    state->getLegalMoves(list);

    if (0 == list->count) {
        return state->playerToMove == FIRST ? -WIN_SCORE : WIN_SCORE;
    }

    int oldAlpha = alpha;
    Move bestMove;
    bestMove.x1 = 0;
    bestMove.y1 = 0;
    bestMove.x2 = 0;
    bestMove.y2 = 0;

    int bestScore = -INF;

    #ifdef SEARCH_USE_HASH_TABLE
    if (0 != pvMove.x1 + pvMove.y1) {
        for (int i = 0; i < list->count; i++) {
            if (list->moves[i].x1 == pvMove.x1 && list->moves[i].y1 == pvMove.y1
                    && list->moves[i].x2 == pvMove.x2 && list->moves[i].y2 == pvMove.y2 
                    && list->moves[i].playerId == pvMove.playerId
                    && list->moves[i].type == pvMove.type) {
                list->moves[i].score = 200000;
                break;
            }
        }
    }
    #endif

    /*PVLine line;*/
    for(int currentMove = 0; currentMove < list->count; currentMove++) {

        // Shift next best move to currentMove position in list
        pickNextMove(currentMove, list);

        state->ply++;
        state->makeMove(list->moves[currentMove], FOR_BOTH);
        score = -alphaBeta(-beta, -alpha, depth - 1, state, settings, TRUE/*, &line*/);
        state->takeMove(list->moves[currentMove], FOR_BOTH);
        state->ply--;

        #ifdef SEARCH_USE_TIME
        if (settings->stopped) {
            return 0;
        }
        #endif
        
        if (score > bestScore) {
            bestScore = score;
            bestMove = list->moves[currentMove];
            bestMove.score = score;

            if (score > alpha) {
                if (score >= beta) {
                    #ifdef SEARCH_USE_HASH_TABLE
                    state->hashTable->storeHashEntry(bestMove, state->hashKey,
                            state->ply, beta, HFBETA, depth);
                    #endif

                    return beta;
                }
                //pvLine->moves[0] = list->moves[currentMove];
                //memcpy(pvLine->moves + 1, line.moves, line.length * sizeof(Move));
                //pvLine->length = line.length + 1;
                alpha = score;
            }
        }
    }

    ASSERT(alpha >= oldAlpha);

    #ifdef SEARCH_USE_HASH_TABLE
    if (alpha != oldAlpha) {
        state->hashTable->storeHashEntry(bestMove, state->hashKey,
                state->ply, bestScore, HFEXACT, depth);
    } else {
        state->hashTable->storeHashEntry(bestMove, state->hashKey,
                state->ply, alpha, HFALPHA, depth);
    }
    #endif

    return alpha;
}

Move searchPositionIterative(State *state, Settings *settings) {
    Move bestMove;
    int bestScore = -INF;
    int delta = -INF;
    int alpha = -INF;
    int beta = INF;
    int start = getTimeMs();
    /*PVLine line;*/

    // Iterative deepening
    for (int currentDepth = 1; currentDepth <= settings->depth; currentDepth++) {
        settings->nodes = 0;

        #ifdef SEARCH_USE_ASPIRATION_WINDOW
        if (currentDepth > 1) {
            delta = 35;
            Move move = state->hashTable->getBestMove(state->hashKey);
            alpha = std::max(move.score - delta, -INF);
            beta  = std::min(move.score + delta,  INF);
        }

        while (true) {
            bestScore = alphaBeta(alpha, beta, currentDepth, state, settings, TRUE/*, &line*/);

            #ifdef SEARCH_USE_TIME
            if (settings->stopped) {
                break;
            }
            #endif

            // In case of failing low/high increase aspiration window and
            // re-search, otherwise exit the loop.
            if (bestScore <= alpha) {
                beta = (alpha + beta) / 2;
                alpha = std::max(bestScore - delta, -INF);
            } else if (bestScore >= beta) {
                alpha = (alpha + beta) / 2;
                beta = std::min(bestScore + delta, INF);
            } else {
                break;
            }
            fprintf(stderr, "aspiration failed\n");
            delta += delta / 4 + 5;

            ASSERT(alpha >= -INF && beta <= INF);
        }
        #endif

        #ifndef SEARCH_USE_ASPIRATION_WINDOW
        bestScore = alphaBeta(alpha, beta, currentDepth, state, settings, TRUE/*, &line*/);
        #endif

        #ifdef SEARCH_USE_TIME
        if (settings->stopped) {
            break;
        } 
        #endif

        bestMove = state->hashTable->getBestMove(state->hashKey);

        fprintf(stderr, "Search depth : %d  nodes : %d time : %f\n",
                 currentDepth, settings->nodes, (getTimeMs() - start) / 1000.0);
        fprintf(stderr, "best move = (%d %d) score = %d\n", bestMove.x1, bestMove.y1, bestMove.score);
        //bestMove = line.moves[0];
    }

    return bestMove;
}

/*******************************************************************************************/
/*******************************************************************************************/
/*******************************************************************************************/
/*******************************************************************************************/

int calculateValue(State *state, int depth) {
    if (0 == depth) {
        return state->evaluate(FOR_ONE);
    }

    MovesList list[1];
    state->getLegalMoves(list);
    int bestValue = -INF;

    for (int i = 0; i < list->count; i++) {
        state->makeMove(list->moves[i], FOR_ONE);
        int value = calculateValue(state, depth - 1);
        state->takeMove(list->moves[i], FOR_ONE);
        bestValue = max(bestValue, value);
    }

    return bestValue;
}

Move getBestMove(State *state, int depth, Settings *settings) {
    MovesList list[1];
    state->getLegalMoves(list);
    Move bestMove;
    int bestValue = -INF;

    for (int i = 0; i < list->count; i++) {
        state->makeMove(list->moves[i], FOR_ONE);
        int value = calculateValue(state, depth) + list->moves[i].score;
        state->takeMove(list->moves[i], FOR_ONE);

        checkUp(settings);

        if (settings->stopped) {
            return bestMove;
        }

        cerr << outType[list->moves[i].type]
             << " " << list->moves[i].playerId
             << " " << dirOut[list->moves[i].dir1()].c_str() 
             << " " << dirOut[list->moves[i].dir2()].c_str() 
             << " value = " << value << endl;

        if (value > bestValue) {
            bestValue = value;
            bestMove = list->moves[i];
        }
    }

    return bestMove;
}

/*******************************************************************************************/
/*******************************************************************************************/
/*******************************************************************************************/
/*******************************************************************************************/

int main() {
    int mapSize;
    cin >> mapSize; cin.ignore();
    int unitsPerPlayer;
    cin >> unitsPerPlayer; cin.ignore();

    dir["SE"] = Point(1,1);
    dir["E"] = Point(0,1);
    dir["NE"] = Point(-1,1);
    dir["SW"] = Point(1,-1);
    dir["W"] = Point(0,-1);
    dir["NW"] = Point(-1,-1);
    dir["S"] = Point(1,0);
    dir["N"] = Point(-1,0);

    dirOut["SE"] = "RD";
    dirOut["E"] = "R";
    dirOut["NE"] = "RU";
    dirOut["SW"] = "LD";
    dirOut["W"] = "L";
    dirOut["NW"] = "LU";
    dirOut["S"] = "D";
    dirOut["N"] = "U";

    State *state = new State(mapSize);

    Move move;

    // game loop
    while (1) {

        // Copy state's map
        Player tFirst[2];
        Player tSecond[2];
        int tMap[MAX_SIZE][MAX_SIZE];

        if (state->gameRound) {
            tFirst[0].x = state->first[0]->x; 
            tFirst[0].y = state->first[0]->y;
            tFirst[1].x = state->first[1]->x;
            tFirst[1].y = state->first[1]->y;

            tSecond[0].x = state->second[0]->x; 
            tSecond[0].y = state->second[0]->y; 
            tSecond[1].x = state->second[1]->x; 
            tSecond[1].y = state->second[1]->y;

            for (int i = 0; i <= mapSize + 1; i++) {
                for (int j = 0; j <= mapSize + 1; j++) {
                    tMap[i][j] = state->map[i][j];                
                }
            } 
        } 

        for (int i = 0; i < mapSize; i++) {
            string row;
            cin >> row; cin.ignore();

             for (int j = 0; j < row.length(); j++) {
                if (row[j] == '.') {
                    state->setValue(i + 1, j + 1, WALL);
                } else {
                    if (row[j] - '0' == 4) {
                        state->setValue(i + 1, j + 1, WALL);
                    } else {
                        state->setValue(i + 1, j + 1, row[j] - '0');
                    }
                }
             }
        }

        int x, y;

        // First player (me)
        cin >> y >> x; cin.ignore();
        state->setPlayer(x + 1, y + 1, 0, 0);

        cin >> y >> x; cin.ignore();
        state->setPlayer(x + 1, y + 1, 0, 1);

        // Second player (opp)
        cin >> y >> x; cin.ignore();
        state->setPlayer(x + 1, y + 1, 1, 0);

        cin >> y >> x; cin.ignore();
        state->setPlayer(x + 1, y + 1, 1, 1);

        int legalActions;
        cin >> legalActions; cin.ignore();

        state->print();

        if (state->gameRound > 0) {
            state->predictOppPosition(tMap, tFirst, tSecond);
        }

        state->hashKey = state->hash();

        string type[200]; 
        int ids[200];
        string dir1[200]; 
        string dir2[200];
        int value[200]; 

        for (int i = 0; i < legalActions; i++) {
            string atype;
            int id;
            string d1;
            string d2;
            cin >> atype >> id >> d1 >> d2; cin.ignore();

            Player *me = state->first[id];

            me->x += dir[d1].first;
            me->y += dir[d1].second;

            type[i] = atype;
            ids[i] = id;
            dir1[i] = d1;
            dir2[i] = d2;

            if (atype == "PUSH&BUILD") {
                value[i] = (state->map[me->x][me->y] * state->map[me->x][me->y])
                                - state->map[me->x + dir[d2].first][me->y + dir[d2].second];
            } else {
                value[i] = (state->map[me->x][me->y] % 4) + 
                           ((state->map[me->x + dir[d2].first]
                              [me->y + dir[d2].second] + 1) % 4);
            }

            me->x -= dir[d1].first;
            me->y -= dir[d1].second;
        }

        int maxValue = value[0];
        int maxIndex = 0;
        for (int i = 0; i < legalActions; i++) {
            if (value[i] > maxValue) {
                maxValue = value[i];
                maxIndex = i;
            }
        }

        cerr << "best " << dirOut[dir1[maxIndex]] << " " << dirOut[dir2[maxIndex]] << endl;
        
        if (legalActions == 0) {
            continue;
        }

        Settings *settings = new Settings();
        settings->timeset = TRUE;
        settings->stoptime = getTimeMs() + 35;

        if (state->oppIsVisible()) {
            move = searchPositionIterative(state, settings);
        } else {
            move = getBestMove(state, 2, settings);
        }

        state->makeMove(move, FOR_ONE);
        state->gameRound++;

        cout << outType[move.type]
             << " " << move.playerId
             << " " << move.dir1() 
             << " " << move.dir2() << endl;
    }
}

