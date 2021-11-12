#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <glm/glm.hpp>
#include <iostream>

#include "glitter.hpp"
#include "marker.hpp"
#include "model.h"
#include "shader.h"

class Player {
   public:
    Marker *currentMarker;
    Marker *targetMarker = nullptr;

    Model playerModel{"../Glitter/resources/objects/player/player.obj"};
    glm::vec3 scale;
    glm::vec3 position;
    bool isMoving = false;
    float movementSpeed = 0.01f;

    Player(Marker &startMarker, glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f)) {
        currentMarker = &startMarker;
        this->scale = scale;
        position = startMarker.position;
    }

    void draw(Shader &shader) {
        glm::mat4 model = glm::mat4(1.0f);
        glm::vec3 translate(position.x, 1.0f, position.z);
        model = glm::translate(model, translate);
        model = glm::scale(model, scale);
        shader.setMat4("model", model);
        playerModel.Draw(shader);
    }

    void setMovementTarget(Marker &target) {
        isMoving = true;
        targetMarker = &target;
    }

    void processMovement() {
        if (!isMoving) return;

        glm::vec2 dirVec =
            glm::vec2(targetMarker->position.x - currentMarker->position.x,
                      targetMarker->position.z - currentMarker->position.z);

        position.x += dirVec.x * movementSpeed;
        position.z += dirVec.y * movementSpeed;

        if (distanceBetweenPoints(targetMarker->position, position) < 0.1f) {
            std::cout << "Movement done" << std::endl;
            currentMarker = targetMarker;
            targetMarker = nullptr;
            position = currentMarker->position;
            isMoving = false;
        }
    }

    static float distanceBetweenPoints(glm::vec3 a, glm::vec3 b) {
        float result =
            glm::sqrt((a.x - b.x) * (a.x - b.x) + (a.z - b.z) * (a.z - b.z));
        // std::cout << result << std::endl;
        return result;
    }
};

#endif
