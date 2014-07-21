#ifndef POINT_H
#define POINT_H

#include "Vector.h"

class Vector;

class Point
{
public:
    float x;
    float y;
    float z;

public:
    Point(float x = 0, float y = 0, float z = 0);

    Point operator+(const Vector &v) const;
    float & operator[](int index);
    const float & operator[](int index) const;
};

#endif
