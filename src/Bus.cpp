#include "Bus.hpp"
#include "Shader.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

bool Bus::loadAssets(const std::string& busPath, const std::string& texPath) {
    busModel = std::make_unique<Model>();
    return busModel->loadFromFile(busPath, texPath);
}

glm::vec3 Bus::localToWorld(const glm::vec3& localOffset) const {
    // Elevate bus by +0.3m (scaled) so wheels sit at Y = 0.0m
    glm::vec3 elevatedPos = position + glm::vec3(0.0f, 0.3f, 0.0f);
    glm::mat4 mat = glm::translate(glm::mat4(1.0f), elevatedPos);
    mat = glm::rotate(mat, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    mat = glm::scale(mat, glm::vec3(0.5f, 0.5f, 0.5f));
    return glm::vec3(mat * glm::vec4(localOffset, 1.0f));
}

glm::vec3 Bus::worldToLocal(const glm::vec3& worldPos) const {
    glm::vec3 elevatedPos = position + glm::vec3(0.0f, 0.3f, 0.0f);
    glm::vec3 diff = worldPos - elevatedPos;
    float cosY = cos(-yaw);
    float sinY = sin(-yaw);
    // Inverse scale (2.0x) to work in raw OBJ space
    return glm::vec3(
        (diff.x * cosY - diff.z * sinY) * 2.0f,
        diff.y * 2.0f,
        (diff.x * sinY + diff.z * cosY) * 2.0f
    );
}

void Bus::update(float deltaTime, bool moveForward, bool moveBackward, bool turnLeft, bool turnRight) {
    if (moveForward && fuel > 0.0f) {
        speed += acceleration * deltaTime;
        fuel = std::max(0.0f, fuel - 1.2f * deltaTime);
    } else if (moveBackward) {
        speed -= braking * deltaTime;
    } else {
        applyFriction(deltaTime);
    }

    speed = std::clamp(speed, maxReverseSpeed, maxSpeed);

    float targetSteering = 0.0f;
    if (turnRight) targetSteering -= maxSteeringAngle;
    if (turnLeft)  targetSteering += maxSteeringAngle;

    if (turnLeft || turnRight) {
        if (currentSteeringAngle < targetSteering) {
            currentSteeringAngle = std::min(targetSteering, currentSteeringAngle + steeringSpeed * deltaTime);
        } else if (currentSteeringAngle > targetSteering) {
            currentSteeringAngle = std::max(targetSteering, currentSteeringAngle - steeringSpeed * deltaTime);
        }
    } else {
        if (currentSteeringAngle > 0.0f) {
            currentSteeringAngle = std::max(0.0f, currentSteeringAngle - steeringReturnSpeed * deltaTime);
        } else if (currentSteeringAngle < 0.0f) {
            currentSteeringAngle = std::min(0.0f, currentSteeringAngle + steeringReturnSpeed * deltaTime);
        }
    }

    if (std::abs(speed) > 0.1f) {
        float yawRate = (speed / wheelBase) * tan(currentSteeringAngle);
        yaw += yawRate * deltaTime;
    }

    glm::vec3 forward(-sin(yaw), 0.0f, -cos(yaw));
    position += forward * speed * deltaTime;
}

void Bus::applyFriction(float deltaTime) {
    if (speed > 0.0f) {
        speed = std::max(0.0f, speed - friction * deltaTime);
    } else if (speed < 0.0f) {
        speed = std::min(0.0f, speed + friction * deltaTime);
    }
}

glm::vec3 Bus::getDriverSeatPosition() const {
    return localToWorld(glm::vec3(-1.2f, 3.2f, -10.5f));
}

glm::vec3 Bus::getExitDoorPosition() const {
    return localToWorld(glm::vec3(2.8f, 0.4f, -5.0f));
}

glm::vec3 Bus::getAisleStartPosition() const {
    return localToWorld(glm::vec3(0.0f, 3.0f, -9.0f));
}

void Bus::resolvePlayerCollision(glm::vec3& playerPos, float playerRadius) const {
    glm::vec3 local = worldToLocal(playerPos);

    // Bounding Box half-extents in raw OBJ space
    float halfW = 2.6f + (playerRadius * 2.0f);
    float halfL = 12.6f + (playerRadius * 2.0f);

    // Ignore collision ONLY on the RIGHT door side (local.x > 0)
    // The LEFT wall (local.x < 0) MUST always collide!
    if (local.x > 0.0f && local.z > -8.0f && local.z < -2.0f) {
        return; 
    }

    if (std::abs(local.x) < halfW && std::abs(local.z) < halfL) {
        float overlapX = halfW - std::abs(local.x);
        float overlapZ = halfL - std::abs(local.z);

        if (overlapX < overlapZ) {
            local.x += (local.x > 0.0f) ? overlapX : -overlapX;
        } else {
            local.z += (local.z > 0.0f) ? overlapZ : -overlapZ;
        }

        playerPos = localToWorld(local);
    }
}

void Bus::render(const Shader& shader) const {
    if (!busModel) return;

    // Elevate mesh vertically so tires sit on ground level Y = 0.0m
    glm::mat4 baseTransform = glm::translate(glm::mat4(1.0f), position + glm::vec3(0.0f, 0.3f, 0.0f));
    baseTransform = glm::rotate(baseTransform, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    baseTransform = glm::scale(baseTransform, glm::vec3(0.5f, 0.5f, 0.5f));

    shader.setMat4("model", baseTransform);
    shader.setVec3("objectColor", glm::vec3(0.35f, 0.38f, 0.42f));

    // Chassis & Rear Wheels
    busModel->drawMeshByName("Cube");
    busModel->drawMeshByName("Cylinder.002");
    busModel->drawMeshByName("Cylinder.005");
    busModel->drawMeshByName("Cylinder.003");
    busModel->drawMeshByName("Cylinder.006");

    // Front Steering Wheels
    glm::vec3 frontAxlePivot(0.0f, 0.4f, -7.8f);

    glm::mat4 frontWheelTransform = glm::translate(baseTransform, frontAxlePivot);
    frontWheelTransform = glm::rotate(frontWheelTransform, currentSteeringAngle, glm::vec3(0.0f, 1.0f, 0.0f));
    frontWheelTransform = glm::translate(frontWheelTransform, -frontAxlePivot);

    shader.setMat4("model", frontWheelTransform);
    busModel->drawMeshByName("Cylinder");
    busModel->drawMeshByName("Cylinder.004");
}