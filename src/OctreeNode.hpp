#ifndef OCTREENODE_H
#define OCTREENODE_H

#include "Vertex.hpp"
#include <vector>

struct OctreeNode
{
    Vertex min;
    Vertex max;
    int depth;
    bool isLeaf;
    OctreeNode* children[8];
    std::vector<int> intersectFaces;
};

OctreeNode* createOctreeNode(Vertex min, Vertex max, int depth);

#endif