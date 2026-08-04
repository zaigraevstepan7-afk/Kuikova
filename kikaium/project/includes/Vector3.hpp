#pragma once

#include <algorithm>
#include <cmath>
#define M_PI 3.141592653589793f
#include "Vector4.hpp"

struct Vector3 {
    float x, y, z;
	
    Vector3() : x(0), y(0), z(0) {}
    Vector3(float x, float y) : x(x), y(y), z(0) {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
    explicit Vector3(Vector4 v) : x(v.x), y(v.y), z(v.z) {}

    static Vector3 Back() { return {0, 0, -1}; };
    static Vector3 Down() { return {0, -1, 0}; };
    static Vector3 Forward() { return {0, 0, 1}; };
    static Vector3 Left() { return {-1, 0, 0}; };
    static Vector3 Right() { return {1, 0, 0}; };
    static Vector3 Up() { return {0, 1, 0}; }
    static Vector3 Zero() { return {0, 0, 0}; }
    static Vector3 One() { return {1, 1, 1}; }
    inline static float Angle(Vector3 from, Vector3 to);
    inline static float Dot(Vector3, Vector3);
    inline static Vector3 ClampMagnitude(Vector3, float);
    inline static float Component(Vector3, Vector3);
    inline static Vector3 Cross(Vector3, Vector3);
    inline static float Distance(Vector3, Vector3);
    inline static Vector3 FromSpherical(float, float, float);
    inline static Vector3 Lerp(Vector3, Vector3, float);
    inline static Vector3 LerpUnclamped(Vector3, Vector3, float);
    inline static Vector3 Max(Vector3, Vector3);
    inline static Vector3 Min(Vector3, Vector3);
    inline static Vector3 MoveTowards(Vector3, Vector3, float);
    inline static Vector3 Orthogonal(Vector3);
    inline static Vector3 RotateTowards(Vector3, Vector3, float, float);
    inline static Vector3 Scale(Vector3, Vector3);
    inline static Vector3 Slerp(Vector3, Vector3, float);
    inline static Vector3 SlerpUnclamped(Vector3, Vector3, float);
    inline static void ToSpherical(Vector3 vector, float &, float &, float &);
    inline static float Magnitude(Vector3);
    inline static float SqrMagnitude(Vector3);
    inline static Vector3 Normalize(Vector3);
    inline static Vector3 NormalizeEuler(Vector3);
    inline static float NormalizeAngle(float f);

    float mmagnitude() const {
        return std::sqrt(x * x + y * y + z * z);
    }


    Vector3 nnormalized() const {
        float m = mmagnitude();
        if (m > 0) {
            return Vector3(x / m, y / m, z / m);
        }
        return Vector3::Zero();
    }

    float length() const {
        return Magnitude(*this);
    }

    Vector3 normalizedEuler() const {
        auto norm = [](float a) -> float {
            a = std::fmod(a + 180.0f, 360.0f);
            if (a < 0.0f) a += 360.0f;
            return a - 180.0f;
        };
        return Vector3(norm(x), norm(y), norm(z));
    }

    const Vector3& operator+=(float v) { x+=v; y+=v; y+=v; return *this; };
    const Vector3& operator-=(float v) { x-=v; y-=v; y-=v; return *this; };
    const Vector3& operator*=(float v) { x*=v; y*=v; z*=v; return *this; };
    const Vector3& operator/=(float v) { x/=v; y/=v; z/=v; return *this; };
    const Vector3& operator+=(Vector3 v) { x+=v.x; y+=v.y; z+=v.z; return *this; };
    const Vector3& operator-=(Vector3 v) { x-=v.x; y-=v.y; z-=v.z; return *this; };
    const Vector3& operator*=(Vector3 v) { x*=v.x; y*=v.y; z*=v.z; return *this; };
    Vector3& operator/=(Vector3 v) { x/=v.x; y/=v.y; z/=v.z; return *this; };
};

#define rad2deg( x ) ( ( x ) * ( 180.f / M_PI ) )
#define deg2rad( x ) ( ( x ) * ( M_PI / 180.0 ) )

struct euler_angles_t
{
public:
    float pitch;
    float yaw;
    float roll;

    euler_angles_t( ) = default;
    euler_angles_t( const Vector3& in ) noexcept
    {
        this->pitch = in.x;
        this->yaw = in.y;
        this->roll = in.z;
    }
    euler_angles_t( const float& _pitch, const float& _yaw, const float& _roll ) noexcept
    {
        this->pitch = _pitch;
        this->yaw = _yaw;
        this->roll = _roll;
    }
    ~euler_angles_t( ) = default;

    euler_angles_t operator+( const euler_angles_t& ang ) const noexcept
    {
        return euler_angles_t {
            this->pitch + ang.pitch,
            this->yaw + ang.yaw,
            this->roll + ang.roll };
    }
    euler_angles_t operator+( const float& _f ) const noexcept
    {
        return euler_angles_t {
            this->pitch + _f,
            this->yaw + _f,
            this->roll + _f };
    }
    euler_angles_t& operator+=( const euler_angles_t& ang ) noexcept
    {
        *this = *this + ang;
        return *this;
    }
    euler_angles_t& operator+=( const float& _f ) noexcept
    {
        *this = *this + _f;
        return *this;
    }

    euler_angles_t operator-( const euler_angles_t& ang ) const noexcept
    {
        return euler_angles_t {
            this->pitch - ang.pitch,
            this->yaw - ang.yaw,
            this->roll - ang.roll };
    }
    euler_angles_t operator-( const float& _f ) const noexcept
    {
        return euler_angles_t {
            this->pitch - _f,
            this->yaw - _f,
            this->roll - _f };
    }
    euler_angles_t& operator-=( const euler_angles_t& ang ) noexcept
    {
        *this = *this - ang;
        return *this;
    }
    euler_angles_t& operator-=( const float& _f ) noexcept
    {
        *this = *this - _f;
        return *this;
    }

    euler_angles_t operator*( const euler_angles_t& ang ) const noexcept
    {
        return euler_angles_t {
            this->pitch * ang.pitch,
            this->yaw * ang.yaw,
            this->roll * ang.roll };
    }
    euler_angles_t operator*( const float& _f ) const noexcept
    {
        return euler_angles_t {
            this->pitch * _f,
            this->yaw * _f,
            this->roll * _f,
        };
    }
    euler_angles_t& operator*=( const euler_angles_t& ang ) noexcept
    {
        *this = *this * ang;
        return *this;
    }
    euler_angles_t& operator*=( const float& _f ) noexcept
    {
        *this = *this * _f;
        return *this;
    }

    euler_angles_t operator/( const euler_angles_t& ang ) const noexcept
    {
        return euler_angles_t {
            this->pitch / ang.pitch,
            this->yaw / ang.yaw,
            this->roll / ang.roll };
    }
    euler_angles_t operator/( const float& _f ) const noexcept
    {
        return euler_angles_t {
            this->pitch / _f,
            this->yaw / _f,
            this->roll / _f };
    }
    euler_angles_t& operator/=( const euler_angles_t& ang ) noexcept
    {
        *this = *this / ang;
        return *this;
    }
    euler_angles_t& operator/=( const float& _f ) noexcept
    {
        *this = *this / _f;
        return *this;
    }

    bool operator==( const euler_angles_t& ang ) const noexcept
    {
        return this->pitch == ang.pitch && this->yaw == ang.yaw && this->roll == ang.roll;
    }
    bool operator!=( const euler_angles_t& ang ) const noexcept
    {
        return !( *this == ang );
    }

    bool is_empty( ) const noexcept
    {
        return std::abs( pitch ) <= FLT_EPSILON &&
            std::abs( yaw ) <= FLT_EPSILON &&
            std::abs( roll ) <= FLT_EPSILON;
    }

    void clampPitch( float fMaxPitch = 70.f ) noexcept
    {
        if ( this->pitch > fMaxPitch )
            this->pitch = fMaxPitch;
        if ( this->pitch < -fMaxPitch )
            this->pitch = -fMaxPitch;
    }

    euler_angles_t clampedPitch( float fMaxPitch = 70.f ) noexcept
    {
        euler_angles_t ret = *this;
        ret.clampPitch( fMaxPitch );
        return ret;
    }

    void normalize( float fMaxPitch = 90.f, bool bClampPitch = true ) noexcept
    {
        if ( bClampPitch )
            this->clampPitch( fMaxPitch );
        else
        {
            if ( this->pitch > fMaxPitch )
                this->pitch -= 360.f;
            if ( this->pitch < -fMaxPitch )
                this->pitch += 360.f;
        }

        if ( this->yaw > 180.f )
            this->yaw -= 360.f;
        if ( this->yaw < -180.f )
            this->yaw += 360.f;

        this->roll = 0.f;
    }

    euler_angles_t normalized( float fMaxPitch = 90.f, bool bClampPitch = true ) const noexcept
    {
        euler_angles_t ret = *this;
        ret.normalize( fMaxPitch, bClampPitch );
        return ret;
    }
};

inline Vector3 operator+(Vector3 lhs, const float rhs) { return {lhs.x + rhs, lhs.y + rhs, lhs.z + rhs}; }
inline Vector3 operator-(Vector3 lhs, const float rhs) { return {lhs.x - rhs, lhs.y - rhs, lhs.z - rhs}; }
inline Vector3 operator*(Vector3 lhs, const float rhs) { return {lhs.x * rhs, lhs.y * rhs, lhs.z * rhs}; }
inline Vector3 operator/(Vector3 lhs, const float rhs) { return {lhs.x / rhs, lhs.y / rhs, lhs.z / rhs}; }
inline Vector3 operator+(const float lhs, Vector3 rhs) { return {lhs + rhs.x, lhs + rhs.y, lhs + rhs.z}; }
inline Vector3 operator-(const float lhs, Vector3 rhs) { return {lhs - rhs.x, lhs - rhs.y, lhs - rhs.z}; }
inline Vector3 operator*(const float lhs, Vector3 rhs) { return {lhs * rhs.x, lhs * rhs.y, lhs * rhs.z}; }
inline Vector3 operator/(const float lhs, Vector3 rhs) { return {lhs / rhs.x, lhs / rhs.y, lhs / rhs.z}; }
inline Vector3 operator+(Vector3 lhs, const Vector3 rhs) { return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z}; }
inline Vector3 operator-(Vector3 lhs, const Vector3 rhs) { return {lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z}; }
inline Vector3 operator*(Vector3 lhs, const Vector3 rhs) { return {lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z}; }
inline Vector3 operator/(Vector3 lhs, const Vector3 rhs) { return {lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z}; }
inline bool operator==(const Vector3 lhs, const Vector3 rhs) { return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z; }
inline bool operator!=(const Vector3 lhs, const Vector3 rhs) { return lhs.x != rhs.x && lhs.y != rhs.y && lhs.z != rhs.z; }
inline Vector3 operator-(Vector3 v) {return v * -1;}

float Vector3::Angle(Vector3 from, Vector3 to) {
    float v = Dot(from, to) / (Magnitude(from) * Magnitude(to));
    v = std::max(v, -1.0f);
    v = std::min(v, 1.0f);
    return acos(v);
}



float Vector3::Dot(Vector3 lhs, Vector3 rhs) {
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}
Vector3 Vector3::ClampMagnitude(Vector3 vector, float maxLength) {
    float length = Magnitude(vector);
    if (length > maxLength) vector *= maxLength / length;
    return vector;
}

float Vector3::Component(Vector3 a, Vector3 b) {
    return Dot(a, b) / Magnitude(b);
}

Vector3 Vector3::Cross(Vector3 lhs, Vector3 rhs) {
    float x = lhs.y * rhs.z - lhs.z * rhs.y;
    float y = lhs.z * rhs.x - lhs.x * rhs.z;
    float z = lhs.x * rhs.y - lhs.y * rhs.x;
    return {x, y, z};
}

float Vector3::Distance(Vector3 a, Vector3 b) {
    return Magnitude(a - b);
}

Vector3 Vector3::FromSpherical(float rad, float theta, float phi) {
    Vector3 v;
    v.x = rad * sin(theta) * cos(phi);
    v.y = rad * sin(theta) * sin(phi);
    v.z = rad * cos(theta);
    return v;
}

Vector3 Vector3::Lerp(Vector3 a, Vector3 b, float t) {
    if (t < 0) return a;
    else if (t > 1) return b;
    return LerpUnclamped(a, b, t);
}

Vector3 Vector3::LerpUnclamped(Vector3 a, Vector3 b, float t) {
    return (b - a) * t + a;
}

Vector3 Vector3::Max(Vector3 a, Vector3 b) {
    float x = a.x > b.x ? a.x : b.x;
    float y = a.y > b.y ? a.y : b.y;
    float z = a.z > b.z ? a.z : b.z;
    return {x, y, z};
}

Vector3 Vector3::Min(Vector3 a, Vector3 b) {
    float x = a.x > b.x ? b.x : a.x;
    float y = a.y > b.y ? b.y : a.y;
    float z = a.z > b.z ? b.z : a.z;
    return {x, y, z};
}

Vector3 Vector3::MoveTowards(Vector3 current, Vector3 target, float maxDistanceDelta) {
    Vector3 d = target - current;
    float m = Magnitude(d);
    if (m < maxDistanceDelta || m == 0)
        return target;
    return current + (d * maxDistanceDelta / m);
}

Vector3 Vector3::Orthogonal(Vector3 v) {
    return v.z < v.x ? Vector3(v.y, -v.x, 0) : Vector3(0, -v.z, v.y);
}

Vector3 Vector3::RotateTowards(Vector3 current, Vector3 target, float maxRadiansDelta, float maxMagnitudeDelta) {
    float magCur = Magnitude(current);
    float magTar = Magnitude(target);
    float newMag = magCur + maxMagnitudeDelta * ((magTar > magCur) - (magCur > magTar));
    newMag = fmin(newMag, fmax(magCur, magTar));
    newMag = fmax(newMag, fmin(magCur, magTar));
    float totalAngle = Angle(current, target) - maxRadiansDelta;
    if (totalAngle <= 0) return Normalize(target) * newMag;
    else if (totalAngle >= M_PI) return -Normalize(target) * newMag;
    Vector3 axis = Cross(current, target);
    float magAxis = Magnitude(axis);
    if (magAxis == 0) axis = Normalize(Cross(current, current + Vector3(3.95, 5.32, -4.24)));
    else axis /= magAxis;
    current = Normalize(current);
    Vector3 newVector = current * cos(maxRadiansDelta) + Cross(axis, current) * sin(maxRadiansDelta);
    return newVector * newMag;
}

Vector3 Vector3::Scale(Vector3 a, Vector3 b) {
    return {a.x * b.x, a.y * b.y, a.z * b.z};
}

Vector3 Vector3::Slerp(Vector3 a, Vector3 b, float t) {
    if (t < 0) return a;
    if (t > 1) return b;
    return SlerpUnclamped(a, b, t);
}

Vector3 Vector3::SlerpUnclamped(Vector3 a, Vector3 b, float t) {
    float magA = Magnitude(a);
    float magB = Magnitude(b);
    a /= magA;
    b /= magB;
    float dot = Dot(a, b);
    dot = fmax(dot, -1.0);
    dot = fmin(dot, 1.0);
    float theta = acos(dot) * t;
    Vector3 relativeVec = Normalize(b - a * dot);
    Vector3 newVec = a * cos(theta) + relativeVec * sin(theta);
    return newVec * (magA + (magB - magA) * t);
}

void Vector3::ToSpherical(Vector3 vector, float &rad, float &theta, float &phi) {
    rad = Magnitude(vector);
    float v = vector.z / rad;
    v = fmax(v, -1.0);
    v = fmin(v, 1.0);
    theta = acos(v);
    phi = atan2(vector.y, vector.x);
}

float Vector3::Magnitude(Vector3 v) {
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

float Vector3::SqrMagnitude(Vector3 v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

Vector3 Vector3::Normalize(Vector3 v) {
    float mag = Magnitude(v);
    if (mag == 0) return Vector3::Zero();
    return v / mag;
}

float Vector3::NormalizeAngle(float f) {
    while (f > 360) f -= 360;
    while (f < 0) f += 360;
    return f;
}

Vector3 Vector3::NormalizeEuler(Vector3 vec) {
    vec.x = NormalizeAngle(vec.x);
    vec.y = NormalizeAngle(vec.y);
    vec.z = NormalizeAngle(vec.z);
    return vec;
}
