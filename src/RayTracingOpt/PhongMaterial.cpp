#include "PhongMaterial.h"
#include <math.h>

PhongMaterial::PhongMaterial(
    const Color &diffuse, const Color &specular, float shininess, float reflectiveness)
    : Material(1 - reflectiveness, reflectiveness, 0)
{
    this->diffuse = diffuse;
    this->specular = specular;
    this->shininess = shininess;
}

Color PhongMaterial::local(const Ray &ray, const Point &position, const Vector &normal)
{
    // Add some light
    static Vector lightDir = Vector(-1, 1, 1).norm();
    static Color lightColor = Color::White();

    float NdotL = normal.dot(lightDir);
    NdotL = (NdotL < 0.0f) ? 0.0f : NdotL;

    Vector H = (lightDir - ray.direction).norm();
    float NdotH = normal.dot(H);
    NdotH = (NdotH < 0.0f) ? 0.0f : NdotH;

    Color diffuseTerm = diffuse * NdotL;
    Color specularTerm = specular * pow(NdotH, shininess);
    return lightColor.mult(diffuseTerm + specularTerm);
}
