#ifndef RADIANCE_CHECKER_MATERIAL_H
#define RADIANCE_CHECKER_MATERIAL_H

#include "Material.h"

class RadianceCheckerMaterial : public Material
{
public:
    enum checker_dir_t { xoz, xoy, yoz };

private:
    float radiance;
    float scale;
    enum checker_dir_t dir;

public:
    RadianceCheckerMaterial(float radiance, float scale, enum checker_dir_t dir = xoz);
    virtual Color local(const Ray &ray, const Point &position, const Vector &normal);
    virtual Color emission(const Point &position);
};

#endif
