#include "TessChunk.h"

TessChunk::TessChunk(glm::vec3 inPosition, int inWidth, int inPatchesPerEdge, unsigned int inTexture, unsigned int inheightMapTexture, glm::vec2 inHeightMapUV, float inHeightMapUVScale)
{
    //ctor
    VAO = 0;
    VBO = 0;
    VAOclear = true;
    VBOclear = true;
    position = inPosition;
    width = inWidth;
    patchesPerEdge = inPatchesPerEdge;
    texture = inTexture;
    heightMap = inheightMapTexture;
    heightMapUV = inHeightMapUV;
    heightMapUVScale = inHeightMapUVScale;
    glPatchParameteri(GL_PATCH_VERTICES, 4); // set up patches for Tessellation

    // Create verts and set up VAO/VBO
    setUpVertices();
    setUpBuffers();

}

TessChunk::~TessChunk()
{
    //dtor
    //delete VAO and VBO
    if(!VAOclear)
        glDeleteVertexArrays(1, &VAO);
    if(!VBOclear)
        glDeleteBuffers(1, &VBO);

    glDeleteTextures(1, &texture);
}

void TessChunk::draw(Shader &shader, glm::mat4 &viewMatrix, glm::mat4 &projectionMatrix, glm::mat4 &projectionMatrix2)
{
    shader.use();

    shader.setInt("heightMap", 0); // set shader uniform to sampler

    glActiveTexture(GL_TEXTURE0); // set sampler to texture
    glBindTexture(GL_TEXTURE_2D, heightMap);

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    shader.setMat4("mpvMatrix", projectionMatrix * viewMatrix * modelMatrix );
    shader.setMat4("mpvMatrix2", projectionMatrix2 * viewMatrix * modelMatrix);
    shader.setMat4("mvMatrix", viewMatrix * modelMatrix);

    glBindVertexArray(VAO);
    glDrawArrays(GL_PATCHES, 0, 4*patchesPerEdge*patchesPerEdge); //send verts as patch to Tessellation shader
}

void TessChunk::setUpVertices()
{
    Vertex tempVertex;
    vertices.reserve(patchesPerEdge*patchesPerEdge * 4 + 1);// expand vector to expected size +1 to avoid unneeded resizes.
    for(int x = 0; x < patchesPerEdge; x++){
        for(int z = 0; z < patchesPerEdge; z++){// note: -z is forward in coord space
            //Create Chunk Patch
            //posCoords x = (Corner x coordinate) + width * x
            //          y = 0.0f
            //          z = (Corner z coordinate) - width * z (-z is north)
            // Position Offset is done with model matrix
            // Texture Offset is done via heightMapUV
            //texCoords uv = (bottom right UV coordinate) + heightMapUVScale/patchesPerEdge * x
            //          heightMapUVScale is from bottom to top (N to S) and left to right (E to W) of UV coords of total chunk and thus must be divided by the number of patches


            //bottom left (0,0)
            tempVertex.posCoords = glm::vec3(width* x, 0.0f, 0.0f - width * z);
            tempVertex.texCoords = heightMapUV + glm::vec2(heightMapUVScale/patchesPerEdge * x, heightMapUVScale/patchesPerEdge * z);
            vertices.push_back(tempVertex);

            //bottom right (1,0)
            tempVertex.posCoords = glm::vec3(width + width* x, 0.0f, 0.0f - width * z);
            tempVertex.texCoords = heightMapUV + glm::vec2(heightMapUVScale/patchesPerEdge + heightMapUVScale/patchesPerEdge * x, 0.0f + heightMapUVScale/patchesPerEdge * z);
            vertices.push_back(tempVertex);

                //top left (0,1)
            tempVertex.posCoords = glm::vec3(width* x, 0.0f, width * -1.0f - width * z);//(-z is north)
            tempVertex.texCoords = heightMapUV + glm::vec2(0.0f + heightMapUVScale/patchesPerEdge * x, heightMapUVScale/patchesPerEdge + heightMapUVScale/patchesPerEdge * z);
            vertices.push_back(tempVertex);

            //top right (1,1)
            tempVertex.posCoords = glm::vec3(width + width* x, 0.0f, width * -1.0f - width * z);//(-z is north)
            tempVertex.texCoords =  heightMapUV + glm::vec2(heightMapUVScale/patchesPerEdge + heightMapUVScale/patchesPerEdge * x, heightMapUVScale/patchesPerEdge + heightMapUVScale/patchesPerEdge * z);
            vertices.push_back(tempVertex);
        }
    }

}

void TessChunk::setUpBuffers()
{
    //If buffer exists, delete old buffer before creating new buffer
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
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);


    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex texture coords
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

    glBindVertexArray(0);
}
