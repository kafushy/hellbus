#include "World.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

World::World() {
    for (int i = -4; i <= 4; ++i) {
        float zPos = i * 22.0f;
        buildings.push_back({ glm::vec3(-14.0f, 10.0f, zPos), glm::vec3(8.0f, 20.0f + (abs(i) % 3) * 6.0f, 14.0f), glm::vec3(0.18f, 0.18f, 0.22f) });
        buildings.push_back({ glm::vec3( 14.0f, 12.0f, zPos + 5.0f), glm::vec3(8.0f, 24.0f - (abs(i) % 2) * 8.0f, 14.0f), glm::vec3(0.15f, 0.15f, 0.2f) });
    }

    crates.push_back({ glm::vec3( 5.0f, 0.5f,  15.0f), 0, false, false });
    crates.push_back({ glm::vec3(-6.0f, 0.5f, -10.0f), 1, false, false });
    crates.push_back({ glm::vec3( 4.0f, 0.5f, -25.0f), 2, false, false });
}

void World::update(float deltaTime, Player& player, Bus& bus, bool interactPressed) {
    if (!interactPressed || player.state == PlayerState::DRIVING) return;

    if (player.carriedItemType != -1) {
        float distToBus = glm::distance(player.position, bus.position);
        for (auto& crate : crates) {
            if (crate.isCarried) {
                crate.isCarried = false;
                if (distToBus < 6.0f) {
                    crate.isDeposited = true;
                    if (crate.type == 0) bus.fuel = std::min(100.0f, bus.fuel + 40.0f);
                    if (crate.type == 1) bus.scrapCount++;
                } else {
                    crate.position = player.position + player.getFrontVector() * 1.5f;
                    crate.position.y = 0.5f;
                }
                break;
            }
        }
        player.carriedItemType = -1;
    } else {
        for (auto& crate : crates) {
            if (!crate.isCarried && !crate.isDeposited) {
                if (glm::distance(player.position, crate.position) < 3.0f) {
                    crate.isCarried = true;
                    player.carriedItemType = crate.type;
                    break;
                }
            }
        }
    }
}

void World::render(const Shader& shader, const CubeMesh& cube, const Player& player) const {
    // Road
    glm::mat4 roadModel = glm::scale(glm::mat4(1.0f), glm::vec3(18.0f, 0.1f, 160.0f));
    shader.setMat4("model", roadModel);
    shader.setVec3("objectColor", glm::vec3(0.12f, 0.12f, 0.14f));
    cube.draw();

    // Curbs
    glm::mat4 leftCurbs = glm::translate(glm::mat4(1.0f), glm::vec3(-9.5f, 0.2f, 0.0f));
    leftCurbs = glm::scale(leftCurbs, glm::vec3(1.2f, 0.3f, 160.0f));
    shader.setMat4("model", leftCurbs);
    shader.setVec3("objectColor", glm::vec3(0.28f, 0.28f, 0.30f));
    cube.draw();

    glm::mat4 rightCurbs = glm::translate(glm::mat4(1.0f), glm::vec3(9.5f, 0.2f, 0.0f));
    rightCurbs = glm::scale(rightCurbs, glm::vec3(1.2f, 0.3f, 160.0f));
    shader.setMat4("model", rightCurbs);
    shader.setVec3("objectColor", glm::vec3(0.28f, 0.28f, 0.30f));
    cube.draw();

    // City Buildings
    for (const auto& b : buildings) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), b.position);
        model = glm::scale(model, b.scale);
        shader.setMat4("model", model);
        shader.setVec3("objectColor", b.color);
        cube.draw();
    }

    // Crates
    for (const auto& crate : crates) {
        if (crate.isDeposited) continue;

        glm::vec3 pos = crate.isCarried ? (player.position + player.getFrontVector() * 0.8f + glm::vec3(0.0f, -0.3f, 0.0f)) : crate.position;
        glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
        model = glm::scale(model, glm::vec3(0.8f, 0.8f, 0.8f));
        shader.setMat4("model", model);

        if (crate.type == 0) shader.setVec3("objectColor", glm::vec3(0.9f, 0.8f, 0.1f));
        else if (crate.type == 1) shader.setVec3("objectColor", glm::vec3(0.85f, 0.45f, 0.1f));
        else shader.setVec3("objectColor", glm::vec3(0.2f, 0.75f, 0.2f));

        cube.draw();
    }

    // Exit Wall
    glm::mat4 exitZone = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.5f, -75.0f));
    exitZone = glm::scale(exitZone, glm::vec3(18.0f, 3.0f, 0.4f));
    shader.setMat4("model", exitZone);
    shader.setVec3("objectColor", glm::vec3(0.12f, 0.35f, 0.32f));
    cube.draw();
}