#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include <stdlib.h>

// --- 1. DEFINITIONS ---
#define RGB15(r,g,b)  ((r)|((g)<<5)|((b)<<10))
#define SCREEN_W      240
#define SCREEN_H      160
#define VRAM          ((volatile u16*)0x06000000)

// Colors
#define C_BG          RGB15(1, 2, 1)      // Dark Green
#define C_ROAD        RGB15(6, 6, 6)      // Dark Grey
#define C_WHITE       RGB15(31,31,31)
#define C_RED         RGB15(26, 5, 5)
#define C_HUD         RGB15(4, 4, 4)
#define C_GOLD        RGB15(31,25,0)
#define C_FUEL_HI     RGB15(0, 31, 0)
#define C_FUEL_LO     RGB15(31, 0, 0)

// --- 2. SPRITE DATA (14x24 Pixels) ---
// 0=Trans, 1=Body, 2=Dark, 3=Glass, 4=Tire, 5=RedLt, 6=YelLt, 7=White, 9=Metal

const u8 SPRITE_CLASSIC[336] = {
    0,0,1,1,1,2,2,2,2,1,1,1,0,0, 0,1,6,1,1,2,2,2,2,1,1,6,1,0,
    0,1,6,1,1,1,7,7,1,1,1,6,1,0, 0,1,1,1,2,1,7,7,1,2,1,1,1,0,
    0,1,1,1,2,1,7,7,1,2,1,1,1,0, 0,1,1,1,1,1,7,7,1,1,1,1,1,0,
    4,1,1,1,1,3,7,7,3,1,1,1,1,4, 4,1,1,1,3,3,7,7,3,3,1,1,1,4,
    4,1,1,3,3,3,7,7,3,3,3,1,1,4, 4,1,1,3,3,3,7,7,3,3,3,1,1,4,
    0,1,1,3,3,3,7,7,3,3,3,1,1,0, 0,1,1,1,1,1,7,7,1,1,1,1,1,0,
    0,1,1,1,1,1,7,7,1,1,1,1,1,0, 0,1,1,1,1,1,7,7,1,1,1,1,1,0,
    0,1,1,1,1,1,7,7,1,1,1,1,1,0, 0,1,1,2,1,1,7,7,1,1,2,1,1,0,
    4,1,1,2,2,2,2,2,2,2,2,1,1,4, 4,1,1,2,2,2,2,2,2,2,2,1,1,4,
    4,1,1,2,2,2,2,2,2,2,2,1,1,4, 4,1,1,2,2,2,2,2,2,2,2,1,1,4,
    0,1,1,1,1,1,7,7,1,1,1,1,1,0, 0,1,1,5,5,1,7,7,1,5,5,1,1,0,
    0,1,1,5,5,1,1,1,1,5,5,1,1,0, 0,0,1,1,1,1,1,1,1,1,1,1,0,0
};

const u8 SPRITE_F1[336] = {
    0,0,0,0,1,1,1,1,1,1,0,0,0,0, 0,0,0,1,1,1,1,1,1,1,1,0,0,0,
    0,0,1,9,9,1,1,1,1,9,9,1,0,0, 4,4,0,0,0,1,1,1,1,0,0,0,4,4,
    4,4,0,9,9,1,7,7,1,9,9,0,4,4, 4,4,0,0,0,1,7,7,1,0,0,0,4,4,
    0,0,0,0,0,1,7,7,1,0,0,0,0,0, 0,0,0,0,0,1,3,3,1,0,0,0,0,0,
    0,0,0,0,1,1,3,3,1,1,0,0,0,0, 0,0,0,0,1,1,3,3,1,1,0,0,0,0,
    0,0,0,1,1,1,6,6,1,1,1,0,0,0, 0,0,0,1,1,1,1,1,1,1,1,0,0,0,
    0,0,1,1,1,1,1,1,1,1,1,1,0,0, 0,0,1,2,1,1,9,9,1,1,2,1,0,0,
    0,0,1,2,1,1,9,9,1,1,2,1,0,0, 0,0,1,2,1,1,9,9,1,1,2,1,0,0,
    4,4,0,0,0,1,1,1,1,0,0,0,4,4, 4,4,0,9,9,1,1,1,1,9,9,0,4,4,
    4,4,0,0,0,1,9,9,1,0,0,0,4,4, 0,0,0,0,0,1,9,9,1,0,0,0,0,0,
    0,0,1,1,1,1,1,1,1,1,1,1,0,0, 0,0,1,1,1,1,1,1,1,1,1,1,0,0,
    0,0,1,1,1,1,5,5,1,1,1,1,0,0, 0,0,0,0,0,2,2,2,2,0,0,0,0,0
};

const u8 SPRITE_TRUCK[336] = {
    0,1,1,1,1,1,1,1,1,1,1,1,1,0, 0,1,1,9,9,9,9,9,9,9,9,1,1,0,
    0,1,6,9,9,9,9,9,9,9,9,6,1,0, 0,1,6,1,1,1,1,1,1,1,1,6,1,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,2,2,2,2,2,2,2,2,2,2,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1, 4,1,3,3,3,3,3,3,3,3,3,3,1,4,
    4,1,3,3,3,3,3,3,3,3,3,3,1,4, 4,1,1,1,1,1,1,1,1,1,1,1,1,4,
    4,1,5,5,5,5,5,5,5,5,5,5,1,4, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    0,2,2,9,9,9,9,9,9,9,9,2,2,0, 4,1,1,1,1,1,1,1,1,1,1,1,1,4,
    4,1,7,7,7,7,7,7,7,7,7,7,1,4, 4,1,7,7,7,7,7,7,7,7,7,7,1,4,
    4,1,7,7,7,7,7,7,7,7,7,7,1,4, 4,1,7,7,7,7,7,7,7,7,7,7,1,4,
    0,1,7,7,7,7,7,7,7,7,7,7,1,0, 0,1,7,7,7,7,7,7,7,7,7,7,1,0,
    4,1,7,7,7,7,7,7,7,7,7,7,1,4, 4,1,7,7,7,7,7,7,7,7,7,7,1,4,
    0,1,1,1,1,5,5,5,5,1,1,1,1,0, 0,0,1,1,1,1,1,1,1,1,1,1,0,0
};

const u8 SPRITE_TANK[336] = {
    0,0,0,0,0,0,9,9,0,0,0,0,0,0, 0,0,0,0,0,0,9,9,0,0,0,0,0,0,
    0,0,0,0,0,0,9,9,0,0,0,0,0,0, 0,2,2,1,1,1,1,1,1,1,1,2,2,0,
    0,2,2,1,1,1,1,1,1,1,1,2,2,0, 0,2,2,1,1,1,2,2,1,1,1,2,2,0,
    0,2,2,1,1,2,2,2,2,1,1,2,2,0, 0,2,2,1,1,2,2,2,2,1,1,2,2,0,
    0,2,2,1,1,2,2,2,2,1,1,2,2,0, 0,2,2,1,1,2,2,2,2,1,1,2,2,0,
    0,2,2,1,1,1,2,2,1,1,1,2,2,0, 0,2,2,1,1,1,1,1,1,1,1,1,2,2,0,
    0,2,2,1,1,1,1,1,1,1,1,2,2,0, 0,2,2,1,1,1,1,1,1,1,1,1,2,2,0,
    0,2,2,1,1,1,1,1,1,1,1,2,2,0, 0,2,2,1,1,1,1,1,1,1,1,1,2,2,0,
    0,2,2,1,1,1,1,1,1,1,1,2,2,0, 0,2,2,1,1,1,1,1,1,1,1,1,2,2,0,
    0,2,2,1,1,1,1,1,1,1,1,2,2,0, 0,2,2,1,1,1,1,1,1,1,1,1,2,2,0,
    0,2,2,1,1,1,5,5,1,1,1,2,2,0, 0,2,2,1,1,1,1,1,1,1,1,1,2,2,0,
    0,2,2,1,1,1,1,1,1,1,1,2,2,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

// --- 3. GAME STRUCTURES ---
typedef struct {
    float x, y;
    float speed;
    float maxSpeed;
    float fuel;
    float maxFuel;
    u16 color;
    int type; // 0=Classic, 1=F1, 2=Truck, 3=Tank
    int active;
} Car;

typedef struct {
    float x, y;
    int active;
    int type; // 0=Coin, 1=Fuel
} Item;

Car player;
Car enemies[3];
Item items[2];

float roadY = 0;
float distance = 0;
int gameState = 0; // 0=Menu, 1=Play, 2=GameOver
int highScore = 0;

// --- 4. RENDERER ---
void drawRect(int x, int y, int w, int h, u16 color) {
    for(int i=0; i<h; i++) {
        for(int j=0; j<w; j++) {
            int dx = x+j; int dy = y+i;
            if(dx >= 0 && dx < SCREEN_W && dy >= 0 && dy < SCREEN_H) {
                VRAM[dy*SCREEN_W + dx] = color;
            }
        }
    }
}

u16 getPixelColor(int id, u16 bodyColor) {
    switch(id) {
        case 1: return bodyColor;
        case 2: return RGB15(5,5,5);   // Dark Grey
        case 3: return RGB15(10,20,31);// Glass
        case 4: return RGB15(2,2,2);   // Tire
        case 5: return RGB15(31,0,0);  // Red Light
        case 6: return RGB15(31,31,0); // Yellow Light
        case 7: return RGB15(31,31,31);// White
        case 9: return RGB15(15,15,15);// Metal
        default: return 0; // Transparent
    }
}

void drawCar(Car* c) {
    int px = (int)c->x;
    int py = (int)c->y;
    if(py < -24 || py > SCREEN_H) return;

    const u8* sprite = SPRITE_CLASSIC;
    if(c->type == 1) sprite = SPRITE_F1;
    if(c->type == 2) sprite = SPRITE_TRUCK;
    if(c->type == 3) sprite = SPRITE_TANK;

    for(int i=0; i<336; i++) {
        int r = i / 14;
        int col = i % 14;
        int pType = sprite[i];
        
        if(pType != 0) {
            u16 color = getPixelColor(pType, c->color);
            int dx = px + col; 
            int dy = py + r;
            if(dx >=0 && dx < SCREEN_W && dy >=0 && dy < SCREEN_H) {
                VRAM[dy*SCREEN_W + dx] = color;
            }
        }
    }
}

void drawText(int x, int y, const char* str, u16 color) {
    // A very simple debug text drawer (uses GBA default font if available, 
    // or typically you'd need a font system. We'll use rectangles for "mock" text)
    // Real text requires including a font header. We will use dots for simplicity.
    int cx = x;
    while(*str) {
        if(*str >= '0' && *str <= '9') {
            // Draw a number of dots equal to the digit
            int d = *str - '0';
            if(d==0) d=10; 
            drawRect(cx, y, 2, d, color);
        } else {
             drawRect(cx, y, 3, 3, color);
        }
        cx += 5;
        str++;
    }
}

// --- 5. LOGIC ---
void initGame() {
    player.x = SCREEN_W / 2 - 7;
    player.y = SCREEN_H - 40;
    player.speed = 0;
    player.maxSpeed = 3.0;
    player.fuel = 100;
    player.maxFuel = 100;
    player.color = C_RED;
    player.type = 0; // Starts with Classic
    player.active = 1;

    for(int i=0; i<3; i++) enemies[i].active = 0;
    for(int i=0; i<2; i++) items[i].active = 0;
    distance = 0;
}

int rectCollide(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2) {
    return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2);
}

void update() {
    scanKeys();
    u16 keys = keysHeld();
    u16 keysD = keysDown();

    // Player
    if(keys & KEY_LEFT) player.x -= 1.5;
    if(keys & KEY_RIGHT) player.x += 1.5;
    if(player.speed < player.maxSpeed) player.speed += 0.02;
    
    // Bounds
    if(player.x < 20) player.x = 20;
    if(player.x > SCREEN_W - 34) player.x = SCREEN_W - 34;
    
    // Fuel
    player.fuel -= 0.05;
    if(player.fuel <= 0) gameState = 2;

    // Road
    roadY += player.speed;
    distance += player.speed;
    if(roadY >= 20) roadY = 0;

    // Enemies
    for(int i=0; i<3; i++) {
        if(enemies[i].active) {
            enemies[i].y += (player.speed * 0.5) + 1.0;
            
            // Collision
            if(rectCollide(player.x, player.y, 14, 24, enemies[i].x, enemies[i].y, 14, 24)) {
                player.fuel -= 15; // Crash penalty
                enemies[i].active = 0;
            }
            if(enemies[i].y > SCREEN_H) enemies[i].active = 0;
        } else if((rand() % 100) < 2) {
            enemies[i].active = 1;
            enemies[i].x = 25 + (rand() % (SCREEN_W - 60));
            enemies[i].y = -30;
            enemies[i].color = RGB15(rand()%31, rand()%31, rand()%31);
            enemies[i].type = rand() % 4; // Random Type
        }
    }

    // Items
    for(int i=0; i<2; i++) {
        if(items[i].active) {
            items[i].y += player.speed;
            if(rectCollide(player.x, player.y, 14, 24, items[i].x, items[i].y, 8, 8)) {
                if(items[i].type == 0) highScore += 50; 
                else player.fuel = (player.fuel + 20 > 100) ? 100 : player.fuel + 20;
                items[i].active = 0;
            }
            if(items[i].y > SCREEN_H) items[i].active = 0;
        } else if((rand() % 200) < 2) {
            items[i].active = 1;
            items[i].x = 30 + (rand() % (SCREEN_W - 70));
            items[i].y = -20;
            items[i].type = rand() % 2; 
        }
    }
}

// --- 6. MAIN ---
int main() {
    // Mode 3 (Bitmap), BG2 Enabled
    REG_DISPCNT = MODE_3 | BG2_ENABLE;

    initGame();

    while(1) {
        VBlankIntrWait();

        if(gameState == 0) { // MENU
            drawRect(0,0,SCREEN_W,SCREEN_H, C_BG);
            drawRect(60, 50, 120, 40, C_RED); // "Logo" background
            // Draw a spinning car preview
            Car p = {113, 100, 0,0,0,0, C_RED, (distance/10) , 1}; // Use distance as timer for animation
            p.type = ((int)distance / 50) % 4; // Cycle types
            drawCar(&p);
            distance++; // Fake timer

            scanKeys();
            if(keysDown() & KEY_START) {
                gameState = 1;
                initGame();
            }

        } else if(gameState == 1) { // PLAY
            update();

            // Background
            drawRect(0, 0, SCREEN_W, SCREEN_H, C_BG);
            drawRect(20, 0, SCREEN_W - 40, SCREEN_H, C_ROAD);

            // Stripes
            for(int i=-1; i<9; i++) {
                int y = i*20 + (int)roadY;
                u16 col = (i%2==0) ? C_WHITE : C_RED;
                drawRect(20, y, 5, 20, col);
                drawRect(SCREEN_W-25, y, 5, 20, col);
            }

            // Entities
            for(int i=0; i<2; i++) {
                if(items[i].active) drawRect((int)items[i].x, (int)items[i].y, 6, 6, (items[i].type==0)?C_GOLD:C_RED);
            }
            for(int i=0; i<3; i++) {
                if(enemies[i].active) drawCar(&enemies[i]);
            }
            drawCar(&player);

            // HUD
            drawRect(5, 5, 50, 4, C_HUD);
            drawRect(5, 5, (int)(player.fuel/2), 4, (player.fuel>30)?C_FUEL_HI:C_FUEL_LO);

        } else { // GAME OVER
            drawRect(0,0,SCREEN_W,SCREEN_H, C_RED);
            scanKeys();
            if(keysDown() & KEY_START) gameState = 0;
        }
    }
}
