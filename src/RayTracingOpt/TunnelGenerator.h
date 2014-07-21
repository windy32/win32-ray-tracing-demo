#ifndef TUNNEL_GENERATOR_H
#define TUNNEL_GENERATOR_H

#include "GeometrySet.h"
#include "Polygon.h"
#include "Polyhedron.h"
#include "Tunnel.h"

class TunnelGenerator
{
private:
    bool createPolyhedron(
        Polygon &front, Polygon &rear, Polyhedron &polyhedron,
        int &connBC, int &connAD, int &connBoth, int &connInvalid);

    /*
private:
    static std::vector<Triangle> triangleList;
    static Triangle *newTriangle(const Point &a, const Point &b, const Point &c);

public:
    static void clear();
    */
public:
    bool create(
        float rectWidth, float rectHeight, float archHeight, // cross section attributes
        float pathRadius, float pathAngle, // path attributes
        int archSegments, int pathSegments, // tessellation attributes
        GeometrySet &scene, Ptr<Material> groundMaterial, Ptr<Material> wallMaterial, 
        Tunnel::Algorithm algorithm);
};

#endif
