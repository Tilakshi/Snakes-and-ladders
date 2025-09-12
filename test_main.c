// test_main.c
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "maze.h"

void runTests(Maze *maze, Player players[]) {
    printf("===== RUNNING TESTS =====\n");
    
    // Test 1: Stair Bidirectionality
    printf("\n1. Testing stair bidirectionality...\n");
    if (maze->stairs_count > 0) {
        Stair test_stair = maze->stairs[0];
        printf("   Testing stair: [%d,%d,%d] -> [%d,%d,%d]\n",
               test_stair.start_floor, test_stair.start_width, test_stair.start_length,
               test_stair.end_floor, test_stair.end_width, test_stair.end_length);
        
        // Test forward direction
        Player test_player = players[0];
        test_player.floor = test_stair.start_floor;
        test_player.width = test_stair.start_width;
        test_player.length = test_stair.start_length;
        test_player.in_maze = true;
        
        handleStairs(maze, &test_player);
        printf("   Forward: [%d,%d,%d] -> [%d,%d,%d] %s\n",
               test_stair.start_floor, test_stair.start_width, test_stair.start_length,
               test_player.floor, test_player.width, test_player.length,
               (test_player.floor == test_stair.end_floor &&
                test_player.width == test_stair.end_width &&
                test_player.length == test_stair.end_length) ? "PASS" : "FAIL");
        
        // Test reverse direction
        test_player.floor = test_stair.end_floor;
        test_player.width = test_stair.end_width;
        test_player.length = test_stair.end_length;
        
        handleStairs(maze, &test_player);
        printf("   Reverse: [%d,%d,%d] -> [%d,%d,%d] %s\n",
               test_stair.end_floor, test_stair.end_width, test_stair.end_length,
               test_player.floor, test_player.width, test_player.length,
               (test_player.floor == test_stair.start_floor &&
                test_player.width == test_stair.start_width &&
                test_player.length == test_stair.start_length) ? "PASS" : "FAIL");
    }
    
    // Test 2: Pole Multi-floor Access
    printf("\n2. Testing pole multi-floor access...\n");
    if (maze->poles_count > 0) {
        Pole test_pole = maze->poles[0];
        printf("   Testing pole: floors %d->%d at [%d,%d]\n",
               test_pole.start_floor, test_pole.end_floor,
               test_pole.width, test_pole.length);
        
        // Test from middle floor
        if (test_pole.start_floor < test_pole.end_floor) {
            int middle_floor = (test_pole.start_floor + test_pole.end_floor) / 2;
            Player test_player = players[0];
            test_player.floor = middle_floor;
            test_player.width = test_pole.width;
            test_player.length = test_pole.length;
            test_player.in_maze = true;
            
            handlePoles(maze, &test_player);
            printf("   From floor %d: landed at floor %d %s\n",
                   middle_floor, test_player.floor,
                   (test_player.floor == test_pole.end_floor) ? "PASS" : "FAIL");
        }
    }
    
    // Test 3: Wall Blocking
    printf("\n3. Testing wall blocking...\n");
    if (maze->walls_count > 0) {
        Wall test_wall = maze->walls[0];
        printf("   Testing wall on floor %d from [%d,%d] to [%d,%d]\n",
               test_wall.floor, test_wall.start_width, test_wall.start_length,
               test_wall.end_width, test_wall.end_length);
        
        // Find a wall cell to test
        int test_width = test_wall.start_width;
        int test_length = test_wall.start_length;
        
        Player test_player = players[0];
        test_player.floor = test_wall.floor;
        test_player.width = test_width - 1; // Position before wall
        test_player.length = test_length;
        test_player.direction = EAST;
        test_player.in_maze = true;
        
        bool can_move = movePlayer(maze, &test_player, 1, 1);
        printf("   Wall blocking test: %s\n",
               (!can_move && test_player.width == test_width - 1) ? "PASS" : "FAIL");
    }
    
    // Test 4: Starting Area Re-entry
    printf("\n4. Testing starting area re-entry...\n");
    Player test_player = players[0];
    test_player.in_maze = true;
    test_player.in_starting_area = false;
    
    // Try to move into starting area
    test_player.floor = 0;
test_player.width = 4;  // Just outside starting area (width 4 is outside 0-3)
test_player.length = 12; // Within starting area length range (8-16)
test_player.direction = NORTH;
    bool moved = movePlayer(maze, &test_player, 1, 1);
    printf("   Starting area blocking: %s\n",
           (!moved && !test_player.in_starting_area) ? "PASS" : "FAIL");
    
    // Test 5: Movement and Direction
    printf("\n5. Testing movement and direction...\n");
    test_player = players[0];
    test_player.floor = 1;
    test_player.width = 5;
    test_player.length = 5;
    test_player.direction = NORTH;
    test_player.in_maze = true;
    
    int start_width = test_player.width;
    int start_length = test_player.length;
    
    movePlayer(maze, &test_player, 2, 2);
    printf("   North movement: from [%d,%d] to [%d,%d] %s\n",
           start_width, start_length, test_player.width, test_player.length,
           (test_player.width == start_width - 2) ? "PASS" : "FAIL");
    
    // Test 6: No Partial Movement
    printf("\n6. Testing no partial movement...\n");
    test_player = players[0];
    test_player.floor = 0;
    test_player.width = 0;  // Edge of maze
    test_player.length = 5;
    test_player.direction = NORTH; // Toward out of bounds
    test_player.in_maze = true;
    
    int original_width = test_player.width;
    bool result = movePlayer(maze, &test_player, 3, 3);
    printf("   No partial movement: %s\n",
           (!result && test_player.width == original_width) ? "PASS" : "FAIL");
    
    printf("\n===== TESTS COMPLETED =====\n");
}

int main() {
    Maze maze;
    createMaze(&maze);
    
    // Load TEST files instead of original ones
    loadStairs(&maze, "stairs.txt");
    loadPoles(&maze, "poles.txt");
    loadWalls(&maze, "walls.txt");
    loadFlag(&maze, "flag.txt");
    
    placeStairs(&maze);
    placePoles(&maze);
    placeWalls(&maze);
    
    Player players[PLAYERS];
    initializePlayers(players);
    
    runTests(&maze, players);
    
    free(maze.stairs);
    free(maze.poles);
    free(maze.walls);
    
    return 0;
}