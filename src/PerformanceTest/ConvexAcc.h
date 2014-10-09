#ifndef CONVEX_ACC_H
#define CONVEX_ACC_H

#include "Accelerator.h"

class ConvexAcc : public Accelerator
{
private:
    enum RayDir { Forward, Backward };
    enum IntersectionTableResult { Hit, Partial, Miss };
    enum { CONVEX_TABLE_SIZE = 100 };

    struct EdgeParam // A * x + B * y + C > 0
    {
        float A, B, C;
    };

    struct EdgeRange
    {
        short start;
        short end;
    };

    IntersectionTableResult intersectionTable[CONVEX_TABLE_SIZE][CONVEX_TABLE_SIZE];
    EdgeRange edgeRangeTable[CONVEX_TABLE_SIZE][CONVEX_TABLE_SIZE];
    std::vector<EdgeParam> edgeParams;
    std::vector<int> intersectionTableYAxis[100][360];

private:
    bool intersectWithPolygon(Ray &ray, int index, Point &origin, Vector &dir, float &distance);
    bool intersectWithPolygonAtOrigin(Ray &ray, float &distance);
    bool inPolygon(const Point &p, int begin, int end);
    IntersectionTableResult calcCellStatus(
        const Point &p1, const Point &p2, const Point &p3, const Point &p4, 
        short &minIndex, short &maxIndex);

public:
    ConvexAcc(Tunnel *tunnel) : Accelerator(tunnel) {}
    virtual void init();
    virtual IntersectResult intersect(Ray &ray);
};

#endif
