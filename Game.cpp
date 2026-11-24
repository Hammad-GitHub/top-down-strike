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
            player->takeDamage(b.getDamage());
            effects.push_back(Effect(current, 0.22f, 7.0f, util::rgba(210, 30, 30)));
            b.deactivate();
        }

        if (b.hitWall())
        {
            effects.push_back(Effect(current, 0.15f, 4.0f, util::rgba(255, 220, 120)));
            b.deactivate();
        }
    }

    bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
                                 [](const Bullet& b) { return !b.isActive(); }),
                  bullets.end());
}

void Game::removeDeadEnemies()
{
    for (size_t i = 0; i < enemies.size();)
    {
        if (!enemies[i]->isAlive())
        {
            effects.push_back(Effect(enemies[i]->getPosition(), 0.6f, 20.0f, util::rgba(160, 20, 20)));
            enemies.erase(enemies.begin() + i);   // unique_ptr deletes the enemy
        }
        else
        {
            i++;
        }
    }
}

void Game::updateBomb(float dt)
{
    // ---- Terrorists plant the bomb
    if (!bomb.isPlanted() && !bomb.isDefused() && !bomb.isExploded())
    {
        Enemy* planter = NULL;
        for (size_t i = 0; i < enemies.size(); i++)
            if (enemies[i]->isPlanter()) planter = enemies[i].get();

        // If nobody carries the bomb (or the carrier died), the closest terrorist picks it up.
        if (planter == NULL && !enemies.empty())
        {
            Vector2 site = map.getSiteCenter(targetSite);
            size_t best = 0;
            for (size_t i = 1; i < enemies.size(); i++)
                if (util::distance(enemies[i]->getPosition(), site) <
                    util::distance(enemies[best]->getPosition(), site))
                    best = i;

            planter = enemies[best].get();
            planter->setPlanter(true);
            planter->setObjective(site);
            if (bomb.isPlanting()) bomb.cancelPlant();
        }

        if (planter != NULL && planter->isAtObjective())
            bomb.plant(dt, planter->getPosition(), targetSite);
        else
            bomb.cancelPlant();

        if (bomb.isPlanted())
        {
            // Bomb is down: the terrorists now guard it.
            for (size_t i = 0; i < enemies.size(); i++)
            {
                enemies[i]->setPlanter(false);
                enemies[i]->setPatrolArea(bomb.getPosition(), 160.0f);
            }
            char buf[96];
            snprintf(buf, sizeof(buf), "The bomb has been planted at SITE %c! Defuse it!",
                     map.getSiteLabel(bomb.getSiteIndex()));
            showMessage(buf, 4.0f);
        }
    }

    // ---- Player defuses (hold E close to the bomb, standing still)
    if (bomb.isPlanted() && player->isAlive() &&
        util::distance(player->getPosition(), bomb.getPosition()) <= DEFUSE_RANGE)
    {
        if (IsKeyDown(KEY_E))
        {
            player->setBusy(true);
            bomb.defuse(dt);
        }
        else
        {
            player->setBusy(false);
            bomb.stopDefuse();
        }
    }
    else
    {
        player->setBusy(false);
        bomb.stopDefuse();
    }

    bomb.update(dt);   // countdown
}

void Game::checkRoundEnd()
{
    if (!player->isAlive())
        endRound(false, "You were eliminated.");
    else if (bomb.isExploded())
        endRound(false, "The bomb exploded. Terrorists win.");
    else if (bomb.isDefused())
        endRound(true, "Bomb defused. Counter-Terrorists win!");
    else if (enemies.empty() && !bomb.isPlanted())
        endRound(true, "All terrorists eliminated. Counter-Terrorists win!");
}

// ================================================================ drawing

void Game::drawCentered(const std::string& text, int y, int size, Color color) const
{
    int w = MeasureText(text.c_str(), size);
    DrawText(text.c_str(), (SCREEN_W - w) / 2, y, size, color);
}

void Game::draw()
{
    BeginDrawing();
    ClearBackground(util::rgba(15, 17, 22));

    if (state == GameState::MENU)
    {
        ShowCursor();
        drawMainMenu();
    }
    else
    {
        if (state == GameState::PLAYING && introTimer <= 0.0f) HideCursor();
        else ShowCursor();

        drawWorld();
        drawHUD();

        if (state == GameState::PLAYING && introTimer <= 0.0f) drawCrosshair();
        else if (state == GameState::PAUSED) drawPauseMenu();
        else if (state == GameState::ROUND_WON) drawRoundWon();
        else if (state == GameState::GAME_OVER || state == GameState::VICTORY) drawEndScreen();
    }

    EndDrawing();
}

void Game::drawWorld() const
{
    BeginMode2D(camera);

    map.draw();
    bomb.draw();

    for (size_t i = 0; i < effects.size(); i++) effects[i].draw();
    for (size_t i = 0; i < bullets.size(); i++) bullets[i].draw();

    // Enemies behind walls are hidden (line of sight), like in classic tactical shooters
    for (size_t i = 0; i < enemies.size(); i++)
        if (isVisibleToPlayer(enemies[i]->getPosition()))
            enemies[i]->draw();                       // virtual call

    player->draw();

    EndMode2D();
}

void Game::drawCrosshair() const
{
    Vector2 m = GetMousePosition();
    int x = (int)m.x, y = (int)m.y;
    Color c = util::rgba(80, 255, 120);
    DrawLine(x - 14, y, x - 5, y, c);
    DrawLine(x + 5, y, x + 14, y, c);
    DrawLine(x, y - 14, x, y - 5, c);
    DrawLine(x, y + 5, x, y + 14, c);
    DrawCircleLines(x, y, 2.0f, c);
}

void Game::drawMinimap() const
{
    Rectangle area = util::rect(SCREEN_W - 230.0f, 10.0f, 220.0f, 165.0f);
    map.drawMinimap(area);
    float s = area.width / map.getWidth();

    // Bomb (while being planted or ticking)
    if (bomb.isPlanting() || bomb.isPlanted())
    {
        Vector2 b = bomb.getPosition();
        DrawCircle((int)(area.x + b.x * s), (int)(area.y + b.y * s), 4.0f, RED);
    }

    // Only terrorists the player can currently see
    for (size_t i = 0; i < enemies.size(); i++)
    {
        Vector2 p = enemies[i]->getPosition();
        if (isVisibleToPlayer(p))
            DrawCircle((int)(area.x + p.x * s), (int)(area.y + p.y * s), 3.0f, util::rgba(255, 90, 90));
    }

    Vector2 p = player->getPosition();
    DrawCircle((int)(area.x + p.x * s), (int)(area.y + p.y * s), 4.0f, util::rgba(80, 255, 120));
}

void Game::drawBombStatus() const
{
    int w = 360, h = 74;
    int x = (SCREEN_W - w) / 2 - 20, y = 10;
    DrawRectangle(x, y, w, h, Fade(BLACK, 0.6f));
    DrawRectangleLines(x, y, w, h, util::rgba(120, 120, 130));

    char buf[96];
    switch (bomb.getState())
    {
        case BombState::NotPlanted:
            DrawText("BOMB: NOT PLANTED", x + 14, y + 10, 20, RAYWHITE);
            snprintf(buf, sizeof(buf), "Terrorists are going for SITE %c", map.getSiteLabel(targetSite));
            DrawText(buf, x + 14, y + 40, 16, util::rgba(255, 200, 80));
            break;

        case BombState::Planting:
            snprintf(buf, sizeof(buf), "BOMB BEING PLANTED  (%d%%)", (int)(bomb.getPlantPercent() * 100.0f));
            DrawText(buf, x + 14, y + 10, 20, util::rgba(255, 170, 40));
            DrawRectangle(x + 14, y + 44, w - 28, 12, util::rgba(30, 30, 30));
            DrawRectangle(x + 14, y + 44, (int)((w - 28) * bomb.getPlantPercent()), 12, util::rgba(255, 170, 40));
            break;

        case BombState::Planted:
        {
            bool blink = ((int)(bomb.getTimeLeft() * 2.0f)) % 2 == 0;
            snprintf(buf, sizeof(buf), "BOMB PLANTED - SITE %c   %.1fs",
                     map.getSiteLabel(bomb.getSiteIndex()), bomb.getTimeLeft());
            DrawText(buf, x + 14, y + 10, 20, blink ? util::rgba(255, 70, 70) : util::rgba(255, 150, 150));
            if (bomb.getDefusePercent() > 0.0f)
            {
                DrawRectangle(x + 14, y + 44, w - 28, 12, util::rgba(30, 30, 30));
                DrawRectangle(x + 14, y + 44, (int)((w - 28) * bomb.getDefusePercent()), 12, util::rgba(80, 180, 255));
                DrawText("DEFUSING...", x + 14, y + 58, 12, util::rgba(120, 200, 255));
            }
            else
            {
                DrawText("Hold [E] next to the bomb to defuse it", x + 14, y + 44, 16, util::rgba(255, 200, 80));
            }
            break;
        }

        case BombState::Defused:
            DrawText("BOMB DEFUSED", x + 14, y + 22, 24, util::rgba(100, 255, 140));
            break;

        case BombState::Exploded:
            DrawText("BOMB EXPLODED", x + 14, y + 22, 24, util::rgba(255, 90, 60));
            break;
    }
}

void Game::drawHUD() const
{
    char buf[96];

    // Damage flash
    if (player->getDamageFlash() > 0.0f)
        DrawRectangle(0, 0, SCREEN_W, SCREEN_H, Fade(RED, std::min(0.35f, player->getDamageFlash())));

    // Round info (top-left)
    DrawRectangle(10, 10, 250, 66, Fade(BLACK, 0.6f));
    snprintf(buf, sizeof(buf), "ROUND %d / %d", round, (int)TOTAL_ROUNDS);
    DrawText(buf, 22, 16, 24, RAYWHITE);
    snprintf(buf, sizeof(buf), "TERRORISTS LEFT: %d", (int)enemies.size());
    DrawText(buf, 22, 46, 18, util::rgba(255, 120, 120));

    drawBombStatus();
    drawMinimap();

    // Health bar (bottom-left)
    float ratio = player->getHealth() / player->getMaxHealth();
    DrawRectangle(20, SCREEN_H - 58, 260, 38, Fade(BLACK, 0.6f));
    DrawRectangle(28, SCREEN_H - 50, 244, 14, util::rgba(40, 40, 40));
    DrawRectangle(28, SCREEN_H - 50, (int)(244 * ratio),14,
                  util::rgba((unsigned char)(255 * (1.0f - ratio)), (unsigned char)(220 * ratio), 50));
    snprintf(buf, sizeof(buf), "HP %d", (int)std::ceil(player->getHealth()));
    DrawText(buf, 28, SCREEN_H - 34, 16, RAYWHITE);

    // Weapon / ammo panel (bottom-right)
    const Weapon& w = player->getCurrentWeapon();
    int px = SCREEN_W - 290, py = SCREEN_H - 128;
    DrawRectangle(px, py, 270, 108, Fade(BLACK, 0.6f));
    DrawText(w.getName().c_str(), px + 12, py + 8, 22, RAYWHITE);
    snprintf(buf, sizeof(buf), "%d / %d", w.getAmmo(), w.getMagazineSize());
    DrawText(buf, px + 12, py + 36, 30, w.getAmmo() == 0 ? util::rgba(255, 90, 90) : util::rgba(255, 220, 120));
    if (w.isReloading())
    {
        DrawText("RELOADING", px + 150, py + 44, 16, util::rgba(120, 200, 255));
        DrawRectangle(px + 12, py + 76, 246, 6, util::rgba(40, 40, 40));
        DrawRectangle(px + 12, py + 76, (int)(246 * w.getReloadProgress()), 6, util::rgba(120, 200, 255));
    }
    for (int i = 0; i < player->getWeaponCount(); i++)
    {
        bool current = (i == player->getCurrentWeaponIndex());
        int sx = px + 12 + i * 62;
        DrawRectangle(sx, py + 86, 56, 18, current ? util::rgba(80, 130, 230) : util::rgba(50, 50, 60));
        snprintf(buf, sizeof(buf), "%d", i + 1);
        DrawText(buf, sx + 22, py + 88, 14, RAYWHITE);
    }

    // Defuse prompt
    if (bomb.isPlanted() && util::distance(player->getPosition(), bomb.getPosition()) <= DEFUSE_RANGE)
        drawCentered("HOLD [E] TO DEFUSE", SCREEN_H - 130, 26, util::rgba(120, 200, 255));

    // Messages
    if (messageTimer > 0.0f && state == GameState::PLAYING)
    {
        int tw = MeasureText(message.c_str(), 22);
        DrawRectangle((SCREEN_W - tw) / 2 - 12, 100, tw + 24, 34, Fade(BLACK, 0.6f));
        drawCentered(message, 106, 22, util::rgba(255, 210, 90));
    }

    // Round intro
    if (state == GameState::PLAYING && introTimer > 0.0f)
    {
        snprintf(buf, sizeof(buf), "ROUND %d", round);
        drawCentered(buf, SCREEN_H / 2 - 70, 64, RAYWHITE);
        snprintf(buf, sizeof(buf), "Get ready...  %.1f", introTimer);
        drawCentered(buf, SCREEN_H / 2 + 5, 28, util::rgba(255, 210, 90));
    }
}

void Game::drawMenuItems(const std::vector<std::string>& items, int startY) const
{
    for (size_t i = 0; i < items.size(); i++)
    {
        bool selected = ((int)i == menuIndex);
        Rectangle r = util::rect(SCREEN_W / 2.0f - 150.0f, (float)(startY + (int)i * 56), 300.0f, 46.0f);
        DrawRectangleRec(r, selected ? util::rgba(70, 120, 220) : util::rgba(40, 44, 54));
        DrawRectangleLines((int)r.x, (int)r.y, (int)r.width, (int)r.height,
                           selected ? RAYWHITE : util::rgba(90, 96, 110));
        int tw = MeasureText(items[i].c_str(), 24);
        DrawText(items[i].c_str(), (int)(r.x + (r.width - tw) / 2), (int)r.y + 11, 24, RAYWHITE);
    }
}

void Game::drawMainMenu() const
{
    DrawRectangleGradientV(0, 0, SCREEN_W, SCREEN_H, util::rgba(20, 24, 34), util::rgba(8, 9, 14));
    drawCentered("TOP-DOWN STRIKE", 90, 64, util::rgba(255, 190, 70));
    drawCentered("Top-down tactical shooter  -  OOP Lab Project", 165, 22, util::rgba(170, 176, 190));
    drawCentered("Defend bomb sites A and B from the Terrorists", 205, 20, util::rgba(120, 200, 255));

    std::vector<std::string> items;
    items.push_back("Start Game");
    items.push_back("Quit");
    drawMenuItems(items, 380);

    int y = 520;
    drawCentered("CONTROLS", y, 20, util::rgba(255, 210, 90));
    drawCentered("WASD move   |   Mouse aim   |   Left click shoot   |   R reload", y + 30, 18, RAYWHITE);
    drawCentered("1-4 / Mouse wheel switch weapon   |   Hold E near the bomb to defuse   |   P or ESC pause", y + 56, 18, RAYWHITE);
    drawCentered("Use UP / DOWN and ENTER, or the mouse, in the menus", y + 90, 16, util::rgba(150, 156, 170));
}

void Game::drawPauseMenu() const
{
    DrawRectangle(0, 0, SCREEN_W, SCREEN_H, Fade(BLACK, 0.65f));
    drawCentered("PAUSED", 200, 56, RAYWHITE);

    std::vector<std::string> items;
    items.push_back("Resume");
    items.push_back("Restart Game");
    items.push_back("Main Menu");
    items.push_back("Quit");
    drawMenuItems(items, 300);
}

void Game::drawRoundWon() const
{
    DrawRectangle(0, 0, SCREEN_W, SCREEN_H, Fade(BLACK, 0.6f));
    char buf[64];
    snprintf(buf, sizeof(buf), "ROUND %d WON", round);
    drawCentered(buf, 220, 60, util::rgba(100, 255, 140));
    drawCentered(endReason, 300, 24, RAYWHITE);
    drawCentered("Next round: more and tougher terrorists.", 345, 20, util::rgba(255, 200, 80));
    drawCentered("Press ENTER to continue", 410, 28, RAYWHITE);
}

void Game::drawEndScreen() const
{
    DrawRectangle(0, 0, SCREEN_W, SCREEN_H, Fade(BLACK, 0.7f));
    if (state == GameState::VICTORY)
    {
        drawCentered("VICTORY!", 180, 72, util::rgba(100, 255, 140));
        drawCentered("You defended the bomb sites in all rounds.", 270, 24, RAYWHITE);
    }
    else
    {
        char buf[64];
        drawCentered("GAME OVER", 180, 72, util::rgba(255, 90, 80));
        snprintf(buf, sizeof(buf), "You reached round %d of %d", round, (int)TOTAL_ROUNDS);
        drawCentered(buf, 270, 24, RAYWHITE);
    }
    drawCentered(endReason, 315, 22, util::rgba(255, 210, 90));

    std::vector<std::string> items;
    items.push_back("Play Again");
    items.push_back("Main Menu");
    items.push_back("Quit");
    drawMenuItems(items, 390);
}
