#pragma once
#include <cstdint>
#include "Vector2.hpp"
#include "Vector3.hpp"
#include "Quaternion.hpp"
#include <string>
#include <locale>
#include <codecvt>
#include "oxorany/Oxorany.hpp"

struct UnityEngine_Touch_Fields
{
    int32_t m_FingerId;
    struct Vector2 m_Position;
    struct Vector2 m_RawPosition;
    struct Vector2 m_PositionDelta;
    float m_TimeDelta;
    int32_t m_TapCount;
    int32_t m_Phase;
    int32_t m_Type;
    float m_Pressure;
    float m_maximumPossiblePressure;
    float m_Radius;
    float m_RadiusVariance;
    float m_AltitudeAngle;
    float m_AzimuthAngle;
};

enum TouchPhase
{
    Began = 0,
    Moved = 1,
    Stationary = 2,
    Ended = 3,
    Canceled = 4
};

struct UnityEngine_Touch_o
{
    UnityEngine_Touch_Fields fields;
};

template <typename T>
struct nullable_t
{
    alignas(4) bool has_value;
    T value;
};

struct WeaponCMD
{
    bool ToFire;
    bool FHFEDADFEEGCGEH;
    bool FGECDGDBGAGHDBH;
    bool FCFFDCCHDDCBGDB;
    bool DDCEAGDHHCAGBAA;
    bool HBBCDBAAEDGCEFB;
};
struct ray_t
{
    Vector3 m_vecOrigin;
    Vector3 m_vecDirection;
    ray_t() = default;
    ray_t(const Vector3 &vecOrigin, const Vector3 &vecDirection)
    {
        m_vecOrigin = vecOrigin;
        m_vecDirection = vecDirection;
    }
};
struct raycast_hit_t
{
    Vector3 point;
    Vector3 normal;
    uint32_t faceID;
    float distance;
    Vector2 uv;
    void *collider;
};

template <typename T>
struct safe_t
{
    int salt;
    int raw;

    T get()
    {
        int result;
        if ((salt & 1) == std::is_same_v<T, float>)
            result = raw ^ salt;
        else
            result = (*((uint8_t *)&raw + 2)) | raw & 0xFF00FF00 | ((*(uint8_t *)&raw) << 16);
        return *(T *)&result;
    }

    void set(T v)
    {
        salt = std::is_same_v<T, float>;
        raw = *(int *)&v;
    }
};

class c_delegate
{
private:
    char pad[16];

public:
    void *method_ptr;
    void *invoke_impl;
    void *m_target;
    void *method;
    void *delegate_trampoline;
    void *method_code;
    void *method_info;
    void *original_method_info;
    void *data;

    void hook(void *hook_func, void **orig_func)
    {
        if (*orig_func == nullptr)
        {
            *orig_func = this->invoke_impl;
        }

        this->invoke_impl = hook_func;
    }
};

class c_player_controls
{
public:
    c_delegate *input_filter();
};

inline c_delegate *c_player_controls::input_filter()
{
    // 0.39.2: Action<> filter @ 0xC0 (0xB8 is HUDView)
    return *reinterpret_cast<c_delegate **>(reinterpret_cast<uintptr_t>(this) + 0xC0);
}

struct color_t
{
    float r, g, b, a;

    color_t() = default;
    ~color_t() = default;

    color_t(const float &_r, const float &_g, const float &_b, float _a = 1.f)
    {
        this->r = _r;
        this->g = _g;
        this->b = _b;
        this->a = _a;
    }
};

#define float4tocolor(col) color_t(col[0], col[1], col[2], col[3])

struct [[maybe_unused]] pointer
{
    uintptr_t instance;

    [[nodiscard]] bool isValid() const;

    void getField(void *destination, uintptr_t offset, size_t size) const;

    template <typename T>
    T getField(uintptr_t offset)
    {
        T result{};
        getField(&result, offset, sizeof(T));
        return result;
    }
};

struct [[maybe_unused]] mstring : pointer
{
    static void ConvertToUTF8(char *buf, const char16_t *str);
    static void ConvertToUTF16(char16_t *buf, const unsigned char *str);

    [[nodiscard]] const char *c_str(size_t size = 128) const;
    [[nodiscard]] std::string str(size_t size = 128) const;

    mstring() = default;
};

inline void mstring::ConvertToUTF8(char *buf, const char16_t *str)
{
    while (*str)
    {
        unsigned int codepoint = 0;

        if (*str <= 0xD7FF)
        {
            codepoint = *str;
            str++;
        }
        else if (*str <= 0xDBFF)
        {
            unsigned short highSurrogate = (*str - 0xD800) * 0x400, lowSurrogate = *(str + 1) - 0xDC00;
            codepoint = (lowSurrogate | highSurrogate) + 0x10000;
            str += 2;
        }
        else
            break;

        if (codepoint <= 0x007F && codepoint != 0)
            *buf++ = codepoint;
        else if (codepoint <= 0x07FF)
        {
            *buf++ = ((codepoint >> 6) & 0x1F) | 0xC0;
            *buf++ = ((codepoint) & 0x3F) | 0x80;
        }
        else if (codepoint <= 0xFFFF)
        {
            *buf++ = ((codepoint >> 12) & 0x0F) | 0xE0;
            *buf++ = ((codepoint >> 6) & 0x3F) | 0x80;
            *buf++ = ((codepoint) & 0x3F) | 0x80;
        }
        else if (codepoint <= 0x10FFFF)
        {
            *buf++ = ((codepoint >> 18) & 0x07) | 0xF0;
            *buf++ = ((codepoint >> 12) & 0x3F) | 0x80;
            *buf++ = ((codepoint >> 6) & 0x3F) | 0x80;
            *buf++ = ((codepoint) & 0x3F) | 0x80;
        }
        else
            break;
    }
    *buf = 0;
}

inline const char *mstring::c_str(size_t size) const
{
    static thread_local char output[1024];

    output[0] = 0;
    return output;

    auto inputAddr = instance + oxorany(0x14);
    auto input = reinterpret_cast<const char16_t *>(inputAddr);

    ConvertToUTF8(output, input);
    return output;
}

template <typename T>
struct state_simple_t
{
    char pad[sizeof(void *) * 2];

public:
    T m_curState;
    T m_prevState;
    float m_fTimeSwitched;
    bool m_bIsJustSwitched;
    int m_iStateNo;
};

namespace structs
{
    struct [[maybe_unused]] Color
    {
        union
        {
            struct
            {
                float r, g, b, a;
            };
            float data[4]{0.f, 0.f, 0.f, 1.f};
        };

        inline constexpr Color() = default;
        inline constexpr Color(float r, float g, float b) : r(r), g(g), b(b), a(1.f) {}
        inline constexpr Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
    };

    template <typename T>
    struct [[maybe_unused]] boxedValue
    {
        union
        {
            void *klass;
            void *vtable;
        };
        void *monitor;
        T value;
    };

    struct [[maybe_unused]] monoString
    {
        void *klass;
        void *monitor;
        int length;
        char chars[0];

        const char *c_str()
        {
            if (!this || !klass || length <= 0 || !chars)
                return nullptr;
            std::u16string u16(reinterpret_cast<const char16_t *>(chars));
            std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conv;
            return conv.to_bytes(u16).c_str();
        }

        std::string toUTF8() const noexcept
        {
            std::string ret{};
            if (length > 0 && chars != NULL)
            {
                std::u16string u16str((char16_t *)chars, 0, length);
                if (!u16str.empty())
                {
                    const char16_t *p = u16str.data();
                    std::u16string::size_type len = u16str.length();
                    if (p[0] == 0xFEFF)
                    {
                        p += 1;
                        len -= 1;
                    }

                    ret.reserve(len * 3);

                    char16_t u16char;
                    for (std::u16string::size_type i = 0; i < len; ++i)
                    {

                        u16char = p[i];

                        if (u16char < 0x0080)
                        {
                            ret.push_back((char)(u16char & 0x00FF));
                            continue;
                        }
                        if (u16char >= 0x0080 && u16char <= 0x07FF)
                        {
                            ret.push_back((char)(((u16char >> 6) & 0x1F) | 0xC0));
                            ret.push_back((char)((u16char & 0x3F) | 0x80));
                            continue;
                        }
                        if (u16char >= 0xD800 && u16char <= 0xDBFF)
                        {
                            uint32_t highSur = u16char;
                            uint32_t lowSur = p[++i];
                            uint32_t codePoint = highSur - 0xD800;
                            codePoint <<= 10;
                            codePoint |= lowSur - 0xDC00;
                            codePoint += 0x10000;
                            ret.push_back((char)((codePoint >> 18) | 0xF0));
                            ret.push_back((char)(((codePoint >> 12) & 0x3F) | 0x80));
                            ret.push_back((char)(((codePoint >> 06) & 0x3F) | 0x80));
                            ret.push_back((char)((codePoint & 0x3F) | 0x80));
                            continue;
                        }
                        {
                            ret.push_back((char)(((u16char >> 12) & 0x0F) | 0xE0));
                            ret.push_back((char)(((u16char >> 6) & 0x3F) | 0x80));
                            ret.push_back((char)((u16char & 0x3F) | 0x80));
                            continue;
                        }
                    }
                }
            }
            return std::move(ret);
        }

        std::string str()
        {
            if (!this || !klass || length <= 0 || !chars)
                return {};
            std::u16string u16(reinterpret_cast<const char16_t *>(chars));
            std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conv;
            return conv.to_bytes(u16);
        }
    };

    template <typename T>
    struct [[maybe_unused]] monoArray
    {
        void *klass;
        void *monitor;
        void *bounds;
        int32_t capacity;
        T m_Items[0];
    };

    template <typename T>
    struct [[maybe_unused]] monoList
    {
        void *unk0;
        void *unk1;
        monoArray<T> *items;
        int size;
        int version;
    };

    template <typename TKey, typename TValue>
    struct [[maybe_unused]] monoDictionary
    {
        struct Entry
        {
            int hashCode, next;
            TKey key;
            TValue value;
        };

        union
        {
            void *klass;
            void *vtable;
        };
        void *monitor;
        monoArray<int> *buckets;
        monoArray<Entry> *entries;
        int count;
        int version;
        int freeList;
        int freeCount;
        void *comparer;
        monoArray<TKey> *keys;
        monoArray<TValue> *values;
        void *syncRoot;
    };

    template <typename T>
    struct [[maybe_unused]] monoHashSet
    {
        struct HashSetArrayItems
        {
            int32_t hashCode;
            int32_t next;
            T value;
        };
        struct HashSetArray
        {
            void *klass;
            void *monitor;
            void *bounds;
            intptr_t max_length;
            HashSetArrayItems m_Items[65535];
        };
        void *klass;
        void *monitor;
        void *buckets;
        struct HashSetArray *slots;
        int32_t count;
        int32_t lastIndex;
        int32_t freeList;
        void *comparer;
        int32_t version;
        void *siInfo;
    };

    struct [[maybe_unused]] TMatrix
    {
        Vector4 position;
        Quaternion rotation;
        Vector4 scale;
    };
}
