#include "Game.h"
#include "Utils.h"
#include <algorithm>
#include <cmath>
#include <cstdio>

static const float DEFUSE_RANGE = 70.0f;

// ================================================================ setup

Game::Game()
    : map(), player(), enemies(), bullets(), effects(), bomb(),
      state(GameState::MENU), round(1), targetSite(0), introTimer(0.0f),
      endReason(), message(), messageTimer(0.0f), menuIndex(0),
      lastMouse(util::vec(0.0f, 0.0f)), running(true)
{
    player.reset(new Player(map.getPlayerSpawn()));

    camera.offset = util::vec(SCREEN_W / 2.0f, SCREEN_H / 2.0f);
    camera.target = map.getPlayerSpawn();
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
}

Game::~Game() {}

void Game::run()
{
    InitWindow(SCREEN_W, SCREEN_H, "Top-Down Strike - OOP Lab Project");
    SetExitKey(0);              // ESC is used for the pause menu, not for quitting
    SetTargetFPS(60);

    while (running && !WindowShouldClose())
    {
        float dt = GetFrameTime();
        if (dt > 0.05f) dt = 0.05f;   // avoid huge steps after a lag spike
        update(dt);
        draw();
    }

    CloseWindow();
}

void Game::setState(GameState newState)
{
    state = newState;
    menuIndex = 0;
}

void Game::showMessage(const std::string& text, float seconds)
{
    message = text;
    messageTimer = seconds;
}

void Game::startNewGame()
{
    round = 1;
    startRound();
}

void Game::startRound()
{
    bullets.clear();
    effects.clear();
    enemies.clear();
    bomb.reset();

    player->resetForRound(map.getPlayerSpawn());   // health back to 100, fresh ammo
    targetSite = GetRandomValue(0, map.getSiteCount() - 1);

    // Enemy mix: more and tougher terrorists every round.
    // 0 = Normal, 1 = Aggressive, 2 = Sniper
    const int pattern[7] = { 0, 1, 0, 2, 0, 1, 2 };
    int count = 2 + round;                          // round 1 -> 3 ... round 5 -> 7

    for (int i = 0; i < count; i++)
    {
        Vector2 spawn = map.randomOpenPoint(map.getEnemySpawn(i), 40.0f, 18.0f);
        Enemy* e = NULL;
        switch (pattern[i % 7])
        {
            case 0:  e = new NormalTerrorist(spawn); break;
            case 1:  e = new AggressiveTerrorist(spawn); break;
            default: e = new SniperTerrorist(spawn); break;
        }
        e->applyDifficulty(round);
        e->setPatrolArea(map.getSiteCenter(targetSite), 220.0f);
        enemies.push_back(std::unique_ptr<Enemy>(e));
    }

    introTimer = 2.5f;
    char buf[96];
    snprintf(buf, sizeof(buf), "Intel: the terrorists are heading to SITE %c!", map.getSiteLabel(targetSite));
    showMessage(buf, 4.5f);
    setState(GameState::PLAYING);
    updateCamera();
}
