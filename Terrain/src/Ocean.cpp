#include "Ocean.h"
#include <iostream>

Ocean::Ocean(unsigned int heightMapInput,  glm::uvec2 heightMapUVsizeInput)
{
    heightMapUVsize = heightMapUVsizeInput;
    heightMap = heightMapInput;

    VAO = 0;
    VBO = 0;
    VAOclear = true;
    VBOclear = true;

    glPatchParameteri(GL_PATCH_VERTICES, 4); // set up patches for Tessellation

    // Create verts and set up VAO/VBO
    //setUpVertices();
    //setUpBuffers();
}


Ocean::~Ocean()
{
    if(!VAOclear)
        glDeleteVertexArrays(1, &VAO);
    if(!VBOclear)
        glDeleteBuffers(1, &VBO);

    //todo delete textures if added

}

void Ocean::draw(Shader& shader, Camera& camera, glm::mat4& projectionMatrix, glm::mat4& projectionMatrix2)
{
    shader.use();

    shader.setInt("heightMap", 0); // set shader uniform to sampler

    glActiveTexture(GL_TEXTURE0); // set sampler to texture
    glBindTexture(GL_TEXTURE_2D, heightMap);

    shader.setMat4("pvMatrix", projectionMatrix * camera.GetViewMatrix() );

    glBindVertexArray(VAO);
    //send verts as patch to Tessellation shader TEMP SET TO FAN
    glDrawArrays(GL_PATCHES, 0, vertices.size());

    glBindVertexArray(0); // unbind

}

float Ocean::linePlaneIntersectT(glm::vec3 P0, glm::vec3 P1, glm::vec3 P2, glm::vec3 La, glm::vec3 Lb){
    glm::vec3 L_ab = Lb - La;
    glm::vec3 P_01 = P1 - P0;
    glm::vec3 P_02 = P2 - P0;
    //float determinate = glm::dot(-L_ab, glm::cross(P_01, P_02));
    float t = (glm::dot( glm::cross(P_01, P_02), (La - P0)))/
              (glm::dot(-L_ab, glm::cross(P_01, P_02)));
    return t;
}

void Ocean::setUpVertices(glm::mat4& viewMatrix, glm::mat4& projectionMatrix)
{
    vertices.clear();
    frustumVerts.clear();
    float oceanLevel = 0;
    glm::mat4 inv = glm::inverse(projectionMatrix * viewMatrix);
    glm::vec4 temp;

    //clip space to frustum world space coords
    //bottom plane
    //close left (0,0)
    temp = inv * glm::vec4(-1,-1,0,1);
    temp /= temp.w;
    frustumVerts.push_back(glm::vec3(temp));
    //close right (1,0)
    temp = inv * glm::vec4(1,-1,0,1);
    temp /= temp.w;
    frustumVerts.push_back(glm::vec3(temp));
    //far left (0,1)
    temp = inv * glm::vec4(-1,-1,1,1);
    temp /= temp.w;
    frustumVerts.push_back(glm::vec3(temp));
    //far right (1,1)
    temp = inv * glm::vec4(1,-1,1,1);
    temp /= temp.w;
    frustumVerts.push_back(glm::vec3(temp));

    //top plane
    //close left (0,0)
    temp = inv * glm::vec4(-1,1,0,1);
    temp /= temp.w;
    frustumVerts.push_back(glm::vec3(temp));
    //close right (1,0)
    temp = inv * glm::vec4(1,1,0,1);
    temp /= temp.w;
    frustumVerts.push_back(glm::vec3(temp));
    //far left (0,1)
    temp = inv * glm::vec4(-1,1,1,1);
    temp /= temp.w;
    frustumVerts.push_back(glm::vec3(temp));
    //far right (1,1)
    temp = inv * glm::vec4(1,1,1,1);
    temp /= temp.w;
    frustumVerts.push_back(glm::vec3(temp));

    /*for(int i = 0; i < 8; i++){

        std::cout << frustumVerts[i].x << " " << frustumVerts[i].y << " " << frustumVerts[i].z << std::endl;
    }*/

    //define Ocean Plane (3 points
    glm::vec3 P0 = glm::vec3(0,oceanLevel,0);
    glm::vec3 P1 = glm::vec3(1,oceanLevel,0);
    glm::vec3 P2 = glm::vec3(0,oceanLevel,1);
    float result;

    //vertical frustum lines close left, close right, far left, far right
    for(int i = 0; i < 4; i++){

        result = this->linePlaneIntersectT(P0, P1, P2, frustumVerts[i], frustumVerts[i+4]);
        if(result < 1.0f && result > 0.0f){
            vertices.push_back(frustumVerts[i] + (frustumVerts[i+4] - frustumVerts[i])* result);
        }
    }

    //(bot) close/far (top) close/far
    for(int i = 0; i < 4; i++){

        result = this->linePlaneIntersectT(P0, P1, P2, frustumVerts[i*2], frustumVerts[(i*2)+1]);
        if(result < 1.0f && result > 0.0f){
            vertices.push_back(frustumVerts[i*2] + (frustumVerts[(i*2)+1] - frustumVerts[i*2])* result);
        }
    }
    //(bot) left/right (top) left/right
    result = this->linePlaneIntersectT(P0, P1, P2, frustumVerts[0], frustumVerts[2]);
    if(result < 1.0f && result > 0.0f){
        vertices.push_back(frustumVerts[0] + (frustumVerts[2] - frustumVerts[0])* result);
    }

    result = this->linePlaneIntersectT(P0, P1, P2, frustumVerts[1], frustumVerts[3]);
    if(result < 1.0f && result > 0.0f){
            vertices.push_back(frustumVerts[1] + (frustumVerts[3] - frustumVerts[1])* result);
    }

    result = this->linePlaneIntersectT(P0, P1, P2, frustumVerts[4], frustumVerts[6]);
    if(result < 1.0f && result > 0.0f){
            vertices.push_back(frustumVerts[4] + (frustumVerts[6] - frustumVerts[4])* result);
    }

    result = this->linePlaneIntersectT(P0, P1, P2, frustumVerts[5], frustumVerts[7]);
    if(result < 1.0f && result > 0.0f){
            vertices.push_back(frustumVerts[5] + (frustumVerts[7] - frustumVerts[5])* result);
    }

    std::cout << std::endl;
    for(int i = 0; i < 4; i++){

        std::cout << vertices[i].x << " " << vertices[i].y << " " << vertices[i].z << std::endl;
    }
}

void Ocean::setUpBuffers()
{
    if(!VAOclear)
        glDeleteVertexArrays(1, &VAO);
    if(!VBOclear)
        glDeleteBuffers(1, &VBO);


    VBOclear = false;// mark buffers as created
    VAOclear = false;

    //Set up Vertex Buffer
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);


    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
