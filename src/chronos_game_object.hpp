#pragma once

#include "chronos_model.hpp"

//std
#include <memory>

namespace Chronos {

struct Transform2dComponent {
    glm::vec2 translation{}; //position offset
    glm::vec2 scale{1.f,1.f};
    float rotation;

    glm::mat2 mat2() 
    {
        const float s = glm::sin(rotation);
        const float c = glm::cos(rotation);
        glm::mat2 rotMatrix{{c,s}, {-s,c}};

        glm::mat2 scaleMat{{scale.x, .0f}, {0.f, scale.y}};
        return rotMatrix * scaleMat;
    }
};

class ChronosGameObject
{
public:
    using id_t = unsigned int;

    static ChronosGameObject createGameObject()
    {
        static id_t currentId = 0;
        return ChronosGameObject{currentId++};
    }

    ChronosGameObject(const ChronosGameObject &) = delete;
    ChronosGameObject &operator=(const ChronosGameObject &) = delete;
    ChronosGameObject(ChronosGameObject &&) = delete;
    ChronosGameObject &operator=(ChronosGameObject &&) = delete;

    id_t getId() { return id; }

    std::shared_ptr<ChronosModel> model{};
    glm::vec3 color{};
    Transform2dComponent transform2d;

private:
    ChronosGameObject(id_t objId) : id{objId} {}

    id_t id;
};
}
