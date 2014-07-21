#include "Vector.h"
#include <math.h>

Vector::Vector(float x, float y, float z)
{
    this->x = x;
    this->y = y;
    this->z = z;
}

Vector::Vector(const Point &start, const Point &end)
{
    x = end.x - start.x;
    y = end.y - start.y;
    z = end.z - start.z;
}

float Vector::length() const
{ 
    return sqrt(x * x + y * y + z * z); 
}

float Vector::sqrLength() const
{ 
    return x * x + y * y + z * z; 
}

float & Vector::operator[](int index)
{
    if (index == 0)
        return x;
    else if (index == 1)
        return y;
    else
        return z;
}

const float & Vector::operator[](int index) const
{
    if (index == 0)
        return x;
    else if (index == 1)
        return y;
    else
        return z;
}

Vector Vector::operator+(const Vector &b) const 
{
    return Vector(x + b.x, y + b.y, z + b.z); 
}

Vector Vector::operator-(const Vector &b) const 
{ 
    return Vector(x - b.x, y - b.y, z - b.z); 
}

Vector Vector::operator*(float b) const 
{ 
    return Vector(x * b, y * b, z * b); 
}

Vector Vector::mult(const Vector &b) const
{ 
    return Vector(x * b.x, y * b.y, z * b.z); 
}

Vector& Vector::norm() 
{ 
    return *this = *this * (1 / sqrt(x * x + y * y + z * z)); 
}

float Vector::dot(const Vector &b) const 
{
    return x * b.x + y * b.y + z * b.z; 
}

float Vector::dot(const Point &p) const
{
    return x * p.x + y * p.y + z * p.z;
}

Vector Vector::cross(const Vector &b)
{
    return Vector(y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x);
}

float Vector::angleTo(const Vector &b) const
{
    return acos(dot(b) * (1.0f / (length() * b.length())));
}