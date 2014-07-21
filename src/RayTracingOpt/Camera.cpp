#include "Camera.h"
#include <math.h>

PerspectiveCamera::PerspectiveCamera(
    const Point &eye, Vector &front, const Vector &up, float ratio, float fov, float forward)
{
    this->eye = eye;
    this->front = front.norm();
    this->up = up;
    this->ratio = ratio;
    this->fov = fov;
    this->forward = forward;

    this->right = front.cross(up).norm();
    this->up = right.cross(front).norm();
    this->xcenter = ratio * 0.5f;
    this->fovScale = tan(fov* (PI  * 0.5f / 180)) * 2;
}

Ray PerspectiveCamera::generateRay(float x, float y)
{
    Vector r = right *((x - xcenter) * fovScale);
    Vector u = up * ((y - 0.5f) * fovScale);
    Vector dir = (front + r + u).norm();
    return Ray(eye + dir * forward, dir);
}
