#include "TessChunk.h"

TessChunk::TessChunk(glm::vec3 inPosition, int inWidth, unsigned int inTexture, unsigned int inheightMapTexture, glm::vec2 inHeightMapUV, float inHeightMapUVScale)
{
    //ctor
    position = inPosition;
    width = inWidth;
    heightMap = inheightMapTexture;
    heightMapUV = inHeightMapUV;
    heightMapUVScale = inHeightMapUVScale;
    glPatchParameteri(GL_PATCH_VERTICES, 4); // set up patches for Tessellation
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

    shader.setInt("heightMap", 0); // set uniform to sampler

    glActiveTexture(GL_TEXTURE0); // set sampler to texture
    glBindTexture(GL_TEXTURE_2D, heightMap);

     glBindVertexArray(VAO);
     glm::mat4 model = glm::mat4(1.0f);
     model = glm::translate(model, position);
     shader.setMat4("model", model);

     glDrawArrays(GL_PATCHES, 0, 4);

}

void TessChunk::setUpVertices()
{
    Vertex tempVertex;
    //Create Chunk Patch
    //bottom left (0,0)
    tempVertex.posCoords = glm::vec3(0.0f, 0.0f, 0.0f);
    tempVertex.texCoords = heightMapUV;
    vertices.push_back(tempVertex);

    //bottom right (1,0)
    tempVertex.posCoords = glm::vec3(width * 1.0f , 0.0f, 0.0f);
    tempVertex.texCoords = heightMapUV + glm::vec2(heightMapUVScale, 0.0f);
    vertices.push_back(tempVertex);

        //top left (0,1)
    tempVertex.posCoords = glm::vec3(0.0f, 0.0f, width * -1.0f); //-z is north
    tempVertex.texCoords = heightMapUV + glm::vec2(0.0f, heightMapUVScale);
    vertices.push_back(tempVertex);

    //top right (1,1)
    tempVertex.posCoords = glm::vec3(width * 1.0f, 0.0f, width * -1.0f); //-z is north
    tempVertex.texCoords =  heightMapUV +glm::vec2(heightMapUVScale, heightMapUVScale);
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
