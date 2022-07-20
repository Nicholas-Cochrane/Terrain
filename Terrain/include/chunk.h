#ifndef CHUNK_H
#define CHUNK_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "stb_image.h"

#include "shader_s.h"

#include <iostream>
#include <math.h>
#include <vector>

struct Vertex {
    glm::vec3 posCoords;
    glm::vec2 texCoords;
};


class Chunk
{
    public:
        Chunk(glm::vec3 positionInit, int subdivisions);
        virtual ~Chunk();

        glm::vec3 position;
        void draw(Shader &shader);


    protected:
        int subdivs; //subdivisions
        std::vector<float> heights;
        std::vector<Vertex> vertices;

        unsigned int texture;
        bool VAOclear, VBOclear = true;
        unsigned int VAO, VBO;
        void setUpVertices();
        void setUpBuffers();

    private:
        unsigned int loadTexture(char const* filepath, GLint formatColor, GLint wrappingParam = GL_REPEAT);
};

#endif // CHUNK_H
