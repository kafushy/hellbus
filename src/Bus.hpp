#ifndef BUS_HPP
#define BUS_HPP

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <algorithm>
#include "Model.hpp"

class Shader;

struct Bus {
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    float yaw = 0.0f;
    float speed = 0.0f;
    float fuel = 100.0f;
    int scrapCount = 0;

    // Steering Physics
    float currentSteeringAngle = 0.0f;
    const float maxSteeringAngle = 0.55f;
    const float steeringSpeed = 1.2f;
    const float steeringReturnSpeed = 2.0f;

    // 12.5m Bus Dimensions & Wheelbase
    const float maxSpeed = 18.0f;
    const float maxReverseSpeed = -5.0f;
    const float acceleration = 7.0f;
    const float braking = 18.0f;
    const float friction = 2.5f;
    const float wheelBase = 7.5f;

    // Interior Aisle Bounds
    const float aisleMinX = -0.65f;
    const float aisleMaxX =  0.65f;
    const float aisleMinZ = -8.8f;
    const float aisleMaxZ =  3.0f;

    // Single Combined 3D Model
    std::unique_ptr<Model> busModel;

    bool loadAssets(const std::string& busPath, const std::string& texPath = "");
    void update(float deltaTime, bool moveForward, bool moveBackward, bool turnLeft, bool turnRight);
    void applyFriction(float deltaTime);
    void render(const Shader& shader) const;
    void resolvePlayerCollision(glm::vec3& playerPos, float playerRadius) const;

    glm::vec3 getDriverSeatPosition() const;
    glm::vec3 getExitDoorPosition() const;
    glm::vec3 getAisleStartPosition() const;
    
    glm::vec3 localToWorld(const glm::vec3& localOffset) const;
    glm::vec3 worldToLocal(const glm::vec3& worldPos) const;

    glm::vec3 clampInsideCabin(const glm::vec3& localOffset) const {
        glm::vec3 clamped = localOffset;
        clamped.x = std::clamp(clamped.x, aisleMinX, aisleMaxX);
        clamped.z = std::clamp(clamped.z, aisleMinZ, aisleMaxZ);
        return clamped;
    }
};

#endif