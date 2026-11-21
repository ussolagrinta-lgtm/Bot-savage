#pragma once

#include <cmath>
#include <cstdint>
#include <optional>

namespace rob {

struct Vec3 {
    float x = 0.f;
    float y = 0.f;
    float z = 0.f;
};

struct Vec2 {
    float x = 0.f;
    float y = 0.f;
};

struct Viewport {
    int width = 0;
    int height = 0;
};

// Unity camera matrices are column-major; right-handed, Y-up (confirm in-game).
struct Matrix4x4 {
    float m[4][4]{};
};

inline float Dot(const Vec3& a, const Vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

inline Vec3 Sub(const Vec3& a, const Vec3& b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }

inline float Length(const Vec3& v) { return std::sqrt(Dot(v, v)); }

inline float Distance(const Vec3& a, const Vec3& b) { return Length(Sub(a, b)); }

inline float DegreesToRadians(float deg) { return deg * 3.14159265358979323846f / 180.f; }

// Screen position when WorldToScreen succeeds; nullopt when behind camera or off frustum.
std::optional<Vec2> WorldToScreen(const Vec3& world, const Matrix4x4& view, const Matrix4x4& projection,
                                  const Viewport& vp);

float HorizontalFovDegrees(const Vec3& from, const Vec3& to, const Vec3& viewForward);

Vec2 CalcMouseDeltaForAngles(float yawDeg, float pitchDeg, float sensitivity);

} // namespace rob