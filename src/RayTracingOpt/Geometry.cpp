#include "Geometry.h"

#define NULL 0

Geometry::Geometry() : material(NULL)
{
}

Geometry::~Geometry()
{
}

void Geometry::setMaterial(const Ptr<Material> &materail)
{
    this->material = material;
}
