#include "Ocean.h"
#include <iostream>
#include <limits>
#include <algorithm>

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
    glUniform3fv(glGetUniformLocation(shader.ID, "testQuad"), 1, glm::value_ptr(testPoint));//TODO remove

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

bool Ocean::isLeft(glm::vec2 a, glm::vec2 b, glm::vec2 c){
    /// c is left of line a->b
     return ((b.x - a.x)*(c.y - a.y) - (b.y - a.y)*(c.x - a.x)) > 0;
}

void Ocean::pushQuad(glm::vec3 corner, float scale, float playerY)
{
    if(corner.y < playerY){ // if player is above ocean
            //bottom left (0,0)
            vertices.push_back(corner);
            //bottom right (1,0)
            vertices.push_back(corner + glm::vec3(scale,0,0));
            //top left (0,1)
            vertices.push_back(corner + glm::vec3(0,0,-scale)); // z is in the negative axis
            //top right (1,1)
            vertices.push_back(corner + glm::vec3(scale,0,-scale));// z is in the negative axis
        }else{// if player is below ocean
            //top left (0,1)
            vertices.push_back(corner + glm::vec3(0,0,-scale)); // z is in the negative axis
            //top right (1,1)
            vertices.push_back(corner + glm::vec3(scale,0,-scale));// z is in the negative axis
            //bottom left (0,0)
            vertices.push_back(corner);
            //bottom right (1,0)
            vertices.push_back(corner + glm::vec3(scale,0,0));
        }
}


void Ocean::setUpVertices(glm::mat4& viewMatrix, glm::mat4& projectionMatrix, glm::vec3& playerPosition, float oceanLevel, int width, int height)
{
    vertices.clear();
    frustumVerts.clear();
    lineVerts.clear();
    std::vector<bool> pass;
    pass.resize((width+1)*(height+1));
    glm::mat4 inv = glm::inverse(projectionMatrix * viewMatrix); //TODO calculate from scratch
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

    //define Ocean Plane (3 points
    glm::vec3 P0 = glm::vec3(0,oceanLevel,0);
    glm::vec3 P1 = glm::vec3(1,oceanLevel,0);
    glm::vec3 P2 = glm::vec3(0,oceanLevel,1);
    float result;

    //bottom of screen to top of screen
    for(int y = 0; y <= height; y++){
        for(int x = 0; x <= width; x++){
            float clipY = (y/static_cast<float>(height))*2 - 1;
            float clipX = (x/static_cast<float>(width))*2 - 1;
            //close left
            temp = inv * glm::vec4(clipX,clipY,0,1);
            temp /= temp.w;
            glm::vec3 temp0 = glm::vec3(temp);
            //far left
            temp = inv * glm::vec4(clipX,clipY,1,1);
            temp /= temp.w;
            glm::vec3 temp2 = glm::vec3(temp);

            result = this->linePlaneIntersectT(P0, P1, P2, temp0, temp2);
            if(result < 1.0f && result > 0.0f){
                lineVerts.push_back(temp0 + (temp2 - temp0)* result);
                pass.at(y*(width+1) + x) = true;
            }else{
                pass.at(y*(width+1) + x) = false;
            }
        }
    }

    if(lineVerts.size() != 0 && lineVerts.size() < (height+1)*(width+1)){
        //if ocean is onscreen but does not fill up the whole screen add far plane line
        result = this->linePlaneIntersectT(P0, P1, P2, frustumVerts[2], frustumVerts[2+4]);
        glm::vec3 farLeft = frustumVerts[2] + (frustumVerts[2+4] - frustumVerts[2])* result;

        result = this->linePlaneIntersectT(P0, P1, P2, frustumVerts[3], frustumVerts[3+4]);
        glm::vec3 farRight = frustumVerts[3] + (frustumVerts[3+4] - frustumVerts[3])* result;
        for(int x = 0; x <= width; x++){
            float t = (x/static_cast<float>(width));
            lineVerts.push_back(glm::mix(farLeft,farRight,t));
            std::cout << "push far plane" << std::endl;
        }
    }

    for(int i = 0; i < pass.size(); i++){

        std::cout << pass.at(i) << std::endl;
    }

    for(int i = 0; i < lineVerts.size(); i++){

        std::cout << lineVerts[i].x << " " << lineVerts[i].y << " " << lineVerts[i].z << std::endl;
    }
    std::cout << "_________"<< std::endl;
    std::cout << "size: " << lineVerts.size() << std::endl;

    if(lineVerts.size() != 0){
        for(int i = 0; i < lineVerts.size()-(width+1); i += width+1){
            for(int x = 0; x < width; x++){
                //bottom left (0,0)
                vertices.push_back(lineVerts.at(i+x));

                //bottom right (1,0)
                vertices.push_back(lineVerts.at(i+x+1));

                //top left (0,1)
                vertices.push_back(lineVerts.at(i+width+1+x));

                //top right (1,1)
                vertices.push_back(lineVerts.at(i+width+1+x+1));

            }

        }
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
