#ifndef CAMERA_H
#define CAMERA_H

#include "Vector.h"
#include "Ray.h"

class Camera
{
private:
    Point eye;
    Vector front;
    Vector up;
    Vector right;

public:
    Camera(const Point &eye, Vector &front, const Vector &up);
    Ray generateRay(float x, float y);
};

#endif
