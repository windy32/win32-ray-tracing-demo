#include "Grid.h"
#include <float.h>

const float Grid::epsilon = 0.0001f;


Grid::Grid(const Point &pos, const Vector &size)
{
    this->pos = pos;
    this->size = size;
}

Grid::Grid(const Point &near, const Point &far)
{
    this->pos = near;
    this->size = Vector(near, far);
}

bool Grid::contains(const Point &p)
{
    Point min = pos;
    Point max = pos + size;

    return
        (p.x > min.x - epsilon) && (p.x < max.x + epsilon) &&
        (p.y > min.y - epsilon) && (p.y < max.y + epsilon) &&
        (p.z > min.z - epsilon) && (p.z < max.z + epsilon);
}

void updateEntryExit(float &entry, float &exit, float newValue)
{
    // entry   exit
    // ----------------
    // MAX     MAX     <-- State 1
    // v1      MAX     <-- State 2
    // v1      v2      <-- State 3 (new value is greater)
    // v2      v1      <-- State 3 (new value is smaller)

    if (entry == FLT_MAX && exit == FLT_MAX) // State 1
    {
        entry = newValue;
    }
    else if (exit == FLT_MAX) // State 2
    {
        if (newValue > entry)
        {
            exit = newValue;
        }
        else
        {
            exit = entry;
            entry = newValue;
        }
    }
    else // State 3
    {
        if (newValue < entry)
        {
            exit = entry;
            entry = newValue;
        }
        else if (newValue < exit)
        {
            exit = newValue;
        }
    }
}

// box / ray intersection
bool Grid::intersect(const Ray &ray, float &entry, float &exit)
{
    Point near = pos;
    Point far = pos + size;

    // Case 1. Plane x = near.x
    // Case 2. Plane x = far.x
    // Case 3. Plane y = near.y
    // Case 4. Plane y = far.y
    // Case 5. Plane z = near.z
    // Case 6. Plane z = far.z
    entry = FLT_MAX;
    exit = FLT_MAX;

    for (int axis = 0; axis < 3; axis++)
    {
        int nextAxis = (axis + 1) % 3;
        int nextNextAxis = (axis + 2) % 3;

        if (fabs(ray.direction[axis]) > 1e-10)
        {
            // near
            float distance = (near[axis] - ray.origin[axis]) / ray.direction[axis];
            Point p = ray.getPoint(distance);
            if (p[nextAxis] >= near[nextAxis] && 
                p[nextAxis] <= far[nextAxis] &&
                p[nextNextAxis] >= near[nextNextAxis] && 
                p[nextNextAxis] <= far[nextNextAxis])
            {
                updateEntryExit(entry, exit, distance);
            }

            // far
            distance = (far[axis] - ray.origin[axis]) / ray.direction[axis];
            p = ray.getPoint(distance);
            if (p[nextAxis] >= near[nextAxis] && 
                p[nextAxis] <= far[nextAxis] &&
                p[nextNextAxis] >= near[nextNextAxis] && 
                p[nextNextAxis] <= far[nextNextAxis] )
            {
                updateEntryExit(entry, exit, distance);
            }
        }
    }

    return entry != FLT_MAX;
}
