#ifndef MATERIAL_H
#define MATERIAL_H

#include "Color.h"
#include "Ray.h"

class Material
{
public:
    // For any material, a + b + c = 1.0
    // ----------------------
    // material      a  b  c
    // ----------------------
    // rough stone: (1, 0, 0)
    // glass:       (0, 1, 0)
    // ideal mirror (0, 0, 1)
    // ----------------------
    float diffusiveness; // a
    float reflectiveness; // b
    float refractiveness; // c

    // For a material that refracts light, the refractive index should also be specified
    // e.g.: n(vacumm) = 1.0, n(soda-lime glass) = 1.46
    float refractive_index;

public:
    Material(float diffusiveness, float reflectiveness, float refractiveness);
    virtual ~Material();

    void setRefractiveIndex(float index);

    // Get the local color of the material at a certain position
    // Note: the parameters "ray" and "normal" are necessary for the PhongMaterial
    virtual Color local(const Ray &ray, const Point &position, const Vector &normal) = 0;

    // Get the emission color of the material at a certain position
    virtual Color emission(const Point &position);
};

#endif
