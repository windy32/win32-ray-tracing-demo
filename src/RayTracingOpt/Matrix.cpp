#include "Matrix.h"

Matrix::Matrix(
    float m11, float m12, float m13, 
    float m21, float m22, float m23,
    float m31, float m32, float m33)
{
    this->m11 = m11;
    this->m12 = m12;
    this->m13 = m13;

    this->m21 = m21;
    this->m22 = m22;
    this->m23 = m23;

    this->m31 = m31;
    this->m32 = m32;
    this->m33 = m33;
}

Point Matrix::operator*(const Point &p) const
{
    Point result;
    result.x = m11 * p.x + m12 * p.y + m13 * p.z;
    result.y = m21 * p.x + m22 * p.y + m23 * p.z;
    result.z = m31 * p.x + m32 * p.y + m33 * p.z;
    return result;
}

Vector Matrix::operator*(const Vector &p) const
{
    Vector result;
    result.x = m11 * p.x + m12 * p.y + m13 * p.z;
    result.y = m21 * p.x + m22 * p.y + m23 * p.z;
    result.z = m31 * p.x + m32 * p.y + m33 * p.z;
    return result;
}
