#include "chunk.h"

Chunk::Chunk(glm::vec3 inPosition, int subdivisions, unsigned int inTexture)
{
    //ctor
    position = inPosition;
    subdivs = subdivisions;
    heights = std::vector<float>(subdivisions*subdivisions + 1, 0.0f);
    setUpVertices();
    setUpBuffers();
    texture = inTexture;

}

Chunk::~Chunk()
{
    //dtor
    if(!VAOclear)
        glDeleteVertexArrays(1, &VAO);
    if(!VBOclear)
        glDeleteBuffers(1, &VBO);

    glDeleteTextures(1, &texture);
}

void Chunk::draw(Shader& shader)
{
    shader.use();

    shader.setInt("texture1",0); // set uniform to sampler

    glActiveTexture(GL_TEXTURE0); // set sampler to texture
    glBindTexture(GL_TEXTURE_2D, texture);

     glBindVertexArray(VAO);
     glm::mat4 model = glm::mat4(1.0f);
     model = glm::translate(model, position);
     shader.setMat4("model", model);
     glDrawArrays(GL_TRIANGLES, 0, subdivs * subdivs * 6);

}

void Chunk::setUpVertices()
{
    Vertex tempVertex;
    for(int x = 0; x < subdivs; x++){
        for(int z = 0; z < subdivs; z++){
            //std::cout << x << ',' << z << std::endl;
            //tri 1
            //bottom left (0,0) (x,z)
            tempVertex.posCoords = glm::vec3(x,heights.at(x*(subdivs-1) + z),-z);
            tempVertex.texCoords = glm::vec2(0.0f, 0.0f);
            vertices.push_back(tempVertex);

            //top right (1,-1)
            tempVertex.posCoords = glm::vec3((x+1),heights.at((x+1)*(subdivs-1) + (z+1)),-z-1);
            tempVertex.texCoords = glm::vec2(1.0f, 1.0f);
            vertices.push_back(tempVertex);

            //top left (0,-1)
            tempVertex.posCoords = glm::vec3(x,heights.at(x*(subdivs-1) + (z+1)),-z-1);
            tempVertex.texCoords = glm::vec2(0.0f, 1.0f);
            vertices.push_back(tempVertex);

            //tri 2
            //top right (1,-1)
            tempVertex.posCoords = glm::vec3((x+1),heights.at((x+1)*(subdivs-1) + (z+1)),-z-1);
            tempVertex.texCoords = glm::vec2(1.0f, 1.0f);
            vertices.push_back(tempVertex);

            //bottom left (0,-0)
            tempVertex.posCoords = glm::vec3(x,heights.at(x*(subdivs-1) + z),-z);
            tempVertex.texCoords = glm::vec2(0.0f, 0.0f);
            vertices.push_back(tempVertex);

            //bottom right (1,0)
            tempVertex.posCoords = glm::vec3((x+1),heights.at((x+1)*(subdivs-1) + z),-z);
            tempVertex.texCoords = glm::vec2(1.0f, 0.0f);
            vertices.push_back(tempVertex);
        }
    }
}

void Chunk::setUpBuffers()
{
    //If buffer exists, delete old buffer before creating new buffer
    if(!VAOclear)
        glDeleteVertexArrays(1, &VAO);
    if(!VBOclear)
        glDeleteBuffers(1, &VBO);

    VBOclear = false;
    VAOclear = false;

    //Set up Vertex Buffer
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);


    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex texture coords
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

    glBindVertexArray(0);
}
