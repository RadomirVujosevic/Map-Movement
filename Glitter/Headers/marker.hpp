#ifndef MARKER_HPP
#define MARKER_HPP

#include <functional>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>

class Marker {
   public:
    int idx;
    glm::vec3 position;
    std::vector<std::reference_wrapper<Marker>> neighbours;

    Marker(int index, glm::vec3 pos = glm::vec3(0.0f, 0.1f, 0.0f)) {
        idx = index;
        position = pos;
    }

    Marker(int index, glm::vec2 pos = glm::vec2(0.0f, 0.0f)) {
        idx = index;
        position = glm::vec3(pos.x, 0.1f, pos.y);
    }

    Marker(int index, float x = 0, float y = 0) {
        idx = index;
        position = glm::vec3(x, 0.1f, y);
    }

    Marker() {
        idx = -1;
        position = glm::vec3();
    }

    void addNeighbour(Marker& marker) { neighbours.push_back(marker); }

    size_t getNeighbourCount() { return neighbours.size(); }

    void writePos() {
        std::cout << "Position of marker " << idx << " is : [" << position.x
                  << ", " << position.z << "]" << std::endl;
    }

    void writeNeighbours() {
        int n = getNeighbourCount();
        if (n == 0) {
            std::cout << " Marker " << idx << " has no neighbours" << std::endl;
        } else {
            std::cout << "Neighbours of marker " << idx << ": [";
            for (auto it : neighbours) {
                std::cout << " " << it.get().idx;
            }
            std::cout << "]" << std::endl;
        }
    }
};

#endif
