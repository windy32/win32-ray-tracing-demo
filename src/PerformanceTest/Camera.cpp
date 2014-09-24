#include "Camera.h"
#include <math.h>

Camera::Camera(const Point &eye, Vector &front, const Vector &up)
{
    this->eye = eye;
    this->front = front.norm();
    this->up = up;

    this->right = front.cross(up).norm();
    this->up = right.cross(front).norm();
}

Ray Camera::generateRay(float x, float y)
{
    Vector r = right *((x - 0.5f) * 1.274f);
    Vector u = up * ((y - 0.5f) * 1.0f);
    Vector dir = (front + r + u).norm();
    return Ray(eye, dir);
}
