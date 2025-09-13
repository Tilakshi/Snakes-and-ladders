#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#include "maze.h"



int main() {
    // Load seed value for random number generation
    unsigned int seed_value;
    FILE *seed_file = fopen("seed.txt", "r");
    
    if (seed_file != NULL) {
        if (fscanf(seed_file, "%u", &seed_value) == 1) {
            srand(seed_value);
            printf("Program seeded with value from seed.txt: %u\n", seed_value);
        } else {
            // Fallback to current time if seed file is invalid
            seed_value = (unsigned int)time(NULL);
            srand(seed_value);
            printf("Failed to read seed from seed.txt. Using time-based seed: %u\n", seed_value);
        }
        fclose(seed_file);
    } else {
        // Fallback to current time if seed file doesn't exist
        seed_value = (unsigned int)time(NULL);
        srand(seed_value);
        printf("seed.txt not found. Using time-based seed: %u\n", seed_value);
    }

    // Initialize maze
    Maze maze;
    createMaze(&maze); // Pass pointer to maze

    // Load maze elements - pass pointer to maze
    loadStairs(&maze, "stairs.txt");
    loadPoles(&maze, "poles.txt");
    loadWalls(&maze, "walls.txt");
    loadFlag(&maze, "flag.txt");

    // Place them in maze - pass pointer to maze
    placeStairs(&maze);
    placePoles(&maze);
    placeWalls(&maze);

    // Print maze layout for debugging - pass pointer to maze
    printf("\n=== INITIAL MAZE STATE ===\n");
    printMazeLayout(&maze);
    printMazeDetailed(&maze);

    // Initialize players
    Player players[PLAYERS];
    initializePlayers(players);
    
    // Print initial player positions
    printPlayerPositions(players, PLAYERS);

    bool game_over = false;
    int dice_throw_count = 0;

    while (!game_over) {
        printf("\n=== ROUND %d ===\n", dice_throw_count + 1);
        
        for (int i = 0; i < PLAYERS; i++) {
            // A direction dice is used every 4th throw AND if the player is in the maze
            bool use_direction_dice = (dice_throw_count % 4 == 0 && players[i].in_maze);
            
            // Pass pointer to maze and number of players
            rollDice(players, &maze, i, use_direction_dice, PLAYERS);

            if (players[i].in_maze &&
                players[i].floor == maze.flag_floor &&
                players[i].width == maze.flag_width &&
                players[i].length == maze.flag_length) {
                printf("Player %c captured the flag! Game over!\n", players[i].name);
                game_over = true;
                break;
            }
        }
        
        if (!game_over) {
            // Print player positions after each round
            printPlayerPositions(players, PLAYERS);
            dice_throw_count++;
        }
    }

    // Free allocated memory
    free(maze.stairs);
    free(maze.poles);
    free(maze.walls);

    // Function declarations for visualization
void printMazeLayout(Maze *maze);
void printMazeDetailed(Maze *maze);
void printPlayerPositions(Player players[], int num_players);

    return 0;
}