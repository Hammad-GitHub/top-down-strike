#include "Weapon.h"
#include "Utils.h"

// ---------------------------------------------------------------- Weapon (base)

Weapon::Weapon(const WeaponStats& weaponStats)
    : stats(weaponStats), ammo(weaponStats.magazineSize), cooldown(0.0f),
      reloadTimer(0.0f), reloading(false)
{
}

Weapon::~Weapon() {}

void Weapon::update(float dt)
{
    if (cooldown > 0.0f) cooldown -= dt;

    if (reloading)
    {
        reloadTimer -= dt;
        if (reloadTimer <= 0.0f)
        {
            reloading = false;
            ammo = stats.magazineSize;
        }
    }
}

// Common rules (ammo, fire rate, reload) live here; the way bullets are created
// is decided by the derived class through the virtual shoot() function.
bool Weapon::tryFire(Vector2 muzzle, float angle, std::vector<Bullet>& bullets)
{
    if (reloading || cooldown > 0.0f) return false;

    if (ammo <= 0)
    {
        startReload();
        return false;
    }

    shoot(muzzle, angle, bullets);   // runtime polymorphism happens here
    ammo--;
    cooldown = stats.fireInterval;

    if (ammo == 0) startReload();    // auto reload on empty magazine
    return true;
}

void Weapon::startReload()
{
    if (!reloading && ammo < stats.magazineSize)
    {
        reloading = true;
        reloadTimer = stats.reloadTime;
    }
}

void Weapon::cancelReload() { reloading = false; reloadTimer = 0.0f; }

void Weapon::refill()
{
    ammo = stats.magazineSize;
    cooldown = 0.0f;
    reloading = false;
    reloadTimer = 0.0f;
}

void Weapon::spawnBullet(std::vector<Bullet>& bullets, Vector2 muzzle, float angle,
                         float spreadDegrees) const
{
    float a = angle + util::degToRad(util::randomFloat(-spreadDegrees, spreadDegrees));
    bullets.push_back(Bullet(muzzle, a, stats.bulletSpeed, stats.damage, stats.bulletRadius,
                             stats.bulletLife, true, stats.bulletColor));
}

float Weapon::getSpread() const { return stats.spread; }
const std::string& Weapon::getName() const { return stats.name; }
int Weapon::getAmmo() const { return ammo; }
int Weapon::getMagazineSize() const { return stats.magazineSize; }
bool Weapon::isReloading() const { return reloading; }
bool Weapon::isAutomatic() const { return stats.automatic; }
float Weapon::getDamage() const { return stats.damage; }
float Weapon::getFireInterval() const { return stats.fireInterval; }
float Weapon::getReloadTime() const { return stats.reloadTime; }
float Weapon::getBulletSpeed() const { return stats.bulletSpeed; }

float Weapon::getReloadProgress() const
{
    if (!reloading) return 0.0f;
    return 1.0f - reloadTimer / stats.reloadTime;
}

// ---------------------------------------------------------------- Pistol

static WeaponStats pistolStats()
{
    WeaponStats s;
    s.name = "Pistol";
    s.damage = 22.0f;
    s.fireInterval = 0.30f;
    s.magazineSize = 12;
    s.reloadTime = 1.2f;
    s.bulletSpeed = 900.0f;
    s.spread = 1.5f;
    s.automatic = false;
    s.bulletLife = 0.9f;
    s.bulletRadius = 3.0f;
    s.bulletColor = util::rgba(255, 230, 120);
    return s;
}

Pistol::Pistol() : Weapon(pistolStats()) {}

void Pistol::shoot(Vector2 muzzle, float angle, std::vector<Bullet>& bullets)
{
    spawnBullet(bullets, muzzle, angle, getSpread());
}

// ---------------------------------------------------------------- Assault Rifle

static WeaponStats rifleStats()
{
    WeaponStats s;
    s.name = "Assault Rifle";
    s.damage = 14.0f;
    s.fireInterval = 0.10f;
    s.magazineSize = 30;
    s.reloadTime = 2.0f;
    s.bulletSpeed = 1100.0f;
    s.spread = 2.0f;
    s.automatic = true;
    s.bulletLife = 0.9f;
    s.bulletRadius = 3.0f;
    s.bulletColor = util::rgba(255, 190, 80);
    return s;
}

AssaultRifle::AssaultRifle() : Weapon(rifleStats()), recoil(0.0f) {}

void AssaultRifle::update(float dt)
{
    Weapon::update(dt);                       // reuse the base class timers
    recoil -= 6.0f * dt;                      // recoil slowly settles down
    if (recoil < 0.0f) recoil = 0.0f;
}

void AssaultRifle::shoot(Vector2 muzzle, float angle, std::vector<Bullet>& bullets)
{
    spawnBullet(bullets, muzzle, angle, getSpread() + recoil);
    recoil += 0.8f;                           // every bullet makes the next less accurate
    if (recoil > 5.0f) recoil = 5.0f;
}

// ---------------------------------------------------------------- Shotgun

static WeaponStats shotgunStats()
{
    WeaponStats s;
    s.name = "Shotgun";
    s.damage = 13.0f;           // per pellet
    s.fireInterval = 0.85f;
    s.magazineSize = 6;
    s.reloadTime = 2.2f;
    s.bulletSpeed = 800.0f;
    s.spread = 11.0f;
    s.automatic = false;
    s.bulletLife = 0.38f;       // short range
    s.bulletRadius = 2.5f;
    s.bulletColor = util::rgba(255, 150, 70);
    return s;
}

Shotgun::Shotgun() : Weapon(shotgunStats()), pellets(8) {}

void Shotgun::shoot(Vector2 muzzle, float angle, std::vector<Bullet>& bullets)
{
    for (int i = 0; i < pellets; i++)          // one trigger pull = many pellets
        spawnBullet(bullets, muzzle, angle, getSpread());
}

// ---------------------------------------------------------------- Sniper Rifle

static WeaponStats sniperStats()
{
    WeaponStats s;
    s.name = "Sniper Rifle";
    s.damage = 100.0f;
    s.fireInterval = 1.4f;
    s.magazineSize = 5;
    s.reloadTime = 2.8f;
    s.bulletSpeed = 2200.0f;
    s.spread = 0.2f;
    s.automatic = false;
    s.bulletLife = 1.2f;
    s.bulletRadius = 4.0f;
    s.bulletColor = util::rgba(150, 230, 255);
    return s;
}

SniperRifle::SniperRifle() : Weapon(sniperStats()) {}

void SniperRifle::shoot(Vector2 muzzle, float angle, std::vector<Bullet>& bullets)
{
    spawnBullet(bullets, muzzle, angle, getSpread());
}
