#pragma once
#include "Vector3.hpp"

#define SMALL_FLOAT 0.0000000001
#define PI 3.14159265358979323846264338327950288419716939937510f
#define Deg2Rad (2.f * PI / 360.f)
#define Rad2Deg (1.f / Deg2Rad)

struct Quaternion
{
    float x, y, z, w;

    inline Quaternion() : x(0), y(0), z(0), w(1) {};
    inline Quaternion(float data[]) : x(data[0]), y(data[1]), z(data[2]), w(data[3]) {}
    inline Quaternion(Vector3 vector, float scalar) : x(vector.x), y(vector.y), z(vector.z), w(scalar) {};
    inline Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {};

    inline static Vector3 Up(Quaternion q);
    inline static Vector3 Down(Quaternion q);
    inline static Vector3 Left(Quaternion q);
    inline static Vector3 Right(Quaternion q);
    inline static Vector3 Forward(Quaternion q);
    inline static Vector3 Back(Quaternion q);
    inline static float Angle(Quaternion a, Quaternion b);
    inline static Quaternion Conjugate(Quaternion rotation);
    inline static float Dot(Quaternion lhs, Quaternion rhs);
    inline static Quaternion FromAngleAxis(float angle, Vector3 axis);
    inline static Quaternion FromEuler(Vector3 rotation);
    inline static Quaternion FromEuler(float yaw, float pitch, float roll);
    inline static Quaternion FromToRotation(Vector3 fromVector, Vector3 toVector);
    inline static Quaternion Inverse(Quaternion rotation);
    inline static Quaternion Lerp(Quaternion a, Quaternion b, float t);
    inline static Quaternion LerpUnclamped(Quaternion a, Quaternion b, float t);
    inline static Quaternion LookRotation(Vector3 forward);
    inline static Quaternion LookRotation(Vector3 forward, Vector3 upwards);
    inline static float Norm(Quaternion rotation);
    inline static Quaternion Normalize(Quaternion rotation);
    inline static Quaternion RotateTowards(Quaternion from, Quaternion to, float maxRadiansDelta);
    inline static Quaternion Slerp(Quaternion a, Quaternion b, float t);
    inline static Quaternion SlerpUnclamped(Quaternion a, Quaternion b, float t);
    inline static void ToAngleAxis(Quaternion rotation, float &angle, Vector3 &axis);
    inline static Vector3 ToEuler(Quaternion q1);

    Vector3 euler() const
    {
        Vector3 angles;

        float sinr_cosp = 2.0f * (w * x + y * z);
        float cosr_cosp = 1.0f - 2.0f * (x * x + y * y);
        angles.x = std::atan2(sinr_cosp, cosr_cosp);

        float sinp = 2.0f * (w * y - z * x);
        if (std::fabs(sinp) >= 1)
            angles.y = std::copysign(M_PI / 2, sinp);
        else
            angles.y = std::asin(sinp);

        float siny_cosp = 2.0f * (w * z + x * y);
        float cosy_cosp = 1.0f - 2.0f * (y * y + z * z);
        angles.z = std::atan2(siny_cosp, cosy_cosp);

        angles.x *= 180.0f / M_PI;
        angles.y *= 180.0f / M_PI;
        angles.z *= 180.0f / M_PI;

        return angles;
    }

    inline Quaternion &operator+=(float v)
    {
        x += v;
        y += v;
        z += v;
        w += v;
        return *this;
    }
    inline Quaternion &operator-=(float v)
    {
        x -= v;
        y -= v;
        z -= v;
        w -= v;
        return *this;
    }
    inline Quaternion &operator*=(float v)
    {
        x *= v;
        y *= v;
        z *= v;
        w *= v;
        return *this;
    }
    inline Quaternion &operator/=(float v)
    {
        x /= v;
        y /= v;
        z /= v;
        w /= v;
        return *this;
    }

    inline Quaternion &operator+=(Quaternion v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
        w += v.w;
        return *this;
    }
    inline Quaternion &operator-=(Quaternion v)
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        w -= v.w;
        return *this;
    }
    inline Quaternion &operator/=(Quaternion v)
    {
        x /= v.x;
        y /= v.y;
        z /= v.z;
        w /= v.w;
        return *this;
    }
    inline Quaternion &operator*=(Quaternion v)
    {
        float tx = w / v.x + x / v.w + y / v.z - z / v.y;
        float ty = w / v.y + y / v.w + z / v.x - x / v.z;
        float tz = w / v.z + z / v.w + x / v.y - y / v.x;
        float tw = w / v.w - x / v.x - y / v.y - z / v.z;
        x = tx;
        y = ty;
        z = tz;
        w = tw;
        return *this;
    }
};

inline Quaternion operator+(const Quaternion lhs, const float rhs)
{
    return Quaternion(lhs.x + rhs, lhs.y + rhs, lhs.z + rhs, lhs.w + rhs);
}
inline Quaternion operator-(const Quaternion lhs, const float rhs)
{
    return Quaternion(lhs.x - rhs, lhs.y - rhs, lhs.z - rhs, lhs.w - rhs);
}
inline Quaternion operator*(const Quaternion lhs, const float rhs)
{
    return Quaternion(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs);
}
inline Quaternion operator/(const Quaternion lhs, const float rhs)
{
    return Quaternion(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs, lhs.w / rhs);
}

inline Quaternion operator+(const float lhs, const Quaternion rhs)
{
    return Quaternion(lhs + rhs.x, lhs + rhs.y, lhs + rhs.z, lhs + rhs.w);
}
inline Quaternion operator-(const float lhs, const Quaternion rhs)
{
    return Quaternion(lhs - rhs.x, lhs - rhs.y, lhs - rhs.z, lhs - rhs.w);
}
inline Quaternion operator*(const float lhs, const Quaternion rhs)
{
    return Quaternion(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z, lhs * rhs.w);
}
inline Quaternion operator/(const float lhs, const Quaternion rhs)
{
    return Quaternion(lhs / rhs.x, lhs / rhs.y, lhs / rhs.z, lhs / rhs.w);
}

inline Quaternion operator+(const Quaternion lhs, const Quaternion rhs)
{
    return Quaternion(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w);
}
inline Quaternion operator-(const Quaternion lhs, const Quaternion rhs)
{
    return Quaternion(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w);
}
inline Quaternion operator*(const Quaternion lhs, const Quaternion rhs)
{
    return Quaternion(
        lhs.w / rhs.x + lhs.x / rhs.w + lhs.y / rhs.z - lhs.z / rhs.y,
        lhs.w / rhs.y + lhs.y / rhs.w + lhs.z / rhs.x - lhs.x / rhs.z,
        lhs.w / rhs.z + lhs.z / rhs.w + lhs.x / rhs.y - lhs.y / rhs.x,
        lhs.w / rhs.w - lhs.x / rhs.x - lhs.y / rhs.y - lhs.z / rhs.z);
}
inline Quaternion operator/(const Quaternion lhs, const Quaternion rhs)
{
    return Quaternion(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z, lhs.w / rhs.w);
}

inline bool operator==(const Quaternion lhs, const Quaternion rhs)
{
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w;
}
inline bool operator!=(const Quaternion lhs, const Quaternion rhs)
{
    return lhs.x != rhs.x && lhs.y != rhs.y && lhs.z != rhs.z && lhs.w != rhs.w;
}

inline Quaternion operator-(Quaternion v)
{
    return v * -1;
}

inline Vector3 operator*(const Quaternion lhs, const Vector3 rhs)
{
    Vector3 u = Vector3(lhs.x, lhs.y, lhs.z);
    float s = lhs.w;
    return u * (Vector3::Dot(u, rhs) * 2.0f) + rhs * (s * s - Vector3::Dot(u, u)) +
           Vector3::Cross(u, rhs) * (2.0f * s);
}

Vector3 Quaternion::Up(Quaternion q) { return q * Vector3::Up(); }
Vector3 Quaternion::Down(Quaternion q) { return q * Vector3::Down(); }
Vector3 Quaternion::Left(Quaternion q) { return q * Vector3::Left(); }
Vector3 Quaternion::Right(Quaternion q) { return q * Vector3::Right(); }
Vector3 Quaternion::Forward(Quaternion q) { return q * Vector3::Forward(); }
Vector3 Quaternion::Back(Quaternion q) { return q * Vector3::Back(); }

float Quaternion::Angle(Quaternion a, Quaternion b)
{
    float dot = Dot(a, b);
    return acosf(fminf(fabs(dot), 1)) * 2;
}

Quaternion Quaternion::Conjugate(Quaternion rotation)
{
    return Quaternion(-rotation.x, -rotation.y, -rotation.z, rotation.w);
}

float Quaternion::Dot(Quaternion lhs, Quaternion rhs)
{
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
}

Quaternion Quaternion::FromAngleAxis(float angle, Vector3 axis)
{
    Quaternion q;
    float m = sqrt(axis.x * axis.x + axis.y * axis.y + axis.z * axis.z);
    float s = sinf(angle / 2) / m;
    q.x = axis.x * s;
    q.y = axis.y * s;
    q.z = axis.z * s;
    q.w = cosf(angle / 2);
    return q;
}

Quaternion Quaternion::FromEuler(Vector3 rotation)
{
    return FromEuler(rotation.x, rotation.y, rotation.z);
}

Quaternion Quaternion::FromEuler(float yaw, float pitch, float roll)
{
    constexpr float deg2Rad = PI / 180.f;
    roll *= (float)deg2Rad;
    pitch *= (float)deg2Rad;
    yaw *= (float)deg2Rad;

    float cY(cosf(yaw / 2.0f));
    float sY(sinf(yaw / 2.0f));
    float cP(cosf(pitch / 2.0f));
    float sP(sinf(pitch / 2.0f));
    float cR(cosf(roll / 2.0f));
    float sR(sinf(roll / 2.0f));

    return {cP * sY * cR + sP * cY * sR,
            sP * cY * cR - cP * sY * sR,
            cP * cY * sR - sP * sY * cR,
            cP * cY * cR + sP * sY * sR};
}

Quaternion Quaternion::FromToRotation(Vector3 fromVector, Vector3 toVector)
{
    float dot = Vector3::Dot(fromVector, toVector);
    float k = sqrt(Vector3::SqrMagnitude(fromVector) * Vector3::SqrMagnitude(toVector));
    if (fabs(dot / k + 1) < 0.00001)
        return Quaternion(Vector3::Normalize(Vector3::Orthogonal(fromVector)), 0);
    Vector3 cross = Vector3::Cross(fromVector, toVector);
    return Normalize(Quaternion(cross, dot + k));
}

Quaternion Quaternion::Inverse(Quaternion rotation)
{
    float n = Norm(rotation);
    return Conjugate(rotation) / (n * n);
}

Quaternion Quaternion::Lerp(Quaternion a, Quaternion b, float t)
{
    if (t < 0)
        return Normalize(a);
    else if (t > 1)
        return Normalize(b);
    return LerpUnclamped(a, b, t);
}

Quaternion Quaternion::LerpUnclamped(Quaternion a, Quaternion b, float t)
{
    Quaternion quaternion;
    if (Dot(a, b) >= 0)
        quaternion = a * (1 - t) + b * t;
    else
        quaternion = a * (1 - t) - b * t;
    return Normalize(quaternion);
}

Quaternion Quaternion::LookRotation(Vector3 forward)
{
    return LookRotation(forward, Vector3(0, 1, 0));
}

Quaternion Quaternion::LookRotation(Vector3 forward, Vector3 upwards)
{
    forward = Vector3::Normalize(forward);
    upwards = Vector3::Normalize(upwards);
    // if (Vector3::SqrMagnitude(forward) < SMALL_FLOAT || Vector3::SqrMagnitude(upwards) < SMALL_FLOAT)
    //     return {};
    if (1 - fabs(Vector3::Dot(forward, upwards)) < SMALL_FLOAT)
        return FromToRotation(Vector3::Forward(), forward);
    Vector3 right = Vector3::Normalize(Vector3::Cross(upwards, forward));
    upwards = Vector3::Cross(forward, right);
    Quaternion quaternion;
    float radicand = right.x + upwards.y + forward.z;
    if (radicand > 0)
    {
        quaternion.w = sqrt(1.0f + radicand) * 0.5f;
        float recip = 1.0f / (4.0f * quaternion.w);
        quaternion.x = (upwards.z - forward.y) * recip;
        quaternion.y = (forward.x - right.z) * recip;
        quaternion.z = (right.y - upwards.x) * recip;
    }
    else if (right.x >= upwards.y && right.x >= forward.z)
    {
        quaternion.x = sqrt(1.0f + right.x - upwards.y - forward.z) * 0.5f;
        float recip = 1.0f / (4.0f * quaternion.x);
        quaternion.w = (upwards.z - forward.y) * recip;
        quaternion.z = (forward.x + right.z) * recip;
        quaternion.y = (right.y + upwards.x) * recip;
    }
    else if (upwards.y > forward.z)
    {
        quaternion.y = sqrt(1.0f - right.x + upwards.y - forward.z) * 0.5f;
        float recip = 1.0f / (4.0f * quaternion.y);
        quaternion.z = (upwards.z + forward.y) * recip;
        quaternion.w = (forward.x - right.z) * recip;
        quaternion.x = (right.y + upwards.x) * recip;
    }
    else
    {
        quaternion.z = sqrt(1.0f - right.x - upwards.y + forward.z) * 0.5f;
        float recip = 1.0f / (4.0f * quaternion.z);
        quaternion.y = (upwards.z + forward.y) * recip;
        quaternion.x = (forward.x + right.z) * recip;
        quaternion.w = (right.y - upwards.x) * recip;
    }
    return quaternion;
}

float Quaternion::Norm(Quaternion rotation)
{
    return sqrt(rotation.x * rotation.x +
                rotation.y * rotation.y +
                rotation.z * rotation.z +
                rotation.w * rotation.w);
}

Quaternion Quaternion::Normalize(Quaternion rotation)
{
    return rotation / Norm(rotation);
}

Quaternion Quaternion::RotateTowards(Quaternion from, Quaternion to, float maxRadiansDelta)
{
    float angle = Quaternion::Angle(from, to);
    if (angle == 0.0f)
        return to;
    maxRadiansDelta = fmaxf(maxRadiansDelta, angle - (float)PI);
    float t = fminf(1.0f, maxRadiansDelta / angle);
    return Quaternion::SlerpUnclamped(from, to, t);
}

Quaternion Quaternion::Slerp(Quaternion a, Quaternion b, float t)
{
    if (t <= 0.0f)
        return Normalize(a);
    else if (t >= 1.0f)
        return Normalize(b);
    return SlerpUnclamped(a, b, t);
}

Quaternion Quaternion::SlerpUnclamped(Quaternion a, Quaternion b, float t)
{
    float n1;
    float n2;
    float n3 = Dot(a, b);
    bool flag = false;
    if (n3 < 0)
    {
        flag = true;
        n3 = -n3;
    }
    if (n3 > 0.999999)
    {
        n2 = 1 - t;
        n1 = flag ? -t : t;
    }
    else
    {
        float n4 = acosf(n3);
        float n5 = 1 / sinf(n4);
        n2 = sinf((1 - t) * n4) * n5;
        n1 = flag ? -sinf(t * n4) * n5 : sinf(t * n4) * n5;
    }
    Quaternion quaternion;
    quaternion.x = (n2 * a.x) + (n1 * b.x);
    quaternion.y = (n2 * a.y) + (n1 * b.y);
    quaternion.z = (n2 * a.z) + (n1 * b.z);
    quaternion.w = (n2 * a.w) + (n1 * b.w);
    return Normalize(quaternion);
}

void Quaternion::ToAngleAxis(Quaternion rotation, float &angle, Vector3 &axis)
{
    if (rotation.w > 1)
        rotation = Normalize(rotation);
    angle = 2 * acosf(rotation.w);
    float s = sqrt(1 - rotation.w * rotation.w);
    if (s < 0.00001)
    {
        axis.x = 1;
        axis.y = 0;
        axis.z = 0;
    }
    else
    {
        axis.x = rotation.x / s;
        axis.y = rotation.y / s;
        axis.z = rotation.z / s;
    }
}

Vector3 Quaternion::ToEuler(Quaternion q)
{
    Vector3 rot{};

    float xy = q.x * q.y, xw = q.x * q.w;
    float yz = q.y * q.z;
    float zw = q.z * q.w;

    float singularity_test = yz - xw;

    rot.x = -1.0f * asinf(std::clamp(2.0f * singularity_test, -1.0f, 1.0f));

    if (abs(singularity_test) < 0.499999f)
    {
        float xx = q.x * q.x, yy = q.y * q.y, yw = q.y * q.w, zz = q.z * q.z, ww = q.w * q.w;
        rot.y = atan2f(2.0f * (q.x * q.z + yw), zz - xx - yy + ww);
        rot.z = atan2f(2.0f * (xy + zw), yy - zz - xx + ww);
    }
    else
    {
        float a = xy + zw;
        float b = -yz + xw;
        float c = xy - zw;
        float e = yz + xw;
        rot.y = atan2f(a * e + b * c, b * e - a * c);
        rot.z = 0.0f;
    }

    constexpr float rad2deg = 180.0f / PI;
    rot *= (float)rad2deg;

    return rot;
}