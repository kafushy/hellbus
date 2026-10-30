#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <glm/glm.hpp>

// Forward declaration to avoid circular include dependency
struct Bus;

enum class PlayerState {
    DRIVING,
    PASSENGER_CABIN,
    ON_FOOT
};

enum class LifeState : uint8_t {
    ALIVE, BLEEDOUT, IN_BUS_BED, LOCKED_CHURCH, LOCKED_JAIL, DEAD
};

struct Bleedout {
    float remaining = 30.0f;
    float maxSeconds = 30.0f;
    bool  beingWorkedOn = false;      // frozen while a teammate interacts
    void reset() { remaining = maxSeconds; beingWorkedOn = false; }
    bool tick(float dt) {             // true == just bled out
        if (beingWorkedOn) return false;
        remaining = std::max(0.0f, remaining - dt);
        return remaining <= 0.0f;
    }
};

struct Player {
    glm::vec3 position = glm::vec3(0.0f, 1.7f, 0.0f);
    glm::vec3 localBusOffset = glm::vec3(0.0f, 3.7f, -4.5f);

    float yaw = 0.0f;
    float pitch = 0.0f;

    float height = 1.7f;
    float busFloorHeight = 2.0f;
    float moveSpeed = 6.0f;
    float yVelocity = 0.0f;
    bool isGrounded = true;

    PlayerState state = PlayerState::ON_FOOT;
float hp = 100.0f;
    float hpMax = 100.0f;
    LifeState life = LifeState::ALIVE;
    Bleedout bleedout;
    float reviveChannel = 0.0f;       // 0..1, fills while teammate holds interact

    bool isDown() const { return life == LifeState::BLEEDOUT; }
    void takeDamage(float dmg);
    void enterBleedout();
    void reviveFromDefib();
    void stabilizeInBusBed();
    void updateLife(float dt, bool teamAtDoor);
    int carriedItemType = -1;

    void processLook(float xOffset, float yOffset);
    void updateOnFoot(float deltaTime, bool w, bool s, bool a, bool d, bool jumpPressed, const Bus& bus);
    void updateInsideCabin(float deltaTime, bool w, bool s, bool a, bool d, bool jumpPressed, const Bus& bus);
    void handleInteraction(Bus& bus);

    glm::vec3 getFrontVector() const;
};

#endif