#ifndef WORLD_HPP
#define WORLD_HPP

#include <glm/glm.hpp>
#include <vector>
#include "Shader.hpp"
#include "CubeMesh.hpp"
#include "Player.hpp"
#include "Bus.hpp"

struct ResourceCrate {
    glm::vec3 position;
    int type; // 0: Fuel, 1: Scrap, 2: Rations
    bool isCarried = false;
    bool isDeposited = false;
};

struct Building {
    glm::vec3 position;
    glm::vec3 scale;
    glm::vec3 color;
};

class World {
public:
    std::vector<Building> buildings;
    std::vector<ResourceCrate> crates;

    World();
    void update(float deltaTime, Player& player, Bus& bus, bool interactPressed);
    void render(const Shader& shader, const CubeMesh& cube, const Player& player) const;
};

#endif