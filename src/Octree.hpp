#ifndef OCTREE_H
#define OCTREE_H

#include "OctreeNode.hpp"
#include <map>
Vertex maxVertex(const std::vector<Vertex>& vertices);
Vertex minVertex(const std::vector<Vertex>& vertices);

class Octree {
public:
    OctreeNode* root;
    int maxDepth;
    std::vector<Vertex> initialVertices;
    std::vector<Face> initialFaces;

    std::vector<Vertex> voxelizedVertices;
    std::vector<Face> voxelizedFaces;


    Octree(Vertex min, Vertex max, int maxDepth, 
           const std::vector<Vertex>& vertices, 
           const std::vector<Face>& faces);
    std::map<int,int> nodeCount;
    std::map<int,int> skippedNodeCount;
    void build();
private:
    void divide(OctreeNode* node);
    void combine(const OctreeNode* node);
    bool intersect(OctreeNode* node, Face f);
    void collectVoxels(const OctreeNode* node);
};
#endif