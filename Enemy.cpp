#include "Enemy.h"
#include "Map.h"
#include "Utils.h"
#include <cmath>

static const float OBJECTIVE_RADIUS = 25.0f;

// ================================================================ Enemy (base)

Enemy::Enemy(Vector2 pos, float r, float spd, float hp, float dmg, float bSpeed,
             float interval, float detect, float range, float spr, Color col)
    : Character(pos, r, spd, hp),
      damage(dmg), bulletSpeed(bSpeed), fireInterval(interval),
      fireTimer(util::randomFloat(0.5f, 1.5f)),
      detectRange(detect), attackRange(range), spread(spr),
      reactionDelay(0.45f), aimWarning(0.0f), color(col),
      seesPlayer(false), aiming(false), sightTime(0.0f), distToPlayer(9999.0f),
      hasLastKnown(false), lastKnown(util::vec(0.0f, 0.0f)),
      patrolCenter(pos), patrolRadius(200.0f), waypoint(pos), hasWaypoint(false),
      waypointTimer(0.0f), stuckTimer(0.0f), detourTimer(0.0f), detourSign(1.0f),
      planter(false), objective(pos)
{
}

Enemy::~Enemy() {}

void Enemy::applyDifficulty(int round)
{
    float f = (float)(round - 1);
    maxHealth *= 1.0f + 0.12f * f;
    health = maxHealth;
    damage *= 1.0f + 0.06f * f;
    speed *= 1.0f + 0.04f * f;
    fireInterval *= std::max(0.6f, 1.0f - 0.06f * f);
    spread *= std::max(0.4f, 1.0f - 0.10f * f);
}

void Enemy::setPatrolArea(Vector2 center, float r)
{
    patrolCenter = center;
    patrolRadius = r;
    hasWaypoint = false;
}

void Enemy::setPlanter(bool value) { planter = value; }
void Enemy::setObjective(Vector2 point) { objective = point; }
bool Enemy::isPlanter() const { return planter; }

bool Enemy::isAtObjective() const
{
    return planter && util::distance(position, objective) <= OBJECTIVE_RADIUS;
}

// Walks towards a point. If the enemy is blocked by a wall for a moment it
// sidesteps (a "detour") so it does not get stuck in corners.
void Enemy::moveToward(float dt, const Map& map, Vector2 destination, float speedMultiplier)
{
    Vector2 toDest = util::sub(destination, position);
    if (util::length(toDest) < 1.0f) return;

    Vector2 dir = util::normalize(toDest);
    if (detourTimer > 0.0f)
    {
        detourTimer -= dt;
        dir = util::rotate(dir, detourSign * 1.2f);
    }

    float expected = speed * speedMultiplier * dt;
    Vector2 before = position;
    moveBy(util::scale(dir, expected), map);
    float moved = util::distance(before, position);

    if (moved < expected * 0.3f)
    {
        stuckTimer += dt;
        if (stuckTimer > 0.35f)
        {
            stuckTimer = 0.0f;
            detourTimer = 0.7f;
            detourSign = (GetRandomValue(0, 1) == 0) ? 1.0f : -1.0f;
            hasWaypoint = false;
        }
    }
    else
    {
        stuckTimer = 0.0f;
    }

    if (!seesPlayer) angle = std::atan2(dir.y, dir.x);
}

void Enemy::patrol(float dt, const Map& map)
{
    waypointTimer -= dt;
    if (!hasWaypoint || waypointTimer <= 0.0f || util::distance(position, waypoint) < 20.0f)
    {
        waypoint = map.randomOpenPoint(patrolCenter, patrolRadius, radius + 4.0f);
        hasWaypoint = true;
        waypointTimer = util::randomFloat(3.0f, 6.0f);
    }
    moveToward(dt, map, waypoint, 0.6f);
}

void Enemy::fireBullet(std::vector<Bullet>& bullets, float bulletRadius, Color bulletColor, float life)
{
    float a = angle + util::degToRad(util::randomFloat(-spread, spread));
    bullets.push_back(Bullet(getMuzzlePosition(), a, bulletSpeed, damage, bulletRadius,
                             life, false, bulletColor));
}

// The AI "template": every enemy runs these steps, and the parts that differ
// between terrorist types (behave / attack) are virtual calls.
void Enemy::update(float dt, const Map& map, Vector2 focus, std::vector<Bullet>& bullets)
{
    if (!isAlive()) return;

    if (fireTimer > 0.0f) fireTimer -= dt;
    if (flashTimer > 0.0f) flashTimer -= dt;

    // 1. Detection: in range AND no wall in between
    distToPlayer = util::distance(position, focus);
    seesPlayer = (distToPlayer <= detectRange) && !map.lineBlocked(position, focus);

    if (seesPlayer)
    {
        sightTime += dt;
        lastKnown = focus;
        hasLastKnown = true;
    }
    else
    {
        sightTime = 0.0f;
    }

    // 2. Movement
    if (planter)
    {
        if (util::distance(position, objective) > OBJECTIVE_RADIUS)
            moveToward(dt, map, objective, 1.0f);
        // otherwise stand still on the bomb site (Game handles the planting timer)
    }
    else if (seesPlayer)
    {
        behave(dt, map, focus, distToPlayer);                // virtual
    }
    else if (hasLastKnown)
    {
        moveToward(dt, map, lastKnown, 0.9f);                // search where the player was seen
        if (util::distance(position, lastKnown) < 30.0f) hasLastKnown = false;
    }
    else
    {
        patrol(dt, map);
    }

    // 3. Aim and attack
    if (seesPlayer)
        angle = std::atan2(focus.y - position.y, focus.x - position.x);

    aiming = seesPlayer && aimWarning > 0.0f && fireTimer < aimWarning && distToPlayer <= attackRange;

    if (seesPlayer && sightTime >= reactionDelay && distToPlayer <= attackRange && fireTimer <= 0.0f)
    {
        attack(bullets);                                     // virtual
        fireTimer = fireInterval;
        flashTimer = 0.06f;
    }
}

void Enemy::draw() const
{
    if (!isAlive()) return;

    // Laser warning line (snipers)
    if (aiming)
    {
        Vector2 end = util::add(position, util::scale(util::fromAngle(angle), distToPlayer));
        DrawLineEx(getMuzzlePosition(), end, 1.5f, Fade(RED, 0.55f));
    }

    float deg = util::radToDeg(angle);
    DrawRectanglePro(util::rect(position.x, position.y, radius + 14.0f, 6.0f),
                     util::vec(0.0f, 3.0f), deg, util::rgba(25, 25, 25));
    DrawCircleV(position, radius, color);
    DrawCircleLines((int)position.x, (int)position.y, radius, util::rgba(40, 10, 10));
    Vector2 head = util::add(position, util::scale(util::fromAngle(angle), radius * 0.35f));
    DrawCircleV(head, radius * 0.5f, util::rgba(35, 35, 35));

    if (flashTimer > 0.0f)
        DrawCircleV(getMuzzlePosition(), 7.0f, util::rgba(255, 220, 120));

    drawHealthBar(radius * 2.4f);

    const char* label = getTypeName();                       // virtual
    int w = MeasureText(label, 10);
    DrawText(label, (int)position.x - w / 2, (int)(position.y - radius - 28.0f), 10, RAYWHITE);

    if (planter)
        DrawText("C4", (int)position.x - 8, (int)(position.y + radius + 4.0f), 12, util::rgba(255, 220, 60));
}

// ================================================================ NormalTerrorist

NormalTerrorist::NormalTerrorist(Vector2 pos)
    : Enemy(pos, 16.0f, 95.0f, 60.0f, 8.0f, 520.0f, 0.8f, 650.0f, 500.0f, 6.0f,
            util::rgba(200, 60, 60)),
      strafeDir(1.0f), strafeTimer(1.0f)
{
}

void NormalTerrorist::behave(float dt, const Map& map, Vector2 target, float dist)
{
    strafeTimer -= dt;
    if (strafeTimer <= 0.0f)
    {
        strafeDir = -strafeDir;
        strafeTimer = util::randomFloat(0.8f, 2.0f);
    }

    if (dist > 320.0f)
    {
        moveToward(dt, map, target, 1.0f);                   // close the distance
    }
    else
    {
        Vector2 toTarget = util::normalize(util::sub(target, position));
        Vector2 sideways = util::vec(-toTarget.y, toTarget.x);
        moveBy(util::scale(sideways, strafeDir * speed * 0.5f * dt), map);   // strafe
    }
}

void NormalTerrorist::attack(std::vector<Bullet>& bullets)
{
    fireBullet(bullets, 4.0f, util::rgba(255, 200, 80), 1.6f);
}

const char* NormalTerrorist::getTypeName() const { return "TERRORIST"; }

// ================================================================ AggressiveTerrorist

AggressiveTerrorist::AggressiveTerrorist(Vector2 pos)
    : Enemy(pos, 16.0f, 150.0f, 70.0f, 5.0f, 560.0f, 0.28f, 750.0f, 380.0f, 10.0f,
            util::rgba(240, 140, 30))
{
    reactionDelay = 0.3f;
}

void AggressiveTerrorist::behave(float dt, const Map& map, Vector2 target, float dist)
{
    if (dist > 110.0f) moveToward(dt, map, target, 1.0f);    // rush in close
}

void AggressiveTerrorist::attack(std::vector<Bullet>& bullets)
{
    fireBullet(bullets, 3.0f, util::rgba(255, 120, 60), 1.0f);
}

const char* AggressiveTerrorist::getTypeName() const { return "RUSHER"; }

// ================================================================ SniperTerrorist

SniperTerrorist::SniperTerrorist(Vector2 pos)
    : Enemy(pos, 15.0f, 70.0f, 40.0f, 38.0f, 420.0f, 2.4f, 950.0f, 850.0f, 0.8f,
            util::rgba(150, 60, 200))
{
    aimWarning = 0.7f;
    reactionDelay = 0.6f;
}

void SniperTerrorist::behave(float dt, const Map& map, Vector2 target, float dist)
{
    if (dist < 400.0f)
    {
        Vector2 away = util::normalize(util::sub(position, target));   // back away
        moveBy(util::scale(away, speed * dt), map);
    }
    else if (dist > 700.0f)
    {
        moveToward(dt, map, target, 0.8f);                             // get back in range
    }
    // between 400 and 700: hold position and shoot
}

void SniperTerrorist::attack(std::vector<Bullet>& bullets)
{
    fireBullet(bullets, 7.0f, util::rgba(255, 70, 255), 2.4f);
}

const char* SniperTerrorist::getTypeName() const { return "SNIPER"; }
