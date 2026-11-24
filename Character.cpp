#include "Character.h"
#include "Map.h"
#include "Utils.h"

Character::Character(Vector2 pos, float r, float spd, float hp)
    : position(pos), radius(r), speed(spd), health(hp), maxHealth(hp),
      angle(0.0f), flashTimer(0.0f)
{
}

Character::~Character() {}

void Character::takeDamage(float amount)
{
    health -= amount;
    if (health < 0.0f) health = 0.0f;
}

bool Character::moveBy(Vector2 delta, const Map& map)
{
    bool moved = false;

    Vector2 next = position;
    next.x += delta.x;
    if (!map.collidesCircle(next, radius)) { position = next; moved = true; }

    next = position;
    next.y += delta.y;
    if (!map.collidesCircle(next, radius)) { position = next; moved = true; }

    return moved;
}

Vector2 Character::getMuzzlePosition() const
{
    return util::add(position, util::scale(util::fromAngle(angle), radius + 14.0f));
}

void Character::drawHealthBar(float width) const
{
    float ratio = health / maxHealth;
    if (ratio < 0.0f) ratio = 0.0f;
    int x = (int)(position.x - width / 2.0f);
    int y = (int)(position.y - radius - 14.0f);
    DrawRectangle(x, y, (int)width, 5, util::rgba(20, 20, 20, 200));
    DrawRectangle(x, y, (int)(width * ratio), 5, util::rgba((unsigned char)(255 * (1.0f - ratio)),
                                                           (unsigned char)(220 * ratio), 40));
}

Vector2 Character::getPosition() const { return position; }
void Character::setPosition(Vector2 p) { position = p; }
float Character::getRadius() const { return radius; }
float Character::getHealth() const { return health; }
float Character::getMaxHealth() const { return maxHealth; }
float Character::getAngle() const { return angle; }
bool Character::isAlive() const { return health > 0.0f; }
