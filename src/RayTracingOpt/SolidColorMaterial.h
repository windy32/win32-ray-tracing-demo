#ifndef SOLID_COLOR_MATERIAL_H
#define SOLID_COLOR_MATERIAL_H

#include "Material.h"

class SolidColorMaterial : public Material
{
private:
    Color localColor;
    Color emissionColor;

public:
    SolidColorMaterial(const Color &local, const Color &emission, 
        float diffusiveness, float reflectiveness, float refractiveness);
    virtual Color local(const Ray &ray, const Point &position, const Vector &normal);
    virtual Color emission(const Point &position);
};

#endif
