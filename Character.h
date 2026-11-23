// Part 1: Abstract base class for Player and Enemy
#ifndef CHARACTER_H
#define CHARACTER_H

#include "raylib.h"
#include "Bullet.h"
#include <vector>

class Map;

class Character
{
protected:
    Vector2 position{};
    float radius = 16.0f;
    float speed = 0.0f;
    float health = 100.0f;
    float maxHealth = 100.0f;
    float angle = 0.0f;
    float flashTimer = 0.0f;

    void drawHealthBar(float width) const;

public:
    Character() = default;
    Character(Vector2 pos, float radius, float speed, float maxHealth);
    virtual ~Character();

    virtual void update(float dt, const Map& map, Vector2 focus,
                        std::vector<Bullet>& bullets) = 0;
    virtual void draw() const = 0;

    virtual void takeDamage(float amount);
    bool moveBy(Vector2 delta, const Map& map);
    Vector2 getMuzzlePosition() const;

    Vector2 getPosition() const;
    void setPosition(Vector2 p);
    float getRadius() const;
    float getHealth() const;
    float getMaxHealth() const;
    float getAngle() const;
    bool isAlive() const;
};

#endif