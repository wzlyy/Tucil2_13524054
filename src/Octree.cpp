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


    Vertex c{
        (node->min.x + node->max.x) * 0.5f,
        (node->min.y + node->max.y) * 0.5f,
        (node->min.z + node->max.z) * 0.5f
    };

    Vertex h{
        (node->max.x - node->min.x) * 0.5f,
        (node->max.y - node->min.y) * 0.5f,
        (node->max.z - node->min.z) * 0.5f
    };


    Vertex tv0{v0.x - c.x, v0.y - c.y, v0.z - c.z};
    Vertex tv1{v1.x - c.x, v1.y - c.y, v1.z - c.z};
    Vertex tv2{v2.x - c.x, v2.y - c.y, v2.z - c.z};


    Vertex e0{tv1.x - tv0.x, tv1.y - tv0.y, tv1.z - tv0.z};
    Vertex e1{tv2.x - tv1.x, tv2.y - tv1.y, tv2.z - tv1.z};
    Vertex e2{tv0.x - tv2.x, tv0.y - tv2.y, tv0.z - tv2.z};

    auto axisTest = [&](float a, float b, float fa, float fb, float v0a, float v0b, float v1a, float v1b, float v2a, float v2b,
                        float ha, float hb) {
        float p0 = a * v0a - b * v0b;
        float p1 = a * v1a - b * v1b;
        float p2 = a * v2a - b * v2b;

        float minP = std::min(p0, std::min(p1, p2));
        float maxP = std::max(p0, std::max(p1, p2));

        float rad = fa * ha + fb * hb;
        return !(minP > rad || maxP < -rad);
    };

    float fex = std::abs(e0.x);
    float fey = std::abs(e0.y);
    float fez = std::abs(e0.z);


    if (!axisTest(e0.z, e0.y, fez, fey, tv0.y, tv0.z, tv1.y, tv1.z, tv2.y, tv2.z, h.y, h.z)) return false;
    if (!axisTest(e0.z, e0.x, fez, fex, tv0.x, tv0.z, tv1.x, tv1.z, tv2.x, tv2.z, h.x, h.z)) return false;
    if (!axisTest(e0.y, e0.x, fey, fex, tv0.x, tv0.y, tv1.x, tv1.y, tv2.x, tv2.y, h.x, h.y)) return false;

    fex = std::abs(e1.x);
    fey = std::abs(e1.y);
    fez = std::abs(e1.z);

    if (!axisTest(e1.z, e1.y, fez, fey, tv0.y, tv0.z, tv1.y, tv1.z, tv2.y, tv2.z, h.y, h.z)) return false;
    if (!axisTest(e1.z, e1.x, fez, fex, tv0.x, tv0.z, tv1.x, tv1.z, tv2.x, tv2.z, h.x, h.z)) return false;
    if (!axisTest(e1.y, e1.x, fey, fex, tv0.x, tv0.y, tv1.x, tv1.y, tv2.x, tv2.y, h.x, h.y)) return false;

    fex = std::abs(e2.x);
    fey = std::abs(e2.y);
    fez = std::abs(e2.z);

    if (!axisTest(e2.z, e2.y, fez, fey, tv0.y, tv0.z, tv1.y, tv1.z, tv2.y, tv2.z, h.y, h.z)) return false;
    if (!axisTest(e2.z, e2.x, fez, fex, tv0.x, tv0.z, tv1.x, tv1.z, tv2.x, tv2.z, h.x, h.z)) return false;
    if (!axisTest(e2.y, e2.x, fey, fex, tv0.x, tv0.y, tv1.x, tv1.y, tv2.x, tv2.y, h.x, h.y)) return false;


    auto min3 = [](float a, float b, float c) {
        return std::min(a, std::min(b, c));
    };
    auto max3 = [](float a, float b, float c) {
        return std::max(a, std::max(b, c));
    };

    if (max3(tv0.x, tv1.x, tv2.x) < -h.x || min3(tv0.x, tv1.x, tv2.x) > h.x) return false;
    if (max3(tv0.y, tv1.y, tv2.y) < -h.y || min3(tv0.y, tv1.y, tv2.y) > h.y) return false;
    if (max3(tv0.z, tv1.z, tv2.z) < -h.z || min3(tv0.z, tv1.z, tv2.z) > h.z) return false;


    Vertex normal{
        e0.y * e1.z - e0.z * e1.y,
        e0.z * e1.x - e0.x * e1.z,
        e0.x * e1.y - e0.y * e1.x
    };

    float d = -(normal.x * tv0.x + normal.y * tv0.y + normal.z * tv0.z);

    float r =
        h.x * std::abs(normal.x) +
        h.y * std::abs(normal.y) +
        h.z * std::abs(normal.z);

    float s = normal.x * 0 + normal.y * 0 + normal.z * 0 + d;

    if (std::abs(s) > r) return false;

    return true;
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
