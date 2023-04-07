#include "Grass.h"

Grass::Grass(unsigned int heightMapInput,  glm::uvec2 heightMapUVsizeInput, unsigned int windMapInput, glm::vec2 worldSizeInput , float newDensity, float newNearLOD, float newFarLOD)
{
    //ctor
    density = newDensity;
    nearLOD = newNearLOD;
    farLOD = newFarLOD;

    heightMap = heightMapInput;
    heightMapUVsize = heightMapUVsizeInput;
    worldSize = worldSizeInput;
    windMap = windMapInput;


    farModel = {glm::vec3(-0.08, 0, 0),
                 glm::vec3(0.08, 0, 0),
                 glm::vec3(0, 1.0, 0)
                };
    nearModel = {glm::vec3(-0.05, 0, 0),
                 glm::vec3(0.05, 0, 0),
                 glm::vec3(-0.06, 0.3, 0),
                 glm::vec3(-0.06, 0.3, 0),
                 glm::vec3(0.05, 0, 0),
                 glm::vec3(0.06, 0.3, 0),

                 glm::vec3(-0.06, 0.3, 0),
                 glm::vec3(0.06, 0.3, 0),
                 glm::vec3(-0.05, 0.6, 0),
                 glm::vec3(-0.05, 0.6, 0),
                 glm::vec3(0.06, 0.3, 0),
                 glm::vec3(0.05, 0.6, 0),

                 glm::vec3(-0.05, 0.6, 0),
                 glm::vec3(0.05, 0.6, 0),
                 glm::vec3(-0.02, 0.9, 0),
                 glm::vec3(-0.02, 0.9, 0),
                 glm::vec3(0.05, 0.6, 0),
                 glm::vec3(0.02, 0.9, 0),

                 glm::vec3(-0.02, 0.9, 0),
                 glm::vec3(0.02, 0.9, 0),
                 glm::vec3(0, 1.0, 0)

                };


    nearVBO = 0;
    farVBO = 0;
    instanceVBO = 0;
    nearVBOclear = true;
    farVBOclear = true;
    instanceVBOclear = true;


    totalChunks = chunksPerLine*chunksPerLine;
    chunkSize = (farLOD*2)/chunksPerLine;
    vertsPerChunkLine = std::round(chunkSize/density);
    setUpVertices();
    setUpBuffers();
}

Grass::~Grass()
{
    //dtor
    if(!nearVBOclear)
        glDeleteBuffers(1, &nearVBO);
    if(!farVBOclear)
        glDeleteBuffers(1, &farVBO);
    if(!instanceVBOclear)
        glDeleteBuffers(1,&instanceVBO);
    if(!nearChunkVAOArray.empty()){
        glDeleteBuffers(nearChunkVAOArray.size(),&nearChunkVAOArray.front());
    }
    if(!farChunkVAOArray.empty()){
        glDeleteBuffers(farChunkVAOArray.size(),&farChunkVAOArray.front());
    }
}


void Grass::changeSettings(float newDensity, float newNearLOD, float newFarLOD)
{
    density = newDensity;
    nearLOD = newNearLOD;
    farLOD = newFarLOD;

    totalChunks = chunksPerLine*chunksPerLine;
    chunkSize = (farLOD*2)/chunksPerLine;
    vertsPerChunkLine = std::round(chunkSize/density);

    setUpVertices();
    setUpBuffers();
}

void Grass::getSettings(float* outDensity, float* outNearLOD, float* outFarLOD)
{
    *outDensity = density;
    *outNearLOD = nearLOD;
    *outFarLOD = farLOD;
}

void Grass::draw(Shader& shader, glm::mat4& viewMatrix, glm::mat4& projectionMatrix, Camera& camera)
{
    glDisable(GL_CULL_FACE);
    shader.use();

    shader.setInt("heightMap", 0); // set shader uniform to sampler
    shader.setInt("windMap", 1);

    glActiveTexture(GL_TEXTURE0); // set sampler to texture
    glBindTexture(GL_TEXTURE_2D, heightMap);
    glActiveTexture(GL_TEXTURE1); // set sampler to texture
    glBindTexture(GL_TEXTURE_2D, windMap);


    int temp = 0;

    glm::mat4 pvMatrix = projectionMatrix * viewMatrix;
    shader.setMat4("pvMatrix", pvMatrix);
    shader.setFloat("farDist", farLOD);
    glUniform3fv(glGetUniformLocation(shader.ID, "playerPos"),1, glm::value_ptr(camera.Position));

    for(unsigned int i = 0; i < farChunkVAOArray.size(); i++){
        glm::vec3 chunkCenter = glm::vec3(((i%chunksPerLine)*chunkSize + (chunkSize/2))-farLOD, 0,((i/chunksPerLine)*chunkSize + (chunkSize/2))-farLOD);
        float modOffsetX = glm::floor((camera.Position.x)/(farLOD*2))*(farLOD*2);
        if(abs((modOffsetX + chunkCenter.x)-camera.Position.x) > farLOD){
            modOffsetX = modOffsetX + farLOD*2;
        }
        float modOffsetZ = glm::floor((camera.Position.z)/(farLOD*2))*(farLOD*2);
        if(abs((modOffsetZ + chunkCenter.z)-camera.Position.z) > farLOD){
            modOffsetZ = modOffsetZ + farLOD*2;
        }
        chunkCenter += glm::vec3(modOffsetX,camera.Elevation,modOffsetZ);
        bool looping = abs((chunkCenter.z)-camera.Position.z) > farLOD-(chunkSize) || abs((chunkCenter.x)-camera.Position.x) > farLOD-(chunkSize);

        glm::vec4 chunkCenterView = pvMatrix * glm::vec4(chunkCenter,1.0);
        if(chunkCenterView.z < -40.0 && !looping){ // do not render chunks not in view
            temp++; ///TODO add x and y axis
            continue;
        }

        if(glm::distance(chunkCenter,camera.Position) < nearLOD){
            shader.setBool("LODdist", 1);
            glBindVertexArray(nearChunkVAOArray.at(i));
            glDrawArraysInstanced(GL_TRIANGLES, 0, nearModel.size() , vertsPerChunkLine*vertsPerChunkLine);
        }else{
            shader.setBool("LODdist", 0);
            glBindVertexArray(farChunkVAOArray.at(i));
            glDrawArraysInstanced(GL_TRIANGLES, 0, farModel.size() , vertsPerChunkLine*vertsPerChunkLine);
        }

    }
    //std::cout << temp << std::endl;

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
    offsetVertices.clear();
    if(farLOD <= 0.0f){
        offsetVertices.resize(0);
        return;
    }

    for(unsigned int chunkY = 0; chunkY < chunksPerLine; chunkY++){
        for(unsigned int chunkX = 0; chunkX < chunksPerLine; chunkX++){
            for(unsigned int vertY = 0; vertY < vertsPerChunkLine; vertY++){
                for(unsigned int vertX = 0; vertX < vertsPerChunkLine; vertX++){
                    float y = ((chunkY*chunkSize)-farLOD) + (chunkSize/vertsPerChunkLine)*vertY;
                    float x = ((chunkX*chunkSize)-farLOD) + (chunkSize/vertsPerChunkLine)*vertX;
                    glm::uvec3 hashVec = pcg3d(glm::uvec3((unsigned int) (y*(vertsPerChunkLine/chunkSize)), (unsigned int) (x*(vertsPerChunkLine/chunkSize)), 7632978u));
                    float hashx = ((static_cast<float>(hashVec.x)/UINT_MAX) - 0.5)* 2;
                    float hashy = ((static_cast<float>(hashVec.y)/UINT_MAX) - 0.5) * 2;
                    offsetVertices.push_back(glm::vec2(x + (hashx * (chunkSize/vertsPerChunkLine)),y + (hashy * (chunkSize/vertsPerChunkLine))));
                }
            }
        }
    }
    std::cout << offsetVertices.size() <<std::endl;
}

void Grass::setUpBuffers()
{
    if(!nearVBOclear)
        glDeleteBuffers(1, &nearVBO);
    if(!farVBOclear)
        glDeleteBuffers(1, &farVBO);
    if(!instanceVBOclear)
        glDeleteBuffers(1,&instanceVBO);
    if(!nearChunkVAOArray.empty()){
        glDeleteVertexArrays(nearChunkVAOArray.size(),&nearChunkVAOArray.front());
    }
    if(!farChunkVAOArray.empty()){
        glDeleteVertexArrays(farChunkVAOArray.size(),&farChunkVAOArray.front());
    }

    nearVBOclear = false;// mark buffers as created
    farVBOclear = false;
    instanceVBOclear = false;

    //set up chunkVAO's
    nearChunkVAOArray.clear();
    nearChunkVAOArray.assign(totalChunks, 0);// set each vao to zero before calling glGenBuffers
    glGenVertexArrays(totalChunks, &nearChunkVAOArray.front());
    farChunkVAOArray.clear();
    farChunkVAOArray.assign(totalChunks, 0);// set each vao to zero before calling glGenBuffers
    glGenVertexArrays(totalChunks, &farChunkVAOArray.front());

    //set up instance  buffer
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * offsetVertices.size(), &offsetVertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //set up Near Model Vertex Buffer
    glGenBuffers(1, &nearVBO);
    glBindBuffer(GL_ARRAY_BUFFER, nearVBO);
    glBufferData(GL_ARRAY_BUFFER, nearModel.size()* sizeof(glm::vec3), &nearModel[0] , GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //set up far Model Vertex Buffer
    glGenBuffers(1, &farVBO);
    glBindBuffer(GL_ARRAY_BUFFER, farVBO);
    glBufferData(GL_ARRAY_BUFFER, farModel.size()* sizeof(glm::vec3), &farModel[0] , GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    for(unsigned int i = 0; i < farChunkVAOArray.size(); i++){
        //Bind to Vertex Array object
        glBindVertexArray(farChunkVAOArray.at(i));

        //Set far Model buffer data
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, farVBO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

        // also set instance data
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO); // this attribute comes from a different vertex buffer
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)(vertsPerChunkLine*vertsPerChunkLine*i*sizeof(glm::vec2)));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glVertexAttribDivisor(1, 1); // tell OpenGL this is an instanced vertex attribute.
    }

    for(unsigned int i = 0; i < nearChunkVAOArray.size(); i++){
        //Bind to Vertex Array object
        glBindVertexArray(nearChunkVAOArray.at(i));

        //Set near Model buffer data
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, nearVBO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

        // also set instance data
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO); // this attribute comes from a different vertex buffer
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)(vertsPerChunkLine*vertsPerChunkLine*i*sizeof(glm::vec2)));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glVertexAttribDivisor(1, 1); // tell OpenGL this is an instanced vertex attribute.
    }
}
