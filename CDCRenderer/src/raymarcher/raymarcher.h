#pragma once
#include "FixedPoint.h"
#include "Scene.h"

const fixed E = fixed(0.01f * 65536);
const fixed E2 = fixed(0.02f * 65536);
const fixed FAR = fixed(100 * 65536);
const Vec3 zeroVec(0, 0, 0);

const Scene *scene = 0;

void loadScene(const uint8_t data = 0)
{
	scene = new Scene();
}

class Collision
{
public:
    fixed d;
    Vec3 p;
    Vec3 n;
    RenderObject *obj;
    Collision():
            d(FIXED_MAX),
            obj(0)
    {
    }

    Collision(const fixed d, const Vec3 &p, const Vec3 &n, RenderObject *obj):
        d(d),
        p(p),
        n(n),
        obj(obj)
    {
    }
};

Collision sdf(const Scene &scene, const Vec3 &p)
{
    fixed minD = FAR;
    RenderObject *minDobj = 0;
    for(int i = 0; i < scene.objectCount; i++)
    {
        fixed d = scene.objects[i]->sdf(p);
        if(d < minD)
        {
            minD = d;
            minDobj = scene.objects[i];
        }
    }
    if(minDobj)
        return Collision(minD, p, veci(0, 0, 0), minDobj);
    return Collision();
}

Vec3 getNormal(const Collision &coll)
{
    fixed dx = coll.obj->sdf(coll.p.add(veci(E, 0, 0))) - coll.obj->sdf(coll.p.sub(veci(E, 0, 0)));
    fixed dy = coll.obj->sdf(coll.p.add(veci(0, E, 0))) - coll.obj->sdf(coll.p.sub(veci(0, E, 0)));
    fixed dz = coll.obj->sdf(coll.p.add(veci(0, 0, E))) - coll.obj->sdf(coll.p.sub(veci(0, 0, E)));
    return veci(dx, dy, dz).normalize();
}

Vec3 getLight(const Vec3 &eyeDir, const Vec3 &normal, const Vec3 &position, const Vec3 &lightPos, const Vec3 &baseColor, Material const *mat)
{
    Vec3 lightDir = lightPos.sub(position).normalize();
    fixed diff = max(normal.dot(lightDir), 0);
    Vec3 halfVec = lightDir.sub(eyeDir).normalize();
    fixed spec = powi16(max(halfVec.dot(normal), 0));
    Vec3 lightColor = vec(1, 1, 1);
    return baseColor.scale(mat->ambient).
        add(baseColor.scale(muli(diff, mat->diffuse))).
        add(lightColor.scale(muli(spec, mat->specular)));
}

bool isInShadow(const Scene &scene, const Vec3 &p, const Vec3 &lightPos)
{
    Vec3 rl = p.sub(lightPos);
    Vec3 dir = rl.normalize();
    fixed dist = rl.length();
    fixed t = 0;
    for (int i = 0; i < 50; i++) {
        Vec3 np = lightPos.add(dir.scale(t));
        Collision c = sdf(scene, np);
        if(!c.obj) return false;
        if (c.d < E)
        {
            return t < dist - (E2+E2+E);
        }
        t = t + c.d;
    }
    return false;
}

Vec3 getReflection(const Scene &scene, const Vec3 &origin, const Vec3 &dir, int depth)
{
    fixed t = 0;
    for (int i = 0; i < 64; i++) {
        Vec3 p = origin.add(dir.scale(t));
        Collision coll = sdf(scene, p);
        if(!coll.obj) return zeroVec;
        if (coll.d < E2) {
            coll.n = getNormal(coll);
            Vec3 color = coll.obj->mat->shade(coll.p, coll.n);
            bool inShadow = isInShadow(scene, p, scene.lightPos);
            Vec3 shaded = getLight(dir, coll.n, coll.p, scene.lightPos, color, coll.obj->mat);
            if (inShadow) shaded = color.scale(coll.obj->mat->ambient);
            if (depth > 0 && coll.obj->mat->reflection > fix(0.0))
            {
                Vec3 refl = getReflection(scene, p.add(coll.n.scale(fix(0.1))), dir.reflect(coll.n), depth - 1);
                shaded = shaded.mix(coll.obj->mat->reflection, refl);
            }
            return shaded.mix(muli(t, scene.fog), scene.skyMat->color);
        }
        t += coll.d;
        if (t > FAR) break;
    }
    return scene.skyMat->shade(origin, dir);
}

Vec3 renderPixel(const Vec3 &pos, const Vec3 &dir, int depth)
{
    return getReflection(*scene, pos, dir, depth);
}
