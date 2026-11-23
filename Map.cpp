#include "Map.h"
#include "Utils.h"

Map::Map()
{
    const float T = (float)TILE_SIZE;

    // Outer walls
    addWall(0, 0, COLS, 1);
    addWall(0, ROWS - 1, COLS, 1);
    addWall(0, 0, 1, ROWS);
    addWall(COLS - 1, 0, 1, ROWS);

    // Inner walls (tile coordinates: column, row, width, height)
    addWall(10, 5, 8, 1);
    addWall(22, 5, 1, 7);
    addWall(28, 9, 8, 1);
    addWall(5, 12, 1, 6);
    addWall(14, 14, 8, 1);
    addWall(26, 16, 1, 8);
    addWall(10, 21, 8, 1);
    addWall(32, 20, 5, 1);

    // Crates (cover / obstacles)
    addCrate(3, 5, 2, 2);
    addCrate(18, 2, 2, 2);
    addCrate(30, 4, 2, 2);
    addCrate(16, 9, 2, 2);
    addCrate(34, 13, 2, 2);
    addCrate(20, 18, 2, 2);
    addCrate(2, 22, 2, 2);
    addCrate(12, 25, 2, 2);
    addCrate(24, 26, 2, 2);
    addCrate(35, 24, 2, 2);

    // Bomb sites
    sites[0] = util::rect(2 * T, 2 * T, 6 * T, 5 * T);     // Site A (top-left)
    sites[1] = util::rect(30 * T, 22 * T, 7 * T, 5 * T);   // Site B (bottom-right)

    playerSpawn = util::vec(780.0f, 1100.0f);

    enemySpawns.push_back(util::vec(1400.0f, 120.0f));
    enemySpawns.push_back(util::vec(1200.0f, 300.0f));
    enemySpawns.push_back(util::vec(1000.0f, 150.0f));
    enemySpawns.push_back(util::vec(640.0f, 160.0f));
    enemySpawns.push_back(util::vec(1500.0f, 700.0f));
    enemySpawns.push_back(util::vec(300.0f, 450.0f));
}

void Map::addWall(int col, int row, int w, int h)
{
    walls.push_back(util::rect((float)(col * TILE_SIZE), (float)(row * TILE_SIZE),
                               (float)(w * TILE_SIZE), (float)(h * TILE_SIZE)));
}

void Map::addCrate(int col, int row, int w, int h)
{
    crates.push_back(util::rect((float)(col * TILE_SIZE), (float)(row * TILE_SIZE),
                                (float)(w * TILE_SIZE), (float)(h * TILE_SIZE)));
}

float Map::getWidth() const { return (float)(COLS * TILE_SIZE); }
float Map::getHeight() const { return (float)(ROWS * TILE_SIZE); }

int Map::getSiteCount() const { return SITE_COUNT; }
Rectangle Map::getSiteRect(int index) const { return sites[index]; }
char Map::getSiteLabel(int index) const { return index == 0 ? 'A' : 'B'; }

Vector2 Map::getSiteCenter(int index) const
{
    return util::vec(sites[index].x + sites[index].width / 2.0f,
                     sites[index].y + sites[index].height / 2.0f);
}

Vector2 Map::getPlayerSpawn() const { return playerSpawn; }
int Map::getEnemySpawnCount() const { return (int)enemySpawns.size(); }
Vector2 Map::getEnemySpawn(int index) const { return enemySpawns[index % enemySpawns.size()]; }

bool Map::collidesCircle(Vector2 center, float radius) const
{
    for (size_t i = 0; i < walls.size(); i++)
        if (CheckCollisionCircleRec(center, radius, walls[i])) return true;
    for (size_t i = 0; i < crates.size(); i++)
        if (CheckCollisionCircleRec(center, radius, crates[i])) return true;
    return false;
}

bool Map::collidesPoint(Vector2 p) const
{
    for (size_t i = 0; i < walls.size(); i++)
        if (CheckCollisionPointRec(p, walls[i])) return true;
    for (size_t i = 0; i < crates.size(); i++)
        if (CheckCollisionPointRec(p, crates[i])) return true;
    return false;
}

bool Map::lineBlocked(Vector2 a, Vector2 b) const
{
    for (size_t i = 0; i < walls.size(); i++)
        if (util::segmentIntersectsRect(a, b, walls[i])) return true;
    for (size_t i = 0; i < crates.size(); i++)
        if (util::segmentIntersectsRect(a, b, crates[i])) return true;
    return false;
}

Vector2 Map::randomOpenPoint(Vector2 center, float radius, float clearance) const
{
    for (int attempt = 0; attempt < 25; attempt++)
    {
        float angle = util::randomFloat(0.0f, 2.0f * util::PI_F);
        float dist = util::randomFloat(0.0f, radius);
        Vector2 p = util::add(center, util::scale(util::fromAngle(angle), dist));
        if (p.x < TILE_SIZE || p.y < TILE_SIZE || p.x > getWidth() - TILE_SIZE || p.y > getHeight() - TILE_SIZE)
            continue;
        if (!collidesCircle(p, clearance)) return p;
    }
    return center;
}

void Map::draw() const
{
    // Checkerboard floor
    DrawRectangle(0, 0, (int)getWidth(), (int)getHeight(), util::rgba(44, 48, 56));
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            if ((r + c) % 2 == 0)
                DrawRectangle(c * TILE_SIZE, r * TILE_SIZE, TILE_SIZE, TILE_SIZE, util::rgba(50, 55, 64));

    // Bomb sites
    Color siteColors[SITE_COUNT] = { util::rgba(255, 200, 60), util::rgba(80, 200, 255) };
    for (int i = 0; i < SITE_COUNT; i++)
    {
        DrawRectangleRec(sites[i], Fade(siteColors[i], 0.14f));
        DrawRectangleLines((int)sites[i].x, (int)sites[i].y, (int)sites[i].width, (int)sites[i].height,
                           Fade(siteColors[i], 0.8f));
        char label[2] = { getSiteLabel(i), '\0' };
        int w = MeasureText(label, 110);
        Vector2 c = getSiteCenter(i);
        DrawText(label, (int)c.x - w / 2, (int)c.y - 55, 110, Fade(siteColors[i], 0.30f));
    }

    // Walls
    for (size_t i = 0; i < walls.size(); i++)
    {
        DrawRectangleRec(walls[i], util::rgba(78, 84, 96));
        DrawRectangleLines((int)walls[i].x, (int)walls[i].y, (int)walls[i].width, (int)walls[i].height,
                           util::rgba(120, 128, 142));
    }

    // Crates
    for (size_t i = 0; i < crates.size(); i++)
    {
        const Rectangle& c = crates[i];
        DrawRectangleRec(c, util::rgba(150, 105, 55));
        DrawRectangleLines((int)c.x, (int)c.y, (int)c.width, (int)c.height, util::rgba(90, 60, 30));
        DrawRectangleLines((int)c.x + 5, (int)c.y + 5, (int)c.width - 10, (int)c.height - 10, util::rgba(110, 75, 38));
        DrawLine((int)c.x + 5, (int)c.y + 5, (int)(c.x + c.width) - 5, (int)(c.y + c.height) - 5, util::rgba(110, 75, 38));
        DrawLine((int)(c.x + c.width) - 5, (int)c.y + 5, (int)c.x + 5, (int)(c.y + c.height) - 5, util::rgba(110, 75, 38));
    }
}

void Map::drawMinimap(Rectangle area) const
{
    float s = area.width / getWidth();
    DrawRectangleRec(area, Fade(BLACK, 0.65f));

    Color siteColors[SITE_COUNT] = { util::rgba(255, 200, 60), util::rgba(80, 200, 255) };
    for (int i = 0; i < SITE_COUNT; i++)
    {
        DrawRectangle((int)(area.x + sites[i].x * s), (int)(area.y + sites[i].y * s),
                      (int)(sites[i].width * s), (int)(sites[i].height * s), Fade(siteColors[i], 0.35f));
        char label[2] = { getSiteLabel(i), '\0' };
        Vector2 c = getSiteCenter(i);
        DrawText(label, (int)(area.x + c.x * s) - 4, (int)(area.y + c.y * s) - 7, 14, siteColors[i]);
    }
    for (size_t i = 0; i < walls.size(); i++)
        DrawRectangle((int)(area.x + walls[i].x * s), (int)(area.y + walls[i].y * s),
                      std::max(1, (int)(walls[i].width * s)), std::max(1, (int)(walls[i].height * s)),
                      util::rgba(150, 156, 170));
    for (size_t i = 0; i < crates.size(); i++)
        DrawRectangle((int)(area.x + crates[i].x * s), (int)(area.y + crates[i].y * s),
                      std::max(1, (int)(crates[i].width * s)), std::max(1, (int)(crates[i].height * s)),
                      util::rgba(150, 105, 55));
    DrawRectangleLines((int)area.x, (int)area.y, (int)area.width, (int)area.height, RAYWHITE);
}
