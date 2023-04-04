#include "Grass.h"

Grass::Grass(unsigned int heightMapInput,  glm::uvec2 heightMapUVsizeInput, glm::vec2 worldSizeInput , float newDensity, float newNearLOD, float newFarLOD)
{
    //ctor
    density = newDensity;
    nearLOD = newNearLOD;
    farLOD = newFarLOD;

    heightMap = heightMapInput;
    heightMapUVsize = heightMapUVsizeInput;
    worldSize = worldSizeInput; ///TODO remove if not needed

    model = {glm::vec3(-0.06, 0, 0),
             glm::vec3(0.06, 0, 0),
             glm::vec3(0, 1.0, 0)
            };

    VAO = 0;
    VBO = 0;
    instanceVBO = 0;
    VAOclear = true;
    VBOclear = true;
    instanceVBOclear = true;

    setUpVertices();
    setUpBuffers();
}

Grass::~Grass()
{
    //dtor
    if(!VAOclear)
        glDeleteVertexArrays(1, &VAO);
    if(!VBOclear)
        glDeleteBuffers(1, &VBO);
    if(!instanceVBOclear)
        glDeleteBuffers(1,&instanceVBO);
}


void Grass::changeSettings(float newDensity, float newNearLOD, float newFarLOD)
{
    density = newDensity;
    nearLOD = newNearLOD;
    farLOD = newFarLOD;

    setUpVertices();
    setUpBuffers();
}

void Grass::getSettings(float* outDensity, float* outNearLOD, float* outFarLOD)
{
    *outDensity = density;
    *outNearLOD = nearLOD;
    *outFarLOD = farLOD;
}

void Grass::draw(Shader& shader, glm::mat4& viewMatrix, glm::mat4& projectionMatrix)
{
    glDisable(GL_CULL_FACE);
    shader.use();

    shader.setInt("heightMap", 0); // set shader uniform to sampler

    glActiveTexture(GL_TEXTURE0); // set sampler to texture
    glBindTexture(GL_TEXTURE_2D, heightMap);

    shader.setMat4("pvMatrix", projectionMatrix * viewMatrix);
    shader.setFloat("nearDist", nearLOD);

    glBindVertexArray(VAO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, model.size() , nearOffsetVertices.size());

    glBindVertexArray(0); // unbind
    glEnable(GL_CULL_FACE);
}

glm::uvec3 Grass::pcg3d(glm::uvec3 v)
{
    v = v * 1664525u + 1013904223u;
	v.x += v.y*v.z; v.y += v.z*v.x; v.z += v.x*v.y;
	v ^= v >> 16u;
	v.x += v.y*v.z; v.y += v.z*v.x; v.z += v.x*v.y;
    return v;
}


void Grass::setUpVertices()
{
    nearOffsetVertices.clear();
    if(farLOD <= 0.0f || nearLOD <= 0.0f){
        nearOffsetVertices.resize(0);
        return;
    }
    nearOffsetVertices.resize(static_cast<int>( (1.0/density)* std::pow(nearLOD*2 , 2)));
    for(float y = -nearLOD; y < nearLOD; y += density){
        for(float x = -nearLOD; x < nearLOD; x += density){
            glm::uvec3 hashVec = pcg3d(glm::uvec3((unsigned int) (y*(1.0/density)), (unsigned int) (x*(1.0/density)), 7632978u));
            float hashx = ((static_cast<float>(hashVec.x)/UINT_MAX) - 0.5)* 2;
            float hashy = ((static_cast<float>(hashVec.y)/UINT_MAX) - 0.5) * 2;
            nearOffsetVertices.push_back(glm::vec2(x + (hashx * density),y + (hashy * density)));
        }
    }
    std::cout << nearOffsetVertices.size() <<std::endl;
}

void Grass::setUpBuffers()
{
    if(!VAOclear)
        glDeleteVertexArrays(1, &VAO);
    if(!VBOclear)
        glDeleteBuffers(1, &VBO);
    if(!instanceVBOclear)
        glDeleteBuffers(1,&instanceVBO);


    VBOclear = false;// mark buffers as created
    VAOclear = false;
    instanceVBOclear = false;

    //set up instance  buffer
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * nearOffsetVertices.size(), &nearOffsetVertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //Set up Vertex Buffer
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, model.size()* sizeof(glm::vec3), &model[0] , GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    //glEnableVertexAttribArray(1);
    //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));

    // also set instance data
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO); // this attribute comes from a different vertex buffer
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(1, 1); // tell OpenGL this is an instanced vertex attribute.
}
