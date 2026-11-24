// Player.h - the Counter-Terrorist controlled with keyboard and mouse.
#ifndef PLAYER_H
#define PLAYER_H

#include "Character.h"
#include "Weapon.h"
#include <memory>
#include <vector>

class Player : public Character
{
private:
    // Composition: the Player OWNS its weapons. unique_ptr<Weapon> lets one
    // vector hold Pistol, AssaultRifle, Shotgun and SniperRifle objects
    // (runtime polymorphism) and deletes them automatically.
    std::vector<std::unique_ptr<Weapon> > weapons;
    int currentWeapon;
    bool busy;            // true while defusing the bomb (cannot move or shoot)
    float damageFlash;    // > 0 right after being hit

    void switchWeapon(int index);

public:
    explicit Player(Vector2 spawn);
    ~Player();

    void update(float dt, const Map& map, Vector2 focus, std::vector<Bullet>& bullets) override;
    void draw() const override;
    void takeDamage(float amount) override;

    void resetForRound(Vector2 spawn);
    void setBusy(bool value);
    bool isBusy() const;
    float getDamageFlash() const;

    const Weapon& getCurrentWeapon() const;
    const Weapon& getWeapon(int index) const;
    int getCurrentWeaponIndex() const;
    int getWeaponCount() const;
};

#endif
