#include "raylib.h"
#include <math.h>

int main() {
    // 1. Setup Window
    InitWindow(800, 600, "Counter-Strike 2D");
    SetTargetFPS(60);

    // 2. Player Variables
    Vector2 playerPos = { 400, 530 }; // Start below the middle divider
    float playerRadius = 15.0f;
    float playerSpeed = 300.0f;       // Pixels per second

    // 3. Map Variables (Walls and Crates)
    Rectangle walls[] = {
        // Outer Walls
        {0, 0, 800, 20},       // Top
        {0, 580, 800, 20},     // Bottom
        {0, 0, 20, 600},       // Left
        {780, 0, 20, 600},     // Right
        // Middle Divider
        {380, 100, 40, 400},
        // Crates (Cover)
        {150, 200, 60, 60},
        {600, 200, 60, 60},
        {200, 400, 60, 60},
        {550, 400, 60, 60}
    };
    int wallCount = sizeof(walls) / sizeof(walls[0]);

    // 4. Game Loop
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        // --- UPDATE LOGIC ---
        
        // Save the old position in case we hit a wall
        Vector2 oldPos = playerPos;

        // Move X (Left/Right)
        if (IsKeyDown(KEY_A)) playerPos.x -= playerSpeed * dt;
        if (IsKeyDown(KEY_D)) playerPos.x += playerSpeed * dt;

        // Check X Collision
        for (int i = 0; i < wallCount; i++) {
            if (CheckCollisionCircleRec(playerPos, playerRadius, walls[i])) {
                playerPos.x = oldPos.x; // Undo the X movement
            }
        }

        // Move Y (Up/Down)
        if (IsKeyDown(KEY_W)) playerPos.y -= playerSpeed * dt;
        if (IsKeyDown(KEY_S)) playerPos.y += playerSpeed * dt;

        // Check Y Collision
        for (int i = 0; i < wallCount; i++) {
            if (CheckCollisionCircleRec(playerPos, playerRadius, walls[i])) {
                playerPos.y = oldPos.y; // Undo the Y movement
            }
        }

        // --- DRAWING ---
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Draw Bomb Sites
        DrawRectangle(50, 50, 150, 150, Fade(RED, 0.3f));   // Site A
        DrawRectangle(600, 50, 150, 150, Fade(RED, 0.3f));  // Site B

        // Draw Walls and Crates
        for (int i = 0; i < wallCount; i++) {
            if (i < 5) DrawRectangleRec(walls[i], DARKGRAY);
            else       DrawRectangleRec(walls[i], BROWN);
        }

        // Draw Player
        DrawCircleV(playerPos, playerRadius, BLUE);

        // Draw Crosshair
        Vector2 mousePos = GetMousePosition();
        DrawCircleLines(mousePos.x, mousePos.y, 10, RED);
        DrawLine(mousePos.x - 15, mousePos.y, mousePos.x + 15, mousePos.y, RED);
        DrawLine(mousePos.x, mousePos.y - 15, mousePos.x, mousePos.y + 15, RED);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}