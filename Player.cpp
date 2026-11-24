#include "Player.h"
#include "Map.h"
#include "Utils.h"

Player::Player(Vector2 spawn)
    : Character(spawn, 16.0f, 210.0f, 100.0f),
      currentWeapon(0), busy(false), damageFlash(0.0f)
{
    weapons.push_back(std::unique_ptr<Weapon>(new Pistol()));
    weapons.push_back(std::unique_ptr<Weapon>(new AssaultRifle()));
    weapons.push_back(std::unique_ptr<Weapon>(new Shotgun()));
    weapons.push_back(std::unique_ptr<Weapon>(new SniperRifle()));
    angle = -util::PI_F / 2.0f;
}

Player::~Player() {}

void Player::switchWeapon(int index)
{
    if (index < 0 || index >= (int)weapons.size() || index == currentWeapon) return;
    weapons[currentWeapon]->cancelReload();
    currentWeapon = index;
}

void Player::update(float dt, const Map& map, Vector2 focus, std::vector<Bullet>& bullets)
{
    if (!isAlive()) return;

    if (flashTimer > 0.0f) flashTimer -= dt;
    if (damageFlash > 0.0f) damageFlash -= dt;

    // Mouse aiming
    angle = std::atan2(focus.y - position.y, focus.x - position.x);

    weapons[currentWeapon]->update(dt);

    if (busy) return;   // defusing: aim only

    // WASD movement
    Vector2 dir = util::vec(0.0f, 0.0f);
    if (IsKeyDown(KEY_W)) dir.y -= 1.0f;
    if (IsKeyDown(KEY_S)) dir.y += 1.0f;
    if (IsKeyDown(KEY_A)) dir.x -= 1.0f;
    if (IsKeyDown(KEY_D)) dir.x += 1.0f;
    dir = util::normalize(dir);                    // same speed on diagonals
    moveBy(util::scale(dir, speed * dt), map);

    // Weapon switching: keys 1-4 or mouse wheel
    if (IsKeyPressed(KEY_ONE)) switchWeapon(0);
    if (IsKeyPressed(KEY_TWO)) switchWeapon(1);
    if (IsKeyPressed(KEY_THREE)) switchWeapon(2);
    if (IsKeyPressed(KEY_FOUR)) switchWeapon(3);

    float wheel = (float)GetMouseWheelMove();
    int count = (int)weapons.size();
    if (wheel > 0.0f) switchWeapon((currentWeapon + 1) % count);
    else if (wheel < 0.0f) switchWeapon((currentWeapon + count - 1) % count);

    // Reload
    if (IsKeyPressed(KEY_R)) weapons[currentWeapon]->startReload();

    // Shooting (automatic weapons fire while the button is held)
    Weapon* w = weapons[currentWeapon].get();
    bool wantsToFire = w->isAutomatic() ? IsMouseButtonDown(MOUSE_BUTTON_LEFT)
                                        : IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    if (wantsToFire && w->tryFire(getMuzzlePosition(), angle, bullets))
        flashTimer = 0.06f;
}

void Player::takeDamage(float amount)
{
    Character::takeDamage(amount);   // reuse the base behaviour
    damageFlash = 0.25f;             // plus a Player-only red flash
}

void Player::draw() const
{
    if (!isAlive()) return;

    Color body = damageFlash > 0.0f ? util::rgba(255, 130, 130) : util::rgba(70, 130, 230);
    float deg = util::radToDeg(angle);

    // Gun barrel
    DrawRectanglePro(util::rect(position.x, position.y, radius + 16.0f, 6.0f),
                     util::vec(0.0f, 3.0f), deg, util::rgba(30, 30, 35));
    // Body
    DrawCircleV(position, radius, body);
    DrawCircleLines((int)position.x, (int)position.y, radius, util::rgba(15, 40, 100));
    // Helmet marker in the facing direction
    Vector2 helmet = util::add(position, util::scale(util::fromAngle(angle), radius * 0.35f));
    DrawCircleV(helmet, radius * 0.55f, util::rgba(40, 90, 190));

    if (flashTimer > 0.0f)
        DrawCircleV(getMuzzlePosition(), 8.0f, util::rgba(255, 240, 150));
}

void Player::resetForRound(Vector2 spawn)
{
    position = spawn;
    health = maxHealth;
    busy = false;
    damageFlash = 0.0f;
    flashTimer = 0.0f;
    angle = -util::PI_F / 2.0f;
    currentWeapon = 0;
    for (size_t i = 0; i < weapons.size(); i++) weapons[i]->refill();
}

void Player::setBusy(bool value) { busy = value; }
bool Player::isBusy() const { return busy; }
float Player::getDamageFlash() const { return damageFlash; }
const Weapon& Player::getCurrentWeapon() const { return *weapons[currentWeapon]; }
const Weapon& Player::getWeapon(int index) const { return *weapons[index]; }
int Player::getCurrentWeaponIndex() const { return currentWeapon; }
int Player::getWeaponCount() const { return (int)weapons.size(); }
