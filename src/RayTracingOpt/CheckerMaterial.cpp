#include "CheckerMaterial.h"
#include <math.h>

CheckerMaterial::CheckerMaterial(float scale, enum checker_dir_t dir, float reflectiveness)
    : Material(1 - reflectiveness, reflectiveness, 0)
{
    this->scale = scale;
    this->dir = dir;
}

Color CheckerMaterial::local(const Ray &ray, const Point &position, const Vector &normal)
{
    float d;
    if (dir == xoz)
        d = abs((floor(position.x * scale) + floor(position.z * scale)));
    else if (dir == yoz)
        d = abs((floor(position.y * scale) + floor(position.z * scale)));
    else // xoy
        d = abs((floor(position.x * scale) + floor(position.y * scale)));

    d = fmod(d, 2);
    return  (d < 1) ? Color(0.15f, 0.15f, 0.15f) /*Color::Black()*/ : Color::White();
}
