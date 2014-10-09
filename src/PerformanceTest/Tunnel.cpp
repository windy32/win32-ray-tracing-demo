#include "Tunnel.h"
#include "Utils.h"

#include "GridAcc.h"
#include "KdTreeAcc.h"
#include "ConvexAcc.h"

Tunnel::Tunnel()
{
    accConvex = NULL;
    accGrid = NULL;
    accKdTree = NULL;

    type = GeometryType::TUNNEL;
}

Tunnel::~Tunnel()
{
    // Delete accelerators
    if (algorithm == RegularGrid || algorithm == FlatGrid)
    {
        delete accGrid;
    }
    else if (algorithm == KdTreeSAH || algorithm == KdTreeStandard)
    {
        delete accKdTree;
    }
    else if (algorithm == Convex || algorithm == ConvexSimple)
    {
        delete accConvex;
    }

    // Delete triangles
    for (unsigned int i = 0; i < surface.size(); i++)
    {
        for (unsigned int j = 0; j < surface[i].size(); j++)
        {
            delete surface[i][j];
        }
    }
}

void Tunnel::init()
{
    if (algorithm == RegularGrid || algorithm == FlatGrid)
    {
        accGrid = new GridAcc(this);
        accGrid->init();
    }
    else if (algorithm == KdTreeSAH || algorithm == KdTreeStandard)
    {
        accKdTree = new KdTreeAcc(this);
        accKdTree->init();
    }
    else if (algorithm == Convex || algorithm == ConvexSimple)
    {
        accConvex = new ConvexAcc(this);
        accConvex->init();
    }
    // else: nothing to do

    Utils::PrintTickCount("Initialization Finished");
}

IntersectResult Tunnel::linearIntersect(Ray &ray)
{
    float minDistance = FLT_MAX;
    IntersectResult minResult(false);

    for (unsigned int segment = 0; segment < surface.size(); segment++)
    {
        for (unsigned int i = 0; i < surface[segment].size(); i++)
        {
            IntersectResult result = surface[segment][i]->intersect(ray);
            if (result.hit && (result.distance < minDistance)) 
            {
                minDistance = result.distance;
                minResult = result;
            }
        }
    }
    return minResult;
}

IntersectResult Tunnel::intersect(Ray &ray)
{
    if (algorithm == RegularGrid || algorithm == FlatGrid)
        return accGrid->intersect(ray);
    else if (algorithm == KdTreeSAH || algorithm == KdTreeStandard)
        return accKdTree->intersect(ray);
    else if (algorithm == Convex || algorithm == ConvexSimple)
        return accConvex->intersect(ray);
    else
        return linearIntersect(ray);
}
