#ifndef MATRIX_H
#define MATRIX_H

#include "Point.h"

class Matrix
{
public:
    float m11, m12, m13;
    float m21, m22, m23;
    float m31, m32, m33;

public:
    // Matrix(float m[3][3]);
    Matrix(
        float m11, float m12, float m13, 
        float m21, float m22, float m23,
        float m31, float m32, float m33);
    Point operator*(const Point &p) const;
    Vector operator*(const Vector &p) const;
};

#endif
