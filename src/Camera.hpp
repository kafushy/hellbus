#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Player.hpp"

class Camera {
public:
    static glm::mat4 getViewMatrix(const Player& player) {
        return glm::lookAt(player.position, player.position + player.getFrontVector(), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    static glm::mat4 getProjectionMatrix(int width, int height) {
        float aspect = (height > 0) ? static_cast<float>(width) / static_cast<float>(height) : 1.0f;
        return glm::perspective(glm::radians(75.0f), aspect, 0.1f, 100.0f);
    }
};

#endif