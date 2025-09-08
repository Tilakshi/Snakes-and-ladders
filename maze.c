#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "maze.h"

// Creating the maze
void createMaze(Maze *maze) {
    for (int f = 0; f < FLOORS; f++) {
        for (int w = 0; w < WIDTH; w++) {
            for (int l = 0; l < LENGTH; l++) {
                maze->blocks[f][w][l] = EMPTY; 
            }
        }
    }

    //First floor implementation
    for (int w = 0; w < 10; w++) {   // Width 0 to 9
        for (int l = 0; l < 25; l++) { // Length 0 to 24
            if((w>=0 && w<4)&&(l>=8 && l<17)){
                maze->blocks[0][w][l] = SRARTING_AREA;
            }
            maze->blocks[0][w][l] = WALKABLE;
        }
    }

    // First rectangular area (10 width x 8 length)
    for (int w = 0; w < 10; w++) {   // Width 0 to 9
        for (int l = 0; l < 8; l++) { // Length 0 to 7
            maze->blocks[1][w][l] = WALKABLE;
        }
    }

    // Second rectangular area (10 width x 8 length)
    for (int w = 0; w < 10; w++) {   // Width 0 to 9
        for (int l = 17; l < 25; l++) { // Length 17 to 24
            maze->blocks[1][w][l] = WALKABLE;
        }
    }

    // Bridge (4 width x 9 length)
    for (int w = 0; w < 4; w++) {   // Width 3 to 6 (4 blocks)
        for (int l = 8; l < 17; l++) { // Length 8 to 16 (9 blocks)
            maze->blocks[1][w][l] = WALKABLE;
        }
    }

    // Assumed to be at length 8-16, spanning full width
    for (int w = 0; w < 10; w++) {   // Width 0 to 9
        for (int l = 8; l < 17; l++) { // Length 8 to 16 (9 blocks)
            maze->blocks[2][w][l] = WALKABLE;
        }
    }

    // Initialize dynamic arrays to NULL and counts to 0
    //To put the maze into clean and well defined starting area. Helps to avoid if these contained any garbage values
    maze->stairs = NULL;
    maze->stairs_count = 0;
    maze->poles = NULL;
    maze->poles_count = 0;
    maze->walls = NULL;
    maze->walls_count = 0;
}

//Loads the data from the given text files
void loadStairs(Maze *maze, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening stairs.txt");
        return;
    }

    int count = 0;
    char line[100];
    while (fgets(line, sizeof(line), file)) count++;
    rewind(file);

    maze->stairs = malloc(count * sizeof(Stair));
    if (maze->stairs == NULL) {
        perror("Failed to allocate memory for stairs");
        fclose(file);
        return;
    }
    int s = 0;
    while (fscanf(file, "[%d, %d, %d, %d, %d, %d]\n",
                  &maze->stairs[s].start_floor,
                  &maze->stairs[s].start_width,
                  &maze->stairs[s].start_length,
                  &maze->stairs[s].end_floor,
                  &maze->stairs[s].end_width,
                  &maze->stairs[s].end_length) == 6) {
        s++;
    }
    maze->stairs_count = s;
    fclose(file);
}

void loadPoles(Maze *maze, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening poles.txt");
        return;
    }

    int count = 0;
    char line[100];
    while (fgets(line, sizeof(line), file)) count++;
    rewind(file);

    maze->poles = malloc(count * sizeof(Pole));
    if (maze->poles == NULL) {
        perror("Failed to allocate memory for poles");
        fclose(file);
        return;
    }
    int p = 0;
    while (fscanf(file, "[%d, %d, %d, %d]\n",
                  &maze->poles[p].start_floor,
                  &maze->poles[p].end_floor,
                  &maze->poles[p].width,
                  &maze->poles[p].length) == 4) {
        p++;
    }
    maze->poles_count = p;
    fclose(file);
}

void loadWalls(Maze *maze, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening walls.txt");
        return;
    }

    int count = 0;
    char line[100];
    while (fgets(line, sizeof(line), file)) count++;
    rewind(file);

    maze->walls = malloc(count * sizeof(Wall));
    if (maze->walls == NULL) {
        perror("Failed to allocate memory for walls");
        fclose(file);
        return;
    }
    int w = 0;
    while (fscanf(file, "[%d, %d, %d, %d, %d]\n",
                  &maze->walls[w].floor,
                  &maze->walls[w].start_width,
                  &maze->walls[w].start_length,
                  &maze->walls[w].end_width,
                  &maze->walls[w].end_length) == 5) {
        w++;
    }
    maze->walls_count = w;
    fclose(file);
}

void loadFlag(Maze *maze, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        // Default flag position if file not found
        maze->flag_floor = 2;
        maze->flag_width = 5;
        maze->flag_length = 12;
        perror("Error opening flag.txt, using default flag position");
        return;
    }

    if (fscanf(file, "[%d, %d, %d]\n",
               &maze->flag_floor,
               &maze->flag_width,
               &maze->flag_length) != 3) {
        fprintf(stderr, "Error reading flag data from flag.txt, using default flag position.\n");
    }
    fclose(file);
}
void loadSeedValue(){
    unsigned int seed_value;
    FILE *seed_file = fopen("seed.txt", "r");

    if (seed_file != NULL) { 
        if (fscanf(seed_file, "%u", &seed_value) == 1) { 
            srand(seed_value);
            printf("Program seeded with value from seed.txt: %u\n", seed_value);
        } else {
           
            printf("Failed to read seed from seed.txt.");
        }
        fclose(seed_file); 
    } else {
        
        printf("seed.txt not found.");
    }
}
   

// ---------- PLACING ----------
void placeStairs(Maze *maze) {
    for (int i = 0; i < maze->stairs_count; i++) {
        Stair *s = &maze->stairs[i];
        if (isValidPosition(s->start_floor, s->start_width, s->start_length, maze))
            maze->blocks[s->start_floor][s->start_width][s->start_length] = STAIR;
        if (isValidPosition(s->end_floor, s->end_width, s->end_length, maze))
            maze->blocks[s->end_floor][s->end_width][s->end_length] = STAIR;
    }
}

void placePoles(Maze *maze) {
    for (int i = 0; i < maze->poles_count; i++) {
        Pole *p = &maze->poles[i];
        int min_floor = (p->start_floor < p->end_floor) ? p->start_floor : p->end_floor;
        int max_floor = (p->start_floor > p->end_floor) ? p->start_floor : p->end_floor;
        for (int f = min_floor; f <= max_floor; f++) {
            if (isValidPosition(f, p->width, p->length, maze))
                maze->blocks[f][p->width][p->length] = POLE;
        }
    }
}

void placeWalls(Maze *maze) {
    for (int i = 0; i < maze->walls_count; i++) {
        Wall *w = &maze->walls[i];
        if (!isValidPosition(w->floor, w->start_width, w->start_length, maze) ||
            !isValidPosition(w->floor, w->end_width, w->end_length, maze)) continue;

        if (w->start_width == w->end_width) {
            int start = (w->start_length < w->end_length) ? w->start_length : w->end_length;
            int end = (w->start_length > w->end_length) ? w->start_length : w->end_length;
            for (int l = start; l <= end; l++)
                maze->blocks[w->floor][w->start_width][l] = WALL;
        } else if (w->start_length == w->end_length) {
            int start = (w->start_width < w->end_width) ? w->start_width : w->end_width;
            int end = (w->start_width > w->end_width) ? w->start_width : w->end_width;
            for (int wd = start; wd <= end; wd++)
                maze->blocks[w->floor][wd][w->start_length] = WALL;
        }
    }
}

//Initializing Players
void initializePlayers(Player players[]) {
    // Initial positions as given in the problem
    players[0] = (Player){'A',0,6,12,NORTH,100,true,false}; // Player A starts in starting area
    players[1] = (Player){'B',0,9,8,WEST,100,true,false};  // Player B starts in starting area
    players[2] = (Player){'C',0,9,16,EAST,100,true,false}; // Player C starts in starting area
}

bool isValidPosition(int floor, int width, int length, Maze *maze) {
    return (maze->blocks[floor][width][length] == WALKABLE);
}

bool isWallBlocked(Maze *maze, int floor, int width, int length) {
    if (!isValidPosition(floor, width, length, maze)) return true;
    // Also consider if the block is an EMPTY (blocked) area of the floor plan
    return (maze->blocks[floor][width][length] == WALL ||
            maze->blocks[floor][width][length] == EMPTY); // Check for EMPTY too
}

bool movePlayer(Maze *maze, Player *player, int steps, int movement_value) {
    for (int i = 0; i < steps; i++) {
        int new_width = player->width;
        int new_length = player->length;

        switch (player->direction) {
            case NORTH: new_width--; break;
            case EAST:  new_length++; break;
            case SOUTH: new_width++; break;
            case WEST:  new_length--; break;
            default: break;
        }

        if (!isValidPosition(player->floor, new_width, new_length, maze) ||
            isWallBlocked(maze, player->floor, new_width, new_length)) {
            printf("Player %c rolls %d on the movement dice and cannot move in the %s. Player remains at [%d, %d, %d]\n",
                   player->name, movement_value, directionToString(player->direction),
                   player->floor, player->width, player->length);
            return false;
        }

        player->width = new_width;
        player->length = new_length;

        // Check the type of block the player landed on AFTER moving
        if (maze->blocks[player->floor][player->width][player->length] == STAIR)
            handleStairs(maze, player);
        else if (maze->blocks[player->floor][player->width][player->length] == POLE)
            handlePoles(maze, player);
    }
    return true;
}

void handleStairs(Maze *maze, Player *player) {
    for (int i = 0; i < maze->stairs_count; i++) {
        Stair *s = &maze->stairs[i];
        // Check if player is on start of stair
        if (player->floor == s->start_floor &&
            player->width == s->start_width &&
            player->length == s->start_length) {
            printf("Player %c lands on [%d, %d, %d] which is a stair cell.\n",
                   player->name, player->floor, player->width, player->length);
            printf("Player %c takes the stairs and now placed at [%d, %d, %d] in floor %d.\n",
                   player->name, s->end_floor, s->end_width, s->end_length, s->end_floor);
            player->floor = s->end_floor;
            player->width = s->end_width;
            player->length = s->end_length;
            return;
        }
        // Check if player is on end of stair
        if (player->floor == s->end_floor &&
            player->width == s->end_width &&
            player->length == s->end_length) {
            printf("Player %c lands on [%d, %d, %d] which is a stair cell.\n",
                   player->name, player->floor, player->width, player->length);
            printf("Player %c takes the stairs and now placed at [%d, %d, %d] in floor %d.\n",
                   player->name, s->start_floor, s->start_width, s->start_length, s->start_floor);
            player->floor = s->start_floor;
            player->width = s->start_width;
            player->length = s->start_length;
            return;
        }
    }
}

void handlePoles(Maze *maze, Player *player) {
    for (int i = 0; i < maze->poles_count; i++) {
        Pole *p = &maze->poles[i];
        // Player must be on the pole's width/length and above its end floor to slide down
        if (player->width == p->width && player->length == p->length &&
            player->floor > p->end_floor && player->floor >= p->start_floor) { // Ensure within pole's vertical span
            printf("Player %c lands on [%d, %d, %d] which is a pole cell.\n",
                   player->name, player->floor, player->width, player->length);
            printf("Player %c slides down and now placed at [%d, %d, %d] in floor %d.\n",
                   player->name, p->end_floor, p->width, p->length, p->end_floor);
            player->floor = p->end_floor;
            return;
        }
    }
}

void rollDice(Player players[], Maze *maze, int player_index, bool use_direction_dice) {
    Player *player = &players[player_index];
    int movement_value = rollMovementDice();
    Direction direction_value = player->direction; // Default to player's current direction

    if (player->in_starting_area) {
        if (movement_value == 6) {
            player->in_maze = true;
            player->in_starting_area = false;
            // Place players at their initial maze entry points
            if (player->name == 'A') { player->width = 5; player->length = 12; player->direction = NORTH;}
            else if (player->name == 'B') { player->width = 9; player->length = 7; player->direction = WEST;}
            else if (player->name == 'C') { player->width = 9; player->length = 17; player->direction = EAST;}
            printf("Player %c is at the starting area and rolls 6 on the movement dice and is placed on [%d, %d, %d] of the maze.\n",
                   player->name, player->floor, player->width, player->length);
        } else {
            printf("Player %c is at the starting area and rolls %d on the movement dice and cannot enter the maze.\n",
                   player->name, movement_value);
        }
        return; // Player turn ends if in starting area
    }

    // Player is in the maze
    if (use_direction_dice) {
        Direction rolled_direction = rollDirectionDice();
        if (rolled_direction != NONE) {
            player->direction = rolled_direction; // Update player's direction
            printf("Player %c rolls %d on the movement dice and %s on the direction dice, changes direction to %s and moves %d cells and is now ",
                   player->name, movement_value, directionToString(rolled_direction),
                   directionToString(player->direction), movement_value);
        } else {
            // Direction dice rolled NONE, keep current direction
            printf("Player %c rolls %d on the movement dice and EMPTY on the direction dice, keeps direction %s and moves %d cells and is now ",
                   player->name, movement_value, directionToString(player->direction), movement_value);
        }
    } else {
        // No direction dice, keep current direction
        printf("Player %c rolls %d on the movement dice and moves %s by %d cells and is now ",
               player->name, movement_value, directionToString(player->direction), movement_value);
    }

    bool moved_successfully = movePlayer(maze, player, movement_value, movement_value); // Pass movement_value for logging
    if (moved_successfully) {
        printf("at [%d, %d, %d].\n", player->floor, player->width, player->length);
    }

    // Assuming movement_points is adjusted somewhere else or not actively used in this snippet's game logic
    printf("Player %c moved %d that cost %d movement points and is left with %d movement points and is moving in the %s.\n",
           player->name, movement_value, movement_value, player->movement_points, directionToString(player->direction));
}


// ---------- RANDOM ----------
int rollMovementDice() { return (rand() % 6) + 1; }

Direction rollDirectionDice() {
    int value = rand() % 6; // Values 0-5
    switch (value) {
        case 0: return NORTH; // Changed to match 0-5 range of rand()%6
        case 1: return EAST;
        case 2: return SOUTH;
        case 3: return WEST;
        case 4: // Fallthrough for NONE
        case 5: return NONE;
        default: return NONE; // Should not happen
    }
}

const char* directionToString(Direction dir) {
    switch (dir) {
        case NORTH: return "NORTH";
        case EAST:  return "EAST";
        case SOUTH: return "SOUTH";
        case WEST:  return "WEST";
        case NONE:  return "NONE"; // Explicitly handle NONE for printing
        default:    return "UNKNOWN"; // Fallback for safety
    }
}