#include "Point.h"

Point::Point(float x, float y, float z)
{
    this->x = x;
    this->y = y;
    this->z = z;
}

/*
Point& Point::operator=(const Point &p)
{
    this->x = p.x;
    this->y = p.y;
    this->z = p.z;
    return *this;
}
*/

Point Point::operator+(const Vector &v) const
{
    return Point(x + v.x, y + v.y, z + v.z);
}

float & Point::operator[](int index)
{
    if (index == 0)
        return x;
    else if (index == 1)
        return y;
    else
        return z;
}

const float & Point::operator[](int index) const
{
    if (index == 0)
        return x;
    else if (index == 1)
        return y;
    else
        return z;
}

/*
Point Point::add(const Vector &v) const
{
    return Point(x + v.x, y + v.y, z + v.z);
}
*/
/*
Point Point::Origin()
{
    return Point(0, 0, 0);
}
*/