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
        Ocean(unsigned int heightMapInput,  glm::uvec2 heightMapUVsizeInput, int widthInput, int heightInput);
        virtual ~Ocean();
        void draw(Shader& shader, glm::mat4& viewMatrix, glm::mat4& projectionMatrix);
        void printLineVerts();

        void setUpVertices(glm::mat4& viewMatrix, glm::mat4& projectionMatrix, glm::vec3& playerPosition, float oceanLevel); // Create verts to pass to VBO
        void setUpBuffers(); // Create VAO/VBO
        void setBuffers();//spit sending vertex data into different function

    protected:
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> lineVerts;
        std::vector<glm::vec3> frustumVerts;
        std::vector<bool> currLine;
        unsigned int width;
        unsigned int height;


        unsigned int heightMap; //Height map texture ID
        glm::uvec2 heightMapUVsize; // width and height of height map

        bool VAOclear, VBOclear; // Has VAO/VBO not been created
        unsigned int VAO, VBO; // VAO/VBO ID's


    private:
       float linePlaneIntersectT(glm::vec3 P0, glm::vec3 P1, glm::vec3 P3, glm::vec3 La, glm::vec3 Lb);
       bool isLeft(glm::vec2 a, glm::vec2 b, glm::vec2 c);
};

#endif // OCEAN_H
