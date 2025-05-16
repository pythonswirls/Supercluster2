#pragma once
#include "FixedPoint.h"

class Material
{
public:
    Vec3 color;
    fixed ambient;
    fixed diffuse;
    fixed specular;
    fixed reflection;

    Material ():
        color(Vec3()),
        ambient(0),
        diffuse(0),
        specular(0),
        reflection(0)
    {
    }

    Material (const Vec3 &color, const fixed ambient, const fixed diffuse, const fixed specular, const fixed reflection):
        color(color),
        ambient(ambient),
        diffuse(diffuse),
        specular(specular),
        reflection(reflection)
    {
    }

    virtual Vec3 shade(const Vec3 &p, const Vec3 &n) const
    {
        return color;
    }
};

class Checker: public Material
{
public:
    Vec3 color2;
    Checker(const Vec3 &color1, const Vec3 &color2, const fixed ambient, const fixed diffuse, const fixed specular, const fixed reflection)
        :Material(color1, ambient, diffuse, specular, reflection),
         color2(color2)
    {
    }

    virtual Vec3 shade(const Vec3 &p, const Vec3 &n) const
    {
        int check = ((p.v[0] >> 16) + (p.v[2] >> 16)) & 1;
        return check ? this->color : this->color2;
    }
};

class Sky: public Material
{
public:
    Vec3 color2;
    Sky(const Vec3 &color1, const Vec3 &color2)
        :Material(color1, 0, 0, 0, 0),
         color2(color2)
    {
    }

    virtual Vec3 shade(const Vec3 &p, const Vec3 &n) const
    {
        return color.add(color2.sub(color).scale(n.v[1]));
    }
};
