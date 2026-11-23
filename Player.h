// Part 1: Counter-Terrorist player class
#ifndef PLAYER_H
#define PLAYER_H

#include "Character.h"
#include "Weapon.h"
#include <memory>
#include <vector>

class Player : public Character
{
private:
    // TODO: Add std::vector<std::unique_ptr<Weapon>> weapons, active index, busy/defusing state, and hit flash timer

public:
    Player();
    ~Player() override = default;

    void update(float dt, const Map& map, Vector2 focus,
                std::vector<Bullet>& bullets) override;
    void draw() const override;
    void takeDamage(float amount) override;

    // TODO: Declare weapon switching, reloading, busy state setters, and HUD getters
};

#endif