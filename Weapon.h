// Part 1: Abstract Weapon class (Template Method) and derived firearms
#ifndef WEAPON_H
#define WEAPON_H

#include "raylib.h"
#include "Bullet.h"
#include <vector>

struct WeaponStats
{
    // TODO: Add damage, fireRate, magazineSize, reloadTime, bulletSpeed, spread, range
};

class Weapon
{
protected:
    // TODO: Add WeaponStats, ammo count, cooldown timer, and reload state

    virtual void shoot(Vector2 origin, float angle, std::vector<Bullet>& bullets) = 0;

public:
    Weapon() = default;
    virtual ~Weapon() = default;

    // TODO: Declare update(float dt), tryFire(...), startReload(), and getters
};

class Pistol : public Weapon
{
protected:
    void shoot(Vector2 origin, float angle, std::vector<Bullet>& bullets) override;
public:
    Pistol();
};

class Shotgun : public Weapon
{
protected:
    void shoot(Vector2 origin, float angle, std::vector<Bullet>& bullets) override;
public:
    Shotgun();
};

class AssaultRifle : public Weapon
{
protected:
    void shoot(Vector2 origin, float angle, std::vector<Bullet>& bullets) override;
public:
    AssaultRifle();
    // TODO: Override update(float dt) for recoil recovery
};

class Sniper : public Weapon
{
protected:
    void shoot(Vector2 origin, float angle, std::vector<Bullet>& bullets) override;
public:
    Sniper();
};

#endif