#ifndef RENDER_SETTING_H
#define RENDER_SETTING_H

#include <limits.h>

struct RenderSetting
{
    // Enable Monte Carlo path tracing.
    //  - When it's disabled, it's necessary to set the max depth,
    //    but termination depth and single tracing depth are not used.
    //  - When it's enabled, all of the three parameters below works.
    // Note: When Monte Carlo path tracing is disabled, a diffusive material
    //       does not reflect light. For simplicity it is recommended to let them emit light.
    bool enableMonteCarlo;

    // Terminate path tracing when the depth is above this value.
    // Set this value to INT_MAX to disable the hard limit of depth.
    int maxDepth;

    // When depth is above this value, terminate path tracing randomly.
    // The probability of termination is related to the local color of the material,
    // darker colors lead to a higher probability of termination.
    // Set this value to INT_MAX to disable random termination.
    // Set this value to 0 to always enable random termination.
    int terminationDepth;

    // In Monte Carlo path tracing, a refractive material can either:
    //  - Generate two rays (one for refraction, and the other for reflection) 
    //    with different weights, or
    //  - Generate one ray (refracted ray or reflected ray) 
    //    with different possibilities
    // The first strategy produces a better image but is slower, 
    // and the second produces poorer images but works faster.
    // However, it is possible to combine these two strategies: generate two rays when
    // the depth is under a specific value, and generate one ray when the depth is higher.
    // Set this value to INT_MAX to always generate two rays.
    // Set this value to 0 to always generate one ray.
    int singleTracingDepth;

    static RenderSetting HighSpeed()
    {
        RenderSetting setting;
        setting.enableMonteCarlo = true;
        setting.maxDepth = 6;
        setting.terminationDepth = 2;
        setting.singleTracingDepth = 0;
        return setting;
    }

    static RenderSetting HighQuality()
    {
        RenderSetting setting;
        setting.enableMonteCarlo = true;
        setting.maxDepth = 8;
        setting.terminationDepth = INT_MAX;
        setting.singleTracingDepth = INT_MAX;
        return setting;
    }

    static RenderSetting Default()
    {
        RenderSetting setting;
        setting.enableMonteCarlo = true;
        setting.maxDepth = INT_MAX;
        setting.terminationDepth = 5;
        setting.singleTracingDepth = 2;
        return setting;
    }

    static RenderSetting Simple()
    {
        RenderSetting setting;
        setting.enableMonteCarlo = false;
        setting.maxDepth = 20;
        setting.terminationDepth = INT_MAX;
        setting.singleTracingDepth = 0;
        return setting;
    }
};

#endif
