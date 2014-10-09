#include "ConvexAcc.h"
#include "Matrix.h"
#include "Utils.h"

#include <map>

bool ConvexAcc::intersectWithPolygon(Ray &ray, int index, Point &origin, Vector &dir, float &distance)
{
    Point p = tunnel->path[index];
    Vector n = tunnel->nvs[index];

    // By transformaton and rotation, map (ray, p, n) to (ray', (0, 0, 0), (0, 0, -1))
    // See http://en.wikipedia.org/wiki/Rotation_matrix
    float theta = PI - atan2(n.x, n.z);
    Matrix matrix(
        cos(theta), 0, sin(theta),
        0,          1, 0,
       -sin(theta), 0, cos(theta));
    Vector offset = Vector(p, Point(0, 0, 0));
    Point newOrigin = matrix * (ray.origin + offset);
    Vector newDir = matrix * ray.direction;

    bool intersect = intersectWithPolygonAtOrigin(Ray(newOrigin, newDir), distance);
    if (!intersect) // with polygon, but intersect with wall
    {
        newOrigin.z = 0;
        newDir.z = 0;
        newDir.norm();
        origin = newOrigin;
        dir = newDir;
    }
    return intersect;
}

bool ConvexAcc::intersectWithPolygonAtOrigin(Ray &ray, float &distance)
{
    if (ray.origin.z * ray.direction.z >= 0)
    {
        return false;
    }

    // o: origin of the ray
    // p: a point on the plane xoy
    Vector op = Vector(ray.origin, Point(0, 0, 0));
    distance = op.z / ray.direction.z;
    Point p = ray.getPoint(distance);

    // +---+---+---+     +---+
    // | o | o | o | ... | o |
    // +---+---+---+     +---+
    //   |<--------------->|
    //          width
    // 1. Map position (x, y) to cell location (i, j)
    float cellWidth = tunnel->width / (CONVEX_TABLE_SIZE - 1.0f);
    float cellHeight = tunnel->height / (CONVEX_TABLE_SIZE - 1.0f);
    float x = p.x;
    float y = p.y;
    int i = (int)((x + tunnel->width / 2) / cellWidth + 0.5f);
    int j = (int)(y / cellHeight + 0.5f);
    i = std::max(i, 0);
    j = std::max(j, 0);
    i = std::min(i, CONVEX_TABLE_SIZE - 1);
    j = std::min(j, CONVEX_TABLE_SIZE - 1);

    if (intersectionTable[i][j] == Hit)
    {
        return true;
    }
    else if (intersectionTable[i][j] == Miss)
    {
        return false;
    }
    else // Partial
    {
        return inPolygon(p, edgeRangeTable[i][j].start, edgeRangeTable[i][j].end);
    }
}

bool ConvexAcc::inPolygon(const Point &p, int begin, int end)
{
    for (int i = begin; i <= end; i++)
    {
        if (edgeParams[i].A * p.x +
            edgeParams[i].B * p.y + 
            edgeParams[i].C < 0.0001f) // avoid leaks
        {
            return false;
        }
    }
    return true;
}

ConvexAcc::IntersectionTableResult ConvexAcc::calcCellStatus(
    const Point &p1, const Point &p2, const Point &p3, const Point &p4, short &minIndex, short &maxIndex)
{
    // P3 +-------+ P4
    //    |       |
    //    |   o   | <-- center
    //    |       |
    // P1 +-------+ P2
    int hit = 0;
    Point points[4] = { p1, p2, p3, p4 };

    for (int i = 0; i < 4; i++)
    {
        Point &p = points[i];
        int inside = 1;

        for (unsigned int j = 0; j < edgeParams.size(); j++)
        {
            if (edgeParams[j].A * p.x +
                edgeParams[j].B * p.y + 
                edgeParams[j].C < 0.0001f) // if point is on the right side (avoid leaks)
            {
                inside = 0;
                break;
            }
        }
        hit += inside;
    }

    if (hit == 4) 
    {
        minIndex = -1;
        maxIndex = -1;
        return Hit;
    }

    if (hit == 0)
    {
        minIndex = -1;
        maxIndex = -1;
        return Miss;
    }

    // Partial
    minIndex = SHRT_MAX;
    maxIndex = SHRT_MIN;

    for (unsigned int i = 0; i < edgeParams.size(); i++)
    {
        short index = (short)i;
        bool intersect = false;
        Point start = tunnel->crossSection.vertices[i];
        Point end = tunnel->crossSection.vertices[(i + 1) % tunnel->crossSection.vertices.size()];
    
        float s1 = edgeParams[i].A * p1.x + edgeParams[i].B * p1.y + edgeParams[i].C;
        float s2 = edgeParams[i].A * p2.x + edgeParams[i].B * p2.y + edgeParams[i].C;
        float s3 = edgeParams[i].A * p3.x + edgeParams[i].B * p3.y + edgeParams[i].C;
        float s4 = edgeParams[i].A * p4.x + edgeParams[i].B * p4.y + edgeParams[i].C;

        if ((s1 > 0.0001 && s2 > 0.0001 && s3 > 0.0001 && s4 > 0.0001) ||
            (s1 < -0.0001 && s2 < -0.0001 && s3 < -0.0001 && s4 < -0.0001)) // all points are on one side -> cannot intersect
        {
        }
        else // may intersect
        {
            if (start.x > p4.x && end.x > p4.x) // cannot intersect
                ;
            else if (start.x < p1.x && end.x < p1.x) // cannot intersect
                ;
            else if (start.y > p4.y && end.y > p4.y) // cannot intersect
                ;
            else if (start.y < p1.y && end.y < p1.y) // cannot intersect
                ;
            else // intersect
            {
                if (index > maxIndex) maxIndex = index;
                if (index < minIndex) minIndex = index;
            }
        }
    }
    return Partial;
}

void ConvexAcc::init()
{
    // Initialize edge params
    //
    // Given P1(x1, y1), P2(x2, y2) and TARGET(x, y)
    // Let v1 = P1P2 = (x2 - x1, y2 - y1)
    //     v2 = P1TARGET = (x - x1, y - y1)
    // If TARGET is on the left side of vector v1, then
    //     v1.cross(v2).z > 0
    // ==> (x2 - x1)(y - y1) - (y2 - y1)(x - x1) > 0
    // ==> (y1 - y2) * x + (x2 - x1) * y + x1 * y2 - x2 * y1 > 0
    //
    // Let A = y1 - y2, 
    //     B = x2 - x1,
    //     C = x1 * y2 - x2 * y1
    // Then
    //     A * x + B * y + C > 0
    //
    // So for a convex polygon, if its N vertices are counterclockwise,
    // it's possible to check if a point in in the polygon with not more than
    // 2 * N multiply operations, 3 * N add operations, and N compare operations.
    Utils::PrintTickCount("Initialize Edge Params");

    for (unsigned int i = 0; i < tunnel->crossSection.vertices.size(); i++)
    {
        Point &p1 = tunnel->crossSection.vertices[i];
        Point &p2 = tunnel->crossSection.vertices[(i + 1) % tunnel->crossSection.vertices.size()];

        EdgeParam param;
        param.A = p1.y - p2.y;
        param.B = p2.x - p1.x;
        param.C = p1.x * p2.y - p2.x * p1.y;

        edgeParams.push_back(param);
    }

    // Initialize intersection table (with the cross section at the origin)
    Utils::PrintTickCount("Initialize Intersection Table");

    for (int i = 0; i < CONVEX_TABLE_SIZE; i++)
    {
        for (int j = 0; j < CONVEX_TABLE_SIZE; j++)
        {
            // P3 +-------+ P4
            //    |       |
            //    |   o   | <-- center
            //    |       |
            // P1 +-------+ P2
            //
            // +---+---+---+     +---+
            // | o | o | o | ... | o |
            // +---+---+---+     +---+
            //   |<--------------->|
            //          width

            float cellWidth = tunnel->width / (CONVEX_TABLE_SIZE - 1.0f);
            float cellHeight = tunnel->height / (CONVEX_TABLE_SIZE - 1.0f);
            Point center = Point(i * cellWidth - tunnel->width / 2, j * cellHeight, 0);
            Point p1 = center + Vector(-cellWidth / 2, -cellHeight / 2);
            Point p2 = center + Vector(cellWidth / 2, -cellHeight / 2);
            Point p3 = center + Vector(-cellWidth / 2, cellHeight / 2);
            Point p4 = center + Vector(cellWidth / 2, cellHeight / 2);

            short minIndex, maxIndex;
            intersectionTable[i][j] = calcCellStatus(p1, p2, p3, p4, minIndex, maxIndex);
            edgeRangeTable[i][j].start = minIndex;
            edgeRangeTable[i][j].end = maxIndex;
        }
    }

    if (tunnel->algorithm == Tunnel::Convex) // not ConvexSimple
    {
        // Initialize intersection table (y axis)
        Utils::PrintTickCount("Initialize Intersection Table (Y Axis)");
    
        //#pragma omp parallel for schedule(dynamic, 1) // OpenMP

        for (int y = 0; y < 100; y++)
        {
            for (int iAngle = 0; iAngle < 360; iAngle++)
            {
                Point targetPoint(0, tunnel->height * (y + 0.5f) / 100.0f, 0);
                float targetAngle = iAngle / 360.0f * PI * 2; // [0, 2 * PI)

                std::multimap<float, int> mapping; // delta angle -> segment index

                // For each segment on the cross section
                for (unsigned int i = 0; i < tunnel->crossSection.vertices.size(); i++)
                {
                    Point p1 = tunnel->crossSection.vertices[i];
                    Point p2 = tunnel->crossSection.vertices[(i + 1) % tunnel->crossSection.vertices.size()];

                    // if the ray(targetPoint, targetAngle) hit segment (P1, P2), then delta angle = 0
                    Vector v1 = Vector(targetPoint, p1);
                    Vector v2 = Vector(targetPoint, p2);
                    Vector v = Vector(cos(targetAngle), sin(targetAngle), 0);
                    float delta;

                    if (v1.cross(v).z > 0 && v.cross(v2).z > 0)
                    {
                        delta = 0;
                    }
                    else
                    {
                        Point midPoint((p1.x + p2.x) / 2, (p1.y + p2.y) / 2, 0);
                        float angle = atan2(midPoint.y - targetPoint.y, midPoint.x - targetPoint.x); // (-PI, PI]
                        delta = targetAngle - angle; // [-PI, 3 * PI)
                        delta = (delta > PI) ? delta - 2 * PI : delta; // [-PI, PI)
                        delta = fabs(delta);
                    }

                    mapping.insert(std::multimap<float, int>::value_type(delta, i));
                }

                std::multimap<float, int>::iterator it;
                for (it = mapping.begin(); it != mapping.end(); ++it)
                {
                    int index = it->second;
                    intersectionTableYAxis[y][iAngle].push_back(index * 2);
                    intersectionTableYAxis[y][iAngle].push_back(index * 2 + 1);
                }
            }
        }
    }
}

IntersectResult ConvexAcc::intersect(Ray &ray)
{
    RayContext &context = ray.context;

    Ray advRay = ray; // the advanced ray
    float distance; // the advanced distance

    if (!context.inTunnel)
    {
        Point newOrigin;
        Vector newDir;

        // into the entrance of the tunnel
        if (intersectWithPolygon(ray, 0, newOrigin, newDir, distance) && ray.direction.dot(tunnel->nvs[0]) > 0)
        {
            context.inTunnel = true; // we know it will gets into the tunnel
            context.segment = 0;
            advRay.origin = ray.getPoint(distance);
            advRay.direction = ray.direction;
        }
        else if (intersectWithPolygon(ray, tunnel->path.size() - 1, newOrigin, newDir, distance) && 
            ray.direction.dot(tunnel->nvs[tunnel->path.size() - 1]) < 0) // into the exit of the tunnel
        {
            context.inTunnel = true;
            context.segment = tunnel->path.size() - 2;
            advRay.origin = ray.getPoint(distance);
            advRay.direction = ray.direction;
        }
        else // the origin of the ray is not in the tunnel, and it does not gets into the tunnel
        {
            return tunnel->linearIntersect(ray);
        }
    }

    if (context.inTunnel)
    {
        int N = tunnel->path.size() - 1; // a path with with N segments has N + 1 nodes (range: 0 to N)
        int begin = context.segment; // the current segment (range: 0 to N - 1)
        enum RayDir dir = ray.direction.dot(tunnel->nvs[begin]) > 0 ? Forward : Backward;

        if (dir == Forward)
        {
            for (int i = begin + 1; i <= N; i++)
            {
                Point newOrigin;
                Vector newDir;

                if (!intersectWithPolygon(advRay, i, newOrigin, newDir, distance)) // intersect with wall
                {
                    if (tunnel->algorithm == Tunnel::ConvexSimple) // linear search in the current segment
                    {
                        for (unsigned int j = 0; j < tunnel->surface[i - 1].size(); j++)
                        {
                            IntersectResult result = tunnel->surface[i - 1][j]->intersect(ray);
                            if (result.hit)
                            {
                                context.segment = i - 1;
                                return result;
                            }
                        }
                    }
                    else // fast intersect
                    {
                        float y = newOrigin.y - newOrigin.x * newDir.y / newDir.x;
                        int index = (int)(99.0f * y / tunnel->height + 0.5f);
                        index = std::max(0, index);
                        index = std::min(99, index);

                        float fAngle = atan2(newDir.y, newDir.x);
                        fAngle = (fAngle < 0) ? fAngle + PI * 2 : fAngle;
                        int iAngle = (int)(fAngle / PI * 180.0f);
                        iAngle = std::max(0, iAngle);
                        iAngle = std::min(359, iAngle);

                        for (unsigned int j = 0; j < intersectionTableYAxis[index][iAngle].size(); j++)
                        {
                            int segmentIndex = intersectionTableYAxis[index][iAngle][j];

                            IntersectResult result = tunnel->surface[i - 1][segmentIndex]->intersect(ray);
                            if (result.hit)
                            {
                                //Utils::DbgPrint("Intersect with triangle %d / %d\n", 
                                //    j, intersectionTableYAxis[index][iAngle].size());
                                context.segment = i - 1;
                                return result;
                            }
                        }

                        // As float point numbers are not accurate by nature, it's not a problem
                        // when it gets here. Now advance the ray to the next polygon and try again.
                        advRay.origin = advRay.getPoint(distance);
                    }
                }
                else // intersect with polygon
                {
                    advRay.origin = advRay.getPoint(distance);
                }
            }

            // The ray intersects with all the polygons on the way
            context.inTunnel = false;
            return IntersectResult(false);
        }
        else if (dir == Backward)
        {
            // It's unlikely to get here (to be implemented)
            Utils::DbgPrint("Backward");
            return IntersectResult(false);
        }
    }

    // Just to avoid warnings. It's impossible to get here
    return IntersectResult(false);
}
