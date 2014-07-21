#ifndef RAY_CONTEXT_H
#define RAY_CONTEXT_H

// Context data associated with the ray object (used in convex accellaration)
struct RayContext
{
    bool inTunnel;
    int segment;

    struct RayContext() : inTunnel(false), segment(-1)
    {
    }
};

#endif
