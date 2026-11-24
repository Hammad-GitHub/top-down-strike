// Utils.h - small math / geometry helpers shared by every class.
// Header-only (all functions are inline) so there is nothing extra to compile.
#ifndef UTILS_H
#define UTILS_H

#include "raylib.h"
#include <cmath>
#include <algorithm>

namespace util
{
    const float PI_F = 3.14159265358979f;

    inline float degToRad(float d) { return d * PI_F / 180.0f; }
    inline float radToDeg(float r) { return r * 180.0f / PI_F; }

    inline Vector2 vec(float x, float y) { Vector2 v; v.x = x; v.y = y; return v; }
    inline Rectangle rect(float x, float y, float w, float h) { Rectangle r; r.x = x; r.y = y; r.width = w; r.height = h; return r; }
    inline Color rgba(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255)
    {
        Color c; c.r = r; c.g = g; c.b = b; c.a = a; return c;
    }

    inline Vector2 add(Vector2 a, Vector2 b) { return vec(a.x + b.x, a.y + b.y); }
    inline Vector2 sub(Vector2 a, Vector2 b) { return vec(a.x - b.x, a.y - b.y); }
    inline Vector2 scale(Vector2 a, float s) { return vec(a.x * s, a.y * s); }
    inline float length(Vector2 a) { return std::sqrt(a.x * a.x + a.y * a.y); }
    inline float distance(Vector2 a, Vector2 b) { return length(sub(a, b)); }

    inline Vector2 normalize(Vector2 a)
    {
        float l = length(a);
        if (l < 0.0001f) return vec(0.0f, 0.0f);
        return vec(a.x / l, a.y / l);
    }

    inline Vector2 fromAngle(float angle) { return vec(std::cos(angle), std::sin(angle)); }

    inline Vector2 rotate(Vector2 v, float angle)
    {
        float c = std::cos(angle), s = std::sin(angle);
        return vec(v.x * c - v.y * s, v.x * s + v.y * c);
    }

    inline float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }

    // Random float in [lo, hi] using raylib's random generator.
    inline float randomFloat(float lo, float hi)
    {
        return lo + (hi - lo) * (float)GetRandomValue(0, 10000) / 10000.0f;
    }

    // Does the segment a->b pass within r of point c? (used for bullet hits so
    // fast bullets cannot "tunnel" through a character between two frames)
    inline bool segmentHitsCircle(Vector2 a, Vector2 b, Vector2 c, float r)
    {
        Vector2 ab = sub(b, a);
        float len2 = ab.x * ab.x + ab.y * ab.y;
        float t = 0.0f;
        if (len2 > 0.0001f)
        {
            t = ((c.x - a.x) * ab.x + (c.y - a.y) * ab.y) / len2;
            t = clampf(t, 0.0f, 1.0f);
        }
        Vector2 closest = add(a, scale(ab, t));
        return distance(closest, c) <= r;
    }

    // Segment vs axis-aligned rectangle (slab method). Used for line of sight.
    inline bool segmentIntersectsRect(Vector2 a, Vector2 b, Rectangle r)
    {
        float tmin = 0.0f, tmax = 1.0f;
        float dx = b.x - a.x, dy = b.y - a.y;

        if (std::fabs(dx) < 0.00001f)
        {
            if (a.x < r.x || a.x > r.x + r.width) return false;
        }
        else
        {
            float t1 = (r.x - a.x) / dx;
            float t2 = (r.x + r.width - a.x) / dx;
            if (t1 > t2) std::swap(t1, t2);
            tmin = std::max(tmin, t1);
            tmax = std::min(tmax, t2);
            if (tmin > tmax) return false;
        }

        if (std::fabs(dy) < 0.00001f)
        {
            if (a.y < r.y || a.y > r.y + r.height) return false;
        }
        else
        {
            float t1 = (r.y - a.y) / dy;
            float t2 = (r.y + r.height - a.y) / dy;
            if (t1 > t2) std::swap(t1, t2);
            tmin = std::max(tmin, t1);
            tmax = std::min(tmax, t2);
            if (tmin > tmax) return false;
        }
        return true;
    }
}

#endif
