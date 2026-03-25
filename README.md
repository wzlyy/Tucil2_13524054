# Tugas Kecil 2 Strategi Algoritma : Voxelization (Divide and Conquer)

## Penjelasan

Program ini merupakan implementasi algoritma voxelization untuk mengubah model 3D berbasis mesh (format .obj) menjadi representasi voxel.

Pendekatan yang digunakan adalah Divide and Conquer dengan struktur data Octree, di mana ruang 3D dibagi secara rekursif menjadi 8 bagian hingga mencapai kedalaman tertentu.

Setiap node pada octree akan menyimpan daftar face yang berpotongan dengan ruang tersebut. Pada tahap akhir, node leaf yang relevan akan dikonversi menjadi voxel berupa kubus yang direpresentasikan dalam format .obj.


## Prasyarat & Instalasi
### Prasyarat:
* Compiler g++

### Instalasi
Clone repository utama
```bash
git clone https://github.com/wzlyy/Tucil2_13524054.git
cd Tucil2_13524054
```
## Kompilasi dan Run Program
```bash
g++ src/main.cpp src/Octree.cpp -o bin/main 
bin/main
```
## Author
Wildan Abdurrahman Ghazali
13524054

