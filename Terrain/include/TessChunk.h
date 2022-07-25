#ifndef TESSCHUNK_H
#define TESSCHUNK_H


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


class TessChunk
{
    public:
        TessChunk(glm::vec3 positionInit, int subdivisions, unsigned int inTexture);
        //position from 0,0 with chunk 0,0 being bottom,left aka SE corner
        virtual ~TessChunk();

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
};

#endif // TESSCHUNK_H
