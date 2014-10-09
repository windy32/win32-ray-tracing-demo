#ifndef KD_TREE_ACC_H
#define KD_TREE_ACC_H

#include "Accelerator.h"

class KdTreeAcc : public Accelerator
{
private:
    enum Axes { XAxis, YAxis, ZAxis, NoAxis }; // "NoAxis" denotes a leaf
    enum KdEventType { End, Planar, Start };

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
    KdNode *root;

    struct StackElem
    {
        KdNode *node;  // pointer of far child
        float t;           // the entry / exit signed distance
        Point pb;          // the coordinates of entry / exit point
        int prev;          // the pointer to the previous stack item
    };

public: // should be exposed to the compare functions for sorting
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
    void buildKdTree(KdNode *node, std::vector<Triangle *> &list, int depth, int &numLeaves, int &leafElements);
    void deleteTree(KdNode *node);
    float split(KdNode *node, int axis, std::vector<Triangle *> &list);
    float splitSAH(KdNode *node, std::vector<Triangle *> &list, int &bestAxis, float &minSAH);

public:
    KdTreeAcc(Tunnel *tunnel) : Accelerator(tunnel) {}
    ~KdTreeAcc();
    virtual void init();
    virtual IntersectResult intersect(Ray &ray);
};

#endif
