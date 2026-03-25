#include "Octree.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <vector>
#include <iostream>
#include <chrono>
using namespace std;


bool loadFile(const string& input, vector<Vertex>& vertices, vector<Face>& faces){
    ifstream file("test/" + input);
    if (!file.is_open()) {
        cout << "File tidak bisa dibuka\n";
        return false;
    }

    string line;
    int lineNumber = 0;

    while (getline(file, line)) {
        lineNumber++;
        if (line.empty()) continue;
        stringstream ss(line);
        string type;
        ss >> type;

        if (type == "v") {
            float x, y, z;

            if (!(ss >> x >> y >> z)) {
                cout << "Error di line " << lineNumber << ": format vertex salah\n";
                return false;
            }
            string extra;
            if (ss >> extra) {
                cout << "Error di line " << lineNumber << ": format vertex salah\n";
                return false;
            }

            vertices.push_back({x, y, z});
        }
        else if (type == "f") {
            int a, b, c;

            if (!(ss >> a >> b >> c)) {
                cout << "Error: format face salah\n";
                return false;
            }


            if (a < 1 || b < 1 || c < 1) {
                cout << "Error di line " << lineNumber << ": index < 1\n";
                return false;
            }

            if (a >(int)vertices.size() ||
                b >(int)vertices.size() ||
                c >(int)vertices.size()) {
                cout << "Error di line " << lineNumber << ": index melebihi jumlah vertex\n";
                return false;
            }
            faces.push_back({a - 1, b - 1, c - 1});
        }
        else {
            cout << "Error di line " << lineNumber << ": keyword tidak valid (" << type << ")\n";
            return false;
        }
    }
    if (vertices.empty() || faces.empty()) {
        cout << "Error: file kosong atau tidak lengkap\n";
        return false;
    }
    return true;
}

void writeFile(const Octree& octree, const string& output){
    ofstream out(output);
    if (!out.is_open()) cout << "Gagal menulis file!\n";
    for (Vertex v : octree.voxelizedVertices){
        out << "v " << v.x << " " << v.y << " " << v.z << "\n";
    }
    for (Face f : octree.voxelizedFaces){
        out << "f " << f.a + 1 << " " << f.b + 1 << " " << f.c + 1 << "\n";
    }

    out.close();
}
int main(){
    string input;
    cout << "Masukkan path file .obj dari folder test: ";
    cin >> input;

    int maxDepth;
    cout << "Masukkan kedalaman maksimum: ";
    cin >> maxDepth;
    vector<Vertex> vertices;
    vector<Face> faces;
    
    if(loadFile(input, vertices, faces)){
        Vertex minV = minVertex(vertices);
        Vertex maxV = maxVertex(vertices);

        Octree octree(minV, maxV, maxDepth, vertices, faces); 
        auto start = chrono::high_resolution_clock::now();
        octree.build();
        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
        string output = "test/voxelized_" + filesystem::path(input).filename().string();
        writeFile(octree, output); 
        cout << "Banyaknya voxel: " << octree.voxelizedVertices.size()/8 << "\n";
        cout << "Banyaknya vertex: " << octree.voxelizedVertices.size() << "\n";
        cout << "Banyaknya faces: " << octree.voxelizedFaces.size() << "\n";


        for (auto &[depth, count] : octree.nodeCount)
            cout << depth << " : " << count << "\n";

        for (auto &[depth, count] : octree.skippedNodeCount)
            cout << depth << " : " << count << " (skipped)\n";

        cout << "Kedalaman octree: " << maxDepth << "\n";
        cout << "Lama waktu: " << duration << " ms\n";
        cout << "Path file disimpan: output/voxelized_" + input << "\n";
    }
    

}
