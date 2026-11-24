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

void Game::endRound(bool won, const std::string& reason)
{
    endReason = reason;
    player->setBusy(false);

    if (won)
        setState(round >= TOTAL_ROUNDS ? GameState::VICTORY : GameState::ROUND_WON);
    else
        setState(GameState::GAME_OVER);
}

// ================================================================ update

int Game::updateMenu(int itemCount, int startY)
{
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) menuIndex = (menuIndex + 1) % itemCount;
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) menuIndex = (menuIndex + itemCount - 1) % itemCount;

    // Mouse support: hovering selects, clicking activates
    Vector2 m = GetMousePosition();
    bool mouseMoved = (m.x != lastMouse.x || m.y != lastMouse.y);
    lastMouse = m;
    for (int i = 0; i < itemCount; i++)
    {
        Rectangle r = util::rect(SCREEN_W / 2.0f - 150.0f, (float)(startY + i * 56), 300.0f, 46.0f);
        if (CheckCollisionPointRec(m, r))
        {
            if (mouseMoved) menuIndex = i;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return i;
        }
    }

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) return menuIndex;
    return -1;
}

void Game::update(float dt)
{
    switch (state)
    {
        case GameState::MENU:
        {
            int choice = updateMenu(2, 380);
            if (choice == 0) startNewGame();
            else if (choice == 1) running = false;
            break;
        }

        case GameState::PLAYING:
            if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P)) setState(GameState::PAUSED);
            else updatePlaying(dt);
            break;

        case GameState::PAUSED:
        {
            if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P)) { state = GameState::PLAYING; break; }
            int choice = updateMenu(4, 300);
            if (choice == 0) state = GameState::PLAYING;
            else if (choice == 1) startNewGame();
            else if (choice == 2) setState(GameState::MENU);
            else if (choice == 3) running = false;
            break;
        }

        case GameState::ROUND_WON:
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
            {
                round++;
                startRound();
            }
            break;

        case GameState::GAME_OVER:
        case GameState::VICTORY:
        {
            int choice = updateMenu(3, 390);
            if (choice == 0) startNewGame();
            else if (choice == 1) setState(GameState::MENU);
            else if (choice == 2) running = false;
            break;
        }
    }
}

Vector2 Game::getMouseWorld() const
{
    Vector2 m = GetMousePosition();
    return util::vec(m.x - camera.offset.x + camera.target.x,
                     m.y - camera.offset.y + camera.target.y);
}

bool Game::isVisibleToPlayer(Vector2 point) const
{
    float d = util::distance(player->getPosition(), point);
    if (d < 140.0f) return true;
    return d < 900.0f && !map.lineBlocked(player->getPosition(), point);
}

void Game::updateCamera()
{
    Vector2 t = player->getPosition();
    t.x = util::clampf(t.x, SCREEN_W / 2.0f, map.getWidth() - SCREEN_W / 2.0f);
    t.y = util::clampf(t.y, SCREEN_H / 2.0f, map.getHeight() - SCREEN_H / 2.0f);
    camera.target = t;
}

void Game::updatePlaying(float dt)
{
    if (messageTimer > 0.0f) messageTimer -= dt;

    // "Get ready" pause at the start of each round
    if (introTimer > 0.0f)
    {
        introTimer -= dt;
        updateCamera();
        return;
    }

    // Polymorphism: each enemy runs its own update() through a base-class pointer
    player->update(dt, map, getMouseWorld(), bullets);
    for (size_t i = 0; i < enemies.size(); i++)
        enemies[i]->update(dt, map, player->getPosition(), bullets);

    separateEnemies();
    updateBullets(dt);
    removeDeadEnemies();
    updateBomb(dt);

    for (size_t i = 0; i < effects.size(); i++) effects[i].update(dt);
    effects.erase(std::remove_if(effects.begin(), effects.end(),
                                 [](const Effect& e) { return e.isFinished(); }),
                  effects.end());

    checkRoundEnd();
    updateCamera();
}

// Keeps terrorists from standing on top of each other.
void Game::separateEnemies()
{
    for (size_t i = 0; i < enemies.size(); i++)
    {
        for (size_t j = i + 1; j < enemies.size(); j++)
        {
            Vector2 d = util::sub(enemies[j]->getPosition(), enemies[i]->getPosition());
            float dist = util::length(d);
            float minDist = enemies[i]->getRadius() + enemies[j]->getRadius();
            if (dist >= minDist) continue;

            Vector2 push = (dist < 0.01f) ? util::vec(0.5f, 0.0f)
                                          : util::scale(util::normalize(d), (minDist - dist) * 0.5f);
            enemies[i]->moveBy(util::scale(push, -1.0f), map);
            enemies[j]->moveBy(push, map);
        }
    }
}

void Game::updateBullets(float dt)
{
    for (size_t i = 0; i < bullets.size(); i++)
    {
        Bullet& b = bullets[i];
        if (!b.isActive()) continue;

        Vector2 previous = b.getPosition();
        b.update(dt, map);
        Vector2 current = b.getPosition();

        // Swept hit test: the whole path travelled this frame is checked.
        if (b.isFromPlayer())
        {
            for (size_t e = 0; e < enemies.size(); e++)
            {
                if (!enemies[e]->isAlive()) continue;
                if (util::segmentHitsCircle(previous, current, enemies[e]->getPosition(),
                                            enemies[e]->getRadius() + b.getRadius()))
                {
                    enemies[e]->takeDamage(b.getDamage());
                    effects.push_back(Effect(current, 0.22f, 7.0f, util::rgba(210, 30, 30)));
                    b.deactivate();
                    break;
                }
            }
        }
        else if (player->isAlive() &&
                 util::segmentHitsCircle(previous, current, player->getPosition(),
                                         player->getRadius() + b.getRadius()))
        {
