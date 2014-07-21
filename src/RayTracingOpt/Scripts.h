#ifndef SCRIPTS_H
#define SCRIPTS_H

#include "Camera.h"
#include "GeometrySet.h"
#include "RenderSetting.h"

typedef void (*LogCallback)(const char *str);
typedef void (*ProgressCallback)(int cur, int total);

typedef int (*RenderProc)(GeometrySet &scene, PerspectiveCamera &camera, RenderSetting &setting, 
                          ProgressCallback progress);

class Script
{
public:
    enum 
    { 
        FLAG_TUNNEL = 0x1,
        FLAG_MONTE_CARLO = 0x2
    };

public:
    const char *name;
    int flags;
    int tunnelSegments; // valid when FLAG_TUNNEL bit is 1
    int samples;        // valid when FLAG_MONTE_CARLO bit is 1

public:
    Script (const char *name, int flags, int tunnelSegments, int samples) 
    {
        this->name = name;
        this->flags = flags;
        this->tunnelSegments = tunnelSegments;
        this->samples = samples;
    }

    virtual void Run(RenderProc render, int tunnelAlgorithm, LogCallback log, ProgressCallback progress,
        int &prepareTime, int &execTime) = 0;
};

extern Script *scripts[5];

// To create a new script, derive the Script class,
// implement the constructor and Run(), then add an instance of the new script
// to the array "scripts[]" in Script.cpp
class Script1 : public Script
{ 
public:
    Script1() : Script("checker ground and balls", FLAG_MONTE_CARLO, 0, 1000) {}
    virtual void Run(RenderProc render, int tunnelAlgorithm, LogCallback log, ProgressCallback progress,
        int &prepareTime, int &execTime); 
};

class Script2 : public Script
{ 
public:
    Script2() : Script("smallpt", FLAG_MONTE_CARLO, 0, 1000) {}
    virtual void Run(RenderProc render, int tunnelAlgorithm, LogCallback log, ProgressCallback progress,
        int &prepareTime, int &execTime); 
};

class Script3 : public Script
{ 
public:
    Script3() : Script("smallpt with stl balls", FLAG_MONTE_CARLO, 300, 0) {}
    virtual void Run(RenderProc render, int tunnelAlgorithm, LogCallback log, ProgressCallback progress,
        int &prepareTime, int &execTime); 
};

class Script4 : public Script
{ 
public:
    Script4() : Script("tunnel (short and wide)", FLAG_TUNNEL, 300, 0) {}
    virtual void Run(RenderProc render, int tunnelAlgorithm, LogCallback log, ProgressCallback progress,
        int &prepareTime, int &execTime); 
};

class Script5 : public Script
{ 
public:
    Script5() : Script("tunnel (long and narrow)", FLAG_TUNNEL, 300, 0) {}
    virtual void Run(RenderProc render, int tunnelAlgorithm, LogCallback log, ProgressCallback progress,
        int &prepareTime, int &execTime); 
};

#endif
