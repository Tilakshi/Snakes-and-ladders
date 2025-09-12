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
    for (int w = 6; w < 10; w++) {       // Width 0 to 3 (4 blocks)
        for (int l = 8; l < 17; l++) {  // Length 8 to 16 (9 blocks)
            maze->blocks[0][w][l] = STARTING_AREA;
        }
    }
    
    // Rest of floor 0 is walkable (except walls will be placed later)
    for (int w = 0; w < 10; w++) {
        for (int l = 0; l < 25; l++) {
            if (maze->blocks[0][w][l] != STARTING_AREA) {
                maze->blocks[0][w][l] = WALKABLE;
            }
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
    for (int w = 6; w < 10; w++) {   // Width 3 to 6 (4 blocks)
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

    //To count the number of lines in a file and then reset the
    //file pointer to the begining
    int count = 0;
    char line[100];//Create a buffer to read each file into
    while (fgets(line, sizeof(line), file)){
        count++;
    } 
    rewind(file);//Go back to the begining
    //This part is essential as we will be reading the file from
    //the begining again

    maze->stairs = malloc(count * sizeof(Stair));
    if (maze->stairs == NULL) {
        perror("Failed to allocate memory for stairs");
        fclose(file);
    }
    int s = 0;
    while (fscanf(file, "[%d, %d, %d, %d, %d, %d]\n",
                  &maze->stairs[s].start_floor,
                  &maze->stairs[s].start_width,
                  &maze->stairs[s].start_length,
                  &maze->stairs[s].end_floor,
                  &maze->stairs[s].end_width,
                  &maze->stairs[s].end_length) == 6) {
        s++;//Keeps track of how many stairs we've successfully read
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
    //After counting lines, the file pointer is at the end of the file
    //With this it goes back to begining of the file.

    maze->poles = malloc(count * sizeof(Pole));
    if (maze->poles == NULL) {//Check if the memory allocation fails
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
    char line[100];//Buffer that provides temporary storage to save the data and delete them afterwards
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
    if (file == NULL) {
        printf("Error: Could not open flag file.\n");
        // Log to output file
        FILE *log = fopen("game_log.txt", "a");
        if (log) {
            fprintf(log, "Error: Could not open flag file %s.\n", filename);
            fclose(log);
        }
        exit(EXIT_FAILURE);//Terminates the program and returns a status code to the OS
    }
    
    int floor, width, length;
    if (fscanf(file, "[%d, %d, %d]\n", &floor, &width, &length) == 3) {
        // Check if position is valid and accessible (not EMPTY or WALL)
        if (!isValidPosition(floor, width, length)) {
            printf("Error: Flag position [%d,%d,%d] is outside maze bounds.\n", floor, width, length);
            // Log error
            FILE *log = fopen("game_log.txt", "a");
            if (log) {
                fprintf(log, "Error: Flag position [%d,%d,%d] is outside maze bounds. Aborting game.\n", floor, width, length);
                fclose(log);
            }
            fclose(file);
            exit(EXIT_FAILURE);
        }
        
        BlockType block = maze->blocks[floor][width][length];
        if (block == EMPTY || block == WALL) {
            printf("Error: Flag position [%d,%d,%d] is in an inaccessible area (EMPTY or WALL).\n", floor, width, length);
            // Log error
            FILE *log = fopen("game_log.txt", "a");
            if (log) {
                fprintf(log, "Error: Flag position [%d,%d,%d] is in an inaccessible area (EMPTY or WALL). Aborting game.\n", floor, width, length);
                fclose(log);
            }
            fclose(file);
            exit(EXIT_FAILURE);
        }
        
        // Valid position
        maze->flag_floor = floor;
        maze->flag_width = width;
        maze->flag_length = length;
        printf("Flag successfully placed at [%d, %d, %d].\n", floor, width, length);
        
    } else {
        printf("Error: Invalid flag format in %s.\n", filename);
        // Log error
        FILE *log = fopen("game_log.txt", "a");
        if (log) {
            fprintf(log, "Error: Invalid flag format in %s. Aborting game.\n", filename);
            fclose(log);
        }
        fclose(file);
        exit(EXIT_FAILURE);
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
   

//Placeing the elements in the maze

void placeStairs(Maze *maze) {
    for (int i = 0; i < maze->stairs_count; i++) {
        Stair *s = &maze->stairs[i];
        
        // Only place if position is valid AND can hold a structure
        if (isValidPosition(s->start_floor, s->start_width, s->start_length) &&
            canPlaceStructure(s->start_floor, s->start_width, s->start_length, maze)) {
            maze->blocks[s->start_floor][s->start_width][s->start_length] = STAIR;
        } else {
            printf("Warning: Cannot place stair at invalid start position [%d,%d,%d]\n",
                   s->start_floor, s->start_width, s->start_length);
        }
        
        if (isValidPosition(s->end_floor, s->end_width, s->end_length) &&
            canPlaceStructure(s->end_floor, s->end_width, s->end_length, maze)) {
            maze->blocks[s->end_floor][s->end_width][s->end_length] = STAIR;
        } else {
            printf("Warning: Cannot place stair at invalid end position [%d,%d,%d]\n",
                   s->end_floor, s->end_width, s->end_length);
        }
    }
}

void placePoles(Maze *maze) {
    for (int i = 0; i < maze->poles_count; i++) {
        Pole *p = &maze->poles[i];
        int min_floor = (p->start_floor < p->end_floor) ? p->start_floor : p->end_floor;
        int max_floor = (p->start_floor > p->end_floor) ? p->start_floor : p->end_floor;
        
        // Only place poles if the position is valid AND can hold a structure
        for (int f = min_floor; f <= max_floor; f++) {
            if (isValidPosition(f, p->width, p->length) &&
                canPlaceStructure(f, p->width, p->length, maze)) {
                maze->blocks[f][p->width][p->length] = POLE;
            } else {
                printf("Warning: Cannot place pole at invalid position [%d,%d,%d] (block type: %d)\n",
                       f, p->width, p->length, 
                       isValidPosition(f, p->width, p->length) ? 
                       maze->blocks[f][p->width][p->length] : -1);
            }
        }
    }
}

void placeWalls(Maze *maze) {
    for (int i = 0; i < maze->walls_count; i++) {
        Wall *w = &maze->walls[i];
        if (!isValidPosition(w->floor, w->start_width, w->start_length) ||
            !isValidPosition(w->floor, w->end_width, w->end_length)) continue;

        if (w->start_width == w->end_width) {
            //This handles the walls that block the horizontal movement of the player
            int start = (w->start_length < w->end_length) ? w->start_length : w->end_length;
            int end = (w->start_length > w->end_length) ? w->start_length : w->end_length;
            for (int l = start; l <= end; l++)
                maze->blocks[w->floor][w->start_width][l] = WALL;
        } else if (w->start_length == w->end_length) {
            //This handles the walls that block the vertical movements of the player
            int start = (w->start_width < w->end_width) ? w->start_width : w->end_width;
            int end = (w->start_width > w->end_width) ? w->start_width : w->end_width;
            for (int wd = start; wd <= end; wd++)
                maze->blocks[w->floor][wd][w->start_length] = WALL;
        }
    }
    //This specific implementation ensures that the walls are always straight lines. 
    //Otherwise it will create diagonal walls, which are not allowed in the maze.
}

//Initializing Players
void initializePlayers(Player players[]) {
    // Initial positions as given in the problem
    players[0] = (Player){'A',0,6,12,NORTH,100,true,false}; // Player A starts in starting area
    players[1] = (Player){'B',0,9,8,WEST,100,true,false};  // Player B starts in starting area
    players[2] = (Player){'C',0,9,16,EAST,100,true,false}; // Player C starts in starting area
}

// Check if position is within maze bounds
bool isValidPosition(int floor, int width, int length) {
    return (floor >= 0 && floor < FLOORS && 
            width >= 0 && width < WIDTH && 
            length >= 0 && length < LENGTH);
}

// Check if position can hold a structure (stair, pole, wall, flag)
bool canPlaceStructure(int floor, int width, int length, Maze *maze) {
    if (!isValidPosition(floor, width, length)) {
        return false;
    }else{
        BlockType block = maze->blocks[floor][width][length];
    // Can only place structures on walkable areas or starting area
    // Cannot place on empty (WALL) blocks
    return (block == WALKABLE || block == STARTING_AREA);
    }
    
}

bool isWallBlocked(Maze *maze, int floor, int width, int length, Player *player) {
    if (!isValidPosition(floor, width, length)) {
        return true;
    }
    
    BlockType block = maze->blocks[floor][width][length];
    
    // Always block WALL and EMPTY
    if (block == WALL || block == EMPTY) {
        return true;
    }
    
    // Block STARTING_AREA for players who are already in the maze
    if (block == STARTING_AREA && player->in_maze) {
        return true;
    }
    
    return false;
}

bool movePlayer(Maze *maze, Player *player, int steps, int movement_value) {
    int steps_remaining = steps;
    int initial_steps = steps;
    
    while (steps_remaining > 0) {
        int new_width = player->width;
        int new_length = player->length;

        switch (player->direction) {
            case NORTH: new_width--; break;
            case EAST:  new_length++; break;
            case SOUTH: new_width++; break;
            case WEST:  new_length--; break;
            default: break;
        }

        if (!isValidPosition(player->floor, new_width, new_length) ||
            isWallBlocked(maze, player->floor, new_width, new_length, player)) {
            printf("Player %c rolls %d on the movement dice and cannot move in the %s. Player remains at [%d, %d, %d]\n",
                   player->name, movement_value, directionToString(player->direction),
                   player->floor, player->width, player->length);
            return false;
        }

        // Move one step
        player->width = new_width;
        player->length = new_length;
        steps_remaining--;

        // Check if we landed on a stair or pole DURING movement
        bool teleported = false;
        
        // Check for stairs
        for (int i = 0; i < maze->stairs_count && !teleported; i++) {
            Stair *s = &maze->stairs[i];
            
            // Check if player is on start of stair
            if (player->floor == s->start_floor &&
                player->width == s->start_width &&
                player->length == s->start_length) {
                
                printf("Player %c encountered a stair at [%d, %d, %d] during movement (%d steps remaining).\n",
                       player->name, player->floor, player->width, player->length, steps_remaining);
                
                // Handle stair movement
                handleStairs(maze, player);
                teleported = true;
            }
            
            // Check if player is on end of stair
            else if (player->floor == s->end_floor &&
                player->width == s->end_width &&
                player->length == s->end_length) {
                
                printf("Player %c encountered a stair at [%d, %d, %d] during movement (%d steps remaining).\n",
                       player->name, player->floor, player->width, player->length, steps_remaining);
                
                // Handle stair movement
                handleStairs(maze, player);
                teleported = true;
            }
        }
        
        // Check for poles if not already teleported by stairs
        for (int i = 0; i < maze->poles_count && !teleported; i++) {
            Pole *p = &maze->poles[i];
            
            // Player must be on the pole's position
            if (player->width == p->width && player->length == p->length &&
                player->floor >= p->end_floor && player->floor <= p->start_floor) {
                
                printf("Player %c encountered a pole at [%d, %d, %d] during movement (%d steps remaining).\n",
                       player->name, player->floor, player->width, player->length, steps_remaining);
                
                // Handle pole movement
                handlePoles(maze, player);
                teleported = true;
            }
        }
        
        // If teleported, continue movement from new position
        if (teleported && steps_remaining > 0) {
            printf("Player %c continues movement with %d steps remaining from [%d, %d, %d].\n",
                   player->name, steps_remaining, player->floor, player->width, player->length);
        }
    }
    
    printf("Player %c completed %d steps and is now at [%d, %d, %d].\n",
           player->name, initial_steps, player->floor, player->width, player->length);
    return true;
}

void handleStairs(Maze *maze, Player *player) {
    for (int i = 0; i < maze->stairs_count; i++) {
        Stair *s = &maze->stairs[i];
        
        // Check if player is on start of stair
        if (player->floor == s->start_floor &&
            player->width == s->start_width &&
            player->length == s->start_length) {
            
            if (maze->blocks[s->end_floor][s->end_width][s->end_length] == STARTING_AREA) {
                printf("Player %c takes stairs to starting area at [%d, %d, %d].\n",
                       player->name, s->end_floor, s->end_width, s->end_length);
                printf("Player %c is returned to starting position and must roll 6 to re-enter maze.\n",
                       player->name);
                
                // Reset player to starting area state
                player->in_maze = false;
                player->in_starting_area = true;
                player->floor = s->end_floor;
                
                // Set to appropriate starting position based on player name
                if (player->name == 'A') { player->width = 6; player->length = 12; }
                else if (player->name == 'B') { player->width = 9; player->length = 8; }
                else if (player->name == 'C') { player->width = 9; player->length = 16; }
                
                return;
            }
            
            printf("Player %c takes the stairs and is now at [%d, %d, %d] in floor %d.\n",
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
            
            printf("Player %c takes the stairs and is now at [%d, %d, %d] in floor %d.\n",
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
        
        // Player must be on the pole's position
        if (player->width == p->width && player->length == p->length) {
            
            // Determine which direction the pole goes
            if (p->start_floor < p->end_floor) {
                // Pole goes upward (should not slide down)
                continue;
            }
            
            // Pole goes downward - check if player is above the end floor
            if (player->floor > p->end_floor && player->floor <= p->start_floor) {
                
                // Check if pole destination is in starting area
                if (maze->blocks[p->end_floor][p->width][p->length] == STARTING_AREA) {
                    printf("Player %c slides down pole to starting area at [%d, %d, %d].\n",
                           player->name, p->end_floor, p->width, p->length);
                    printf("Player %c is returned to starting position and must roll 6 to re-enter maze.\n",
                           player->name);
                    
                    // Reset player to starting area state
                    player->in_maze = false;
                    player->in_starting_area = true;
                    player->floor = p->end_floor;
                    
                    // Set to appropriate starting position
                    if (player->name == 'A') { player->width = 6; player->length = 12; }
                    else if (player->name == 'B') { player->width = 9; player->length = 8; }
                    else if (player->name == 'C') { player->width = 9; player->length = 16; }
                    
                    return;
                }

                printf("Player %c slides down and is now at [%d, %d, %d] in floor %d.\n",
                       player->name, p->end_floor, p->width, p->length, p->end_floor);
                player->floor = p->end_floor;
                return;
            }
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
        case 4: // Fallthrough for NONEzz
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