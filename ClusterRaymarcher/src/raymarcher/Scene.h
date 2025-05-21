#pragma once
#include "FixedPoint.h"
#include "Material.h"
#include "RenderObject.h"

class Scene
{
public:
    Vec3 cameraPos;
    Vec3 lightPos;
    Material *skyMat;
    fixed fog;
    int objectCount;
    RenderObject **objects;

    Scene()
    {
        this->cameraPos = vec(0.3f, 1.0f, -6.0f);
        this->lightPos = vec(0.0f, 5.0f, -2.0f);
        this->skyMat = new Sky(vec(0.4f, 0.4f, 0.7f), vec(0.8f, 0.8f, 1.0f));
        this->fog = fix(0.05f);
        objects = new RenderObject*[4];
        objects[0] = new Sphere(vec(0.5f, 0.7f, 1.0f), fix(1.5f), new Material(vec(1.0f, 0.5f, 0.7f), fix(0.2f), fix(0.8f), fix(0.8f), fix(0.2f)));
        objects[1] = new Box(vec(2.5f, 0.0f, -1.5f), vec(1.0f, 1.0f, 1.0f), new Material(vec(0.4f, 0.4f, 1.0f), fix(0.2f), fix(0.8f), fix(0.2f), fix(0.2f)));
        objects[2] = new Cylinder(vec(-3.0f, 1.0f, -1.0f), vec(1.0f, 2.0f, 0.0f), new Material(vec(0.3f, 0.7f, 0.3f), fix(0.2f), fix(0.8f), fix(0.2f), fix(0.2f)));
        objects[3] = new PlaneY(vec(0.0f, -1.0f, 0.0f), new Checker(vec(0.6f, 0.1f, 0.1f), vec(1.0f, 1.0f, 1.0f), fix(0.2f), fix(0.8f), fix(0.1f), fix(0.2f)));
        objectCount = 4;
    }
};


