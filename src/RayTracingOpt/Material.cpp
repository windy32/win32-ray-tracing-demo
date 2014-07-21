#include "Material.h"

Material::Material(float diffusiveness, float reflectiveness, float refractiveness)
{
    this->diffusiveness = diffusiveness;
    this->reflectiveness = reflectiveness;
    this->refractiveness = refractiveness;
}

void Material::setRefractiveIndex(float index)
{
    refractive_index = index;
}

Material::~Material()
{
}

Color Material::emission(const Point &position)
{
    return Color::Black();
}