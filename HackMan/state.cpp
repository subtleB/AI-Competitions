#include "defs.h"
#include "state.h"
#include "hash.h"
#include <iostream>
#include <algorithm>
#include <queue>

using namespace std;

State::State() {
    this->first = new Player();
    this->second = new Player();
    this->first->x = 0;
    this->first->y = 0;
    this->first->prevX = 0;
    this->first->prevY = 0;
    this->second->x = 0;
    this->second->y = 0;
    this->second->prevX = 0;
    this->second->prevY = 0;
    this->first->id = 0;
    this->second->id = 1;
    this->first->killedBugs = 0;
    this->second->killedBugs = 0;
    this->round = 0;
    this->snippetsEaten = 0;
    this->collectedSnippets = 0;
    this->snippetsList->nSnippets = 0;
    this->bugsList->nBugs = 0;
    this->first->is_paralyzed = false;
    this->second->is_paralyzed = false;
    this->first->has_weapon = false;
    this->second->has_weapon = false;
    this->first->snippets = 0;
    this->second->snippets = 0;
    this->first->centerBonus = 0;
    this->second->centerBonus = 0;
    this->first->space = 0;
    this->second->space = 0;

    this->history[ply].weaponsList->nSnippets = 0;
    this->history[ply].snippetsList->nSnippets = 0;
    this->history[ply].bugs->nBugs = 0;

    this->hisPly = 0;
    this->ply = 0;

    this->enemySpawned = false;
    this->weaponSpawned = false;

    for (int i = 0; i < HEIGHT + 2; i++){
        this->map[i][0] = WALL;
        this->map[i][WIDTH + 1] = WALL;
    }

    for (int i = 0; i < WIDTH + 2; i++){
        this->map[0][i] = WALL;
        this->map[HEIGHT + 1][i] = WALL;
    }
}

void State::printMap() {
    cerr << endl;
    cerr << "influence = " << (influence[first->x][first->y][second->x][second->y] / 20.0) << endl;
    cerr << "snippets eaten = " << snippetsEaten << endl;
    cerr << "pos round = " << this->round << endl;
    for (int i = 0; i < HEIGHT + 2; i++) {
        for (int j = 0; j < WIDTH + 2; j++) {

            bool bug = false;
            for (int k = 0; k < bugsList->nBugs; k++) {
                if (i == bugsList->bugs[k].x && j == bugsList->bugs[k].y) {
                    cerr << "B ";
                    bug = true;
                }
            }

            if (bug) {
                continue;
            } 

            if (i == this->first->x && j == this->first->y) {
                cerr << "0 ";
            } else if (i == this->second->x && j == this->second->y) {
                cerr << "1 ";
            } else if (WALL == this->map[i][j]) {
                cerr << "# ";
            } else if (FREE_CELL == this->map[i][j]) {
                cerr << "  ";
            } else {
                cerr << this->map[i][j] << " ";
            }
        }
        cerr << endl;
    }
}

void State::getLegalMoves(MovesList* list, Player *player) const {
    list->nMoves = 0;

    //list->moves[list->nMoves  ].direction = PASS;
    //list->moves[list->nMoves++].score = 0;

    if (this->map[player->x + 1][player->y] != WALL) {
        list->moves[list->nMoves  ].direction = DOWN;
        list->moves[list->nMoves++].score = 0;
    }

    if (this->map[player->x - 1][player->y] != WALL) {
        list->moves[list->nMoves  ].direction = UP;
        list->moves[list->nMoves++].score = 0;
    }

    if (this->map[player->x][player->y + 1] != WALL) {
        list->moves[list->nMoves  ].direction = RIGHT;
        list->moves[list->nMoves++].score = 0;
    }

    if (this->map[player->x][player->y - 1] != WALL) {
        list->moves[list->nMoves  ].direction = LEFT;
        list->moves[list->nMoves++].score = 0;
    }

    ASSERT(list->nMoves > 0 && list->nMoves <= 5);
}

void State::makeEngineMove(Move &move, Player *player) {
    ASSERT(move.direction >= 0 && move.direction <= 4);
    ASSERT(player->x > 0 && player->x < HEIGHT + 1);
    ASSERT(player->y > 0 && player->y < WIDTH + 1);

    player->prevX = player->x;
    player->prevY = player->y;
    player->x += dx[move.direction];
    player->y += dy[move.direction];

    ASSERT(player->x > 0 && player->x < HEIGHT + 1);
    ASSERT(player->y > 0 && player->y < WIDTH + 1);
}


double State::getPChase() {
    double pChase = 0.2 + ((round - 1) * 0.004);
    if (pChase > 1.0) return 1.0;
    if (pChase < 0.0) return 0.0;
    return pChase;
}

int State::updateWinner() {
    if (first->snippets < 0 && second->snippets < 0) {
        return 3;
    } else if (first->snippets < 0) {
        return 1;
    } else if (second->snippets < 0) {
        return 0;
    }

    if (round >= 200) {
        if (first->snippets == second->snippets) {
            return 3;
        } else if (first->snippets > second->snippets) {
            return 0;
        } else if (first->snippets < second->snippets) {
            return 1;
        }
        ASSERT(false);
    }

    return -1;
}

void State::makeMove(Move &move, Player *player) {
    ASSERT(move.direction >= 0 && move.direction <= 4);
    ASSERT(player->x > 0 && player->x < HEIGHT + 1);
    ASSERT(player->y > 0 && player->y < WIDTH + 1);

    //this->history[this->ply].hashKey = this->hashKey;
    this->history[this->ply].move = move;
    this->history[this->ply].prevX = player->prevX;
    this->history[this->ply].prevY = player->prevY;
    //this->hashKey ^= playerHash[player->x][player->y][player->id];

    player->prevX = player->x;
    player->prevY = player->y;

    if (!player->is_paralyzed) {
        player->x += dx[move.direction];
        player->y += dy[move.direction];
    }

    ASSERT(player->x > 0 && player->x < HEIGHT + 1);
    ASSERT(player->y > 0 && player->y < WIDTH + 1);

    player->centerBonus += centerBonus[player->x][player->y];
    player->space += space[player->x][player->y];
    //this->hashKey ^= playerHash[player->x][player->y][player->id];
    this->ply++;
}

void State::takeMove(Player *player) {
    ASSERT(player->x > 0 && player->x < HEIGHT + 1);
    ASSERT(player->y > 0 && player->y < WIDTH + 1);
    ASSERT(this->round >= 0);

    this->ply--;

    player->centerBonus -= centerBonus[player->x][player->y];
    player->space -= space[player->x][player->y];
    player->prevX = this->history[this->ply].prevX;
    player->prevY = this->history[this->ply].prevY;
    //this->hashKey = this->history[this->ply].hashKey;
    Move move = this->history[this->ply].move;

    ASSERT(move.direction >= 0 && move.direction <= 4);

    if (!player->is_paralyzed) {
        player->x -= dx[move.direction];
        player->y -= dy[move.direction];
    }
}

U64 State::positionKey() const {
    U64 key = 0;

    key ^= playerHash[this->first->x][this->first->y][this->first->id];
    key ^= playerHash[this->second->x][this->second->y][this->second->id];

    key ^= roundHash[this->round];

    for (int i = 0; i < this->snippetsList->nSnippets; i++) {
        key ^= snippetHash[this->snippetsList->snippets[i].x]
                          [this->snippetsList->snippets[i].y];
    }

    for (int i = 0; i < this->bugsList->nBugs; i++) {
        key ^= bugsHash[this->bugsList->bugs[i].x]
                          [this->bugsList->bugs[i].y];
    }

    for (int i = 0; i < this->weaponsList->nSnippets; i++) {
        key ^= weaponsHash[this->weaponsList->snippets[i].x]
                          [this->weaponsList->snippets[i].y];
    }

    return key;
}

int State::distToNearestUnit(Player *player, int unitId) const {
    int path[HEIGHT + 2][WIDTH + 2];
    for (int i = 0; i < HEIGHT + 2; i++) {
        for (int j = 0; j < WIDTH + 2; j++) {
            path[i][j] = 0;
        }
    }

    queue<Point> q;

    q.push(Point(player->x, player->y));
    path[player->x][player->y] = 1;  
    
    while(!q.empty()) {
        Point top = q.front();
        q.pop();

        if (this->map[top.first][top.second] == unitId) {
            return path[top.first][top.second];
        }

        for (int i = 0; i < 4; i++) {
            Point cur = Point(top.first + dx[i], top.second + dy[i]);
            if (this->map[cur.first][cur.second] != WALL
                    && path[cur.first][cur.second] == 0) {
                q.push(cur);
                path[cur.first][cur.second] = path[top.first][top.second] + 1;  
            }
        }   
    }
    return 0;
}

int State::minDistToSnippets(int x, int y) {
    int minValue = INF;
    for (int i = snippetsList->nSnippets - 1; i >= 0 ; i--) {
        if (snippetsList->snippets[i].visited) {
            continue;
        }

        snippetsList->snippets[i].visited = TRUE;
        int value = distances[x][y][snippetsList->snippets[i].x]
                                   [snippetsList->snippets[i].y] 
                + minDistToSnippets(snippetsList->snippets[i].x, 
                                    snippetsList->snippets[i].y);
        snippetsList->snippets[i].visited = FALSE;
        if (value < minValue) {
            minValue = value;
        }
    }

    return minValue == INF ? 0 : minValue;
}

int State::sumDistanceToSnippsAndWeapons() {
    int sum = 0;
    for (int i = 0; i < snippetsList->nSnippets; i++) {
        sum += distances[first->x][first->y][snippetsList->snippets[i].x]
                                    [snippetsList->snippets[i].y];
        sum -= distances[second->x][second->y][snippetsList->snippets[i].x]
                                    [snippetsList->snippets[i].y];
    }

    for (int i = 0; i < weaponsList->nSnippets; i++) {
        sum += distances[first->x][first->y][weaponsList->snippets[i].x]
                                    [weaponsList->snippets[i].y];
        sum -= distances[second->x][second->y][weaponsList->snippets[i].x]
                                    [weaponsList->snippets[i].y];
    }

    return sum;
}

int State::numbSnippetsCloserToPlayer() {
    int sum = 0;
    for (int i = 0; i < snippetsList->nSnippets; i++) {
        if (distances[first->x][first->y][snippetsList->snippets[i].x]
                                    [snippetsList->snippets[i].y]
            < distances[second->x][second->y][snippetsList->snippets[i].x]
                                    [snippetsList->snippets[i].y]) {
            sum++;
        }

        if (distances[first->x][first->y][snippetsList->snippets[i].x]
                                    [snippetsList->snippets[i].y]
            > distances[second->x][second->y][snippetsList->snippets[i].x]
                                    [snippetsList->snippets[i].y]) {
            sum--;
        }
    }
    return sum;
}

double State::nextDestinationValue(Player *p1, Player *p2) {
    int nSnippets = snippetsList->nSnippets;

    // Go to the best position relative to the opponent
    if (0 == nSnippets) {
        return min(distances[p1->x][p1->y][9][7], 
                   distances[p1->x][p1->y][9][14]);

    } else if (1 == nSnippets) {
        Snippet snip1 = snippetsList->snippets[0];
        if (distances[p1->x][p1->y][snip1.x][snip1.y]
                < distances[p2->x][p2->y][snip1.x][snip1.y]) {
            return distances[p1->x][p1->y][snip1.x][snip1.y];
        }

    } else if (2 == nSnippets) {
        Snippet snip1 = snippetsList->snippets[0];
        Snippet snip2 = snippetsList->snippets[1];

        // We are closer to both snipps
        if ((distances[p1->x][p1->y][snip1.x][snip1.y] 
                < distances[p2->x][p2->y][snip1.x][snip1.y]) 
                && (distances[p1->x][p1->y][snip2.x][snip2.y] 
                < distances[p2->x][p2->y][snip2.x][snip2.y])) {

            // Try to pick both
            if ((distances[p1->x][p1->y][snip1.x][snip1.y]
                    + distances[snip2.x][snip2.y][snip1.x][snip1.y])
                    < distances[p2->x][p2->y][snip2.x][snip2.y]) {

                // Go to the snip1 and then to the snip2
                return distances[p1->x][p1->y][snip1.x][snip1.y]
                            + distances[p1->x][p1->y][snip2.x][snip2.y] * 0.2;
            }

            if ((distances[p1->x][p1->y][snip2.x][snip2.y]
                    + distances[snip2.x][snip2.y][snip1.x][snip1.y])
                    < distances[p2->x][p2->y][snip1.x][snip1.y]) {

                // Go to the snip2 ant then to the snip1
                return distances[p1->x][p1->y][snip2.x][snip2.y]
                            + distances[p1->x][p1->y][snip1.x][snip1.y] * 0.2; 
            }

            // Cant pick both, try to pick best
            if (distances[p1->x][p1->y][snip1.x][snip1.y]
                    < distances[p1->x][p1->y][snip2.x][snip2.y]) {

                // Go to the snip1
                int center = min(distances[snip1.x][snip1.y][9][7], 
                                 distances[snip1.x][snip1.y][9][14]);
                return distances[p1->x][p1->y][snip1.x][snip1.y] + center * 0.2; 
            } else {

                // Go to the snip2
                int center = min(distances[snip2.x][snip2.y][9][7], 
                                 distances[snip2.x][snip2.y][9][14]);
                return distances[p1->x][p1->y][snip2.x][snip2.y] + center * 0.2; 
            }
        }
        
        if (distances[p1->x][p1->y][snip1.x][snip1.y]
                < distances[p2->x][p2->y][snip1.x][snip1.y]) {

            // Go to the snip1
            int center = min(distances[snip1.x][snip1.y][9][7], 
                             distances[snip1.x][snip1.y][9][14]);
            return distances[p1->x][p1->y][snip1.x][snip1.y] + center * 0.2;
        } else {

            // Go to the snip2
            int center = min(distances[snip2.x][snip2.y][9][7], 
                             distances[snip2.x][snip2.y][9][14]);
            return distances[p1->x][p1->y][snip2.x][snip2.y] + center * 0.2; 
        }

    }

    return 0;
    
}


double State::evaluate() {
    double eval = 0;

        /*
         1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0
       1 .,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,4,.,.,.,
       2 .,0,0,0,0,0,.,0,0,0,0,0,0,.,0,0,0,0,0,.,
       3 .,0,.,.,.,.,.,0,0,0,0,0,0,.,.,.,.,.,0,.,
       4 .,0,.,0,0,0,.,.,.,0,0,.,.,.,0,0,0,.,0,.,
       5 .,.,.,.,.,0,0,0,.,0,0,.,0,0,0,.,.,.,.,.,
       6 .,0,0,0,.,0,.,.,.,.,.,.,.,.,0,.,0,0,0,.,
       7 .,.,.,0,.,0,.,0,0,0,0,0,0,.,0,.,0,.,.,.,
       8 0,0,.,0,.,.,.,0,0,0,0,0,0,.,.,.,0,.,0,0,
       9 .,.,.,0,0,0,.,0,0,0,0,0,0,.,0,0,0,.,4,.,
      10 .,0,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,0,.,
      11 .,0,0,0,.,0,0,0,0,0,0,0,0,0,0,.,0,0,0,.,
      12 .,0,0,0,.,.,.,.,.,.,.,.,.,.,.,.,0,0,0,.,
      13 .,0,0,0,.,0,0,0,.,0,0,.,0,0,0,.,0,0,0,.,
      14 .,.,.,.,.,.,.,.,.,0,0,.,.,.,.,.,.,.,.,.
    */

    /*
     0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
     0 15 15 15 17 19 21 23 21 20 19 19 20 21 23 21 19 17 15 15 15  0
     0 17  0  0  0  0  0 23  0  0  0  0  0  0 23  0  0  0  0  0 17  0
     0 19  0 21 21 21 22 24  0  0  0  0  0  0 24 22 21 21 21  0 19  0
     0 21  0 22  0  0  0 23 22 22  0  0 22 22 23  0  0  0 22  0 21  0
     0 23 23 23 21 20  0  0  0 23  0  0 23  0  0  0 20 21 23 23 23  0
     0 22  0  0  0 19  0 24 24 25 23 23 25 24 24  0 19  0  0  0 22  0
     0 21 20 20  0 19  0 25  0  0  0  0  0  0 25  0 19  0 20 20 21  0
     0  0  0 21  0 20 23 27  0  0  0  0  0  0 27 23 20  0 21  0  0  0
     0 18 20 24  0  0  0 28  0  0  0  0  0  0 28  0  0  0 24 20 18  0
     0 16  0 26 30 34 32 30 25 22 20 20 22 25 30 32 34 30 26  0 16  0
     0 15  0  0  0 33  0  0  0  0  0  0  0  0  0  0 33  0  0  0 15  0
     0 14  0  0  0 31 27 25 24 23 23 23 23 24 25 27 31  0  0  0 14  0
     0 14  0  0  0 27  0  0  0 21  0  0 21  0  0  0 27  0  0  0 14  0
     0 15 17 19 22 24 22 20 19 19  0  0 19 19 20 22 24 22 19 17 15  0
     0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0 */


    int firstMinDist = INF;
    int secondMinDist = INF;
    int nearSnippets = 0;
    for (int i = 0; i < snippetsList->nSnippets; i++) {
        int x = snippetsList->snippets[i].x;
        int y = snippetsList->snippets[i].y;

        if (distances[x][y][first->x][first->y] < firstMinDist) {
            firstMinDist = distances[x][y][first->x][first->y];
        }

        if (distances[x][y][second->x][second->y] < secondMinDist) {
            secondMinDist = distances[x][y][second->x][second->y];
        }

    }

    int numBugsChasing = 0;
    for (int i = 0; i < bugsList->nBugs; i++) {
        int x = bugsList->bugs[i].x;
        int y = bugsList->bugs[i].y;

        if (distances[x][y][first->x][first->y] < distances[x][y][second->x][second->y]) {
            numBugsChasing--;
        }

        if (distances[x][y][first->x][first->y] > distances[x][y][second->x][second->y]) {
            numBugsChasing++;
        }
    }

    // Influence table
    eval += (numBugsChasing * 8) * (0.2 + (round * 0.004));
    eval += influence[first->x][first->y][second->x][second->y] / 30.0;
    eval += (first->space - second->space) / (105.0);
    eval += (first->centerBonus - second->centerBonus) / 40.0;

    // For first
    eval += 150.0 * first->snippets;
    eval += 100.0 * first->killedBugs;
    eval += -4.0 * firstMinDist;
    eval += -0.4 * nextDestinationValue(first, second);

    if (snippetsList->nSnippets < 6) {
        eval += -minDistToSnippets(first->x, first->y);
    }

    if (first->has_weapon) {
        eval += 105.0;
    }

    // For second
    eval -= 150.0 * second->snippets;
    eval -= 100.0 * second->killedBugs;
    eval -= -4.0 * secondMinDist;
    eval -= -0.4 * nextDestinationValue(second, first);

    if (snippetsList->nSnippets < 6) {
        eval -= -minDistToSnippets(second->x, second->y);
    }

    if (second->has_weapon) {
        eval -= 105.0;
    }

    return eval;
}

