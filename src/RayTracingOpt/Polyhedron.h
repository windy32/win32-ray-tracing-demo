#ifndef POLYHEDRON_H
#define POLYHEDRON_H

#include <vector>
#include "Polygon.h"

class Polyhedron
{
public:
    enum ConnectionType
    { 
        BC, // P(i+1)P'(i)
        AD, // P(i)P'(i+1)
        Both, 
        Invalid
    };

public:
    Polygon front;
    Polygon rear;

    // true if P(i+1) and Q(i) are connected, 
    // false if P(i) and Q(i+1) are connected
    std::vector<ConnectionType> connections;

    // If the polyhedron is convex
    bool convex;
};

#endif
