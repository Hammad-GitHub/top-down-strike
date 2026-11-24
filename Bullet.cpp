#include "Bullet.h"
#include "Map.h"
#include "Utils.h"

Bullet::Bullet(Vector2 pos, float angle, float speed, float dmg, float rad,
               float life, bool player, Color col)
    : position(pos), velocity(util::scale(util::fromAngle(angle), speed)),
      damage(dmg), radius(rad), lifetime(life), active(true),
      fromPlayer(player), wallHit(false), color(col)
{
}

void Bullet::update(float dt, const Map& map)
{
    if (!active) return;

    lifetime -= dt;
    if (lifetime <= 0.0f) { active = false; return; }

    // Move in small sub-steps so fast bullets cannot jump through thin walls.
    Vector2 step = util::scale(velocity, dt);
    int steps = (int)(util::length(step) / 6.0f) + 1;
    Vector2 inc = util::scale(step, 1.0f / (float)steps);

    for (int i = 0; i < steps; i++)
    {
        Vector2 next = util::add(position, inc);
        if (map.collidesPoint(next))
        {
            active = false;
            wallHit = true;
            return;
        }
        position = next;
    }
}

void Bullet::draw() const
{
    if (!active) return;
    Vector2 tail = util::sub(position, util::scale(util::normalize(velocity), 14.0f));
    DrawLineEx(tail, position, radius * 1.1f, Fade(color, 0.55f));
    DrawCircleV(position, radius, color);
}

void Bullet::deactivate() { active = false; wallHit = false; }

Vector2 Bullet::getPosition() const { return position; }
float Bullet::getDamage() const { return damage; }
float Bullet::getRadius() const { return radius; }
bool Bullet::isActive() const { return active; }
bool Bullet::isFromPlayer() const { return fromPlayer; }
bool Bullet::hitWall() const { return wallHit; }

// ---------------------------------------------------------------- Effect

Effect::Effect(Vector2 pos, float duration, float sz, Color col)
    : position(pos), life(duration), maxLife(duration), size(sz), color(col)
{
}

void Effect::update(float dt) { life -= dt; }
bool Effect::isFinished() const { return life <= 0.0f; }

void Effect::draw() const
{
    float t = life / maxLife;                 // 1 -> 0
    float r = size * (1.0f + (1.0f - t));     // grows while fading
    DrawCircleV(position, r, Fade(color, t * 0.8f));
}
