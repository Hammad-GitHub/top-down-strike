// Weapon.h - abstract Weapon base class and four concrete weapons.
#ifndef WEAPON_H
#define WEAPON_H

#include "raylib.h"
#include "Bullet.h"
#include <string>
#include <vector>

// All the numbers that make one weapon different from another.
struct WeaponStats
{
    std::string name;
    float damage;          // damage of ONE bullet / pellet
    float fireInterval;    // seconds between shots (fire rate)
    int magazineSize;      // magazine capacity
    float reloadTime;      // seconds to reload
    float bulletSpeed;     // pixels per second
    float spread;          // random inaccuracy in degrees
    bool automatic;        // hold the mouse button to keep firing?
    float bulletLife;      // seconds a bullet lives (== range / speed)
    float bulletRadius;
    Color bulletColor;
};

// ABSTRACT class: shoot() is pure virtual, so a plain Weapon cannot be created.
class Weapon
{
private:
    WeaponStats stats;
    int ammo;
    float cooldown;
    float reloadTimer;
    bool reloading;

protected:
    // Helper for derived classes: creates one bullet with random spread.
    void spawnBullet(std::vector<Bullet>& bullets, Vector2 muzzle, float angle,
                     float spreadDegrees) const;
    float getSpread() const;

public:
    explicit Weapon(const WeaponStats& weaponStats);
    virtual ~Weapon();

    virtual void update(float dt);                       // timers (cooldown / reload)
    virtual void shoot(Vector2 muzzle, float angle,
                       std::vector<Bullet>& bullets) = 0; // each weapon fires differently

    bool tryFire(Vector2 muzzle, float angle, std::vector<Bullet>& bullets);
    void startReload();
    void cancelReload();
    void refill();

    const std::string& getName() const;
    int getAmmo() const;
    int getMagazineSize() const;
    bool isReloading() const;
    float getReloadProgress() const;   // 0..1
    bool isAutomatic() const;
    float getDamage() const;
    float getFireInterval() const;
    float getReloadTime() const;
    float getBulletSpeed() const;
};

class Pistol : public Weapon
{
public:
    Pistol();
    void shoot(Vector2 muzzle, float angle, std::vector<Bullet>& bullets) override;
};

class AssaultRifle : public Weapon
{
private:
    float recoil;     // grows while you keep firing, makes bullets less accurate
public:
    AssaultRifle();
    void update(float dt) override;
    void shoot(Vector2 muzzle, float angle, std::vector<Bullet>& bullets) override;
};

class Shotgun : public Weapon
{
private:
    int pellets;
public:
    Shotgun();
    void shoot(Vector2 muzzle, float angle, std::vector<Bullet>& bullets) override;
};

class SniperRifle : public Weapon
{
public:
    SniperRifle();
    void shoot(Vector2 muzzle, float angle, std::vector<Bullet>& bullets) override;
};

#endif
