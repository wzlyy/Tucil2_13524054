#ifndef OCTREENODE_H
#define OCTREENODE_H

#include "Vertex.hpp"
#include <vector>
class OctreeNode {
public:
    Vertex min;  
    Vertex max;
    int depth;
    bool isLeaf;
    
    OctreeNode* children[8];
    std::vector<int> intersectFaces;        
    OctreeNode(Vertex min, Vertex max, int depth)
        : min(min), max(max), depth(depth), isLeaf(false)
    {
        for (int i = 0; i < 8; i++)
            children[i] = nullptr;
    }
};

#endif