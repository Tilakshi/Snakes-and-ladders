// In maze.hjj 
#ifndef MAZE_H
#define MAZE_H

#define FLOORS 3
#define WIDTH 10
#define LENGTH 25
#define PLAYERS 3 // Assuming 3 players

// REPLACE these #defines:
typedef enum {
    WALKABLE,      // 0
    EMPTY,         // 1  
    STAIR,         // 2
    POLE,          // 3
    WALL,          // 4
    STARTING_AREA  // 5
} BlockType;

typedef enum {
    NORTH, EAST, SOUTH, WEST, NONE
} Direction;

typedef struct {
    int floor;
    int width;
    int length;
} Position;

typedef struct {
    int start_floor;
    int start_width;
    int start_length;
    int end_floor;
    int end_width;
    int end_length;
} Stair;

typedef struct {
    int start_floor;
    int end_floor;
    int width;
    int length;
} Pole;

typedef struct {
    int floor;
    int start_width;
    int start_length;
    int end_width;
    int end_length;
} Wall;


typedef struct {
    char name;
    int floor;
    int width;
    int length;
    Direction direction;
    int movement_points; // You're not actively decrementing this in the provided rollDice.
    bool in_starting_area; // True if player is outside the main maze, waiting for a 6
    bool in_maze;          // True if player has entered the maze
} Player;


typedef struct {
    BlockType blocks[FLOORS][WIDTH][LENGTH]; // 3D array for the maze
    Stair *stairs;
    int stairs_count;
    Pole *poles;
    int poles_count;
    Wall *walls;
    int walls_count;
    int flag_floor;
    int flag_width;
    int flag_length;
} Maze;

// Function Prototypes (ensure these match your definitions in maze.c)
void createMaze(Maze *maze);
void loadStairs(Maze *maze, const char *filename);
void loadPoles(Maze *maze, const char *filename);
void loadWalls(Maze *maze, const char *filename);
void loadFlag(Maze *maze, const char *filename);
void placeStairs(Maze *maze);
void placePoles(Maze *maze);
void placeWalls(Maze *maze);
void initializePlayers(Player players[]);
bool isValidPosition(int floor, int width, int length);
bool isWallBlocked(Maze *maze, int floor, int width, int length, Player *player);
bool movePlayer(Maze *maze, Player *player, int steps, int movement_value);
void handleStairs(Maze *maze, Player *player);
void handlePoles(Maze *maze, Player *player);
void rollDice(Player players[], Maze *maze, int player_index, bool use_direction_dice);
int rollMovementDice();
Direction rollDirectionDice();
const char* directionToString(Direction dir);
bool canPlaceStructure(int floor, int width, int length, Maze *maze);

#endif 