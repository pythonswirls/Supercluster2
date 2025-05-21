#pragma once

#include <stdint.h>
#include <math.h>

typedef int32_t fixed;

const fixed FIXED_ONE = 0x10000;
const fixed FIXED_THREE_HALFS = 0x18000;
const fixed FIXED_MAX = 0x7fffffff;

fixed fix(const float f)
{
    return int32_t(f * 65536);
}

fixed unfix(const fixed i)
{
    return i / 65536.f;
}

fixed muli(const fixed a, const fixed b)
{
    return ((int64_t)a * b) >> 16;
}

fixed divi(const fixed a, const fixed b)
{
    return ((int64_t)a << 16) / b;
}

fixed floori(const fixed a)
{
    return a & 0xffff0000;
}

fixed powi16(const fixed a)
{
    fixed a2 = muli(a, a);
    fixed a4 = muli(a2, a2);
    return muli(a4, a4);
}

fixed min(const fixed a, const fixed b)
{
    return b < a ? b : a;
}

fixed max(const fixed a, const fixed b)
{
    return b > a ? b : a;
}

fixed clamp(const fixed x, const fixed a, const fixed b)
{
    return min(max(a, x), b);
}

int __lzcnt(const fixed a)
{
    fixed na = a;
    int r = 32;
    if (na >= 0x00010000) { na >>= 16; r -= 16; }
    if (na >= 0x00000100) { na >>=  8; r -=  8; }
    if (na >= 0x00000010) { na >>=  4; r -=  4; }
    if (na >= 0x00000004) { na >>=  2; r -=  2; }
    r -= na - (na & (na >> 1));
    return r;
}

fixed fix2binFloat(const fixed a)
{
    int i = __lzcnt(a & 0x7fffffff);
    fixed sign = a & 0x80000000;
    fixed exp = (15 - i) + 127;
    fixed mantissa = ((a << i) >> 8) & 0x7fffff;
    return sign | (exp << 23) | mantissa;
}

fixed binFloat2fix(const fixed a)
{
    if(!a) return 0;
    fixed sign = a & 0x80000000;
    fixed exp = ((a >> 23) & 0xff) - 127;
    fixed mantissa = 0x800000 | (a & 0x7fffff);
    return sign | ((mantissa << 7) >> (14 - exp));
}

fixed b_rsqrt(const fixed number)
{
    fixed i;
    fixed x2;
    fixed y;
    const fixed threehalfs = 0x18000;

    x2 = number >> 1;
    y  = fix2binFloat(number);
    i  = y;                                     // evil floating point bit level hacking
    i  = 0x5f3759df - (i >> 1);                 // what the fuck?
    y  = binFloat2fix(i);

    y  = muli(y, threehalfs - (muli(x2, muli(y, y))));   // 1st iteration
    y  = muli(y, threehalfs - (muli(x2, muli(y, y))));   // 2nd iteration, this can be removed

    return y;
}

fixed rsqrti(const fixed a)
{
    return b_rsqrt(a);
}

fixed sqrti(const fixed a)
{
    return muli(a, rsqrti(a));
}

class Vec3;

class Vec3
{
    public:
    fixed v[3];

    Vec3()
    {
        v[0] = 0;
        v[1] = 0;
        v[2] = 0;
    }

    Vec3(const fixed x, const fixed y, const fixed z)
    {
        v[0] = x;
        v[1] = y;
        v[2] = z;
    }

    Vec3(const Vec3& c)
    {
        v[0] = c.v[0];
        v[1] = c.v[1];
        v[2] = c.v[2];
    }

    Vec3 sub(const Vec3 &v2) const
    {
        return Vec3(v[0] - v2.v[0], v[1] - v2.v[1], v[2] - v2.v[2]);
    }

    Vec3 add(const Vec3 &v2) const
    {
        return Vec3(v[0] + v2.v[0], v[1] + v2.v[1], v[2] + v2.v[2]);
    }

    Vec3 scale(const fixed s) const
    {
        return Vec3(muli(v[0], s), muli(v[1], s), muli(v[2], s));
    }

    fixed dot(const Vec3 &v2) const
    {
        return muli(v[0], v2.v[0]) + muli(v[1], v2.v[1]) + muli(v[2], v2.v[2]);
    }

    Vec3 reflect(const Vec3 &n) const
    {
        return sub(n.scale(2 * dot(n)));
    }

    Vec3 clamp(const fixed a, const fixed b) const
    {
        return Vec3(::clamp(v[0], a, b), ::clamp(v[1], a, b), ::clamp(v[2], a, b));
    }

    Vec3 abs() const
    {
        return Vec3(::abs(v[0]), ::abs(v[1]), ::abs(v[2]));
    }

    Vec3 mix(const fixed s, const Vec3 &v) const
    {
        fixed s2 = ::clamp(s, 0, FIXED_ONE);
        return v.scale(s2).add(scale(FIXED_ONE - s2));
    }

    fixed length() const
    {
        fixed d2 = dot(*this);
        return sqrti(d2);
    }

    Vec3 normalize() const
    {
        fixed l2 = dot(*this);
        return scale(rsqrti(l2));
    }
};

Vec3 veci(const fixed x, const fixed y, const fixed z)
{
    return Vec3(x, y, z);
}

Vec3 vec(const float x, const float y, const float z)
{
    return Vec3(fix(x), fix(y), fix(z));
}
