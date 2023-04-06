#ifndef GRASS_H
#define GRASS_H

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

class Grass
{
    public:
        Grass(unsigned int heightMapInput,  glm::uvec2 heightMapUVsizeInput, glm::vec2 worldSizeInput , float newDensity, float newNearLOD, float newFarLOD);
        virtual ~Grass();
        void changeSettings(float newDensity, float newNearLOD, float newFarLOD);
        void getSettings (float* outDensity, float* outNearLOD, float* outFarLOD);
        void draw(Shader& shader, glm::mat4& viewMatrix, glm::mat4& projectionMatrix, Camera& camera);

        void setUpVertices(); // Create verts to pass to VBO
        void setUpBuffers(); // Create VAO/VBO and sending vertex data into different function

    protected:
        glm::uvec3 pcg3d(glm::uvec3 v);

        std::vector<glm::vec2> offsetVertices;
        std::vector<glm::vec3> farModel;
        std::vector<glm::vec3> nearModel;
        float density;
        float nearLOD;
        float farLOD;

        float chunkSize;
        unsigned int vertsPerChunkLine;
        unsigned int chunksPerLine = 20;
        unsigned int totalChunks;

        bool  nearVBOclear, farVBOclear, instanceVBOclear; // Has VAO/VBO not been created
        unsigned int  nearVBO, farVBO, instanceVBO; // VAO/VBO ID's
        std::vector<unsigned int> nearChunkVAOArray;
        std::vector<unsigned int> farChunkVAOArray;


        unsigned int heightMap; //Height map texture ID
        glm::uvec2 heightMapUVsize; // width and height of height map
        glm::vec2 worldSize;// width and height of heightmap in world space
};

#endif // GRASS_H
