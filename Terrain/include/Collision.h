#ifndef COLLISION_H_INCLUDED
#define COLLISION_H_INCLUDED
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Collider{
    virtual bool collidesWith(Collider c){return false;};
};

struct AABB: public Collider{//Axis Aligned Bounding Box
    glm::vec3 center {0.f, 0.f, 0.f};
    glm::vec3 extents {0.f, 0.f, 0.f};
};

struct Capsual : public Collider{
    glm::vec3 pointA{0.f, 0.f, 0.f};
    glm::vec3 pointB{0.f, 0.f, 0.f};
    float radius;
};

#endif // COLLISION_H_INCLUDED
