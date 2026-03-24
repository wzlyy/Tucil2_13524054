#ifndef OCTREE_H
#define OCTREE_H

#include "OctreeNode.hpp"

class Octree {
public:
    OctreeNode* root;
    int maxDepth;

    Octree(Vertex min, Vertex max, int maxDepth);
    void build();
private:
    void divide(OctreeNode* node);
    bool intersect(OctreeNode* node, Face faces);
};
#endif