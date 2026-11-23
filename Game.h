// Part 3: Top-level Game orchestrator, composition, and GameState machine
#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "Map.h"
#include "Player.h"
#include "Enemy.h"
#include "Bullet.h"
#include "Bomb.h"
#include <memory>
#include <vector>

enum class GameState
{
    MENU,
    PLAYING,
    PAUSED,
    ROUND_WON,
    GAME_OVER,
    VICTORY
};

class Game
{
private:
    GameState state = GameState::MENU;
    Map map;
    std::unique_ptr<Player> player;
    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<Bullet> bullets;
    std::vector<Effect> effects;
    Bomb bomb;
    Camera2D camera{};

    // TODO: Add round counter, max rounds, timers, and private update/draw/round helpers

public:
    Game();
    ~Game();

    void run();
};

#endif