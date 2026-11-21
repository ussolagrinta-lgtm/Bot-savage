#include "math_util.hpp"

namespace rob {

std::optional<Vec2> WorldToScreen(const Vec3& world, const Matrix4x4& view, const Matrix4x4& projection,
                                  const Viewport& vp) {
    if (vp.width <= 0 || vp.height <= 0)
        return std::nullopt;

    float clipX = view.m[0][0] * world.x + view.m[1][0] * world.y + view.m[2][0] * world.z + view.m[3][0];
    float clipY = view.m[0][1] * world.x + view.m[1][1] * world.y + view.m[2][1] * world.z + view.m[3][1];
    float clipZ = view.m[0][2] * world.x + view.m[1][2] * world.y + view.m[2][2] * world.z + view.m[3][2];
    float clipW = view.m[0][3] * world.x + view.m[1][3] * world.y + view.m[2][3] * world.z + view.m[3][3];

    float projX = projection.m[0][0] * clipX + projection.m[1][0] * clipY + projection.m[2][0] * clipZ +
                  projection.m[3][0] * clipW;
    float projY = projection.m[0][1] * clipX + projection.m[1][1] * clipY + projection.m[2][1] * clipZ +
                  projection.m[3][1] * clipW;
    float projZ = projection.m[0][2] * clipX + projection.m[1][2] * clipY + projection.m[2][2] * clipZ +
                  projection.m[3][2] * clipW;
    float projW = projection.m[0][3] * clipX + projection.m[1][3] * clipY + projection.m[2][3] * clipZ +
                  projection.m[3][3] * clipW;

    if (projW < 0.1f)
        return std::nullopt;

    float ndcX = projX / projW;
    float ndcY = projY / projW;

    Vec2 screen;
    screen.x = (ndcX + 1.f) * 0.5f * static_cast<float>(vp.width);
    screen.y = (1.f - ndcY) * 0.5f * static_cast<float>(vp.height);
    return screen;
}

float HorizontalFovDegrees(const Vec3& from, const Vec3& to, const Vec3& viewForward) {
    Vec3 dir = Sub(to, from);
    float len = Length(dir);
    if (len < 1e-4f)
        return 180.f;
    dir.x /= len;
    dir.y /= len;
    dir.z /= len;

    Vec3 fwd = viewForward;
    float fwdLen = Length(fwd);
    if (fwdLen > 1e-4f) {
        fwd.x /= fwdLen;
        fwd.y /= fwdLen;
        fwd.z /= fwdLen;
    }

    float dot = Dot(fwd, dir);
    if (dot > 1.f)
        dot = 1.f;
    if (dot < -1.f)
        dot = -1.f;
    return std::acos(dot) * 180.f / 3.14159265358979323846f;
}

Vec2 CalcMouseDeltaForAngles(float yawDeg, float pitchDeg, float sensitivity) {
    // Uncalibrated — tune against your in-game sensitivity.
    const float scale = sensitivity * 10.f;
    return {yawDeg * scale, -pitchDeg * scale};
}

} // namespace rob