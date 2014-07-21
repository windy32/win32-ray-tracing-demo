#include "Plane.h"
#include "Sphere.h"
#include "Triangle.h"

#include "CheckerMaterial.h"
#include "RadianceCheckerMaterial.h"
#include "SolidColorMaterial.h"
#include "GlassMaterial.h"
#include "RandomColorMaterial.h"
#include "PhongMaterial.h"

#include "TunnelGenerator.h"

#include "Utils.h"
#include "Scripts.h"

Script *scripts[5] = 
{
    new Script1(), new Script2(), new Script3(), new Script4(), new Script5()
};

void Script1::Run(RenderProc render, int tunnelAlgorithm, LogCallback log, ProgressCallback progress, 
                  int &prepareTime, int &execTime)
{
    GeometrySet scene;

    // 1. Prepare objects
    Plane *plane = new Plane(Vector(0, 1, 0), 0); // Ground
    Sphere *sphere1 = new Sphere(Point(-10, 15, -30), 15);
    Sphere *sphere2 = new Sphere(Point(20, 10, -20), 10);

    plane->material = Ptr<Material>(new RadianceCheckerMaterial(1.2f, 0.025f));
    sphere1->material = Ptr<Material>(new SolidColorMaterial(Color::White(), Color::Black(), 1, 0, 0));
    sphere2->material = Ptr<Material>(new SolidColorMaterial(Color::White(), Color::Black(), 1, 0, 0));

    scene.add(plane);
    scene.add(sphere1);
    scene.add(sphere2);

    // 2. Prepare camera
    PerspectiveCamera camera(
        Point(0, 15, 30), // eye
        Vector(0, 0, -1), // front
        Vector(0, 1, 0),  // up
        1.3333f,          // ratio (width : height = 4 : 3)
        65,               // fov (field of view)
        0);               // forward

    // 3. Prepare render setting
    RenderSetting renderSetting = RenderSetting::Default();

    // 4.Render
    prepareTime = 0;
    execTime = render(scene, camera, renderSetting, progress);
}

void Script2::Run(RenderProc render, int tunnelAlgorithm, LogCallback log, ProgressCallback progress, 
                  int &prepareTime, int &execTime)
{
    GeometrySet scene;

    // 1. Prepare objects
    Plane *plane1 = new Plane(Vector(1, 0, 0), 1); // Left
    Plane *plane2 = new Plane(Vector(1, 0, 0), 99);  // Right
    Plane *plane3 = new Plane(Vector(0, 1, 0), 0); // Bottom
    Plane *plane4 = new Plane(Vector(0, 1, 0), 81.6f);  // Top
    Plane *plane5 = new Plane(Vector(0, 0, 1), 0);   // Front
    Plane *plane6 = new Plane(Vector(0, 0, 1), 170); // Back

    Sphere *sphere1 = new Sphere(Point(50, 681.6f - 0.27f, 81.6f), 600); // Light
    Sphere *sphere2 = new Sphere(Point(27, 16.5f, 47), 16.5f);           // Mirror
    Sphere *sphere3 = new Sphere(Point(73, 16.5f, 78), 16.5f);           // Glass

    plane1->material = Ptr<Material>(new SolidColorMaterial(Color(0.75f, 0.25f, 0.25f), Color::Black(), 1, 0, 0));
    plane2->material = Ptr<Material>(new SolidColorMaterial(Color(0.25f, 0.25f, 0.75f), Color::Black(), 1, 0, 0));
    plane3->material = Ptr<Material>(new SolidColorMaterial(Color(0.75f, 0.75f, 0.75f), Color::Black(), 1, 0, 0));
    plane4->material = Ptr<Material>(new SolidColorMaterial(Color(0.75f, 0.75f, 0.75f), Color::Black(), 1, 0, 0));
    plane5->material = Ptr<Material>(new SolidColorMaterial(Color(0.75f, 0.75f, 0.75f), Color::Black(), 1, 0, 0));
    plane6->material = Ptr<Material>(new SolidColorMaterial(Color(0, 0, 0),             Color::Black(), 1, 0, 0));

    sphere1->material = Ptr<Material>(new SolidColorMaterial(Color::Black(), Color(24, 24, 24), 1, 0, 0));
    sphere2->material = Ptr<Material>(new SolidColorMaterial(Color::White(), Color::Black(), 0, 1, 0));
    sphere3->material = Ptr<Material>(new GlassMaterial());

    scene.add(plane1);
    scene.add(plane2);
    scene.add(plane3);
    scene.add(plane4);
    scene.add(plane5);
    scene.add(plane6);

    scene.add(sphere1);
    scene.add(sphere2);
    scene.add(sphere3);

    // 2. Prepare camera
    PerspectiveCamera camera(
        Point(50, 52, 295),    // eye
        Vector(0,-0.045f, -1), // front
        Vector(0, 1, -0.045f), // up
        1.3333f,               // ratio (width : height = 4 : 3)
        28,                    // fov (field of view)
        140.0f);               // forward

    // 3. Prepare render setting
    RenderSetting renderSetting = RenderSetting::Default();

    // 4.Render
    prepareTime = 0;
    execTime = render(scene, camera, renderSetting, progress);
}

void Script3::Run(RenderProc render, int tunnelAlgorithm, LogCallback log, ProgressCallback progress, 
                  int &prepareTime, int &execTime)
{
    GeometrySet scene;

    // 1. Prepare objects
    Plane *plane1 = new Plane(Vector(1, 0, 0), 1); // Left
    Plane *plane2 = new Plane(Vector(1, 0, 0), 99);  // Right
    Plane *plane3 = new Plane(Vector(0, 1, 0), 0); // Bottom
    Plane *plane4 = new Plane(Vector(0, 1, 0), 81.6f);  // Top
    Plane *plane5 = new Plane(Vector(0, 0, 1), 0);   // Front
    Plane *plane6 = new Plane(Vector(0, 0, 1), 170); // Back

    Sphere *sphere1 = new Sphere(Point(50, 681.6f - 0.27f, 81.6f), 600); // Light

    plane1->material = Ptr<Material>(new SolidColorMaterial(Color(0.75f, 0.25f, 0.25f), Color::Black(), 1, 0, 0));
    plane2->material = Ptr<Material>(new SolidColorMaterial(Color(0.25f, 0.25f, 0.75f), Color::Black(), 1, 0, 0));
    plane3->material = Ptr<Material>(new SolidColorMaterial(Color(0.75f, 0.75f, 0.75f), Color::Black(), 1, 0, 0));
    plane4->material = Ptr<Material>(new SolidColorMaterial(Color(0.75f, 0.75f, 0.75f), Color::Black(), 1, 0, 0));
    plane5->material = Ptr<Material>(new SolidColorMaterial(Color(0.75f, 0.75f, 0.75f), Color::Black(), 1, 0, 0));
    plane6->material = Ptr<Material>(new SolidColorMaterial(Color(0, 0, 0),             Color::Black(), 1, 0, 0));

    sphere1->material = Ptr<Material>(new SolidColorMaterial(Color::Black(), Color(24, 24, 24), 1, 0, 0));

    scene.add(plane1);
    scene.add(plane2);
    scene.add(plane3);
    scene.add(plane4);
    scene.add(plane5);
    scene.add(plane6);

    scene.add(sphere1);

    Matrix matrix(
        1, 0, 0, 
        0, 1, 0,
        0, 0, 1);
    Vector offset(50, 0, 40);
    scene.addStlFile("ball.stl", Ptr<Material>(new GlassMaterial()), matrix, offset);

    // 2. Prepare camera
    PerspectiveCamera camera(
        Point(50, 52, 295),    // eye
        Vector(0,-0.045f, -1), // front
        Vector(0, 1, -0.045f), // up
        1.3333f,               // ratio (width : height = 4 : 3)
        28,                    // fov (field of view)
        140.0f);               // forward

    // 3. Prepare render setting
    RenderSetting renderSetting = RenderSetting::Default();

    // 4.Render
    prepareTime = 0;
    execTime = render(scene, camera, renderSetting, progress);
}

// short wide tunnel
void Script4::Run(RenderProc render, int tunnelAlgorithm, LogCallback log, ProgressCallback progress, 
                  int &prepareTime, int &execTime)
{
    GeometrySet scene;

    // 1. Prepare objects
    Utils::DbgPrint("Starting Script (%d segments)\r\n", tunnelSegments);
    Utils::PrintTickCount("Current Time");

    Sphere *ball = new Sphere(Point(74.12f, 15, -96.59f), 15);
    ball->material = Ptr<Material>(new PhongMaterial(Color(1, 0, 0), Color::White(), 16));
    scene.add(ball);

    Plane *ground = new Plane(Vector(0, 1, 0), -0.01f);
    ground->material = Ptr<Material>(new SolidColorMaterial(Color(0.25, 0.25, 0.25), Color::Black(), 1, 0, 0));
    scene.add(ground);

    Plane *sky = new Plane(Vector(0, 1, 0), 1000);
    sky->material = Ptr<Material>(new SolidColorMaterial(Color::White(), Color::Black(), 1, 0, 0));
    scene.add(sky);

    Utils::PrintTickCount("Creating Tunnel");

    TunnelGenerator g; // Add a tunnel
    g.create(50, 25, 25, 100, PI * 0.416667f, tunnelSegments, tunnelSegments, scene, 
        Ptr<Material>(new CheckerMaterial(0.05f)), // ground material
        Ptr<Material>(new SolidColorMaterial(Color::Black(), Color::Black(), 0.333f, 0.667f, 0)), // wall material
        (Tunnel::Algorithm)tunnelAlgorithm); // accellaration algorithm

    int t1 = Utils::GetTickCount();
    Tunnel *tunnel = (Tunnel *)scene.last();
    tunnel->init();
    int t2 = Utils::GetTickCount();
    prepareTime = t2 - t1;

    // 2. Prepare camera
    PerspectiveCamera inTunnel(
        Point(0, 25, 20),     // eye
        Vector(0, 0, -1),      // front
        Vector(0, 1, 0),       // up
        1.3333f,               // ratio (width : height = 4 : 3)
        65,                    // fov (field of view)
        0.0f);                 // forward

    // 3. Prepare render setting
    RenderSetting renderSetting = RenderSetting::Simple();

    // 4.Render
    Utils::PrintTickCount("Render");
    execTime = render(scene, inTunnel, renderSetting, progress);
    Utils::PrintTickCount("Finished");
    Utils::DbgPrint("\r\n");
}

// long narrow tunnel
void Script5::Run(RenderProc render, int tunnelAlgorithm, LogCallback log, ProgressCallback progress, 
                  int &prepareTime, int &execTime)
{
    GeometrySet scene;

    // 1. Prepare objects
    Utils::DbgPrint("Starting Script (%d segments)\r\n", tunnelSegments);
    Utils::PrintTickCount("Current Time");

    Sphere *ball = new Sphere(Point(5000, 15, -5000), 15);
    ball->material = Ptr<Material>(new PhongMaterial(Color(1, 0, 0), Color::White(), 16));
    scene.add(ball);

    Plane *ground = new Plane(Vector(0, 1, 0), -0.01f);
    ground->material = Ptr<Material>(new SolidColorMaterial(Color(0.25, 0.25, 0.25), Color::Black(), 1, 0, 0));
    scene.add(ground);

    Plane *sky = new Plane(Vector(0, 1, 0), 1000);
    sky->material = Ptr<Material>(new SolidColorMaterial(Color::White(), Color::Black(), 1, 0, 0));
    scene.add(sky);

    Utils::PrintTickCount("Creating Tunnel");

    TunnelGenerator g; // Add a tunnel
    g.create(50, 25, 25, 5000, PI * 0.5f, tunnelSegments, tunnelSegments, scene, 
        Ptr<Material>(new CheckerMaterial(0.05f)), // ground material
        Ptr<Material>(new SolidColorMaterial(Color::Black(), Color::Black(), 0.333f, 0.667f, 0)), // wall material
        (Tunnel::Algorithm)tunnelAlgorithm); // accellaration algorithm

    int t1 = Utils::GetTickCount();
    Tunnel *tunnel = (Tunnel *)scene.last();
    tunnel->init();
    int t2 = Utils::GetTickCount();
    prepareTime = t2 - t1;

    // 2. Prepare camera
    PerspectiveCamera inTunnel(
        Point(0, 25, 20),     // eye
        Vector(0, 0, -1),      // front
        Vector(0, 1, 0),       // up
        1.3333f,               // ratio (width : height = 4 : 3)
        65,                    // fov (field of view)
        0.0f);                 // forward

    // 3. Prepare render setting
    RenderSetting renderSetting = RenderSetting::Simple();

    // 4.Render
    Utils::PrintTickCount("Render");
    execTime = render(scene, inTunnel, renderSetting, progress);
    Utils::PrintTickCount("Finished");
    Utils::DbgPrint("\r\n");
}
