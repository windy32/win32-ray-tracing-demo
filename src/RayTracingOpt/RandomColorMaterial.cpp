#include "RandomColorMaterial.h"
#include <stdlib.h> // we don't need a high quality random number generator

RandomColorMaterial::RandomColorMaterial(bool randomLocal, bool randomEmission) : Material(1, 0, 0)
{
    this->randomLocal = randomLocal;
    this->randomEmission = randomEmission;

    if (randomLocal)
    {
        float r = rand() / float(RAND_MAX);
        float g = rand() / float(RAND_MAX);
        float b = rand() / float(RAND_MAX);
        localColor = Color(r, g, b);
    }
    else
        localColor = Color::Black();

    if (randomEmission)
    {
        float r = rand() / float(RAND_MAX);
        float g = rand() / float(RAND_MAX);
        float b = rand() / float(RAND_MAX);
        emissionColor = Color(r, g, b);
    }
    else
        emissionColor = Color::Black();
}

Color RandomColorMaterial::local(const Ray &ray, const Point &position, const Vector &normal)
{
    return localColor;
}

Color RandomColorMaterial::emission(const Point &position)
{
    return emissionColor;
}
