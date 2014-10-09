#ifndef ACCELERATOR_H
#define ACCELERATOR_H

#include "Tunnel.h"

class Accelerator 
{
protected:
    Tunnel *tunnel;

public:
    Accelerator(Tunnel *tunnel) : tunnel(tunnel) {}
    virtual void init() = 0;
    virtual IntersectResult intersect(Ray &ray) = 0;
};

#endif
