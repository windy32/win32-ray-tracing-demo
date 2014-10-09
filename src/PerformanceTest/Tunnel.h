#ifndef TUNNEL_H
#define TUNNEL_H

#include <vector>
#include "Polygon.h"
#include "Triangle.h"

class GridAcc;
class KdTreeAcc;
class ConvexAcc;

class Tunnel : public Geometry
{
public:
    Polygon crossSection; // the cross section at the origin
    std::vector<Point> path;
    std::vector<std::vector<Triangle *>> surface;
    std::vector<Vector> nvs; // normal vectors of the polygons

    // Size of the bounding rectangle of the tunnel's cross section
    float height;
    float width;

    // The algorithm used in tunnel-ray intersection
    enum Algorithm 
    { 
        Linear = 0,
        RegularGrid = 1, FlatGrid = 2, 
        KdTreeStandard = 3, KdTreeSAH = 4, 
        Convex = 5, ConvexSimple = 6 // the same order with the combobox items
    } algorithm;

private:
    GridAcc *accGrid;
    KdTreeAcc *accKdTree;
    ConvexAcc *accConvex;

public:
    Tunnel();
    ~Tunnel();
    void init();
    IntersectResult linearIntersect(Ray &ray);
    virtual IntersectResult intersect(Ray &ray);
};

#endif
