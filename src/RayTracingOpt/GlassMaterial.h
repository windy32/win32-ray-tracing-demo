#ifndef GLASS_MATERIAL_H
#define GLASS_MATERIAL_H

#include "SolidColorMaterial.h"

class GlassMaterial : public SolidColorMaterial
{
public:
    GlassMaterial(float refractive_index = 1.46);
};

#endif
