// Enemy.h - abstract Enemy class and the three Terrorist types.
#ifndef ENEMY_H
#define ENEMY_H

#include "Character.h"

// ABSTRACT class: behave(), attack() and getTypeName() are pure virtual, so each
// terrorist type MUST define its own behaviour. The shared AI skeleton (detect the
// player, patrol, remember where the player was, plant the bomb) lives in update().
class Enemy : public Character
{
protected:
    float damage;
    float bulletSpeed;
    float fireInterval;
    float fireTimer;
    float detectRange;      // how far it can see the player
    float attackRange;      // how far it will shoot
    float spread;           // aim inaccuracy in degrees
    float reactionDelay;    // seconds between spotting the player and firing
    float aimWarning;       // > 0: shows a laser line this many seconds before a shot
    Color color;

    // AI state
    bool seesPlayer;
    bool aiming;
    float sightTime;
    float distToPlayer;
    bool hasLastKnown;
    Vector2 lastKnown;
    Vector2 patrolCenter;
    float patrolRadius;
    Vector2 waypoint;
    bool hasWaypoint;
    float waypointTimer;
    float stuckTimer;
    float detourTimer;
    float detourSign;

    // Bomb duty
    bool planter;
    Vector2 objective;

    void moveToward(float dt, const Map& map, Vector2 destination, float speedMultiplier);
    void patrol(float dt, const Map& map);
    void fireBullet(std::vector<Bullet>& bullets, float bulletRadius, Color bulletColor, float life);

    // How this particular terrorist moves while it can see the player.
    virtual void behave(float dt, const Map& map, Vector2 target, float dist) = 0;
    // How this particular terrorist shoots.
    virtual void attack(std::vector<Bullet>& bullets) = 0;

public:
    Enemy(Vector2 pos, float radius, float speed, float health, float damage,
          float bulletSpeed, float fireInterval, float detectRange, float attackRange,
          float spread, Color color);
    virtual ~Enemy();

    void update(float dt, const Map& map, Vector2 focus, std::vector<Bullet>& bullets) override;
    void draw() const override;

    virtual const char* getTypeName() const = 0;

    void applyDifficulty(int round);
    void setPatrolArea(Vector2 center, float radius);
    void setPlanter(bool value);
    void setObjective(Vector2 point);
    bool isPlanter() const;
    bool isAtObjective() const;
};

// Walks towards the player, then strafes sideways while shooting.
class NormalTerrorist : public Enemy
{
private:
    float strafeDir;
    float strafeTimer;
protected:
    void behave(float dt, const Map& map, Vector2 target, float dist) override;
    void attack(std::vector<Bullet>& bullets) override;
public:
    explicit NormalTerrorist(Vector2 pos);
    const char* getTypeName() const override;
};

// Fast and tough: rushes at the player and fires very often.
class AggressiveTerrorist : public Enemy
{
protected:
    void behave(float dt, const Map& map, Vector2 target, float dist) override;
    void attack(std::vector<Bullet>& bullets) override;
public:
    explicit AggressiveTerrorist(Vector2 pos);
    const char* getTypeName() const override;
};

// Fragile but deadly: keeps its distance and fires slow, powerful bullets.
class SniperTerrorist : public Enemy
{
protected:
    void behave(float dt, const Map& map, Vector2 target, float dist) override;
    void attack(std::vector<Bullet>& bullets) override;
public:
    explicit SniperTerrorist(Vector2 pos);
    const char* getTypeName() const override;
};

#endif
