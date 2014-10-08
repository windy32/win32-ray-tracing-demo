#ifndef TUNNEL_H
#define TUNNEL_H

#include <vector>
#include "Triangle.h"
#include "Polygon.h"

class Tunnel : public Geometry
{
public:
    Polygon crossSection; // the cross section at the origin
    std::vector<Point> path;
    std::vector<std::vector<Triangle *>> surface;
    std::vector<Vector> nvs; // normal vectors of the polygons

    // The algorithm used in tunnel-ray intersection
    enum Algorithm 
    { 
        Linear = 0,
        RegularGrid = 1, FlatGrid = 2, 
        KdTreeStandard = 3, KdTreeSAH = 4, 
        Convex = 5, ConvexSimple = 6 // the same order with the combobox items
    } algorithm;

    // whether a point on the cross section is a critical point
    enum { FLAG_NONE = 0, FLAG_CRITICAL = 1 };

    // Size of the bounding rectangle of the tunnel intersection
    float height;
    float width;

private: // Convex polygon acceleration
    enum RayDir { Forward, Backward };
    enum { CONVEX_TABLE_SIZE = 100 };

    struct EdgeParam // A * x + B * y + C > 0
    {
        float A, B, C;
    };
    std::vector<EdgeParam> edgeParams;

    // decide whether a point is in a convex polygon
    enum IntersectionTableResult { Hit, Partial, Miss };
    IntersectionTableResult intersectionTable[CONVEX_TABLE_SIZE][CONVEX_TABLE_SIZE];

    struct EdgeRange
    {
        short start;
        short end;
    };
    EdgeRange edgeRangeTable[CONVEX_TABLE_SIZE][CONVEX_TABLE_SIZE];

private: // Convex polyhedron acceleration
    std::vector<int> intersectionTableYAxis[100][360];
    //short intersectionTableFull[100][100][360][20]; // 144 MB

private: // Grid Acceleration 

    struct RegularGrid
    {
        Point origin;
        float cellSizeX;
        float cellSizeY;
        float cellSizeZ;
        int xLength;
        int yLength;
        int zLength;

        std::vector<std::vector<Triangle *>> data;

        std::vector<Triangle *> &get(int x, int y, int z)
        {
            return data[(x * yLength + y) * zLength + z];
        }
    } grid;

private: // k-d tree acceleration

    // The recursive ray traversal algorithm TA_rec_B for the k-d tree
    // "Heuristic Ray Shooting Algorithms" by Vlastimil Vavran (Appendix C)

    enum Axes { XAxis, YAxis, ZAxis, NoAxis }; // "NoAxis" denotes a leaf
    struct KdNode
    {
        std::vector<Geometry *> list; // list of enclosed objects
        KdNode *left;  // pointer to the left child
        KdNode *right; // pointer to the right child
        Axes axis;         // orientation of the splitting plane
        float splitPlane;  // position of the splitting plane

        Point min;
        Point max;
    };
    struct StackElem
    {
        KdNode *node;  // pointer of far child
        float t;           // the entry / exit signed distance
        Point pb;          // the coordinates of entry / exit point
        int prev;          // the pointer to the previous stack item
    };
    KdNode *root;

public:
    enum KdEventType { End, Planar, Start };
    struct KdEvent
    {
        Triangle *triangle;
        float position;
        KdEventType type;

        KdEvent(Triangle *triangle, float position, KdEventType type)
        {
            this->triangle = triangle;
            this->position = position;
            this->type = type;
        }
    };

private:
    bool intersectWithPolygon(Ray &ray, int index, Point &origin, Vector &dir, float &distance);
    bool intersectWithPolygonAtOrigin(Ray &ray, float &distance);
    bool inPolygon(const Point &p, int begin, int end);
    IntersectionTableResult calcCellStatus(
        const Point &p1, const Point &p2, const Point &p3, const Point &p4, short &minIndex, short &maxIndex);
    void getIndexInGrid(const Point &p, int &i, int &j, int&k);

    IntersectResult linearIntersect(Ray &ray);
    IntersectResult gridIntersect(Ray &ray);
    IntersectResult fastIntersect(Ray &ray);
    //IntersectResult kdTreeLinearIntersect(Ray &ray);
    IntersectResult kdTreeIntersect(Ray &ray);

    void initConvex();
    void initGrid();
    void initKdTree();
    void buildKdTree(KdNode *node, std::vector<Triangle *> &list, int depth, int &numLeaves, int &leafElements);
    void deleteTree(KdNode *node);
    float split(KdNode *node, int axis, std::vector<Triangle *> &list);
    float splitSAH(KdNode *node, std::vector<Triangle *> &list, int &bestAxis, float &minSAH);

public:
    Tunnel();
    ~Tunnel();
    void init();
    virtual IntersectResult intersect(Ray &ray);
};

#endif
