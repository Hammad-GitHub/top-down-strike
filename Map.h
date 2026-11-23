// Map.h - the tactical map: walls, crates (obstacles), two bomb sites and spawn points.
#ifndef MAP_H
#define MAP_H

#include "raylib.h"
#include <vector>

class Map
{
public:
    enum { TILE_SIZE = 40, COLS = 40, ROWS = 30, SITE_COUNT = 2 };

    Map();

    void draw() const;
    void drawMinimap(Rectangle area) const;

    // Collision queries used by characters and bullets
    bool collidesCircle(Vector2 center, float radius) const;
    bool collidesPoint(Vector2 p) const;
    bool lineBlocked(Vector2 a, Vector2 b) const;   // line of sight test

    // A random free position near "center" (used for spawning / patrolling)
    Vector2 randomOpenPoint(Vector2 center, float radius, float clearance) const;

    float getWidth() const;
    float getHeight() const;

    int getSiteCount() const;
    Vector2 getSiteCenter(int index) const;
    Rectangle getSiteRect(int index) const;
    char getSiteLabel(int index) const;

    Vector2 getPlayerSpawn() const;
    int getEnemySpawnCount() const;
    Vector2 getEnemySpawn(int index) const;

private:
    std::vector<Rectangle> walls;
    std::vector<Rectangle> crates;
    Rectangle sites[SITE_COUNT];
    std::vector<Vector2> enemySpawns;
    Vector2 playerSpawn;

    void addWall(int col, int row, int w, int h);
    void addCrate(int col, int row, int w, int h);
};

#endif
