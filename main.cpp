#include "raylib.h"

int main() {
    InitWindow(800, 600, "Flappy Bird");
    SetTargetFPS(60);

    // --- Bird state ---
    float birdX = 200;
    float birdY = 300;
    float birdSize = 40;
    float velocityY = 0;
    float gravity = 1500.0f;

    // --- Game state ---
    bool gameOver = false;
    int score = 0;

    // --- Pipes ---
    struct Pipe {
        float x;
        float gapY;
        bool scored;
    };
    const float pipeW = 70;
    const float gapH = 150;
    const float pipeSpeed = 200;
    const int PIPE_COUNT = 3;
    Pipe pipes[PIPE_COUNT];

    for (int i = 0; i < PIPE_COUNT; i++) {
        pipes[i].x = 800 + i * 300;
        pipes[i].gapY = GetRandomValue(100, 350);
        pipes[i].scored = false;
    }

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        // --- Update (only when alive) ---
        if (!gameOver) {
            if (IsKeyPressed(KEY_SPACE)) velocityY = -500;

            velocityY += gravity * dt;
            birdY += velocityY * dt;

            if (birdY + birdSize >= 600) { birdY = 600 - birdSize; gameOver = true; }
            if (birdY < 0) { birdY = 0; velocityY = 0; }

            // Move pipes
            for (int i = 0; i < PIPE_COUNT; i++) {
                pipes[i].x -= pipeSpeed * dt;

                // Score when bird passes pipe
                if (!pipes[i].scored && pipes[i].x + pipeW < birdX) {
                    score++;
                    pipes[i].scored = true;
                }

                // Recycle pipe off-screen
                if (pipes[i].x + pipeW < 0) {
                    float maxX = 0;
                    for (int j = 0; j < PIPE_COUNT; j++)
                        if (pipes[j].x > maxX) maxX = pipes[j].x;
                    pipes[i].x = maxX + 300;
                    pipes[i].gapY = GetRandomValue(100, 350);
                    pipes[i].scored = false;
                }
            }

            // Collision
            Rectangle birdRect = { birdX, birdY, birdSize, birdSize };
            for (int i = 0; i < PIPE_COUNT; i++) {
                Rectangle topPipe = { pipes[i].x, 0, pipeW, pipes[i].gapY };
                Rectangle botPipe = { pipes[i].x, pipes[i].gapY + gapH, pipeW, 600 };
                if (CheckCollisionRecs(birdRect, topPipe) ||
                    CheckCollisionRecs(birdRect, botPipe)) {
                    gameOver = true;
                }
            }
        } else {
            // Restart
            if (IsKeyPressed(KEY_R)) {
                birdY = 300;
                velocityY = 0;
                score = 0;
                gameOver = false;
                for (int i = 0; i < PIPE_COUNT; i++) {
                    pipes[i].x = 800 + i * 300;
                    pipes[i].gapY = GetRandomValue(100, 350);
                    pipes[i].scored = false;
                }
            }
        }

        // --- Draw ---
        BeginDrawing();
        ClearBackground(SKYBLUE);

        // Pipes
        for (int i = 0; i < PIPE_COUNT; i++) {
            DrawRectangle(pipes[i].x, 0, pipeW, pipes[i].gapY, GREEN);
            DrawRectangle(pipes[i].x, pipes[i].gapY + gapH, pipeW, 600, GREEN);
        }

        // Bird
        DrawRectangle(birdX, birdY, birdSize, birdSize, YELLOW);

        // HUD
        DrawText(TextFormat("Score: %d", score), 10, 10, 30, WHITE);
        if (gameOver) DrawText("GAME OVER - press R", 280, 250, 30, RED);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}