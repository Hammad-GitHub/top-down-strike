// Character.h - abstract base class for everything that moves and has health.
#ifndef CHARACTER_H
#define CHARACTER_H

#include "raylib.h"
#include "Bullet.h"
#include <vector>

class Map;

class Character
{
protected:
    Vector2 position;
    float radius;
    float speed;
    float health;
    float maxHealth;
    float angle;         // facing direction in radians
    float flashTimer;    // > 0 while the muzzle flash is visible

    void drawHealthBar(float width) const;

public:
    Character(Vector2 pos, float radius, float speed, float maxHealth);
    virtual ~Character();

    // Pure virtual functions -> Character is an abstract class.
    // "focus" is the point the character cares about: the mouse position (in
    // world space) for the Player, and the player's position for enemies.
    virtual void update(float dt, const Map& map, Vector2 focus,
                        std::vector<Bullet>& bullets) = 0;
    virtual void draw() const = 0;

    virtual void takeDamage(float amount);

    // Moves with wall collision; X and Y are tested separately so the
    // character slides along walls instead of getting stuck.
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
