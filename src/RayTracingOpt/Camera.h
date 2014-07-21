#ifndef CAMERA_H
#define CAMERA_H

#include "Vector.h"
#include "Ray.h"

class PerspectiveCamera
{
private:
    Point eye;
    Vector front;
    Vector up;
    Vector right;
    float  ratio;
    float  xcenter;
    float  fov;
    float  fovScale;
    float  forward;

public:
    PerspectiveCamera(const Point &eye, Vector &front, const Vector &up, float ratio, float fov, float forward = 0.0f);
    Ray generateRay(float x, float y);
};

#endif
