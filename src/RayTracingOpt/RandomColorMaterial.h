#ifndef RANDOM_COLOR_MATERIAL_H
#define RANDOM_COLOR_MATERIAL_H

#include "Material.h"

// This class is for debugging only.
class RandomColorMaterial : public Material
{
private:
    bool randomLocal;
    bool randomEmission;
    Color localColor;
    Color emissionColor;

public:
    RandomColorMaterial(bool randomLocal, bool randomEmission);
    virtual Color local(const Ray &ray, const Point &position, const Vector &normal);
    virtual Color emission(const Point &position);
};

#endif
