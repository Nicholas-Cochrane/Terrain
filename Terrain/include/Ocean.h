#ifndef OCEAN_H
#define OCEAN_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "stb_image.h"
#include <glad/glad.h>
#include <GL/glfw3.h>

#include "shader_s.h"
#include "camera.h"

#include <iostream>
#include <math.h>
#include <vector>

class Ocean
{
    public:
        Ocean(unsigned int heightMapInput,  glm::uvec2 heightMapUVsizeInput);
        virtual ~Ocean();
        void draw(Shader &shader, Camera &camera, glm::mat4 &projectionMatrix, glm::mat4 &projectionMatrix2);

        void setUpVertices(glm::mat4& viewMatrix, glm::mat4& projectionMatrix, glm::vec3& playerPosition); // Create verts to pass to VBO
        void setUpBuffers(); // Create VAO/VBO

    protected:
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> frustumVerts;
        glm::vec3 testPoint;

        unsigned int heightMap; //Height map texture ID
        glm::uvec2 heightMapUVsize; // width and height of height map

        bool VAOclear, VBOclear; // Has VAO/VBO not been created
        unsigned int VAO, VBO; // VAO/VBO ID's


    private:
       float linePlaneIntersectT(glm::vec3 P0, glm::vec3 P1, glm::vec3 P3, glm::vec3 La, glm::vec3 Lb);
       bool isLeft(glm::vec2 a, glm::vec2 b, glm::vec2 c);
       void pushQuad(glm::vec3 corner, float scale, float playerY);
};

#endif // OCEAN_H
