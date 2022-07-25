#include "TessChunk.h"

TessChunk::TessChunk(glm::vec3 inPosition, int subdivisions, unsigned int inTexture)
{
    //ctor
    position = inPosition;
    subdivs = subdivisions;
    heights = std::vector<float>(subdivisions*subdivisions + 1, 0.0f);
    setUpVertices();
    setUpBuffers();
    texture = inTexture;

}

TessChunk::~TessChunk()
{
    //dtor
    if(!VAOclear)
        glDeleteVertexArrays(1, &VAO);
    if(!VBOclear)
        glDeleteBuffers(1, &VBO);

    glDeleteTextures(1, &texture);
}

void TessChunk::draw(Shader& shader)
{
    shader.use();

    shader.setInt("texture1",0); // set uniform to sampler

    glActiveTexture(GL_TEXTURE0); // set sampler to texture
    glBindTexture(GL_TEXTURE_2D, texture);

     glBindVertexArray(VAO);
     glm::mat4 model = glm::mat4(1.0f);
     shader.setMat4("model", model);

     glDrawArrays(GL_PATCHES, 0, 4*subdivs*subdivs);

}

void TessChunk::setUpVertices()
{
    Vertex tempVertex;
    //Create Chunk Patch
    //bottom left (0,0)
    tempVertex.posCoords = glm::vec3(0.0f, 0.0f, 0.0f);
    tempVertex.texCoords = glm::vec2(0.0f, 0.0f);
    vertices.push_back(tempVertex);

    //bottom right (1,0)
    tempVertex.posCoords = glm::vec3(subdivs * 1.0f , 0.0f, 0.0f);
    tempVertex.texCoords = glm::vec2(1.0f, 0.0f);
    vertices.push_back(tempVertex);

        //top left (0,1)
    tempVertex.posCoords = glm::vec3(0.0f, 0.0f, subdivs * -1.0f); //-z is north
    tempVertex.texCoords = glm::vec2(0.0f, 1.0f);
    vertices.push_back(tempVertex);

    //top right (1,1)
    tempVertex.posCoords = glm::vec3(subdivs * 1.0f, 0.0f, subdivs * -1.0f); //-z is north
    tempVertex.texCoords = glm::vec2(1.0f, 1.0f);
    vertices.push_back(tempVertex);

}

void TessChunk::setUpBuffers()
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
