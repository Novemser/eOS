#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"
#include "io.h"
#include "system.h"
#include "time.h"
#include "stdarg.h"

void run();
void printMap();
void initMap();
void move(int dx, int dy);
void update();
void changeDirection(char key);
void generateFood();

char getMapValue(int value);

// Map dimensions
const int mapwidth = 20;
const int mapheight = 20;

const int size = 20 * 20;

// The tile values for the map
int map[400];

// Snake head details
int headxpos;
int headypos;
int direction;

// Amount of food the snake has (How long the body is)
int food = 3;

// Determine if game is running
int running;

int main()
{
    for (int i = 0; i < 400; ++i)
    {
        map[i]=0;
    }
    run();

    return 0;
}

// Main game function
void run()
{
    // Initialize the map
    initMap();
    running = 1;
    char rdbuf[128];

    while (running) {
        int r = read(0, rdbuf, 70);
        rdbuf[r] = 0;
        // If a key is pressed
        changeDirection(*rdbuf);
        // Upate the map
        update();

        // Print the map
        printMap();

        // wait 0.5 seconds
        //milli_delay(500);
    }

    // Print out game over text
    printf("Game Over!\nYour score is:%d\n", food);

    // Stop console from closing instantly
    //std::cin.ignore();
}

// Changes snake direction from input
void changeDirection(char key) {
    /*
      W
    A + D
      S

      1
    4 + 2
      3
    */
    switch (key) {
    case 'w':
        if (direction != 2) direction = 0;
        break;
    case 'd':
        if (direction != 3) direction = 1;
        break;
    case 's':
        if (direction != 4) direction = 2;
        break;
    case 'a':
        if (direction != 5) direction = 3;
        break;
    }
}

// Moves snake head to new location
void move(int dx, int dy) {
    // determine new head position
    int newx = headxpos + dx;
    int newy = headypos + dy;

    // Check if there is food at location
    if (map[newx + newy * mapwidth] == -2) {
        // Increase food value (body length)
        food++;

        // Generate new food on map
        generateFood();
    }

    // Check location is free
    else if (map[newx + newy * mapwidth] != 0) {
        running = 0;
    }

    // Move head to new location
    headxpos = newx;
    headypos = newy;
    map[headxpos + headypos * mapwidth] = food + 1;

}

int rand()
{
    struct time t;
    MESSAGE msg;
    msg.type = GET_RTC_TIME;
    msg.BUF= &t;
    send_recv(BOTH, TASK_SYS, &msg);

    return t.day*t.second;
}

// Generates new food on map
void generateFood() {
    int x = 0;
    int y = 0;
    do {
        // Generate random x and y values within the map
        x = rand() % (mapwidth - 2) + 1;
        y = rand() % (mapheight - 2) + 1;

        // If location is not free try again
    } while (map[x + y * mapwidth] != 0);

    // Place new food
    map[x + y * mapwidth] = -2;
}

// Updates the map
void update() {
    // Move in direction indicated
    switch (direction) {
    case 0: move(-1, 0);
        break;
    case 1: move(0, 1);
        break;
    case 2: move(1, 0);
        break;
    case 3: move(0, -1);
        break;
    }

    // Reduce snake values on map by 1
    for (int i = 0; i < size; i++) {
        if (map[i] > 0) map[i]--;
    }
}

// Initializes map
void initMap()
{
    // Places the initual head location in middle of map
    headxpos = mapwidth / 2;
    headypos = mapheight / 2;
    map[headxpos + headypos * mapwidth] = 1;

    // Places top and bottom walls 
    for (int x = 0; x < mapwidth; ++x) {
        map[x] = -1;
        map[x + (mapheight - 1) * mapwidth] = -1;
    }

    // Places left and right walls
    for (int y = 0; y < mapheight; y++) {
        map[0 + y * mapwidth] = -1;
        map[(mapwidth - 1) + y * mapwidth] = -1;
    }

    // Generates first food
    generateFood();
}

// Prints the map to console
void printMap()
{
    for (int x = 0; x < mapwidth; ++x) {
        for (int y = 0; y < mapheight; ++y) {
            // Prints the value at current x,y location
            printf("%c", getMapValue(map[x + y * mapwidth]));
        }
        // Ends the line for next x value
        printf("\n");
    }
}

// Returns graphical character for display from map value
char getMapValue(int value)
{
    // Returns a part of snake body
    if (value > 0) return 'o';

    switch (value) {
        // Return wall
    case -1: return 'X';
        // Return food
    case -2: return 'O';
    }
}

