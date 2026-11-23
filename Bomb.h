// Part 2: C4 Bomb state machine
#ifndef BOMB_H
#define BOMB_H

#include "raylib.h"

enum class BombState
{
    NotPlanted,
    Planting,
    Planted,
    Defused,
    Exploded
};

class Bomb
{
private:
    BombState state = BombState::NotPlanted;
    // TODO: Add position, siteIndex, plantTime, plantProgress, explodeTime, timeLeft, defuseTime, defuseProgress

public:
    Bomb();

    void reset();
    void plant(float dt, Vector2 where, int site);
    void cancelPlant();
    void defuse(float dt);
    void stopDefuse();
    void update(float dt);
    void draw() const;

    // TODO: Declare state, site, position, progress, and countdown getters
};

#endif