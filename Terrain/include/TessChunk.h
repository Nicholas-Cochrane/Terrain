#ifndef TESSCHUNK_H
#define TESSCHUNK_H


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "stb_image.h"
#include <glad/glad.h>
#include <GL/glfw3.h>

#include "shader_s.h"

#include <iostream>
#include <math.h>
#include <vector>

struct Vertex {
    glm::vec3 posCoords;
    glm::vec2 texCoords;
};


class TessChunk
{
    public:
        TessChunk(glm::vec3 inPosition, int inWidth, int inPatchesPerEdge, unsigned int inTexture, unsigned int inheightMapTexture, glm::vec2 inHeightMapUV, float inHeightMapUVScale);
        virtual ~TessChunk();

        glm::vec3 position; //position from 0,0 with chunk 0,0 being bottom,left aka SE corner
        void prepShader(const Shader& shader);
        void draw(const Shader &shader, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix, const glm::mat4 &projectionMatrix2);


    protected:
        int width; // patch width
        int patchesPerEdge; // number of patches in one axis ex: if 3 then 3*3 = 9 total chunks
        std::vector<Vertex> vertices;

        unsigned int heightMap; //Height map texture ID
        glm::vec2 heightMapUV; // root's UV on height map
        float heightMapUVScale; // distance on UV map from South to North and/or East to West

        unsigned int texture; // ground texture
        bool VAOclear, VBOclear; // Has VAO/VBO not been created
        unsigned int VAO, VBO; // VAO/VBO ID's
        void setUpVertices(); // Create verts to pass to VBO
        void setUpBuffers(); // Create VAO/VBO

    private:
};

#endif // TESSCHUNK_H
