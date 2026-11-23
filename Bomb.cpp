#include "Bomb.h"
#include "Utils.h"
#include <cmath>
#include <cstdio>

Bomb::Bomb()
    : state(BombState::NotPlanted), position(util::vec(0.0f, 0.0f)), siteIndex(0),
      plantTime(3.0f), plantProgress(0.0f), explodeTime(40.0f), timeLeft(40.0f),
      defuseTime(5.0f), defuseProgress(0.0f)
{
}

void Bomb::reset()
{
    state = BombState::NotPlanted;
    plantProgress = 0.0f;
    timeLeft = explodeTime;
    defuseProgress = 0.0f;
}

void Bomb::plant(float dt, Vector2 where, int site)
{
    if (state == BombState::NotPlanted)
    {
        state = BombState::Planting;
        position = where;
        siteIndex = site;
        plantProgress = 0.0f;
    }
    if (state != BombState::Planting) return;

    plantProgress += dt;
    if (plantProgress >= plantTime)
    {
        state = BombState::Planted;
        timeLeft = explodeTime;
        defuseProgress = 0.0f;
    }
}

void Bomb::cancelPlant()
{
    if (state == BombState::Planting)
    {
        state = BombState::NotPlanted;
        plantProgress = 0.0f;
    }
}

void Bomb::defuse(float dt)
{
    if (state != BombState::Planted) return;
    defuseProgress += dt;
    if (defuseProgress >= defuseTime) state = BombState::Defused;
}

void Bomb::stopDefuse()
{
    if (state == BombState::Planted) defuseProgress = 0.0f;
}

void Bomb::update(float dt)
{
    if (state == BombState::Planted)
    {
        timeLeft -= dt;
        if (timeLeft <= 0.0f)
        {
            timeLeft = 0.0f;
            state = BombState::Exploded;
        }
    }
}

void Bomb::draw() const
{
    if (state == BombState::NotPlanted) return;

    int x = (int)position.x, y = (int)position.y;

    if (state == BombState::Exploded)
    {
        DrawCircleV(position, 150.0f, Fade(util::rgba(255, 120, 20), 0.45f));
        DrawCircleV(position, 90.0f, Fade(util::rgba(255, 220, 80), 0.6f));
        return;
    }

    // C4 block
    DrawRectangle(x - 15, y - 10, 30, 20, util::rgba(45, 70, 45));
    DrawRectangleLines(x - 15, y - 10, 30, 20, util::rgba(20, 30, 20));
    DrawRectangle(x - 10, y - 4, 20, 8, util::rgba(20, 20, 20));

    if (state == BombState::Planting)
    {
        DrawRectangle(x - 30, y - 34, 60, 8, util::rgba(20, 20, 20, 220));
        DrawRectangle(x - 30, y - 34, (int)(60.0f * getPlantPercent()), 8, util::rgba(255, 170, 40));
        DrawText("PLANTING", x - 28, y - 48, 12, util::rgba(255, 200, 80));
    }
    else if (state == BombState::Planted)
    {
        float period = 0.15f + 0.85f * (timeLeft / explodeTime);   // blinks faster and faster
        bool on = std::fmod(timeLeft, period) < period * 0.5f;
        DrawCircleV(util::vec((float)x, (float)y), 4.0f, on ? util::rgba(255, 40, 40) : util::rgba(90, 20, 20));
        DrawCircleLines(x, y, 70.0f, Fade(RED, 0.35f));                // defuse range

        char buf[32];
        snprintf(buf, sizeof(buf), "%.1f", timeLeft);
        int w = MeasureText(buf, 18);
        DrawText(buf, x - w / 2, y - 36, 18, util::rgba(255, 80, 80));

        if (defuseProgress > 0.0f)
        {
            DrawRectangle(x - 30, y + 18, 60, 8, util::rgba(20, 20, 20, 220));
            DrawRectangle(x - 30, y + 18, (int)(60.0f * getDefusePercent()), 8, util::rgba(80, 180, 255));
            DrawText("DEFUSING", x - 30, y + 30, 12, util::rgba(120, 200, 255));
        }
    }
    else if (state == BombState::Defused)
    {
        DrawCircleV(util::vec((float)x, (float)y), 4.0f, util::rgba(60, 255, 100));
        DrawText("DEFUSED", x - 28, y - 30, 14, util::rgba(100, 255, 140));
    }
}

BombState Bomb::getState() const { return state; }
bool Bomb::isPlanting() const { return state == BombState::Planting; }
bool Bomb::isPlanted() const { return state == BombState::Planted; }
bool Bomb::isDefused() const { return state == BombState::Defused; }
bool Bomb::isExploded() const { return state == BombState::Exploded; }
Vector2 Bomb::getPosition() const { return position; }
int Bomb::getSiteIndex() const { return siteIndex; }
float Bomb::getPlantPercent() const { return plantProgress / plantTime; }
float Bomb::getDefusePercent() const { return defuseProgress / defuseTime; }
float Bomb::getTimeLeft() const { return timeLeft; }

