#include "Plane.h"

Plane::Plane(const Vector& normal, float dist)
    : normal(normal), dist(dist)
{
    this->position = Point(0, 0, 0) + normal * dist;
    type = GeometryType::PLANE;
}

IntersectResult Plane::intersect(Ray &ray)
{
    IntersectResult result(false);

    const Point &p = position; // p: a point on the plane
    const Point &o = ray.origin; // o: the origin of the ray
    Vector op = Vector(o, p);

    // Make sure that vector n points to the "correct" direction
    bool back = normal.dot(op) > 0; // is the ray from the "back" side of the plane?
    Vector n = back ? normal : normal * -1;

    if (n.dot(ray.direction) > 0)
    {
        float distance = op.dot(n) / ray.direction.dot(n);
        if (distance >= 0.0005f)
        {
            result.hit = true;
            result.geometry = this;
            result.distance = op.dot(n) / ray.direction.dot(n);
            result.position = ray.getPoint(result.distance);
            result.normal = normal;
        }
    }
    return result;
}
