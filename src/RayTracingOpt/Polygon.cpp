#include "Polygon.h"

bool Polygon::IsConvex()
{
    if (vertices.size() < 3)
        return false;

    for (unsigned int i = 0; i < vertices.size(); i++)
    {
        if (vertices[i].z != 0)
            return false; // The polygon must be in the xoy plain
    }

    // The vertices should be counterclockwise, which is not verified here
    for (unsigned int i = 0; i < vertices.size(); i++)
    {
        int j = (i + 1) % vertices.size();

        Vector selected = Vector(vertices[i], vertices[j]);
        Vector zAxis = Vector(0, 0, 1);
        Vector norm = selected.cross(zAxis);

        for (unsigned int k = 0; k < vertices.size(); k++)
        {
            if (k != i && k != j)
            {
                Vector target = Vector(vertices[i], vertices[k]);
                if (norm.dot(target) > 0)
                    return false;
            }
        }
    }
    return true;
}
