#include "Player.hpp"
#include "Bus.hpp"
#include <algorithm>
#include <cmath>

void Player::processLook(float xOffset, float yOffset) {
    float sensitivity = 0.0025f;
    yaw -= xOffset * sensitivity;
    pitch += yOffset * sensitivity;
    pitch = std::clamp(pitch, -1.4f, 1.4f);
}

void Player::updateOnFoot(float deltaTime, bool w, bool s, bool a, bool d, bool jumpPressed, const Bus& bus) {
    glm::vec3 front(-sin(yaw), 0.0f, -cos(yaw));
    front = glm::normalize(front);
    glm::vec3 right(cos(yaw), 0.0f, -sin(yaw));

    glm::vec3 moveDir(0.0f);
    if (w) moveDir += front;
    if (s) moveDir -= front;
    if (a) moveDir -= right;
    if (d) moveDir += right;

    if (glm::length(moveDir) > 0.0f) {
        position += glm::normalize(moveDir) * moveSpeed * deltaTime;
    }

    // Convert world position into bus local space
    glm::vec3 localPos = bus.worldToLocal(position);

    // Jump & Ground Gravity
    if (jumpPressed && isGrounded) {
        yVelocity = 5.5f;
        isGrounded = false;
    }

    if (!isGrounded) {
        yVelocity -= 15.0f * deltaTime;
        position.y += yVelocity * deltaTime;
        if (position.y <= height) {
            position.y = height;
            yVelocity = 0.0f;
            isGrounded = true;
        }
    } else {
        position.y = height;
    }

    // WALK-IN THRESHOLD (Right door side only: local.x > 1.2m, Z between -8.0m and -2.0m)
    if (localPos.x > 1.2f && localPos.x < 2.5f && localPos.z > -8.0f && localPos.z < -2.0f) {
        state = PlayerState::PASSENGER_CABIN;
        localBusOffset = localPos;
    }
}

void Player::updateInsideCabin(float deltaTime, bool w, bool s, bool a, bool d, bool jumpPressed, const Bus& bus) {
    float relYaw = yaw - bus.yaw;
    glm::vec3 front(-sin(relYaw), 0.0f, -cos(relYaw));
    glm::vec3 right(cos(relYaw), 0.0f, -sin(relYaw));

    glm::vec3 moveDir(0.0f);
    if (w) moveDir += front;
    if (s) moveDir -= front;
    if (a) moveDir -= right;
    if (d) moveDir += right;

    if (glm::length(moveDir) > 0.0f) {
        localBusOffset += glm::normalize(moveDir) * moveSpeed * deltaTime;
    }

    // Eye level inside cabin (Y = 3.0m in local OBJ space)
    float targetLocalY = 3.0f;

    if (jumpPressed && isGrounded) {
        yVelocity = 4.5f;
        isGrounded = false;
    }

    if (!isGrounded) {
        yVelocity -= 15.0f * deltaTime;
        localBusOffset.y += yVelocity * deltaTime;

        if (localBusOffset.y <= targetLocalY) {
            localBusOffset.y = targetLocalY;
            yVelocity = 0.0f;
            isGrounded = true;
        }
    } else {
        // Smooth height adaptation without hard snaps
        localBusOffset.y = glm::mix(localBusOffset.y, targetLocalY, 12.0f * deltaTime);
    }

    // Clamping limits inside cabin:
    // Left Wall is solid (-1.8m local limit)
    localBusOffset.x = std::clamp(localBusOffset.x, -1.8f, 2.5f);

    // If not at the right door threshold, clamp right wall as well
    if (localBusOffset.z <= -8.0f || localBusOffset.z >= -2.0f) {
        localBusOffset.x = std::clamp(localBusOffset.x, -1.8f, 1.8f);
    }

    // Aisle Length limits (Cockpit to Rear)
    localBusOffset.z = std::clamp(localBusOffset.z, -11.5f, 11.5f);

    // Compute continuous World Position
    position = bus.localToWorld(localBusOffset);

    // WALK-OUT THRESHOLD (Right door side step out)
    if (localBusOffset.x > 2.4f && localBusOffset.z > -8.0f && localBusOffset.z < -2.0f) {
        state = PlayerState::ON_FOOT;
        // Keep position continuous in world space
    }
}

void Player::handleInteraction(Bus& bus) {
    if (state == PlayerState::DRIVING) {
        state = PlayerState::PASSENGER_CABIN;
        yVelocity = 0.0f;
        isGrounded = true;
        localBusOffset = glm::vec3(0.0f, 3.0f, -8.0f);
    } 
    else if (state == PlayerState::PASSENGER_CABIN) {
        if (localBusOffset.z < -8.5f) {
            state = PlayerState::DRIVING;
        }
    }
}

glm::vec3 Player::getFrontVector() const {
    glm::vec3 front;
    front.x = -sin(yaw) * cos(pitch);
    front.y = sin(pitch);
    front.z = -cos(yaw) * cos(pitch);
    return glm::normalize(front);
}