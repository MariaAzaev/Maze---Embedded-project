/*
 * File:   LabC5.c
 * Author: Maria Azaev
 *
 * Created on August 20, 2023
 * 
 */

#include <stdlib.h>
#include <stdio.h>

#include "System/system.h"
#include "System/delay.h"
#include "oledDriver/oledC.h"
#include "oledDriver/oledC_colors.h"
#include "oledDriver/oledC_shapes.h"
#include "i2cDriver/i2c1_driver.h"
#include "Accel_i2c.h"

#define MAX_WALLS 100
#define SOME_THRESHOLD 2000
#define MOVEMENT_SCALE 2

#define EASY 1
#define MEDIUM 2
#define HARD 3

int currentDifficulty = MEDIUM;
int timeRemaining = 0;
float ballMovementSpeed = 0.0;

const struct {
    int timeAllowed;  
    float movementSpeed;
} difficultyParameters[] = {
    [EASY] = {40, 0.8},
    [MEDIUM] = {30, 1.0},
    [HARD] = {20, 1.2}
};

typedef struct {
    int x;
    int y;
    int radius;
} Endpoint;

typedef struct {
    char type;  
    int x;
    int y;
    int length; 
} Wall;

Wall walls[MAX_WALLS];
int wallCount = 0;

typedef struct {
    int x;
    int y;
    int radius;
} Ball;

Ball ball;


void AddWall(char type, int x, int y, int length) {
    if (wallCount < MAX_WALLS) {
        walls[wallCount].type = type;
        walls[wallCount].x = x;
        walls[wallCount].y = y;
        walls[wallCount].length = length;
        wallCount++;
    }
}

void InitializeWalls1() {
    AddWall('H', 32, 15, 60);
    AddWall('H', 34, 38, 33);
    AddWall('H', 67, 57, 14);
    AddWall('H', 17, 75, 43);
    AddWall('H', 0, 91, 33);
    AddWall('H', 52, 86, 33);
    AddWall('H', 10, 109, 33);
    AddWall('H', 52, 109, 33);
    AddWall('H', 15, 138, 33);
    AddWall('H', 45, 153, 33);
    AddWall('H', 10, 171, 26);
    AddWall('H', 62, 182, 33);
   
    AddWall('V', 15, 38, 27);
    AddWall('V', 67, 30, 35);
    AddWall('V', 35, 125, 42);
    AddWall('V', 86, 122, 63);
    AddWall('V', 10, 159, 63);
    AddWall('V', 67, 153, 22);
    AddWall('V', 51, 175, 22);
    AddWall('V', 79, 163, 22);
    AddWall('V', 36, 192, 42); 
}

void InitializeWalls2() {
    AddWall('H', 32, 15, 60);
    AddWall('H', 10, 52, 33);
    AddWall('H', 17, 75, 43);
    AddWall('H', 0, 91, 33);
    AddWall('H', 62, 93, 33);
    AddWall('H', 29, 109, 33);
    AddWall('H', 19, 135, 33);
   
    AddWall('V', 10, 20, 72);
    AddWall('V', 70, 50, 63);
    AddWall('V', 10, 159, 63);
    AddWall('V', 77, 131, 22);
    AddWall('V', 70, 165, 22);
    AddWall('V', 46, 192, 42);
}

void InitializeWalls3() {
    AddWall('H', 32, 15, 60);
    AddWall('H', 44, 55, 33);
    AddWall('H', 0, 79, 33);
    AddWall('H', 52, 86, 33);
    AddWall('H', 59, 105, 33);
    AddWall('H', 10, 116, 33);
    AddWall('H', 40, 131, 33);
   
    AddWall('V', 10, 40, 25);
    AddWall('V', 10, 159, 63);
    AddWall('V', 25, 192, 42);
    AddWall('V', 57, 181, 22);
    AddWall('V', 81, 165, 22); 
}

void InitializeWalls4() {
    AddWall('H', 32, 15, 60);
    AddWall('H', 62, 81, 33);
    AddWall('H', 20, 40, 33);
    AddWall('H', 62, 118, 33);
   
    AddWall('V', 10, 40, 25);
    AddWall('V', 70, 38, 23);
    AddWall('V', 37, 62, 22);
    AddWall('V', 85, 59, 22);
    AddWall('V', 29, 125, 44);
    AddWall('V', 17, 192, 42);
    AddWall('V', 58, 192, 22);
    AddWall('V', 81, 165, 22);
}

void DrawWall(const Wall* wall, uint16_t color) {
    if (wall->type == 'H') {
        for(int x = wall->x; x < (wall->x + wall->length); x++) {
            oledC_DrawPoint(x, wall->y, color);
        }
    } else if (wall->type == 'V') { 
        for(int y = wall->y; y < (wall->y + wall->length); y++) {
            oledC_DrawPoint(wall->x, y, color);
        }
    }
}

void DrawMoreWalls() {
    for (int i = 0; i < wallCount; i++) {
        uint16_t wallColor = OLEDC_COLOR_WHITE; 
        
        if (walls[i].type == 'H') {
            wallColor = OLEDC_COLOR_BLUE;
        } else if (walls[i].type == 'V') {
            wallColor = OLEDC_COLOR_GREEN; 
        }
        
        DrawWall(&walls[i], wallColor);
    }
}

void InitializeBall() {
    ball.x = 10; 
    ball.y = 10;
    ball.radius = 3; 
}

void DrawBall(Ball* b, uint16_t color) {
    if (color == OLEDC_COLOR_RED) {
        oledC_DrawCircle(b->x, b->y, b->radius, OLEDC_COLOR_RED);
    } else if (color == OLEDC_COLOR_BLUE) {
        oledC_DrawCircle(b->x, b->y, b->radius, OLEDC_COLOR_BLUE);
    } else if (color == OLEDC_COLOR_GREEN) {
        oledC_DrawCircle(b->x, b->y, b->radius, OLEDC_COLOR_GREEN);
    } else {
        oledC_DrawCircle(b->x, b->y, b->radius, color);
    }
}

int IsCollision(int new_x, int new_y) {
    for (int i = 0; i < wallCount; i++) {
        Wall w = walls[i];
        if (w.type == 'H') {
            if ((new_y >= w.y && new_y <= w.y + 2) && 
                ((new_x >= w.x && new_x <= w.x + w.length) || (ball.x >= w.x && ball.x <= w.x + w.length))) {
                return 1;
            }
        } else if (w.type == 'V') {
            if ((new_x >= w.x && new_x <= w.x + 2) && 
                ((new_y >= w.y && new_y <= w.y + w.length) || (ball.y >= w.y && ball.y <= w.y + w.length))) {
                return 1;
            }
        }
    }
    return 0;
}

void MoveTheBall(Ball* b, int accel_x, int accel_y) {
    int new_x = b->x - (int)(accel_x * ballMovementSpeed * MOVEMENT_SCALE);
    int new_y = b->y + (int)(accel_y * ballMovementSpeed * MOVEMENT_SCALE);

    if (!IsCollision(new_x, b->y)) {
        DrawBall(b, OLEDC_COLOR_BLACK); 
        b->x = new_x;
        DrawBall(b, OLEDC_COLOR_RED);  
    }

    if (!IsCollision(b->x, new_y)) {
        DrawBall(b, OLEDC_COLOR_BLACK);
        b->y = new_y;
        DrawBall(b, OLEDC_COLOR_RED);
    }
}

void User_Initialize(void)
{
    TRISA |= (1<<11);
    TRISA &= ~(1<<8);
    TRISA &= ~(1<<9);
    
    TRISB |= (1<<12);
    ANSB |= (1<<12);

    AD1CON1 = 0x00;
    AD1CON1bits.SSRC = 0;
    AD1CON1bits.FORM = 0;
    AD1CON1bits.MODE12 = 0;
    AD1CON1bits.ADON = 1;

    AD1CON2 = 0;
   
    AD1CON3 = 0x00;
    AD1CON3bits.ADCS = 0xFF;
    AD1CON3bits.SAMC = 0x10;
    
}

void errorStop(char *msg)
{
    oledC_DrawString(0, 20, 2, 2, msg, OLEDC_COLOR_DARKRED);
    for (;;)
        ;
}


int S1Button() {
    static int prevButtonState = 1; 
    int currentButtonState = (PORTA & (1 << 11)) ? 1 : 0; 

    if (currentButtonState == 0 && prevButtonState == 1) {
        prevButtonState = currentButtonState;
        return 1; 
    } else if (currentButtonState == 1) { 
        prevButtonState = currentButtonState;
    }

    return 0; 
}


int S2Button() {
    static int prevButtonState = 1;  
    int currentButtonState = (PORTA & (1 << 12)) ? 1 : 0;  

    if (currentButtonState == 0 && prevButtonState == 1) { 
        prevButtonState = currentButtonState;
        return 1; 
    } else if (currentButtonState == 1) { 
        prevButtonState = currentButtonState;
    }

    return 0;
}

void LevelMenu(int level) {
    oledC_DrawString(10, 20, 1, 1, "Difficulty:", OLEDC_COLOR_BLUE);

    const char* levelOptions[] = {"Easy", "Medium", "Hard"}; 

    int yOffset = 35;  
    int spacing = 15; 

    for (int i = 0; i < sizeof(levelOptions) / sizeof(levelOptions[0]); i++) {
        if (i + 1 == level) {
            oledC_DrawString(20, yOffset + i * spacing, 1, 1, levelOptions[i], OLEDC_COLOR_RED);
        } else {
            oledC_DrawString(20, yOffset + i * spacing, 1, 1, levelOptions[i], OLEDC_COLOR_WHITE);
        }
    }
}

int SelectLevel() {
    int pot = 0;
    int pot1 = 0;
    int level = 1; 
    LevelMenu(level);

    int potRange = 1023;
    int threshold1 = potRange / 3;
    int threshold2 = 2 * potRange / 3;

    while (1) {
        AD1CHS = 0x0008;
        AD1CON1bits.SAMP = 1;
        for (int i = 0; i < 100; i++);
        AD1CON1bits.SAMP = 0;
        while (AD1CON1bits.DONE == 0);
        pot = ADC1BUF0;

        if (pot < threshold1) {
            oledC_DrawRectangle(3, 0, 15, 8, OLEDC_COLOR_BLACK);
            level = 1;
        } else if (pot < threshold2) {
            oledC_DrawRectangle(3, 0, 15, 8, OLEDC_COLOR_BLACK);
            level = 2;
        } else {
            oledC_DrawRectangle(3, 0, 15, 8, OLEDC_COLOR_BLACK);
            level = 3;
        }

        if (pot1 != pot) {
            LevelMenu(level);
            pot1 = pot;
        }

        if (S2Button()) {
            currentDifficulty = level;
            timeRemaining = difficultyParameters[currentDifficulty].timeAllowed;
            ballMovementSpeed = difficultyParameters[currentDifficulty].movementSpeed; 
            return level;
        }
    }
}

void MazeMenu(int maze) {
    oledC_DrawString(10, 20, 1, 1, "Maze Type:", OLEDC_COLOR_BLUE);

    const char* mazeOptions[] = {"Maze A", "Maze B", "Maze C", "Maze D"};

    int yOffset = 35;  
    int spacing = 15; 

    for (int i = 0; i < sizeof(mazeOptions) / sizeof(mazeOptions[0]); i++) {
        if (i + 1 == maze) {
            oledC_DrawString(20, yOffset + i * spacing, 1, 1, mazeOptions[i], OLEDC_COLOR_RED);
        } else {
            oledC_DrawString(20, yOffset + i * spacing, 1, 1, mazeOptions[i], OLEDC_COLOR_WHITE);
        }
    }
}

int SelectMaze() {
    int pot = 0;
    int pot1 = 0;
    int maze = 1; 
    MazeMenu(maze);

    int potRange = 1023; 
    int threshold1 = potRange / 3;
    int threshold2 = 2 * potRange / 3; 
    int threshold3 = 768; 

    while (1) {
        AD1CHS = 0x0008;
        AD1CON1bits.SAMP = 1;
        for (int i = 0; i < 100; i++);
        AD1CON1bits.SAMP = 0;
        while (AD1CON1bits.DONE == 0);
        pot = ADC1BUF0;

        if (pot < threshold1) {   
            oledC_DrawRectangle(3, 0, 15, 8, OLEDC_COLOR_BLACK);
            maze = 1;
        } else if (pot < threshold2) {
            oledC_DrawRectangle(3, 0, 15, 8, OLEDC_COLOR_BLACK);
            maze = 2;
        } else if (pot < threshold3) {
            oledC_DrawRectangle(3, 0, 15, 8, OLEDC_COLOR_BLACK);
            maze = 3;
        } else {                 
            oledC_DrawRectangle(3, 0, 15, 8, OLEDC_COLOR_BLACK);
            maze = 4;
        }

        if (pot1 != pot) {
            MazeMenu(maze);
            pot1 = pot;
        }

        if (S2Button()) {
            return maze;
        }
    }
}


void displayTimer(int timeRemaining) {
    char timeText[10];
    sprintf(timeText, "%02ds", timeRemaining);
    oledC_DrawRectangle(0, 0, 20, 8, OLEDC_COLOR_BLACK); 
    oledC_DrawString(0, 0, 1, 1, timeText, OLEDC_COLOR_WHITE);
}

int calculateScore(int timeRemaining, int difficulty) {
    int baseScore = 1000;
    int timeFactor = 10;
    int difficultyFactor = 100;
    
    int score = baseScore + (timeFactor * (difficultyParameters[difficulty].timeAllowed - timeRemaining))
                           + (difficultyFactor * difficulty);
    
    return score;
}


/*
                         Main application
 */
int main(void)
{
    unsigned char id = 0;
    I2Cerror rc;
    int x, y, z;
    unsigned char xyz[6] = {0};
    int pot = 0;
    int pot1 = 0;
   
    SYSTEM_Initialize();
    User_Initialize();

    oledC_setBackground(OLEDC_COLOR_BLACK);
    oledC_clearScreen();

    i2c1_driver_driver_close();
    i2c1_open();

    rc = i2cReadSlaveRegister(0x3A, 0, &id);

    if (rc == OK)
        if(id==0xE5)
            oledC_DrawString(10, 10, 2, 2, "ADXL345", OLEDC_COLOR_BLACK);
        else
            errorStop("Acc!Found");
    else
        errorStop("I2C Error");

    rc = i2cWriteSlave(0x3A, 0x2D, 8);
   
   
    int difficulty = 1; 
    difficulty = SelectLevel();
    oledC_clearScreen();
    
    int maze = 1; 
    maze = SelectMaze();
    oledC_clearScreen();
    
    Endpoint endpoint;
    endpoint.x = 30;
    endpoint.y = 4;
    endpoint.radius = 3;
    DrawBall(&endpoint, OLEDC_COLOR_YELLOW);
    
    switch (maze) {
        case 1:
            InitializeWalls1();
            endpoint.x = 30;
            endpoint.y = 4;
            break;
        case 2:
            InitializeWalls2();
            endpoint.x = 30;
            endpoint.y = 4;
            break;
        case 3:
            InitializeWalls3();
            endpoint.x = 30;
            endpoint.y = 4;
            break;
        case 4:
            InitializeWalls4();
            endpoint.x = 30;
            endpoint.y = 4;
            break;
    }
    
    DrawMoreWalls();
    
    Ball redBall;
    redBall.x = 87;
    redBall.y = 7;
    redBall.radius = 3;
    DrawBall(&redBall, OLEDC_COLOR_RED);
    
    for (;;) {
        int is_collision_endpoint = ((redBall.x >= endpoint.x - endpoint.radius) &&
                                     (redBall.x <= endpoint.x + endpoint.radius) &&
                                     (redBall.y >= endpoint.y - endpoint.radius) &&
                                     (redBall.y <= endpoint.y + endpoint.radius));

        if (is_collision_endpoint) {
            oledC_DrawString(0, 40, 2, 2, "You", OLEDC_COLOR_GREEN);
            oledC_DrawString(40, 60, 2, 2, "won!", OLEDC_COLOR_GREEN);
            DELAY_milliseconds(1000);
        
        oledC_DrawString(0, 80, 1, 1, "Press any button", OLEDC_COLOR_WHITE);
    
        while (!S1Button() && !S2Button()) {
   
        }
    
        oledC_DrawRectangle(0, 100, 128, 30, OLEDC_COLOR_BLACK);
    
        int score = calculateScore(timeRemaining, currentDifficulty);
        char scoreText[20];
        sprintf(scoreText, "Your score: %d", score);
        oledC_DrawString(0, 60, 1, 1, scoreText, OLEDC_COLOR_WHITE);

        DELAY_milliseconds(2000);

        break;
}
    
        if (timeRemaining > 0) {
            displayTimer(timeRemaining);
            DELAY_milliseconds(1000);
            timeRemaining--;
        } else {
            oledC_DrawString(0, 40, 2, 2, "Time's", OLEDC_COLOR_RED);
            oledC_DrawString(40, 60, 2, 2, "up!", OLEDC_COLOR_RED);
            oledC_DrawString(0, 80, 1, 1, "Press any button", OLEDC_COLOR_WHITE);

            while (!S1Button() && !S2Button()) {
       
            }

            oledC_DrawRectangle(10, 100, 80, 30, OLEDC_COLOR_BLACK);
            break; 
    }
    
    
        for (int i = 0; i < 6; ++i) {
            rc = i2cReadSlaveRegister(0x3A, 0x32 + i, &xyz[i]);
            DELAY_microseconds(1000);
        }

        x = xyz[0] + xyz[1] * 256;
        y = xyz[2] + xyz[3] * 256;
        z = xyz[4] + xyz[5] * 256;

        MoveTheBall(&redBall, x / 100, y / 100);
    }
}