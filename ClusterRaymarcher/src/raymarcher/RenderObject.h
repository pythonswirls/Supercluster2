#pragma once
#include "FixedPoint.h"
#include "Material.h"

class RenderObject
{
public:
    Vec3 pos;
    Material const *mat;

    RenderObject(const Vec3 &pos, Material const *mat):
        pos(pos),
        mat(mat)
    {
    }

    virtual fixed sdf(const Vec3 &p) const
    {
        return FIXED_MAX;
    }
};

class Sphere: public RenderObject
{
public:
    fixed r;
    Sphere(const Vec3 &pos, const fixed r, Material const *mat):
        RenderObject(pos, mat),
        r(r)
    {
    }

    virtual fixed sdf(const Vec3 &p) const
    {
        return p.sub(pos).length() - r;
    }
};

class Box: public RenderObject
{
public:
    Vec3 dim;
    Box(const Vec3 &pos, const Vec3 &dim, Material const *mat):
        RenderObject(pos, mat),
        dim(dim)
    {
    }

    virtual fixed sdf(const Vec3 &p) const
    {
        Vec3 d = p.sub(pos).abs().sub(dim);
        fixed dist = max(d.v[0], max(d.v[1], d.v[2]));
        return dist;
    }
};

class Cylinder: public RenderObject
{
public:
    Vec3 dim;
    Cylinder(const Vec3 &pos, const Vec3 &dim, Material const *mat):
        RenderObject(pos, mat),
        dim(dim)
    {
    }

    virtual fixed sdf(const Vec3 &p) const
    {
        Vec3 rel = p.sub(pos);
        Vec3 rel2 = Vec3(rel.v[0], 0, rel.v[2]);
        fixed dxz = rel2.length() - dim.v[0];
        fixed dy = max(rel.v[1] - dim.v[1], -rel.v[1] - dim.v[1]);
        return max(dxz, dy);
    }
};

class PlaneX: public RenderObject
{
public:
    PlaneX(const Vec3 &pos, Material const *mat):
        RenderObject(pos, mat)
    {
    }

    virtual fixed sdf(const Vec3 &p) const
    {
        return p.v[0] - pos.v[0];
    }
};

class PlaneY: public RenderObject
{
public:
    PlaneY(const Vec3 &pos, Material const *mat):
        RenderObject(pos, mat)
    {
    }

    virtual fixed sdf(const Vec3 &p) const
    {
        return p.v[1] - pos.v[1];
    }
};

class PlaneZ: public RenderObject
{
public:
    PlaneZ(const Vec3 &pos, Material const *mat):
        RenderObject(pos, mat)
    {
    }

    virtual fixed sdf(const Vec3 &p) const
    {
        return p.v[2] - pos.v[2];
    }
};
