#ifndef TUNNEL_GENERATOR_H
#define TUNNEL_GENERATOR_H

#include "Polygon.h"
#include "Polyhedron.h"
#include "Tunnel.h"
#include "GeometrySet.h"

class TunnelGenerator
{
private:
    bool createPolyhedron(
        Polygon &front, Polygon &rear, Polyhedron &polyhedron,
        int &connBC, int &connAD, int &connBoth, int &connInvalid);

public:
    bool create(
        float rectWidth, float rectHeight, float archHeight, // cross section attributes
        float pathRadius, float pathAngle, // path attributes
        int archSegments, int pathSegments, // tessellation attributes
        GeometrySet &scene, 
        Tunnel::Algorithm algorithm);
};

#endif
