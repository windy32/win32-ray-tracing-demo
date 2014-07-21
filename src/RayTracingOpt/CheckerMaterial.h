#ifndef CHECKER_MATERIAL_H
#define CHECKER_MATERIAL_H

#include "Material.h"

class CheckerMaterial : public Material
{
public:
    enum checker_dir_t { xoz, xoy, yoz };

private:
    float scale;
    enum checker_dir_t dir;

public:
    CheckerMaterial(float scale, enum checker_dir_t dir = xoz, float reflectiveness = 0);
    virtual Color local(const Ray &ray, const Point &position, const Vector &normal);
};

#endif
