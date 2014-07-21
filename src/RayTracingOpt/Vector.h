#ifndef VECTOR_H
#define VECTOR_H

#include "Point.h"

#include <math.h>

const float PI = 3.14159265359f;

class Point;

class Vector
{
public:
    float x;
    float y;
    float z;

public:
    Vector(float x = 0, float y = 0, float z = 0);
    Vector(const Point &start, const Point &end);

    float length() const;
    float sqrLength() const;
    float & operator[](int index);
    const float & operator[](int index) const;

    Vector operator+(const Vector &b) const;
    Vector operator-(const Vector &b) const;
    Vector operator*(float b) const;
    Vector mult(const Vector &b) const;
    Vector& norm();
    float dot(const Vector &b) const;
    float dot(const Point &p) const;
    Vector cross(const Vector &b);

    float angleTo(const Vector &b) const;
};

#endif
