// Game.h - owns every object in the game (composition) and runs the main loop.
#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "Map.h"
#include "Player.h"
#include "Enemy.h"
#include "Bullet.h"
#include "Bomb.h"
#include <memory>
#include <string>
#include <vector>

enum class GameState { MENU, PLAYING, PAUSED, ROUND_WON, GAME_OVER, VICTORY };

class Game
{
public:
    enum { SCREEN_W = 1280, SCREEN_H = 720, TOTAL_ROUNDS = 5 };

    Game();
    ~Game();

    void run();    // opens the window and runs the loop until the player quits

private:
    // ---- Composition: the Game HAS-A map, player, enemies, bullets, bomb ...
    Map map;
    std::unique_ptr<Player> player;
    std::vector<std::unique_ptr<Enemy> > enemies;   // base-class pointers -> polymorphism
    std::vector<Bullet> bullets;
    std::vector<Effect> effects;
    Bomb bomb;
    Camera2D camera;

    // ---- Game flow
    GameState state;
    int round;
    int targetSite;          // the site the terrorists are going for this round
    float introTimer;        // short "get ready" pause at the start of every round
    std::string endReason;
    std::string message;
    float messageTimer;
    int menuIndex;
    Vector2 lastMouse;
    bool running;

    // ---- Logic
    void update(float dt);
    void startNewGame();
    void startRound();
    void endRound(bool won, const std::string& reason);
    void setState(GameState newState);
    void showMessage(const std::string& text, float seconds);

    void updatePlaying(float dt);
    void updateBullets(float dt);
    void updateBomb(float dt);
    void removeDeadEnemies();
    void separateEnemies();
    void checkRoundEnd();
    void updateCamera();

    Vector2 getMouseWorld() const;
    bool isVisibleToPlayer(Vector2 point) const;
    int updateMenu(int itemCount, int startY);

    // ---- Drawing
    void draw();
    void drawWorld() const;
    void drawHUD() const;
    void drawBombStatus() const;
    void drawMinimap() const;
    void drawCrosshair() const;
    void drawMainMenu() const;
    void drawPauseMenu() const;
    void drawRoundWon() const;
    void drawEndScreen() const;
    void drawMenuItems(const std::vector<std::string>& items, int startY) const;
    void drawCentered(const std::string& text, int y, int size, Color color) const;
};

#endif
