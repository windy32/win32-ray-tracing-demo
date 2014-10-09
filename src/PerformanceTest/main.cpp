#include "TunnelGenerator.h"
#include "Plane.h"
#include "Camera.h"
#include "Utils.h"

// the scene
GeometrySet scene;
Tunnel::Algorithm algorithm;

// (fixed) cross section attributes
const float RECT_WIDTH = 50;
const float RECT_HEIGHT = 25;
const float ARCH_HEIGHT = 25;

// (default) path attributes
float PATH_RADIUS = 1000;
float PATH_ANGLE = PI * 0.5f;

// (default) tessellation attributes
int ARCH_SEG = 150;
int PATH_SEG = 150;

// (default) number of rays
int N = 1000;

// (fixed) max tracing depth
const int MAX_DEPTH = 200;

IntersectResult trace(GeometrySet &scene, Ray &r, int depth)
{
    IntersectResult result = scene.intersect(r);
    if (!result.hit)
    {
		return result;
    }

    Geometry *obj = result.geometry;
    Point &p = result.position;
    Vector &n = result.normal; // points to the outside
    Vector nl = (n.dot(r.direction) < 0) ? n : n * -1; // points to the ray

    if (++depth > MAX_DEPTH) // add a hard limit and avoid stack overflow
	{
		result.hit = false;
		return result;
	}
    
    // The ray hits the plane at the exit of the tunnel
    if (result.geometry->type == GeometryType::PLANE)
    {
        return result;
    }

    // reflect on the tunnel wall
    Vector v = r.direction - nl * 2 * nl.dot(r.direction);
    Ray newRay(p, v);
    newRay.context = r.context;
    return trace(scene, newRay, depth);
}

Tunnel *init_scene()
{
    // create tunnel
    TunnelGenerator g;
    Tunnel *tunnel;
    g.create(RECT_WIDTH, RECT_HEIGHT, ARCH_HEIGHT, 
        PATH_RADIUS, PATH_ANGLE, ARCH_SEG, PATH_SEG, scene, algorithm);
    tunnel = (Tunnel *)scene.last();

    // place a plane at the exit of the tunnel
    Point exit(
        PATH_RADIUS * (1 - cos(PATH_ANGLE)),
        0,
        -PATH_RADIUS * sin(PATH_ANGLE));
    Vector normal(sin(PATH_ANGLE), 0, -cos(PATH_ANGLE));
    float distance = PATH_RADIUS * sin(PATH_ANGLE);
    Plane *plane = new Plane(normal, distance);
    scene.add(plane);

    return tunnel;
}

void parse_params(int argc, char *argv[])
{
    if (argc != 7)
    {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "   - PerformaceTest PathRadius PathAngle ArchSeg PathSeg N Algorithm\n");
        fprintf(stderr, "Algorithms:\n");
        fprintf(stderr, "   - linear (Linear)\n");
        fprintf(stderr, "   - rgrid (Regular Grid)\n");
        fprintf(stderr, "   - fgrid (Flat Grid)\n");
        fprintf(stderr, "   - kdtree (K-d Tree)\n");
        fprintf(stderr, "   - sah (K-d Tree (SAH))\n");
        fprintf(stderr, "   - convex (Convex)\n");
        fprintf(stderr, "   - convex_s (Convex Simple)\n");
        fprintf(stderr, "Example:\n");
        fprintf(stderr, "   - PerformaceTest 1000 1.5708 150 150 1000 convex");

        exit(-1);
    }
    else
    {
        sscanf_s(argv[1], "%f", &PATH_RADIUS);
        sscanf_s(argv[2], "%f", &PATH_ANGLE);
        sscanf_s(argv[3], "%d", &ARCH_SEG);
        sscanf_s(argv[4], "%d", &PATH_SEG);
        sscanf_s(argv[5], "%d", &N);

        if (strcmp(argv[6], "rgrid") == 0)
            algorithm = Tunnel::RegularGrid;
        else if (strcmp(argv[6], "fgrid") == 0)
            algorithm = Tunnel::FlatGrid;
        else if (strcmp(argv[6], "kdtree") == 0)
            algorithm = Tunnel::KdTreeStandard;
        else if (strcmp(argv[6], "sah") == 0)
            algorithm = Tunnel::KdTreeSAH;
        else if (strcmp(argv[6], "convex") == 0)
            algorithm = Tunnel::Convex;
        else if (strcmp(argv[6], "convex_s") == 0)
            algorithm = Tunnel::ConvexSimple;
        else
            algorithm = Tunnel::Linear;
    }
}

int main(int argc, char *argv[])
{
    // parse commandline
    parse_params(argc, argv);

    // create scene
    int t0 = Utils::GetTickCount();
    Tunnel *tunnel = init_scene();

    // preprocess
    int s0 = Utils::GetMemorySize();
    int t1 = Utils::GetTickCount();
    tunnel->init();
    int t2 = Utils::GetTickCount();
    int s1 = Utils::GetMemorySize();

    // Create camera
    Camera camera(
        Point(0, 25, 5),  // eye
        Vector(0, 0, -1), // front
        Vector(0, 1, 0)); // up

    // ray tracing
    int t3 = Utils::GetTickCount();
    for (int i = 0; i < N; i++)
    {
        float dx = rand() / (float)RAND_MAX;
        float dy = rand() / (float)RAND_MAX;
        Ray ray = camera.generateRay(dx, dy);
        IntersectResult result = trace(scene, ray, 0);

        if (!result.hit)
        {
            fprintf(stderr, "Warning: ray %d / %d missed\n", i + 1, N);
        }
    }
    int t4 = Utils::GetTickCount();

    // output
    printf("%.0f\t%d\t%d\t%.1lf\t%.1lf\t%.1lf\t%.2lf MB\n", PATH_RADIUS, ARCH_SEG, PATH_SEG,
        (double)(t1 - t0), (double)(t2 - t1), (double)(t4 - t3), (double)(s1 - s0) / (1024 * 1024));

    return 0;
}
