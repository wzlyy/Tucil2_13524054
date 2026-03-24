#include "Octree.hpp"
std::vector<Face> faces;
Octree::Octree(Vertex min, Vertex max, int maxDepth){
    this->root = new OctreeNode(min, max, 0);
    this->maxDepth = maxDepth;
}
void Octree::build(){
    divide(root);
}
void Octree::divide(OctreeNode* node){
    if (node->depth >= maxDepth) {
        node->isLeaf = true;
        return;
    }

    Vertex mid{
        (node->min.x + node->max.x) / 2,
        (node->min.y + node->max.y) / 2,
        (node->min.z + node->max.z) / 2
    };

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

        node->children[i] = new OctreeNode(minChild, maxChild, node->depth + 1);

        for (int idx : node->intersectFaces) {
            if (intersect(node->children[i], faces[idx])) {
                node->children[i]->intersectFaces.push_back(idx);
            }
        }
        divide(node->children[i]);
    }
}