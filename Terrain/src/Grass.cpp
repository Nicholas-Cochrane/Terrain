#include "Grass.h"

Grass::Grass(unsigned int heightMapInput,  glm::uvec2 heightMapUVsizeInput, unsigned int windMapInput, unsigned int grassHeightMapInput,  glm::vec2 worldSizeInput, float newDensity, float newNearLOD, float newFarLOD)
{
    //ctor
    density = newDensity;
    nearLOD = newNearLOD;
    farLOD = newFarLOD;

    heightMap = heightMapInput;
    heightMapUVsize = heightMapUVsizeInput;
    worldSize = worldSizeInput;
    windMap = windMapInput;
    grassHeightMap = grassHeightMapInput;


    /*farModel = {glm::vec3(-0.08, 0, 0),
                 glm::vec3(0.08, 0, 0),
                 glm::vec3(0, 1.0, 0)
                };*/


    farModel = {glm::vec3(-0.05, 0, 0),
                 glm::vec3(0.05, 0, 0),
                 glm::vec3(-0.05, 0.6, 0),
                 glm::vec3(-0.05, 0.6, 0),
                 glm::vec3(0.05, 0, 0),
                 glm::vec3(0.05, 0.6, 0),

                 glm::vec3(-0.05, 0.6, 0),
                 glm::vec3(0.05, 0.6, 0),
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

void Grass::draw(Shader& shader, glm::mat4& viewMatrix, glm::mat4& projectionMatrix, Camera& camera, bilinearMapReader& heightMapReader, float heightMapMax, float gameSize)
{
    glDisable(GL_CULL_FACE);
    shader.use();

    shader.setInt("heightMap", 0); // set shader uniform to sampler
    shader.setInt("windMap", 1);
    shader.setInt("grassHeightMap", 2);

    glActiveTexture(GL_TEXTURE0); // set sampler to texture
    glBindTexture(GL_TEXTURE_2D, heightMap);
    glActiveTexture(GL_TEXTURE1); // set sampler to texture
    glBindTexture(GL_TEXTURE_2D, windMap);
    glActiveTexture(GL_TEXTURE2); // set sampler to texture
    glBindTexture(GL_TEXTURE_2D, grassHeightMap);



    glm::mat4 pvMatrix = projectionMatrix * viewMatrix;
    shader.setMat4("pvMatrix", pvMatrix);
    shader.setFloat("farDist", farLOD);

    unsigned int numberOfCorners = (chunksPerLine+1)*(chunksPerLine+1);
    std::vector<glm::vec4> cornerViewCoords;
    std::vector<glm::vec4> chunkCenters;
    chunkCenters.resize(chunksPerLine * chunksPerLine);
    cornerViewCoords.resize(numberOfCorners);

    for(unsigned int c = 0; c < numberOfCorners; c++){ ///TODO add look up for elevation
        glm::vec3 corner = glm::vec3(((c%(chunksPerLine + 1))*chunkSize) -farLOD, 0, ((c/(chunksPerLine + 1))*chunkSize) -farLOD );
        float modOffsetX = glm::floor((camera.Position.x)/(farLOD*2))*(farLOD*2);
        if(abs((modOffsetX + corner.x)-camera.Position.x) > farLOD){
            modOffsetX = modOffsetX + farLOD*2;
        }
        float modOffsetZ = glm::floor((camera.Position.z)/(farLOD*2))*(farLOD*2);
        if(abs((modOffsetZ + corner.z)-camera.Position.z) > farLOD){
            modOffsetZ = modOffsetZ + farLOD*2;
        }
        corner += glm::vec3(modOffsetX,0,modOffsetZ); ///TODO add look up for elevation
        corner.y = heightMapReader.read(corner.x, -corner.z, gameSize, 0) * heightMapMax;
        //bool looping = abs((corner.z)-camera.Position.z) > farLOD-(chunkSize) || abs((corner.x)-camera.Position.x) > farLOD-(chunkSize);

        cornerViewCoords.at(c) = pvMatrix * glm::vec4(corner, 1.0);
        cornerViewCoords.at(c).x = cornerViewCoords.at(c).x / cornerViewCoords.at(c).w;
        cornerViewCoords.at(c).y = cornerViewCoords.at(c).y / cornerViewCoords.at(c).w;
    }

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
        float distToCenter = glm::distance(chunkCenter,camera.Position) ;

        int currCorner = i + (i/(chunksPerLine));

        if(((  cornerViewCoords.at(currCorner).z < -1.1 // top "left" corner inbounds check (not behind camera)
           && cornerViewCoords.at(currCorner + 1).z < -1.1 //top "right"
           && cornerViewCoords.at(currCorner + chunksPerLine + 1).z < -1.1 // bottom "left"
           && cornerViewCoords.at(currCorner + chunksPerLine + 2).z < -1.1)// bottom "right"
           ||( cornerViewCoords.at(currCorner).x < -1.1 //left
           && cornerViewCoords.at(currCorner + 1).x < -1.1
           && cornerViewCoords.at(currCorner + chunksPerLine + 1).x < -1.1
           && cornerViewCoords.at(currCorner + chunksPerLine + 2).x < -1.1)
           ||( cornerViewCoords.at(currCorner).x > 1.1 //right
           && cornerViewCoords.at(currCorner + 1).x > 1.1
           && cornerViewCoords.at(currCorner + chunksPerLine + 1).x > 1.1
           && cornerViewCoords.at(currCorner + chunksPerLine + 2).x > 1.1)
           ||( cornerViewCoords.at(currCorner).y > 1.1 //above (ex: when looking down)
           && cornerViewCoords.at(currCorner + 1).y > 1.1
           && cornerViewCoords.at(currCorner + chunksPerLine + 1).y > 1.1
           && cornerViewCoords.at(currCorner + chunksPerLine + 2).y > 1.1)
           ||( cornerViewCoords.at(currCorner).y < -1.1 // below
           && cornerViewCoords.at(currCorner + 1).y < -1.1
           && cornerViewCoords.at(currCorner + chunksPerLine + 1).y < -1.1
           && cornerViewCoords.at(currCorner + chunksPerLine + 2).y < -1.1))

           && distToCenter > chunkSize) // not close to camera to avoid clipping chuck that takes up whole screen but edges are off screen
           {
            // if  all corners are off screen
            continue;
        }

        if(distToCenter < nearLOD){
            shader.setBool("LODdist", 1);
            glBindVertexArray(nearChunkVAOArray.at(i));
            glDrawArraysInstanced(GL_TRIANGLES, 0, nearModel.size() , vertsPerChunkLine*vertsPerChunkLine);
        }else if(glm::distance(chunkCenter,camera.Position) < (2.0f * farLOD) ){ // if more than twice farLOD away do not render (ex: when flying)
            shader.setBool("LODdist", 0);
            glBindVertexArray(farChunkVAOArray.at(i));
            glDrawArraysInstanced(GL_TRIANGLES, 0, farModel.size() , vertsPerChunkLine*vertsPerChunkLine);
        }

    }
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
