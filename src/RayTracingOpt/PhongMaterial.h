#ifndef PHONG_MATERIAL_H
#define PHONG_MATERIAL_H

#include "Material.h"

class PhongMaterial : public Material
{
private:
    Color diffuse;
    Color specular;
    float shininess;

public:
    PhongMaterial(
        const Color &diffuse, const Color &specular, float shininess, float reflectiveness = 0);
    virtual Color local(const Ray &ray, const Point &position, const Vector &normal);
};

#endif
