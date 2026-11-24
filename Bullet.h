// Bullet.h - projectiles and small visual impact effects.
#ifndef BULLET_H
#define BULLET_H

#include "raylib.h"

class Map;   // forward declaration (Bullet.cpp includes the full header)

class Bullet
{
private:
    Vector2 position;
    Vector2 velocity;
    float damage;
    float radius;
    float lifetime;      // seconds left before the bullet disappears (limits range)
    bool active;
    bool fromPlayer;
    bool wallHit;        // true if it stopped because it hit a wall this frame
    Color color;

public:
    Bullet(Vector2 pos, float angle, float speed, float damage, float radius,
           float lifetime, bool fromPlayer, Color color);

    void update(float dt, const Map& map);
    void draw() const;
    void deactivate();

    Vector2 getPosition() const;
    float getDamage() const;
    float getRadius() const;
    bool isActive() const;
    bool isFromPlayer() const;
    bool hitWall() const;
};

// A short-lived expanding circle used for hit sparks, blood and deaths.
class Effect
{
private:
    Vector2 position;
    float life;
    float maxLife;
    float size;
    Color color;

public:
    Effect(Vector2 pos, float duration, float size, Color color);
    void update(float dt);
    void draw() const;
    bool isFinished() const;
};

#endif
