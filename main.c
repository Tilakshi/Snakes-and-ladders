#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h> // Keep this for fallback if seed.txt fails

#include "maze.h" // Include your maze header

// --- NEW FUNCTION TO LOAD SEED (OR INTEGRATED DIRECTLY IN MAIN) ---
// For clarity, I'll put it directly into main as discussed, but if you want
// a helper function, it would go *above* main in this file.

int main() {
    Maze maze;
    createMaze(&maze); // This now sets the fixed floor plan

    // Load maze elements (dynamic elements from files)
    loadStairs(&maze, "stairs.txt");
    loadPoles(&maze, "poles.txt");
    loadWalls(&maze, "walls.txt");
    loadFlag(&maze, "flag.txt");

    // Place them in maze (onto the pre-defined floor plan)
    placeStairs(&maze);
    placePoles(&maze);
    placeWalls(&maze);

    // Initialize players
    Player players[PLAYERS];
    initializePlayers(players); // Note: Initial player positions are set here, and 'in_starting_area' is true

    bool game_over = false;
    int dice_throw_count = 0;

    while (!game_over) {
        for (int i = 0; i < PLAYERS; i++) {
            // A direction dice is used every 4th throw AND if the player is in the maze
            bool use_direction_dice = (dice_throw_count % 4 == 0 && players[i].in_maze);
            rollDice(players, &maze, i, use_direction_dice);

            if (players[i].in_maze &&
                players[i].floor == maze.flag_floor &&
                players[i].width == maze.flag_width &&
                players[i].length == maze.flag_length) { // Corrected access to flag_length
                printf("Player %c captured the flag! Game over!\n", players[i].name);
                game_over = true;
                break;
            }
        }
        if (!game_over) {
            dice_throw_count++;
        }
    }

    // Free allocated memory
    free(maze.stairs);
    free(maze.poles);
    free(maze.walls);

    return 0;
}