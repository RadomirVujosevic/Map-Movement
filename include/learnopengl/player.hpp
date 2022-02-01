#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <glm/glm.hpp>
#include <iostream>

#include "marker.hpp"
#include "model.h"
#include "shader.h"

class Player
{
public:
    Marker *currentMarker;
    Marker *targetMarker = nullptr;
    Marker playerMarker;
    const float markerScaleRatio = 30.0f;
    const float yoffset = 0.2f;
    Model markerModel{"resources/objects/marker/marker.obj"};
    Model playerModel{"resources/objects/viking/viking.obj"};
    glm::vec3 scale;
    glm::vec3 position;
    bool isMoving = false;
    float movementSpeed = 0.01f;

    Player(Marker &startMarker, glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f))
    {
        currentMarker = &startMarker;
        this->scale = scale;
        position = startMarker.position;
        playerMarker = Marker(0, position);
    }

    void draw(Shader &shader)
    {
        glm::mat4 model = glm::mat4(1.0f);
        glm::vec3 translate{position.x, yoffset, position.z};
        model = glm::translate(model, translate);
        model = glm::scale(model, scale);
        shader.setMat4("model", model);
        playerModel.Draw(shader);

        model = glm::mat4(1.0f);

        translate = glm::vec3{position.x, 0.2f, position.z};
        model = glm::translate(model, translate);

        glm::vec3 markerScale = scale;
        markerScale *= markerScaleRatio;
        model = glm::scale(model, markerScale);
        model = glm::rotate(model, (float)glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        shader.setMat4("model", model);
        markerModel.Draw(shader);
    }

    void setRandomMovementTarget()
    {
        if (!currentMarker->neighbours.size())
        {
            std::cout << "Marker has no neighbours" << std::endl;
        }
        else if (isMoving)
        {
            // std::cout << "Already moving" << std::endl;
        }
        else
        {
            isMoving = true;
            int targetIndex = rand() % currentMarker->neighbours.size();
            targetMarker = &currentMarker->neighbours[targetIndex];
        }
    }

    void setMovementTarget(Marker &target)
    {
        isMoving = true;
        targetMarker = &target;
    }

    void processMovement()
    {
        if (!isMoving)
            return;

        glm::vec2 dirVec =
            glm::vec2(targetMarker->position.x - currentMarker->position.x,
                      targetMarker->position.z - currentMarker->position.z);

        position.x += dirVec.x * movementSpeed;
        position.z += dirVec.y * movementSpeed;

        if (distanceBetweenPoints(targetMarker->position, position) < 0.1f)
        {
            std::cout << "Movement done" << std::endl;
            currentMarker = targetMarker;
            targetMarker = nullptr;
            position = currentMarker->position;
            isMoving = false;
        }
    }

    static float distanceBetweenPoints(glm::vec3 a, glm::vec3 b)
    {
        float result =
            glm::sqrt((a.x - b.x) * (a.x - b.x) + (a.z - b.z) * (a.z - b.z));
        // std::cout << result << std::endl;
        return result;
    }
};

#endif
