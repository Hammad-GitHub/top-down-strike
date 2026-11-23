// Bomb.h - the C4: planting, countdown, defusing and explosion.
#ifndef BOMB_H
#define BOMB_H

#include "raylib.h"

enum class BombState { NotPlanted, Planting, Planted, Defused, Exploded };

class Bomb
{
private:
    BombState state;
    Vector2 position;
    int siteIndex;

    float plantTime;        // seconds needed to plant
    float plantProgress;
    float explodeTime;      // seconds from "planted" to explosion
    float timeLeft;
    float defuseTime;       // seconds needed to defuse
    float defuseProgress;

public:
    Bomb();

    void reset();
    void plant(float dt, Vector2 where, int site);   // call every frame while planting
    void cancelPlant();
    void defuse(float dt);                           // call every frame while defusing
    void stopDefuse();
    void update(float dt);
    void draw() const;

    BombState getState() const;
    bool isPlanting() const;
    bool isPlanted() const;
    bool isDefused() const;
    bool isExploded() const;
    Vector2 getPosition() const;
    int getSiteIndex() const;
    float getPlantPercent() const;    // 0..1
    float getDefusePercent() const;   // 0..1
    float getTimeLeft() const;
};

#endif
