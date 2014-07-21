#include "GlassMaterial.h"

GlassMaterial::GlassMaterial(float refractive_index)
    : SolidColorMaterial(Color::White(), Color::Black(), 0, 0, 1)
{
    setRefractiveIndex(refractive_index);
}
