// Part 2: Abstract Enemy AI base and Normal, Aggressive, and Sniper terrorists
#ifndef ENEMY_H
#define ENEMY_H

#include "Character.h"

class Enemy : public Character
{
protected:
    // TODO: Add combat stats (damage, bulletSpeed, fireInterval, detectRange, attackRange, spread, reactionDelay, aimWarning, color)
    // TODO: Add AI state (seesPlayer, lastKnown, patrol waypoint, stuck/detour timers, planter flag, objective)

    virtual void behave(float dt, const Map& map, Vector2 target, float dist) = 0;
    virtual void attack(std::vector<Bullet>& bullets) = 0;

public:
    Enemy() = default;
    ~Enemy() override;

    void update(float dt, const Map& map, Vector2 focus,
                std::vector<Bullet>& bullets) override;
    void draw() const override;

    virtual const char* getTypeName() const = 0;

    // TODO: Declare applyDifficulty(int round), setPatrolArea, setPlanter, setObjective, isPlanter, isAtObjective
};

class NormalTerrorist : public Enemy
{
protected:
    void behave(float dt, const Map& map, Vector2 target, float dist) override;
    void attack(std::vector<Bullet>& bullets) override;
public:
    explicit NormalTerrorist(Vector2 pos);
    const char* getTypeName() const override;
};

class AggressiveTerrorist : public Enemy
{
protected:
    void behave(float dt, const Map& map, Vector2 target, float dist) override;
    void attack(std::vector<Bullet>& bullets) override;
public:
    explicit AggressiveTerrorist(Vector2 pos);
    const char* getTypeName() const override;
};

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