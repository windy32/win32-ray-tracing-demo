#include "RadianceCheckerMaterial.h"
#include <math.h>

RadianceCheckerMaterial::RadianceCheckerMaterial(float radiance, float scale, enum checker_dir_t dir)
    : Material(1, 0, 0)
{
    this->radiance = radiance;
    this->scale = scale;
    this->dir = dir;
}

Color RadianceCheckerMaterial::local(const Ray &ray, const Point &position, const Vector &normal)
{
    return Color(0.15f, 0.15f, 0.15f); // Dark gray
}

Color RadianceCheckerMaterial::emission(const Point &position)
{
    float d;
    if (dir == xoz)
        d = abs((floor(position.x * scale) + floor(position.z * scale)));
    else if (dir == yoz)
        d = abs((floor(position.y * scale) + floor(position.z * scale)));
    else // xoy
        d = abs((floor(position.x * scale) + floor(position.y * scale)));

    d = fmod(d, 2);
    return  (d < 1) ? Color(radiance, radiance, radiance) : Color::Black();
}
