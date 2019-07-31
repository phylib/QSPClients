#include <iostream>
#include "QuadTree.h"

int main() {
    std::cout << "Hello, World!" << std::endl;

    quadtree::QuadTree quadTree(quadtree::Point(0,0), quadtree::Point(16, 16), 1);

    return 0;
}