#include "SolidColorMaterial.h"

SolidColorMaterial::SolidColorMaterial(const Color &local, const Color &emission, 
    float diffusiveness, float reflectiveness, float refractiveness)
    : Material(diffusiveness, reflectiveness, refractiveness)
{
    this->localColor = local;
    this->emissionColor = emission;
}

Color SolidColorMaterial::local(const Ray &ray, const Point &position, const Vector &normal)
{
    return localColor;
}

Color SolidColorMaterial::emission(const Point &position)
{
    return emissionColor;
}
