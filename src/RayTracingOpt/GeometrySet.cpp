#include "GeometrySet.h"
#include "Triangle.h"
#include <float.h>
#include <stdio.h>

GeometrySet::~GeometrySet()
{
    clear();
}

void GeometrySet::add(Geometry* geometry)
{
    geometries.push_back(geometry);
}

Geometry *GeometrySet::last()
{
    if (geometries.size() == 0)
        return NULL;
    return geometries.back();
}

bool GeometrySet::addStlFile(const char *filename, Ptr<Material> material)
{
    Matrix matrix(
        1, 0, 0, 
        0, 1, 0, 
        0, 0, 1);
    Vector offset(0, 0, 0);
    return addStlFile(filename, material, matrix, offset);
}

bool GeometrySet::addStlFile(const char *filename, Ptr<Material> material, const Matrix &matrix, const Vector &offset)
{
    // Open file
    FILE *fp = NULL;
    if (fopen_s(&fp, filename, "rb") != 0)
        return false;

    char header[80];
    int count;
    float nx, ny, nz; // normal vector
    float x1, y1, z1;
    float x2, y2, z2;
    float x3, y3, z3;
    short attribute;

    fread(header, 80, 1, fp);
    fread(&count, sizeof(int), 1, fp);

    for (int i = 0; i < count; i++)
    {
        fread(&nx, sizeof(float), 1, fp);
        fread(&ny, sizeof(float), 1, fp);
        fread(&nz, sizeof(float), 1, fp);

        fread(&x1, sizeof(float), 1, fp);
        fread(&y1, sizeof(float), 1, fp);
        fread(&z1, sizeof(float), 1, fp);

        fread(&x2, sizeof(float), 1, fp);
        fread(&y2, sizeof(float), 1, fp);
        fread(&z2, sizeof(float), 1, fp);

        fread(&x3, sizeof(float), 1, fp);
        fread(&y3, sizeof(float), 1, fp);
        fread(&z3, sizeof(float), 1, fp);

        fread(&attribute, sizeof(short), 1, fp);

        Point p1 = matrix * Point(x1, y1, z1) + offset;
        Point p2 = matrix * Point(x2, y2, z2) + offset;
        Point p3 = matrix * Point(x3, y3, z3) + offset;
        Vector n = matrix * Vector(nx, ny, nz).norm();

        Triangle *t = new Triangle(p1, p2, p3, n);

        // Each geometry object has a pointer to a material object.
        // To avoid deleting a same material object twice in the destructors of the geometry objects, 
        // they're first cloned here.
        // Smart pointer also works, but it's not used here.
        t->material = material;
        geometries.push_back(t);
    }
    return true;
}

void GeometrySet::clear()
{
    for (unsigned int i = 0; i < geometries.size(); i++)
        delete geometries[i];
    geometries.clear();
}

IntersectResult GeometrySet::intersect(Ray &ray)
{
    float minDistance = FLT_MAX;
    IntersectResult minResult(false);

    for (unsigned int i = 0; i < geometries.size(); i++)
    {
        IntersectResult result = geometries[i]->intersect(ray);
        if (result.hit && (result.distance < minDistance)) 
        {
            minDistance = result.distance;
            minResult = result;
        }
    }
    return minResult;
}
