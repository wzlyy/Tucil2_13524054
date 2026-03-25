#include "Octree.hpp"
#include "OctreeNode.hpp"


#define MAX_CONCURRENT_DEPTH 2

OctreeNode* createOctreeNode(Vertex min, Vertex max, int depth)
{
    OctreeNode* node = new OctreeNode;

    node->min = min;
    node->max = max;
    node->depth = depth;
    node->isLeaf = false;

    for (int i = 0; i < 8; i++) {
        node->children[i] = nullptr;
    }

    return node;
}

Vertex maxVertex(const std::vector<Vertex>& vertices){
    Vertex maxVertex = vertices[0];

    for (const auto& v : vertices) {
        if (v.x > maxVertex.x) maxVertex.x = v.x;
        if (v.y > maxVertex.y) maxVertex.y = v.y;
        if (v.z > maxVertex.z) maxVertex.z = v.z;
    }
    return maxVertex;
}
Vertex minVertex(const std::vector<Vertex>& vertices){
    Vertex minVertex = vertices[0];
    for (const auto& v : vertices) {
        if (v.x < minVertex.x) minVertex.x = v.x;
        if (v.y < minVertex.y) minVertex.y = v.y;
        if (v.z < minVertex.z) minVertex.z = v.z;

    }
    return minVertex;
}
Octree::Octree(Vertex min, Vertex max, int maxDepth,
            const std::vector<Vertex>& vertices,
            const std::vector<Face>& faces) 
{
    root = createOctreeNode(min, max, 0);
    this->maxDepth = maxDepth;
    this->initialVertices = vertices;
    this->initialFaces = faces;
    for (int i = 0; i < (int)faces.size(); i++)
        root->intersectFaces.push_back(i);
}
void Octree::build(){
    divide(root);
    combine(root);
}
bool Octree::intersect(OctreeNode* node, Face f) {
    Vertex v0 = initialVertices[f.a]; 
    Vertex v1 = initialVertices[f.b];
    Vertex v2 = initialVertices[f.c];

    float minX = node->min.x, minY = node->min.y, minZ = node->min.z;
    float maxX = node->max.x, maxY = node->max.y, maxZ = node->max.z;

    auto inside = [&](Vertex v) {
        return (v.x >= minX && v.x <= maxX &&
                v.y >= minY && v.y <= maxY &&
                v.z >= minZ && v.z <= maxZ);
    };

    if (inside(v0) || inside(v1) || inside(v2))
        return true;

    
    Vertex triMin, triMax;

    triMin.x = std::min(v0.x, std::min(v1.x, v2.x));
    triMin.y = std::min(v0.y, std::min(v1.y, v2.y));
    triMin.z = std::min(v0.z, std::min(v1.z, v2.z));

    triMax.x = std::max(v0.x, std::max(v1.x, v2.x));
    triMax.y = std::max(v0.y, std::max(v1.y, v2.y));
    triMax.z = std::max(v0.z, std::max(v1.z, v2.z));

    bool overlap =
        (node->min.x <= triMax.x && node->max.x >= triMin.x) &&
        (node->min.y <= triMax.y && node->max.y >= triMin.y) &&
        (node->min.z <= triMax.z && node->max.z >= triMin.z);

    return overlap;
}
void Octree::divide(OctreeNode* node){
    if (node->depth >= maxDepth || node->intersectFaces.empty()) {
        node->isLeaf = true;
        return;
    }

    Vertex mid{
        (node->min.x + node->max.x) / 2,
        (node->min.y + node->max.y) / 2,
        (node->min.z + node->max.z) / 2
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 8; i++) {
        Vertex minChild;
        Vertex maxChild;

        switch (i)
        {
            case 0:
                minChild = Vertex{node->min.x, node->min.y, node->min.z};
                maxChild = Vertex{mid.x, mid.y, mid.z};
                break;

            case 1:
                minChild = Vertex{mid.x, node->min.y, node->min.z};
                maxChild = Vertex{node->max.x, mid.y, mid.z};
                break;

            case 2:
                minChild = Vertex{node->min.x, mid.y, node->min.z};
                maxChild = Vertex{mid.x, node->max.y, mid.z};
                break;

            case 3:
                minChild = Vertex{mid.x, mid.y, node->min.z};
                maxChild = Vertex{node->max.x, node->max.y, mid.z};
                break;

            case 4:
                minChild = Vertex{node->min.x, node->min.y, mid.z};
                maxChild = Vertex{mid.x, mid.y, node->max.z};
                break;

            case 5:
                minChild = Vertex{mid.x, node->min.y, mid.z};
                maxChild = Vertex{node->max.x, mid.y, node->max.z};
                break;

            case 6:
                minChild = Vertex{node->min.x, mid.y, mid.z};
                maxChild = Vertex{mid.x, node->max.y, node->max.z};
                break;

            case 7:
                minChild = Vertex{mid.x, mid.y, mid.z};
                maxChild = Vertex{node->max.x, node->max.y, node->max.z};
                break;
        }

        node->children[i] = createOctreeNode(minChild, maxChild, node->depth + 1);

        for (int idx : node->intersectFaces) {
            if (intersect(node->children[i], initialFaces[idx])) {
                node->children[i]->intersectFaces.push_back(idx);
            }
        }
        if(node->children[i]->intersectFaces.empty()) {
            std::lock_guard<std::mutex> lock(mtx);
            skippedNodeCount[node->children[i]->depth]++;
        }
        if(node->depth <= MAX_CONCURRENT_DEPTH && activeThreads < maxThreads){
            activeThreads++;

            threads.emplace_back([this, i, node]() {
                this->divide(node->children[i]);
                activeThreads--;
            });
        }
        else{
            divide(node->children[i]);
        }
    }
    for (auto &t : threads) t.join();
}
void Octree::combine(const OctreeNode* node) 
{
    {
        std::lock_guard<std::mutex> lock(mtx);
        nodeCount[node->depth]++;
    }
    if (node->isLeaf && !node->intersectFaces.empty()) {
            collectVoxels(node);   
            return;
    }
    for (int i = 0; i < 8; i++) {
            if (node->children[i])
                combine(node->children[i]);
    }
}
void Octree::collectVoxels(const OctreeNode* node){
    Vertex min = node->min;
    Vertex max = node->max;

    int startIdx = (int)voxelizedVertices.size(); 

    // 8 cube vertices
    voxelizedVertices.push_back({min.x, min.y, min.z}); // 0
    voxelizedVertices.push_back({max.x, min.y, min.z}); // 1
    voxelizedVertices.push_back({max.x, max.y, min.z}); // 2
    voxelizedVertices.push_back({min.x, max.y, min.z}); // 3
    voxelizedVertices.push_back({min.x, min.y, max.z}); // 4
    voxelizedVertices.push_back({max.x, min.y, max.z}); // 5
    voxelizedVertices.push_back({max.x, max.y, max.z}); // 6
    voxelizedVertices.push_back({min.x, max.y, max.z}); // 7

    // 12 triangle faces
    voxelizedFaces.push_back({startIdx + 0, startIdx + 1, startIdx + 2}); // bottom
    voxelizedFaces.push_back({startIdx + 0, startIdx + 2, startIdx + 3});

    voxelizedFaces.push_back({startIdx + 4, startIdx + 5, startIdx + 6}); // top
    voxelizedFaces.push_back({startIdx + 4, startIdx + 6, startIdx + 7});

    voxelizedFaces.push_back({startIdx + 0, startIdx + 1, startIdx + 5}); // front
    voxelizedFaces.push_back({startIdx + 0, startIdx + 5, startIdx + 4});

    voxelizedFaces.push_back({startIdx + 1, startIdx + 2, startIdx + 6}); // right
    voxelizedFaces.push_back({startIdx + 1, startIdx + 6, startIdx + 5});

    voxelizedFaces.push_back({startIdx + 2, startIdx + 3, startIdx + 7}); // back
    voxelizedFaces.push_back({startIdx + 2, startIdx + 7, startIdx + 6});

    voxelizedFaces.push_back({startIdx + 3, startIdx + 0, startIdx + 4}); // left
    voxelizedFaces.push_back({startIdx + 3, startIdx + 4, startIdx + 7});
}
