#include "TunnelGenerator.h"
#include "Triangle.h"
#include "Tunnel.h"

bool TunnelGenerator::createPolyhedron(
    Polygon &front, Polygon &rear, Polyhedron &polyhedron,
    int &connBC, int &connAD, int &connBoth, int &connInvalid)
{
    const float DOT_TOLERANCE = 0.001f; // 0.06 degree

    // Initialize statistics
    connBC = 0;
    connAD = 0;
    connBoth = 0;
    connInvalid = 0;

    // Initialize polyhedron
    polyhedron.front = front;
    polyhedron.rear = rear;
    polyhedron.connections.clear();
    polyhedron.convex = true;

    // Set up connections
    for (unsigned int j = 0; j < front.vertices.size(); j++)
    {
        int k = (j + 1) % front.vertices.size();

        Point A = polyhedron.front.vertices[j];
        Point B = polyhedron.front.vertices[k];
        Point C = polyhedron.rear.vertices[j];
        Point D = polyhedron.rear.vertices[k];

        // 1. Triangle CBA and CDB
        Vector nCBA = Vector(C, B).cross(Vector(B, A)).norm();
        Vector nCDB = Vector(C, D).cross(Vector(D, B)).norm();
        bool ok1 = true;

        // 1.1 CBA
        Vector vAD = Vector(A, D).norm();
        if (nCBA.dot(vAD) > DOT_TOLERANCE)
        {
            ok1 = false;
        }

        if (ok1 == true)
        {
            for (unsigned int m = 0; m < front.vertices.size(); m++)
            {
                if (m != j && m != k)
                {
                    Point E = polyhedron.front.vertices[m];
                    Point F = polyhedron.rear.vertices[m];
                    Vector vAE = Vector(A, E).norm();
                    Vector vAF = Vector(A, F).norm();
                    if (nCBA.dot(vAE) > DOT_TOLERANCE || 
                        nCBA.dot(vAF) > DOT_TOLERANCE)
                    {
                        ok1 = false;
                        break;
                    }
                }
            }
        }

        // 1.2 CDB
        Vector vCA = Vector(C, A).norm();
        if (nCDB.dot(vCA) > DOT_TOLERANCE)
        {
            ok1 = false;
        }

        if (ok1 == true)
        {
            for (unsigned int m = 0; m < front.vertices.size(); m++)
            {
                if (m != j && m != k)
                {
                    Point E = polyhedron.front.vertices[m];
                    Point F = polyhedron.rear.vertices[m];
                    Vector vCE = Vector(C, E).norm();
                    Vector vCF = Vector(C, F).norm();
                    if (nCDB.dot(vCE) > DOT_TOLERANCE ||
                        nCDB.dot(vCF) > DOT_TOLERANCE)
                    {
                        ok1 = false;
                        break;
                    }
                }
            }
        }

        // 2. Triangle ADB and ACD
        Vector nADB = Vector(A, D).cross(Vector(D, B)).norm();
        Vector nACD = Vector(A, C).cross(Vector(C, D)).norm();
        bool ok2 = true;

        // 2.1 ADB
        Vector vAC = Vector(A, C).norm();
        if (nADB.dot(vAC) > DOT_TOLERANCE)
        {
            ok2 = false;
        }

        if (ok2 == true)
        {
            for (unsigned int m = 0; m < front.vertices.size(); m++)
            {
                if (m != j && m != k)
                {
                    Point E = polyhedron.front.vertices[m];
                    Point F = polyhedron.rear.vertices[m];
                    Vector vAE = Vector(A, E).norm();
                    Vector vAF = Vector(A, F).norm();
                    if (nADB.dot(vAE) > DOT_TOLERANCE ||
                        nADB.dot(vAF) > DOT_TOLERANCE)
                    {
                        ok2 = false;
                        break;
                    }
                }
            }
        }

        // 2.2 ADC
        Vector vAB = Vector(A, B).norm();
        if (nACD.dot(vAB) > DOT_TOLERANCE)
        {
            ok2 = false;
        }

        if (ok2 == true)
        {
            for (unsigned int m = 0; m < front.vertices.size(); m++)
            {
                if (m != j && m != k)
                {
                    Point E = polyhedron.front.vertices[m];
                    Point F = polyhedron.rear.vertices[m];
                    Vector vAE = Vector(A, E).norm();
                    Vector vAF = Vector(A, F).norm();
                    if (nACD.dot(vAE) > DOT_TOLERANCE || 
                        nACD.dot(vAF) > DOT_TOLERANCE)
                    {
                        ok2 = false;
                        break;
                    }
                }
            }
        }

        if (ok1 == true && ok2 == false)
        {
            polyhedron.connections.push_back(Polyhedron::BC);
            connBC += 1;
        }
        else if (ok1 == false && ok2 == true)
        {
            polyhedron.connections.push_back(Polyhedron::AD);
            connAD += 1;
        }
        else if (ok1 == true && ok2 == true)
        {
            polyhedron.connections.push_back(Polyhedron::Both);
            connBoth += 1;
        }
        else // ok1 == false && ok2 == false
        {
            polyhedron.convex = false;
            polyhedron.connections.push_back(Polyhedron::Invalid);
            connInvalid += 1;
        }
    }

    return connInvalid == 0;
}

bool TunnelGenerator::create(
    float rectWidth, float rectHeight, float archHeight, // cross section attributes
    float pathRadius, float pathAngle, // path attributes
    int archSegments, int pathSegments, // tessellation attributes
    GeometrySet &scene, 
    Tunnel::Algorithm algorithm)
{
    Tunnel *tunnel = new Tunnel();
    tunnel->height = rectHeight + archHeight;
    tunnel->width = rectWidth;
    tunnel->algorithm = algorithm;

    // 1. Create the cross section at the origin
    // ------------------------------------------------------------------------------------
    // The cross section of a tunnel (it consists of a rectangle and a half ellipse).
    // When rectHeight = 0, it is simply a half ellipse
    //                ___ --------- ___              ---
    //             _/                   \_            |
    //           /                        \          archHeight
    //         /                           \          |
    // P(N-1) + - - - - - - - - - - - - - - + P(1)  ---
    //        |                             |         |
    //        |                             |         |
    //        |                             |        rectHeight
    //        |                             |         |
    //        |               Origin        |         |
    // PN     +--------------*--------------+ P(0)  ---
    //        |<-------- rectWidth -------->|

    // 1.1 Add P0
    tunnel->crossSection.vertices.push_back(Point(rectWidth * 0.5f, 0.0, 0.0));
    tunnel->crossSection.flags.push_back(Tunnel::FLAG_CRITICAL);

    // 1.2 Add P1 - P(N-1)
    for (int i = 0; i <= archSegments; i++)
    {
        float angle = PI * i / archSegments;
        tunnel->crossSection.vertices.push_back(Point(
            cos(angle) * rectWidth * 0.5f,
            sin(angle) * archHeight + rectHeight,
            0.0f));
        tunnel->crossSection.flags.push_back(
            (i == 0 || i == archSegments) ? Tunnel::FLAG_CRITICAL : Tunnel::FLAG_NONE);
    }

    // 1.3 Add PN
    tunnel->crossSection.vertices.push_back(Point(-rectWidth * 0.5f, 0.0, 0.0));
    tunnel->crossSection.flags.push_back(Tunnel::FLAG_CRITICAL);

    // 2. Initialize the path and the tunnel surface
    for (int i = 0; i < pathSegments; i++)
    {
        float theta1 = pathAngle * i / pathSegments;
        float theta2 = pathAngle * (i + 1) / pathSegments;
        Point p1(pathRadius * (1.0f - cos(theta1)), 0.0f, -pathRadius * sin(theta1));
        Point p2(pathRadius * (1.0f - cos(theta2)), 0.0f, -pathRadius * sin(theta2));
    
        if (i == 0)
        {
            tunnel->path.push_back(p1);
            tunnel->path.push_back(p2);
        }
        else
        {
            tunnel->path.push_back(p2);
        }

        tunnel->surface.push_back(std::vector<Triangle *>());
    }

    // 2.1 Initialize normal vectors
    int N = pathSegments;

    //    1     2     3    ...    N       segment
    // o-----o-----o-----o-----o-----o   path
    // 0     1     2     3     4 ...  N   vertex
    for (int i = 1; i <= N; i++)
    {
        Point start = tunnel->path[i - 1];
        Point end = tunnel->path[i];
        Vector dir = Vector(start, end).norm();
        Vector prevDir = Vector(start, end).norm();
        Vector nextDir = Vector(start, end).norm();
    
        //        [start]   vector dir    [end]
        //               o-------------->o
        //     vector   /                 \   vector
        //    prev dir /                   \ next dir
        //            /                     \
        //           o                       o
        // [path i-2]                         [path i+1]
        if (i > 1)
            prevDir = Vector(tunnel->path[i - 2], tunnel->path[i - 1]).norm();

        if (i < N)
            nextDir = Vector(tunnel->path[i], tunnel->path[i + 1]).norm();

        Vector v1 = ((prevDir + dir) * 0.5).norm();
        Vector v2 = ((dir + nextDir) * 0.5).norm();

        if (i == 1)
        {
            tunnel->nvs.push_back(v1);
            tunnel->nvs.push_back(v2);
        }
        else
        {
            tunnel->nvs.push_back(v2);
        }
    }

    // 3. Traverse the path
    for (int i = 0; i < N; i++)
    {
        float offsetAngle1 = Vector(0, 0, -1).angleTo(tunnel->nvs[i]);
        float offsetAngle2 = Vector(0, 0, -1).angleTo(tunnel->nvs[i + 1]);
        
        // 3.1 Create polygons "front" and "rear"
        Polygon front, rear;
        for (unsigned int j = 0; j < tunnel->crossSection.vertices.size(); j++)
        {
            Point p = tunnel->crossSection.vertices[j];
            front.vertices.push_back(Point(
                p.x * cos(offsetAngle1) - p.z * sin(offsetAngle1), 
                p.y, 
                p.x * sin(offsetAngle1) + p.z * cos(offsetAngle1)) + Vector(Point(0, 0, 0), tunnel->path[i]));
            rear.vertices.push_back(Point(
                p.x * cos(offsetAngle2) - p.z * sin(offsetAngle2), 
                p.y, 
                p.x * sin(offsetAngle2) + p.z * cos(offsetAngle2)) + Vector(Point(0, 0, 0), tunnel->path[i + 1]));
        }

        // 3.2 Create polyhedron
        int connBC, connAD, connBoth, connInvalid;
        Polyhedron polyhedron;

        if (!createPolyhedron(front, rear, polyhedron, connBC, connAD, connBoth, connInvalid))
        {
            fprintf(stderr, "Polyhedron %d is not convex!\n", i + 1);
            // return false; // when OpenMP is enabled, there should not be returns
        }
        else // 3.3 Add to scene
        {
            for (unsigned int j = 0; j < tunnel->crossSection.vertices.size(); j++)
            {
                Point A = front.vertices[j];
                Point B = front.vertices[(j + 1) % front.vertices.size()];
                Point C = rear.vertices[j];
                Point D = rear.vertices[(j + 1) % rear.vertices.size()];

                if (polyhedron.connections[j] == Polyhedron::AD)
                {
                    Triangle *tACD = new Triangle(A, C, D);
                    Triangle *tADB = new Triangle(A, D, B);
                    tunnel->surface[i].push_back(tACD);
                    tunnel->surface[i].push_back(tADB);
                }
                else // BC or Both
                {
                    Triangle *tCDB = new Triangle(C, D, B);
                    Triangle *tCBA = new Triangle(C, B, A);
                    tunnel->surface[i].push_back(tCDB);
                    tunnel->surface[i].push_back(tCBA);
                }
            }
        }
    }
    scene.add(tunnel);
    return true;
}
