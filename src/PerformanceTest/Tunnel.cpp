#include "Tunnel.h"
#include "Matrix.h"
#include "Utils.h"
#include "Grid.h"
#include <queue>
#include <map>
#include <algorithm>

Tunnel::Tunnel()
{
    root = NULL;
    type = GeometryType::TUNNEL;
}

Tunnel::~Tunnel()
{
    if (root != NULL)
    {
        deleteTree(root);
    }

    for (unsigned int i = 0; i < surface.size(); i++)
    {
        for (unsigned int j = 0; j < surface[i].size(); j++)
        {
            delete surface[i][j];
        }
    }
}

bool Tunnel::intersectWithPolygon(Ray &ray, int index, Point &origin, Vector &dir, float &distance)
{
    Point p = path[index];
    Vector n = nvs[index];

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

bool Tunnel::intersectWithPolygonAtOrigin(Ray &ray, float &distance)
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
    float cellWidth = width / (CONVEX_TABLE_SIZE - 1.0f);
    float cellHeight = height / (CONVEX_TABLE_SIZE - 1.0f);
    float x = p.x;
    float y = p.y;
    int i = (int)((x + width / 2) / cellWidth + 0.5f);
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

bool Tunnel::inPolygon(const Point &p, int begin, int end)
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

Tunnel::IntersectionTableResult Tunnel::calcCellStatus(
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
        Point start = crossSection.vertices[i];
        Point end = crossSection.vertices[(i + 1) % crossSection.vertices.size()];
    
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

void Tunnel::init()
{
    if (algorithm == RegularGrid || algorithm == FlatGrid)
    {
        initGrid();
    }
    else if (algorithm == KdTreeSAH || algorithm == KdTreeStandard)
    {
        initKdTree();
    }
    else if (algorithm == Convex || algorithm == ConvexSimple)
    {
        initConvex();
    }
    // else: nothing to do

    Utils::PrintTickCount("Initialization Finished");
}

void Tunnel::initConvex()
{
    Utils::PrintTickCount("Initialize Normal Vectors");

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

    for (unsigned int i = 0; i < crossSection.vertices.size(); i++)
    {
        Point &p1 = crossSection.vertices[i];
        Point &p2 = crossSection.vertices[(i + 1) % crossSection.vertices.size()];

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

            float cellWidth = width / (CONVEX_TABLE_SIZE - 1.0f);
            float cellHeight = height / (CONVEX_TABLE_SIZE - 1.0f);
            Point center = Point(i * cellWidth - width / 2, j * cellHeight, 0);
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

    if (algorithm == Convex) // not ConvexSimple
    {
        // Initialize intersection table (y axis)
        Utils::PrintTickCount("Initialize Intersection Table (Y Axis)");
    
        //#pragma omp parallel for schedule(dynamic, 1) // OpenMP

        for (int y = 0; y < 100; y++)
        {
            for (int iAngle = 0; iAngle < 360; iAngle++)
            {
                Point targetPoint(0, height * (y + 0.5f) / 100.0f, 0);
                float targetAngle = iAngle / 360.0f * PI * 2; // [0, 2 * PI)

                std::multimap<float, int> mapping; // delta angle -> segment index

                // For each segment on the cross section
                for (unsigned int i = 0; i < crossSection.vertices.size(); i++)
                {
                    Point p1 = crossSection.vertices[i];
                    Point p2 = crossSection.vertices[(i + 1) % crossSection.vertices.size()];

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

void Tunnel::initGrid()
{
    Utils::PrintTickCount("Initialize Grid");

    // 1. Get the range of the triangles
    float min_x = FLT_MAX, min_y = FLT_MAX, min_z = FLT_MAX;
    float max_x = -FLT_MAX, max_y = -FLT_MAX, max_z = -FLT_MAX;

    for (unsigned int i = 0; i < surface.size(); i++)
    {
        for (unsigned int j = 0; j < surface[i].size(); j++)
        {
            Point min, max;
            Triangle *t = surface[i][j];
            t->getBoundingBox(min, max);

            min_x = std::min(min_x, min.x);
            min_y = std::min(min_y, min.y);
            min_z = std::min(min_z, min.z);

            max_x = std::max(max_x, max.x);
            max_y = std::max(max_y, max.y);
            max_z = std::max(max_z, max.z);
        }
    }

    float width = max_x - min_x;
    float height = max_y - min_y;
    float depth = max_z - min_z;

    if (algorithm == RegularGrid)
    {
        float maxLength = std::max(std::max(width, height), depth);

        // Cut the longest dimension into 400 pieces
        float size = maxLength / 399;
        grid.origin = Point(min_x - size / 2, min_y - size / 2, min_z - size / 2);
        grid.cellSizeX = size;
        grid.cellSizeY = size;
        grid.cellSizeZ = size;
        grid.xLength = (int)(width / grid.cellSizeX + 1.5f);
        grid.yLength = (int)(height / grid.cellSizeY + 1.5f);
        grid.zLength = (int)(depth / grid.cellSizeZ + 1.5f);
        grid.data.clear();
        grid.data.resize(grid.xLength * grid.yLength * grid.zLength);
    }
    else // FlatGrid
    {
        // Cut each dimension into 400 pieces
        grid.cellSizeX = width / 399;
        grid.cellSizeY = height / 399;
        grid.cellSizeZ = depth / 399;
        grid.origin = Point(
            min_x - grid.cellSizeX / 2, 
            min_y - grid.cellSizeY / 2,
            min_z - grid.cellSizeZ / 2);
        grid.xLength = 400;
        grid.yLength = 400;
        grid.zLength = 400;
        grid.data.clear();
        grid.data.resize(grid.xLength * grid.yLength * grid.zLength);
    }

    Utils::DbgPrint("Grid Size: %d x %d x %d\n", grid.xLength, grid.yLength, grid.zLength);

    // For each triangle
    for (unsigned int m = 0; m < surface.size(); m++)
    {
        for (unsigned int n = 0; n < surface[m].size(); n++)
        {
            Triangle *t = surface[m][n];
            Point min, max;
            t->getBoundingBox(min, max);

            int x_begin = (int)((min.x - grid.origin.x) / grid.cellSizeX);
            int y_begin = (int)((min.y - grid.origin.y) / grid.cellSizeY);
            int z_begin = (int)((min.z - grid.origin.z) / grid.cellSizeZ);

            int x_end = (int)((max.x - grid.origin.x) / grid.cellSizeX);
            int y_end = (int)((max.y - grid.origin.y) / grid.cellSizeY);
            int z_end = (int)((max.z - grid.origin.z) / grid.cellSizeZ);

            // Traverse the grid
            for (int i = x_begin; i <= x_end; i++)
            {
                for (int j = y_begin; j <= y_end; j++)
                {
                    for (int k = z_begin; k <= z_end; k++)
                    {
#if 0
                        Grid cell(grid.origin + Vector(i * size, j * size, k * size), Vector(size, size, size));
                        if (t->intersectWithGrid(cell))
                        {
                            grid.get(i, j, k).push_back(t);
                        }
#else
                        // Use a simple way to construct the grid, which is about 8 times faster.
                        // However, grid traversing is 20% slower
                        grid.get(i, j, k).push_back(t);
#endif
                    }
                }
            }
        }
    }

#if 0 // Debug output
    for (int i = 0; i < grid.xLength; i++)
    {
        for (int j = 0; j < grid.yLength; j++)
        {
            Utils::DbgPrint("\n[%d, %d]", i, j);
            for (int k = 0; k < grid.zLength; k++)
            {
                Utils::DbgPrint(" %d", grid.get(i, j, k).size());
            }
        }
    }
#endif
}

void Tunnel::initKdTree()
{
    Utils::PrintTickCount("Initialize k-d tree");

    root = new KdNode();

    // Initialize the geometry list
    std::vector<Triangle *> list;
    list.reserve(surface.size() * surface[0].size());

    for (unsigned int i = 0; i < surface.size(); i++)
    {
        for (unsigned int j = 0; j < surface[i].size(); j++)
        {
            // Only triangles are supported temporarily
            list.push_back((Triangle *)surface[i][j]);
        }
    }

    // Init the boundry of the root node
    float min_x = FLT_MAX, min_y = FLT_MAX, min_z = FLT_MAX;
    float max_x = -FLT_MAX, max_y = -FLT_MAX, max_z = -FLT_MAX;

    for (unsigned int i = 0; i < surface.size(); i++)
    {
        for (unsigned int j = 0; j < surface[i].size(); j++)
        {
            Point min, max;
            Triangle *t = surface[i][j];
            t->getBoundingBox(min, max);

            min_x = std::min(min_x, min.x);
            min_y = std::min(min_y, min.y);
            min_z = std::min(min_z, min.z);

            max_x = std::max(max_x, max.x);
            max_y = std::max(max_y, max.y);
            max_z = std::max(max_z, max.z);
        }
    }

    root->min = Point(min_x, min_y, min_z);
    root->max = Point(max_x, max_y, max_z);

    // Build the tree
    int leaves = 0;
    int leafElements = 0;
    buildKdTree(root, list, 0, leaves, leafElements);
    Utils::DbgPrint("Total leaves: %d\r\n", leaves);
    Utils::DbgPrint("Average Leaf Size: %d\r\n", leafElements / leaves);
}

bool cmpTriangleXAxis(const Triangle *t1, const Triangle *t2)
{
    float x1 = (t1->a.x + t1->b.x + t1->c.x) / 3;
    float x2 = (t2->a.x + t2->b.x + t2->c.x) / 3;
    return x1 < x2;
}

bool cmpTriangleYAxis(const Triangle *t1, const Triangle *t2)
{
    float y1 = (t1->a.y + t1->b.y + t1->c.y) / 3;
    float y2 = (t2->a.y + t2->b.y + t2->c.y) / 3;
    return y1 < y2;
}

bool cmpTriangleZAxis(const Triangle *t1, const Triangle *t2)
{
    float z1 = (t1->a.z + t1->b.z + t1->c.z) / 3;
    float z2 = (t2->a.z + t2->b.z + t2->c.z) / 3;
    return z1 < z2;
}

bool cmpKdEvent(const Tunnel::KdEvent a, const Tunnel::KdEvent b)
{
    return (a.position < b.position) || 
        ((a.position == b.position) && (int)a.type < (int)b.type);
}

void Tunnel::buildKdTree(KdNode *node, std::vector<Triangle *> &list, int depth, int &numLeaves, int &leafElements)
{
#define DUMP_TREE 0

    if (list.size() <= 8 || depth > 18) // This should be leaf node
    {
#if DUMP_TREE
        Utils::SysDbgPrint("%02d ", depth);
        for (int i = 0; i < depth; i++)
        {
            Utils::SysDbgPrint("  ");
        }
        Utils::SysDbgPrint("Leaf (%d)\n", list.size());
#endif
        node->axis = NoAxis;
        node->splitPlane = 0.0f; // whatever
        node->left = NULL;
        node->right = NULL;
        node->list.assign(list.begin(), list.end());
        numLeaves += 1;
        leafElements += list.size();
        return;
    }

    // Split the triangle list
    int axis;
    float median;
    float sah;

    if (algorithm == KdTreeStandard)
    {
        // Select axis based on depth so that axis cycles through all valid values
        // use order x -> y -> z -> x ...
        axis = depth % 3;
        median = split(node, axis, list); // the list may be sorted in split(), 
                                          // but no element would be deleted
    }
    else // KdTreeSAH
    {
        median = splitSAH(node, list, axis, sah);

        // Automatic termination
        if (sah > 1.5f * list.size())
        {
            node->axis = NoAxis;
            node->splitPlane = 0.0f; // whatever
            node->left = NULL;
            node->right = NULL;
            node->list.assign(list.begin(), list.end());
            numLeaves += 1;
            leafElements += list.size();
            return;
        }
    }

    // Create node and construct subtrees
    node->axis = (Axes)axis;
    node->splitPlane = median;
    node->left = new KdNode();
    node->right = new KdNode();
    // node->list remains empty

    node->left->min = node->min;
    node->left->max = node->max;
    node->right->min = node->min;
    node->right->max = node->max;

    node->left->max[axis] = median;
    node->right->min[axis] = median;

#if DUMP_TREE
    Utils::SysDbgPrint("%02d ", depth);
    for (int i = 0; i < depth; i++)
    {
        Utils::SysDbgPrint("  ");
    }
    if (axis == 0)
        Utils::SysDbgPrint("X (%d) split_plane = %.2f\n", list.size(), median);
    else if (axis == 1)
        Utils::SysDbgPrint("Y (%d) split_plane = %.2f\n", list.size(), median);
    else // axis == 2
        Utils::SysDbgPrint("Z (%d) split_plane = %.2f\n", list.size(), median);
#endif

    std::vector<Triangle *> leftPart;
    std::vector<Triangle *> rightPart;

    for (unsigned int i = 0; i < list.size(); i++)
    {
        if (list[i]->a[axis] < median ||
            list[i]->b[axis] < median ||
            list[i]->c[axis] < median)
        {
            leftPart.push_back(list[i]);
        }

        if (list[i]->a[axis] >= median ||
            list[i]->b[axis] >= median ||
            list[i]->c[axis] >= median)
        {
            rightPart.push_back(list[i]);
        }
    }

    buildKdTree(node->left, leftPart, depth + 1, numLeaves, leafElements);
    buildKdTree(node->right, rightPart, depth + 1, numLeaves, leafElements);
}

void Tunnel::deleteTree(KdNode *node)
{
    if (node->left != NULL)
        deleteTree(node->left);
    if (node->right != NULL)
        deleteTree(node->right);
    delete node;
}

float Tunnel::split(KdNode *node, int axis, std::vector<Triangle *> &list)
{
    // simple split: sort the triangle by barycenter, and select the mid value in the sorted list
    if (axis == XAxis)
    {
        std::sort(list.begin(), list.end(), cmpTriangleXAxis);
    }
    else if (axis == YAxis)
    {
        std::sort(list.begin(), list.end(), cmpTriangleYAxis);
    }
    else // ZAxis
    {
        std::sort(list.begin(), list.end(), cmpTriangleZAxis);
    }

    // Select the median point
    Triangle *t = list[list.size() / 2];
    float median = (t->a[axis] + t->b[axis] + t->c[axis]) / 3;
    return median;
}

float Tunnel::splitSAH(KdNode *node, std::vector<Triangle *> &list, int &bestAxis, float &minSAH)
{
    minSAH = FLT_MAX;
    float minPosition;

    for (int axis = 0; axis < 3; axis++)
    {
        // Generate event list
        std::vector<KdEvent> events;

        for (unsigned int i = 0; i < list.size(); i++)
        {
            // Clip triangle to box
            Point min, max;
            list[i]->getBoundingBox(min, max);

            if (min[axis] == max[axis])
            {
                events.push_back(KdEvent(list[i], min[axis], Planar));
            }
            else
            {
                events.push_back(KdEvent(list[i], min[axis], Start));
                events.push_back(KdEvent(list[i], max[axis], End));
            }
        }

        // Sort event list
        std::sort(events.begin(), events.end(), cmpKdEvent);

        // Sweep all candidate split planes
        int NL = 0;
        int NP = 0;
        int NR = list.size();

        for (unsigned int i = 0; i < events.size(); )
        {
            float position = events[i].position;
            int PS = 0; // p(+) p_start
            int PE = 0; // p(-) p_end
            int PP = 0; // p(|) p_planar

            while (i < events.size() && 
                events[i].position == position && events[i].type == End)
            {
                PE += 1; i += 1;
            }

            while (i < events.size() && 
                events[i].position == position && events[i].type == Planar)
            {
                PP += 1; i += 1;
            }

            while (i < events.size() && 
                events[i].position == position && events[i].type == Start)
            {
                PS += 1; i += 1;
            }

            // Move plane onto p
            NP = PP; NR -= PP; NR -= PE;
            
            // Calculate SAH
            // KT: Traversal constant (1)
            // KI: Intersection constant (1.5)
            // SA: Surface area (total)
            // SAL: Surface area (left)
            // SAR: Surface area (right)
            // Cost = KT + KI * ((SAL / SA) * (NL + NP) + (SAR / SA) * NR)
            int nextAxis = (axis + 1) % 3; // x -> y -> z -> x ...
            int prevAxis = (axis + 2) % 3; // z -> y -> x -> z ...
            Vector boxSize = Vector(node->min, node->max);

            float width = node->max[axis] - node->min[axis];
            float leftWidth = position - node->min[axis];
            float rightWidth = node->max[axis] - position;
            float height = boxSize[nextAxis];
            float depth = boxSize[prevAxis];

            float SAL = leftWidth * height + leftWidth * depth + height * depth;
            float SAR = rightWidth * height + rightWidth * depth + height * depth;
            float SA = width * height + width * depth + height * depth;

            float SAH = 1 + 1.5f * ((SAL / SA) * NL + SAR / SA * (NR + NP));
            if (SAH < minSAH)
            {
                minSAH = SAH;
                minPosition = position;
                bestAxis = axis;
            }

            NL += PS; NL += PP; NP = 0;
        }
    }

    return minPosition;
}

IntersectResult Tunnel::linearIntersect(Ray &ray)
{
    float minDistance = FLT_MAX;
    IntersectResult minResult(false);

    for (unsigned int segment = 0; segment < surface.size(); segment++)
    {
        for (unsigned int i = 0; i < surface[segment].size(); i++)
        {
            IntersectResult result = surface[segment][i]->intersect(ray);
            if (result.hit && (result.distance < minDistance)) 
            {
                minDistance = result.distance;
                minResult = result;
            }
        }
    }
    return minResult;
}

void Tunnel::getIndexInGrid(const Point &p, int &i, int &j, int&k)
{
    i = (int)((p.x - grid.origin.x) / grid.cellSizeX);
    j = (int)((p.y - grid.origin.y) / grid.cellSizeY);
    k = (int)((p.z - grid.origin.z) / grid.cellSizeZ);
    if (i < 0) i = 0;
    if (j < 0) j = 0;
    if (k < 0) k = 0;
    if (i > grid.xLength - 1) i = grid.xLength - 1;
    if (j > grid.yLength - 1) j = grid.yLength - 1;
    if (k > grid.zLength - 1) k = grid.zLength - 1;
}

IntersectResult Tunnel::gridIntersect(Ray &ray)
{
    Point near = grid.origin;
    Point far = grid.origin + Vector(
        grid.cellSizeX * grid.xLength, 
        grid.cellSizeY * grid.yLength, 
        grid.cellSizeZ * grid.zLength);

    // Current traversal state
    int cur_i, cur_j, cur_k; // the index in the grid
    float cur_d; // distance along the ray
    Point cur_p; // position

    // Is the origin of the ray outside of the grid?
    if (ray.origin.x < near.x || ray.origin.x > far.x ||
        ray.origin.y < near.y || ray.origin.y > far.y ||
        ray.origin.z < near.z || ray.origin.z > far.z)
    {
        float entryDistance, exitDistance;
        Grid sceneBox(grid.origin, Vector(
            grid.cellSizeX * grid.xLength, 
            grid.cellSizeY * grid.yLength, 
            grid.cellSizeZ * grid.zLength));

        if (sceneBox.intersect(ray, entryDistance, exitDistance))
        {
            // Advance the ray to a grid boundary
            cur_d = entryDistance;
            cur_p = ray.getPoint(entryDistance);
            getIndexInGrid(cur_p, cur_i, cur_j, cur_k);
        }
        else
        {
            return IntersectResult(false);
        }
    }
    else // the origin of the ray is in the grid
    {
        cur_p = ray.origin;
        cur_d = 0;
        getIndexInGrid(ray.origin, cur_i, cur_j, cur_k);
    }

    // Start traversing the grid
    while (true)
    {
        // See if the ray intersects with some triangle in the current cell
        std::vector<Triangle *> &list = grid.get(cur_i, cur_j, cur_k);
        IntersectResult minResult(false);
        float minDistance = FLT_MAX;

        for (unsigned int i = 0; i < list.size(); i++)
        {
            IntersectResult result = list[i]->intersect(ray);
            if (result.hit && result.distance < minDistance)
            {
                minResult = result;
                minDistance = result.distance;
            }
        }

        if (minResult.hit)
            return minResult;

        // Advance to the next cell with the 3D version of the DDA algorithm
        // http://en.wikipedia.org/wiki/Digital_differential_analyzer_(graphics_algorithm)
        Point p1 = grid.origin + Vector(
            cur_i * grid.cellSizeX, 
            cur_j * grid.cellSizeY, 
            cur_k * grid.cellSizeZ);
        Point p2 = grid.origin + Vector(
            (cur_i + 1) * grid.cellSizeX, 
            (cur_j + 1) * grid.cellSizeY, 
            (cur_k + 1) * grid.cellSizeZ);

        float dx = 0; // this is not delta_x, but is distance_on_x_axis
        float dy = 0; 
        float dz = 0;
        int di = 0;
        int dj = 0;
        int dk = 0;

        if (ray.direction.x > 0) // Plane x = p2.x
        {
            float cos_theta = ray.direction.x;
            dx = (p2.x - cur_p.x) / cos_theta;
            di = 1;
        }
        else // Plane x = p1.x
        {
            float cos_theta = -ray.direction.x;
            dx = (cur_p.x - p1.x) / cos_theta;
            di = -1;
        }

        if (ray.direction.y > 0) // Plane y = p2.y
        {
            float cos_theta = ray.direction.y;
            dy = (p2.y - cur_p.y) / cos_theta;
            dj = 1;
        }
        else // Plane y = p1.y
        {
            float cos_theta = -ray.direction.y;
            dy = (cur_p.y - p1.y) / cos_theta;
            dj = -1;
        }

        if (ray.direction.z > 0) // Plane z = p2.z
        {
            float cos_theta = ray.direction.z;
            dz = (p2.z - cur_p.z) / cos_theta;
            dk = 1;
        }
        else
        {
            float cos_theta = -ray.direction.z;
            dz = (cur_p.z - p1.z) / cos_theta;
            dk = -1;
        }

        // Advance
        if (dx < dy && dx < dz) // min = dx
        {
            cur_i += di;
            cur_d += dx;
            cur_p = ray.getPoint(cur_d);
        }
        else if (dy < dz) // min = dy
        {
            cur_j += dj;
            cur_d += dy;
            cur_p = ray.getPoint(cur_d);
        }
        else // min = dz
        {
            cur_k += dk;
            cur_d += dz;
            cur_p = ray.getPoint(cur_d);
        }

        // Leave the grid
        if (cur_i < 0 || cur_i > grid.xLength - 1 ||
            cur_j < 0 || cur_j > grid.yLength - 1 ||
            cur_k < 0 || cur_k > grid.zLength - 1)
        {
            break;
        }
    }

    return IntersectResult(false);
}

IntersectResult Tunnel::fastIntersect(Ray &ray)
{
    RayContext &context = ray.context;

    Ray advRay = ray; // the advanced ray
    float distance; // the advanced distance

    if (!context.inTunnel)
    {
        Point newOrigin;
        Vector newDir;

        // into the entrance of the tunnel
        if (intersectWithPolygon(ray, 0, newOrigin, newDir, distance) && ray.direction.dot(nvs[0]) > 0)
        {
            context.inTunnel = true; // we know it will gets into the tunnel
            context.segment = 0;
            advRay.origin = ray.getPoint(distance);
            advRay.direction = ray.direction;
        }
        else if (intersectWithPolygon(ray, path.size() - 1, newOrigin, newDir, distance) && 
            ray.direction.dot(nvs[path.size() - 1]) < 0) // into the exit of the tunnel
        {
            context.inTunnel = true;
            context.segment = path.size() - 2;
            advRay.origin = ray.getPoint(distance);
            advRay.direction = ray.direction;
        }
        else // the origin of the ray is not in the tunnel, and it does not gets into the tunnel
        {
            return linearIntersect(ray);
        }
    }

    if (context.inTunnel)
    {
        int N = path.size() - 1; // a path with with N segments has N + 1 nodes (range: 0 to N)
        int begin = context.segment; // the current segment (range: 0 to N - 1)
        enum RayDir dir = ray.direction.dot(nvs[begin]) > 0 ? Forward : Backward;

        if (dir == Forward)
        {
            for (int i = begin + 1; i <= N; i++)
            {
                Point newOrigin;
                Vector newDir;

                if (!intersectWithPolygon(advRay, i, newOrigin, newDir, distance)) // intersect with wall
                {
                    if (algorithm == ConvexSimple) // linear search in the current segment
                    {
                        for (unsigned int j = 0; j < surface[i - 1].size(); j++)
                        {
                            IntersectResult result = surface[i - 1][j]->intersect(ray);
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
                        int index = (int)(99.0f * y / height + 0.5f);
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

                            IntersectResult result = surface[i - 1][segmentIndex]->intersect(ray);
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
            Utils::DbgPrint("Backward");
#if 0
            for (int i = begin; i >= 0; i--)
            {
                Point newOrigin;
                Vector newDir;

                if (!intersectWithPolygon(ray, i, newOrigin, newDir, distance)) // intersect with wall
                {
                    if (fabs(newDir.x) > 0.707f) // use y axis
                    {
                        float y = newOrigin.y - newOrigin.x * newDir.y / newDir.x;
                        int index = (int)(99.0f * y / height + 0.5f);
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

                            IntersectResult result = surface[i - 1][segmentIndex]->intersect(ray);
                            if (result.hit)
                            {
                                Utils::DbgPrint("Intersect with triangle %d / %d\n", 
                                    j, intersectionTableYAxis[index][iAngle].size());
                                context->segment = i - 1;
                                return result;
                            }
                        }

                        // As float point numbers are not accurate by nature, it's not a problem
                        // when it gets here
                    }
                    else // use a line which parallel to x axis
                    {
                        float x = newOrigin.x + (height / 2 - newOrigin.y) * (newDir.x / newDir.y);
                        int index = (int)((x / width + 0.5f) * 99 + 0.5f);
                        index = std::max(0, index);
                        index = std::min(99, index);

                        float fAngle = atan2(newDir.y, newDir.x);
                        fAngle = (fAngle < 0) ? fAngle + PI * 2 : fAngle;
                        int iAngle = (int)(fAngle / PI * 180.0f);
                        iAngle = std::max(0, iAngle);
                        iAngle = std::min(359, iAngle);

                        for (unsigned int j = 0; j < intersectionTableXAxis[index][iAngle].size(); j++)
                        {
                            int segmentIndex = intersectionTableXAxis[index][iAngle][j];

                            IntersectResult result = surface[i - 1][segmentIndex]->intersect(ray);
                            if (result.hit)
                            {
                                Utils::DbgPrint("Intersect with triangle %d / %d\n", 
                                    j, intersectionTableXAxis[index][iAngle].size());
                                context->segment = i - 1;
                                return result;
                            }
                        }
                    }

                    advRay.origin = advRay.getPoint(distance);
                }
                else
                {
                    advRay.origin = advRay.getPoint(distance);
                }
            }

            // The ray intersects with all the polygons on the way
            context->inTunnel = false;
            return IntersectResult(false);
#endif
        }
    }

    // Just to avoid warnings. It's impossible to get here
    return IntersectResult(false);
}

IntersectResult Tunnel::kdTreeIntersect(Ray &ray)
{
    // Based on the C-pseudo code in page 157
    float a; // entry signed distance
    float b; // exit signed distance
    float t; // signed distance to the splitting plane

    // Intersect ray with sceneBox, find the entry and exit signed distance
    Grid sceneBox(root->min, root->max);
    if (!sceneBox.intersect(ray, a, b))
        return IntersectResult(false);

    // Stack required for traversal to store far children
    StackElem stack[50];

    // Pointers to the far child node and current node
    KdNode *farChild;
    KdNode *currNode;
    currNode = root;

    // Setup initial entry point
    int enPt = 0;
    stack[enPt].t = a; // Set the signed distance

    // Distinguish between internal and external origin
    if (a >= 0) // a ray with external origin
        stack[enPt].pb = ray.origin + ray.direction * a;
    else // a ray with internal origin
        stack[enPt].pb = ray.origin;

    // Setup initial exit point in the stack
    int exPt = 1;
    stack[exPt].t = b;
    stack[exPt].pb = ray.origin + ray.direction * b;
    stack[exPt].node = NULL; // Termination flag

    // Loop, traverse through the whole kd-tree
    while (currNode != NULL)
    {
        // Loop until a leaf is found
        while (currNode->axis != NoAxis)
        {
            // Retrive position of splitting plane
            float splitVal = currNode->splitPlane;

            // The current axis
            int axis = (int)currNode->axis;
            int nextAxis = (axis + 1) % 3; // x -> y -> z -> x ...
            int prevAxis = (axis + 2) % 3; // z -> y -> x -> z ...

            if (stack[enPt].pb[axis] <= splitVal)
            {
                // Case N1, N2, N3, P5, Z2 and Z3
                if (stack[exPt].pb[axis] <= splitVal)
                {
                    currNode = currNode->left;
                    continue;
                }

                // Case Z1
                if (stack[exPt].pb[axis] == splitVal)
                {
                    currNode = currNode->right;
                    continue;
                }

                // Case N4
                farChild = currNode->right;
                currNode = currNode->left;
            }
            else // stack[enPt].pb[axis] > splitVal
            {
                // Case P1, P2, P3 and N5
                if (splitVal < stack[exPt].pb[axis])
                {
                    currNode = currNode->right;
                    continue;
                }

                // Case P4
                farChild = currNode->left;
                currNode = currNode->right;
            }
            // Case P4 or N4 (traverse both children)

            // Signed distance to the splitting plane
            t = (splitVal - ray.origin[axis]) / ray.direction[axis];

            // Setup the new exit point
            int tmp = exPt++;
            if (exPt == enPt)
                exPt += 1;

            // Push values onto the stack
            stack[exPt].prev = tmp;
            stack[exPt].t = t;
            stack[exPt].node = farChild;
            stack[exPt].pb[axis] = splitVal;
            stack[exPt].pb[nextAxis] = ray.origin[nextAxis] + t * ray.direction[nextAxis];
            stack[exPt].pb[prevAxis] = ray.origin[prevAxis] + t * ray.direction[prevAxis];
        }

        // Current node is the leaf, empty or full
        float minDistance = FLT_MAX;
        IntersectResult minResult(false);

        for (unsigned int i = 0; i < currNode->list.size(); i++)
        {
            IntersectResult result = currNode->list[i]->intersect(ray);
            if (result.hit && 
                result.distance >= stack[enPt].t - 0.001f && 
                result.distance <= stack[exPt].t + 0.001f &&
                result.distance < minDistance)
            {
                minResult = result;
                minDistance = result.distance;
            }
        }
        
        if (minResult.hit)
            return minResult;

        // Pop from stack
        enPt = exPt; // The signed distance intervals are adjacent

        // Retrieve the pointer to the next node,
        // it is possible that ray traversal terminates
        currNode = stack[exPt].node;

        exPt = stack[enPt].prev;
    }

    // currNode = NULL, ray leaves the scene
    return IntersectResult(false);
}

IntersectResult Tunnel::intersect(Ray &ray)
{
    if (algorithm == RegularGrid || algorithm == FlatGrid)
        return gridIntersect(ray);
    else if (algorithm == KdTreeSAH || algorithm == KdTreeStandard)
        return kdTreeIntersect(ray);
    else if (algorithm == Convex || algorithm == ConvexSimple)
        return fastIntersect(ray);
    else
        return linearIntersect(ray);
}
