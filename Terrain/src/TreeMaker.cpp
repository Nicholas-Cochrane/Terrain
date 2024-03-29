#include "TreeMaker.h"
#include <iostream>
#include <math.h>
#include <vector>
#include <stack>
#include <stdexcept>
#include <cstdio>

TreeMaker::TreeMaker()
{
    //ctor
}

TreeMaker::~TreeMaker()
{
    //dtor
}

void TreeMaker::createNewTree()
{
    float startRadius = 0.4;
    treeModel newTree;
    //root node
    treeNode currNode(glm::vec3(0.0f,-1.0f,0.0f), startRadius);
    newTree.branchTree.push_back(currNode);
    size_t lastNodeIndex = 0;

    //next node
    currNode.nodeCoords = glm::vec3(0.5f,2.0f,0.5f);
    currNode.radius = startRadius;
    newTree.branchTree.push_back(currNode);
    //link parent to child (lastNode -> currNode)
    newTree.branchTree.at(lastNodeIndex).children.push_back(newTree.branchTree.size()-1);
    lastNodeIndex++;

    //next node
    currNode.nodeCoords = glm::vec3(1.0f,3.0f,1.0f);
    currNode.radius = startRadius;
    newTree.branchTree.push_back(currNode);
    //link parent to child (lastNode -> currNode)
    newTree.branchTree.at(lastNodeIndex).children.push_back(newTree.branchTree.size()-1);
    lastNodeIndex++;

    //next node
    currNode.nodeCoords = glm::vec3(1.5f,3.5f,1.5f);
    currNode.radius = startRadius;
    newTree.branchTree.push_back(currNode);
    //link parent to child (lastNode -> currNode)
    newTree.branchTree.at(lastNodeIndex).children.push_back(newTree.branchTree.size()-1);
    lastNodeIndex++;

    //next node
    currNode.nodeCoords = glm::vec3(2.5f,3.5f,2.5f);
    currNode.radius = startRadius;
    newTree.branchTree.push_back(currNode);
    //link parent to child (lastNode -> currNode)
    newTree.branchTree.at(lastNodeIndex).children.push_back(newTree.branchTree.size()-1);
    lastNodeIndex++;

    //next node
    currNode.nodeCoords = glm::vec3(3.0f,3.5f,3.0f);
    currNode.radius = startRadius;
    newTree.branchTree.push_back(currNode);
    //link parent to child (lastNode -> currNode)
    newTree.branchTree.at(lastNodeIndex).children.push_back(newTree.branchTree.size()-1);
    lastNodeIndex++;

    //next node
    currNode.nodeCoords = glm::vec3(6.0f,3.5f,6.0f);
    currNode.radius = startRadius;
    newTree.branchTree.push_back(currNode);
    //link parent to child (lastNode -> currNode)
    newTree.branchTree.at(lastNodeIndex).children.push_back(newTree.branchTree.size()-1);
    lastNodeIndex++;


    createTreeMesh(newTree); // create mesh, vao, vbo
    treeList.push_back(newTree);// add finished tree to list
}

glm::mat3 TreeMaker::rotateToVec(const glm::vec3 target)
{//note: if rotation does not work correction trying a transformation to (0,0) and back
    glm::vec3 targetNorm = glm::normalize(target);
    const glm::vec3 down = glm::normalize(glm::vec3(0,-1,0));
    glm::vec3 v = glm::cross(down, targetNorm);
    float s = v.length(); // sin of angle
    float c = glm::dot(down, targetNorm); // cos of angle

    glm::mat3 M = glm::mat3(0, -v.z, v.y,
                            v.z, 0, -v.x,
                            -v.y, v.x, 0
                            );
    glm::mat3 identity = glm::mat4(1.0f);
    return identity + M + (M*M)*((1-c)/(s*s));
}


void TreeMaker::createMeshFromRings(std::vector<Trees::Vertex>& currRing, std::vector<Trees::Vertex>& lastRing, std::vector<Trees::Vertex>& mesh, float currTexY, float lastTexY)
{
    //Create mesh from rings (clockwise order)
    if(currRing.size() != lastRing.size()){
            if(currRing.size() > lastRing.size()){
                std::size_t quotient = currRing.size()/lastRing.size();
                std::vector<std::size_t> numberOfTris(lastRing.size(), quotient); //lastRing(smaller ring) --> currRing(bigger ring)
                std::size_t unallocatedTris = currRing.size() - (lastRing.size()*quotient);
                if(unallocatedTris != 0){
                    std::size_t gapSize = lastRing.size()/unallocatedTris;
                    float gapFloat= static_cast<float>(lastRing.size())/static_cast<float>(unallocatedTris);

                    std::size_t i = 0;
                    while(i < lastRing.size() && unallocatedTris > 0){
                        if((static_cast<float>(lastRing.size()-i)/static_cast<float>(unallocatedTris)) > gapFloat){
                            i +=1;
                        }
                        numberOfTris.at(i) += 1;
                        i += gapSize;
                        unallocatedTris--;
                    }
                }
                /*int total = 0;
                for(auto &elem: numberOfTris){
                    std::cout << elem << ", ";
                    total += elem;
                }
                std::cout << total <<std::endl;*/


                //for edge tex wrap case

                mesh.push_back(Vertex(lastRing.at(0).posCoords, glm::vec2(1.0f,lastTexY), lastRing.at(0).normCoords)); // start point on last ring (small)
                mesh.push_back(Vertex(currRing.at(0).posCoords, glm::vec2(1.0f,currTexY), currRing.at(0).normCoords)); // start point on curr ring (large)
                mesh.push_back(currRing.at(currRing.size()-1)); // end point on curr ring (large)

                size_t postAdd = 0; // how many points to go from 0 forwards
                size_t preAdd = 0; // how many points to go from 0 backwards
                size_t currLargeRingPoint = 0;
                if(numberOfTris.at(0) <= 3){ // is 1,2 or 3 points

                    mesh.push_back(Vertex(lastRing.at(0).posCoords, glm::vec2(1.0f,lastTexY), lastRing.at(0).normCoords)); //start point on last ring (small)
                    mesh.push_back(currRing.at(currRing.size()-1)); // end point on curr ring (large)
                    mesh.push_back(lastRing.at(lastRing.size()-1)); // corresponding point on last ring (small)

                    if(numberOfTris.at(0) != 1){
                        postAdd = numberOfTris.at(0)-1;
                        currLargeRingPoint = postAdd;
                    }
                }else{
                    if(numberOfTris.at(0)%2 == 0){ // if even
                        preAdd = numberOfTris.at(0)/2;
                        postAdd = preAdd;
                        currLargeRingPoint = postAdd;
                    }else{
                        preAdd = (numberOfTris.at(0)-1)/2;
                        postAdd = preAdd +1;
                        currLargeRingPoint = postAdd;
                    }
                }

                //preAdd (goes to texture.x = 1)
                //Pre add must connect to smallring n-1 on farthest point
                if(preAdd > 0){
                    mesh.push_back(Vertex(lastRing.at(0).posCoords, glm::vec2(1.0f,0.0f), lastRing.at(0).normCoords)); //start point on last ring (small)
                    mesh.push_back(currRing.at(currRing.size()-preAdd)); // end point on curr ring (large)
                    mesh.push_back(lastRing.at(lastRing.size()-1)); // previous point on last ring (small)
                    preAdd--;
                }
                for(;preAdd > 0; preAdd--){
                    mesh.push_back(Vertex(lastRing.at(0).posCoords, glm::vec2(1.0f,0.0f), lastRing.at(0).normCoords)); //start point on last ring (small)
                    mesh.push_back(currRing.at(currRing.size()-preAdd));
                    mesh.push_back(currRing.at(currRing.size()-(preAdd+1)));
                }

                //postAdd(comes from texture.x = 0)
                for(;postAdd > 0; postAdd--){
                    mesh.push_back(lastRing.at(0)); //start point on last ring (small)
                    mesh.push_back(currRing.at(postAdd));
                    mesh.push_back(currRing.at(postAdd-1));
                }

                //smallring points from [1] to [n]
                for(std::size_t point = 1; point < lastRing.size(); point++){
                    size_t targetPoint = currLargeRingPoint + numberOfTris.at(point);
                    //connect to last point
                    mesh.push_back(lastRing.at(point)); //current point on last ring (small)
                    mesh.push_back(currRing.at(currLargeRingPoint)); //last point used on larger ring
                    mesh.push_back(lastRing.at(point-1)); //last point on last ring (small)
                    currLargeRingPoint++;

                    for(;currLargeRingPoint < targetPoint; currLargeRingPoint++){
                        mesh.push_back(lastRing.at(point)); //current point on last ring (small)
                        mesh.push_back(currRing.at(currLargeRingPoint)); //large ring
                        mesh.push_back(currRing.at(currLargeRingPoint-1)); //large ring
                    }
                    //complete final tri with out incrementing currLargeRingPoint
                    mesh.push_back(lastRing.at(point)); //current point on last ring (small)
                    mesh.push_back(currRing.at(currLargeRingPoint)); //large ring
                    mesh.push_back(currRing.at(currLargeRingPoint-1)); //large ring
                }
            }else{
                ///TODO currRing --> lastRing
                std::size_t quotient = lastRing.size()/currRing.size();
                std::vector<std::size_t> numberOfTris(currRing.size(), quotient); //currRing(smaller ring) --> lastRing(bigger ring)
                std::size_t unallocatedTris = lastRing.size() - (currRing.size()*quotient);
                if(unallocatedTris != 0){
                    std::size_t gapSize = currRing.size()/unallocatedTris;
                    float gapFloat= static_cast<float>(currRing.size())/static_cast<float>(unallocatedTris);

                    std::size_t i = 0;
                    while(i < currRing.size() && unallocatedTris > 0){
                        if((static_cast<float>(currRing.size()-i)/static_cast<float>(unallocatedTris)) > gapFloat){
                            i +=1;
                        }
                        numberOfTris.at(i) += 1;
                        i += gapSize;
                        unallocatedTris--;
                    }
                }
                /*int total = 0;
                for(auto &elem: numberOfTris){
                    std::cout << elem << ", ";
                    total += elem;
                }
                std::cout << total <<std::endl;*/


                //for edge tex wrap case

                mesh.push_back(Vertex(currRing.at(0).posCoords, glm::vec2(1.0f,currTexY), currRing.at(0).normCoords)); // start point on curr ring (small)
                mesh.push_back(lastRing.at(lastRing.size()-1)); // end point on last ring (large)
                mesh.push_back(Vertex(lastRing.at(0).posCoords, glm::vec2(1.0f,lastTexY), lastRing.at(0).normCoords)); // start point on last ring (large)

                size_t postAdd = 0; // how many points to go from 0 forwards
                size_t preAdd = 0; // how many points to go from 0 backwards
                size_t lastLargeRingPoint = 0;
                if(numberOfTris.at(0) <= 3){ // is 1,2 or 3 points

                    mesh.push_back(Vertex(currRing.at(0).posCoords, glm::vec2(1.0f,currTexY), currRing.at(0).normCoords)); //start point on curr ring (small)
                    mesh.push_back(currRing.at(currRing.size()-1)); // corresponding point on curr ring (small)
                    mesh.push_back(lastRing.at(lastRing.size()-1)); // end point on last ring (large)

                    if(numberOfTris.at(0) != 1){
                        postAdd = numberOfTris.at(0)-1;
                        lastLargeRingPoint = postAdd;
                    }
                }else{
                    if(numberOfTris.at(0)%2 == 0){ // if even
                        preAdd = numberOfTris.at(0)/2;
                        postAdd = preAdd;
                        lastLargeRingPoint = postAdd;
                    }else{
                        preAdd = (numberOfTris.at(0)-1)/2;
                        postAdd = preAdd +1;
                        lastLargeRingPoint = postAdd;
                    }
                }

                //preAdd (goes to texture.x = 1)
                //Pre add must connect to smallring n-1 on farthest point
                if(preAdd > 0){
                    mesh.push_back(Vertex(currRing.at(0).posCoords, glm::vec2(1.0f,0.0f), currRing.at(0).normCoords)); //start point on curr ring (small)
                    mesh.push_back(currRing.at(currRing.size()-1)); // previous point on curr ring (small)
                    mesh.push_back(lastRing.at(lastRing.size()-preAdd)); // end point on last ring (large)
                    preAdd--;
                }
                for(;preAdd > 0; preAdd--){
                    mesh.push_back(Vertex(currRing.at(0).posCoords, glm::vec2(1.0f,0.0f), currRing.at(0).normCoords)); //start point on curr ring (small)
                    mesh.push_back(lastRing.at(lastRing.size()-(preAdd+1)));
                    mesh.push_back(lastRing.at(lastRing.size()-preAdd));
                }

                //postAdd(comes from texture.x = 0)
                for(;postAdd > 0; postAdd--){
                    mesh.push_back(currRing.at(0)); //start point on curr ring (small)
                    mesh.push_back(lastRing.at(postAdd-1));
                    mesh.push_back(lastRing.at(postAdd));
                }

                //smallring points from [1] to [n]
                for(std::size_t point = 1; point < currRing.size(); point++){
                    size_t targetPoint = lastLargeRingPoint + numberOfTris.at(point);
                    //connect to curr point
                    mesh.push_back(currRing.at(point)); //lastent point on curr ring (small)
                    mesh.push_back(currRing.at(point-1)); //curr point on curr ring (small)
                    mesh.push_back(lastRing.at(lastLargeRingPoint)); //curr point used on larger ring
                    lastLargeRingPoint++;

                    for(;lastLargeRingPoint < targetPoint; lastLargeRingPoint++){
                        mesh.push_back(currRing.at(point)); //lastent point on curr ring (small)
                        mesh.push_back(lastRing.at(lastLargeRingPoint-1)); //large ring
                        mesh.push_back(lastRing.at(lastLargeRingPoint)); //large ring
                    }
                    //complete final tri with out incrementing lastLargeRingPoint
                    mesh.push_back(currRing.at(point)); //lastent point on curr ring (small)
                    mesh.push_back(lastRing.at(lastLargeRingPoint-1)); //large ring
                    mesh.push_back(lastRing.at(lastLargeRingPoint)); //large ring
                }
            }

        }else{
            // each point in the current ring corresponds to a point in the last ring
            for(std::size_t i = 1; i < currRing.size(); i++){
                mesh.push_back(currRing.at(i-1)); // previous point
                mesh.push_back(lastRing.at(i-1)); // corresponding point on last ring
                mesh.push_back(currRing.at(i)); // current point

                mesh.push_back(currRing.at(i)); // current point
                mesh.push_back(lastRing.at(i-1)); // prev point on last ring
                mesh.push_back(lastRing.at(i)); // corresponding point on last ring
            }

            mesh.push_back(Vertex(currRing.at(0).posCoords, glm::vec2(1.0f,currTexY), currRing.at(0).normCoords)); // start
            mesh.push_back(currRing.at(currRing.size()-1)); // current point
            mesh.push_back(Vertex(lastRing.at(0).posCoords, glm::vec2(1.0f,lastTexY), lastRing.at(0).normCoords)); // start point on last ring

            mesh.push_back(currRing.at(currRing.size()-1)); // current point
            mesh.push_back(lastRing.at(lastRing.size()-1)); // corresponding point on last ring
            mesh.push_back(Vertex(lastRing.at(0).posCoords, glm::vec2(1.0f,lastTexY), lastRing.at(0).normCoords)); // start on last ring

        }
}


void TreeMaker::createTreeMesh(treeModel& newTree)
{
    std::size_t currNodeIndex(0);
    treeNode &rootNode = newTree.branchTree.at(currNodeIndex);
    std::vector<std::vector<Vertex>> rings;
    rings.resize(newTree.branchTree.size());
    std::vector<Vertex> &rootRing = rings.at(currNodeIndex);
    unsigned int points = static_cast<int>((2.0f*M_PI*rootNode.radius)/0.4f);// highRes
    float angle = (M_PI*2.0f)/static_cast<float>(points); // in radians

    //push first point
    rootRing.push_back(Vertex(
                            glm::vec3(rootNode.radius * std::sin(0.0f), 0.0f, rootNode.radius * std::cos(0.0f))+ rootNode.nodeCoords, // position
                            glm::vec2(0.0f, 0.0f), //texture
                            glm::normalize(glm::vec3((rootNode.radius+1.0f) * std::sin(0.0f), 0.0f, (rootNode.radius+1.0f) * std::cos(0.0f)) - glm::vec3(rootNode.radius * std::sin(0.0f), 0.0f, rootNode.radius * std::cos(0.0f))))); // normal

    for(float fPoint = 1.0f; fPoint < static_cast<float>(points); fPoint += 1.0f){
        rootRing.push_back(Vertex(
                                glm::vec3(rootNode.radius * std::sin(angle * fPoint), 0.0f, rootNode.radius * std::cos(angle * fPoint))+ rootNode.nodeCoords, //position
                                glm::vec2(1.0f/static_cast<float>(points) * fPoint, 0.0f),  //texture
                                glm::normalize( //normal
                                               glm::vec3((rootNode.radius+1.0f) * std::sin(angle * fPoint), 0.0f, (rootNode.radius+1.0f) * std::cos(angle * fPoint))
                                               - glm::vec3(rootNode.radius * std::sin(angle * fPoint), 0.0f, rootNode.radius * std::cos(angle * fPoint)))));


    }
    std::stack<std::size_t> nodeIndexStack;
    nodeIndexStack.push(currNodeIndex);

    while(!nodeIndexStack.empty()){
        std::size_t lastNodeIndex = nodeIndexStack.top();
        nodeIndexStack.pop();
        std::vector<Vertex> &lastRing = rings.at(lastNodeIndex);
        treeNode &lastNode = newTree.branchTree.at(lastNodeIndex);

        for(auto &currNodeIndex : lastNode.children){
            //currNodeIndex = lastNode.children.front();
            nodeIndexStack.push(currNodeIndex);
            std::vector<Vertex> &currRing = rings.at(currNodeIndex);
            treeNode &currNode = newTree.branchTree.at(currNodeIndex);
            float textureDist = glm::distance(lastNode.nodeCoords, currNode.nodeCoords) + lastRing.at(0).texCoords.y;

            points = static_cast<int>((2.0f*M_PI*currNode.radius)/0.4f);// highRes
            float angle = (M_PI*2.0f)/static_cast<float>(points); // in radians

            ///TODO USE
            ///r = r*(1+math.sin((h+0.26)+theta*3)*a*sin((h+0.46)) + math.sin((h+1.13)+theta*4)*b*sin((h+1.73)) + math.sin((h+2.23)+theta*5)*b*sin((h+2.58)))
            // a = 0.06, b = 0.003, c = 0.45

            //First ring point [0]
            currNode.rotationMatrix =  rotateToVec(lastNode.nodeCoords-currNode.nodeCoords); ///TODO MOVE TO NODE CREATION
            glm::vec3 ringPoint = (glm::vec3(currNode.radius * std::sin(0.0f), 0.0f, currNode.radius * std::cos(0.0f)) + currNode.nodeCoords) * currNode.rotationMatrix;
            //Expand ring by one unit to find normal vector;
            glm::vec3 ringPointExpanded = (glm::vec3((currNode.radius+1.0f) * std::sin(0.0f), 0.0f, (currNode.radius+1.0f) * std::cos(0.0f)) + currNode.nodeCoords) * currNode.rotationMatrix;
            currRing.push_back(Vertex(
                                    ringPoint,                                      //Position
                                    glm::vec2(0.0f,textureDist),                    //Texture
                                    glm::normalize(ringPointExpanded-ringPoint)));  //Normal

            //Second point [1] to final point [n-1]
            for(float fPoint = 1.0f; fPoint < static_cast<float>(points); fPoint += 1.0f){
                glm::vec3 ringPoint = (glm::vec3(currNode.radius * std::sin(angle * fPoint), 0.0f, currNode.radius * std::cos(angle * fPoint)) + currNode.nodeCoords) * currNode.rotationMatrix;
                //Expand ring by one unit to find normal vector;
                glm::vec3 ringPointExpanded = (glm::vec3((currNode.radius+1.0f) * std::sin(angle * fPoint), 0.0f, (currNode.radius+1.0f) * std::cos(angle * fPoint)) + currNode.nodeCoords) * currNode.rotationMatrix;
                currRing.push_back(Vertex(
                                        ringPoint,                                                         //Position
                                        glm::vec2(1.0f/static_cast<float>(points) * fPoint, textureDist),  //Texture
                                        glm::normalize(ringPointExpanded-ringPoint)));                     //Normal

            }

            //Create mesh from rings
            if(currNode.nodeCoords.y > lastNode.nodeCoords.y){
                createMeshFromRings(currRing, lastRing, newTree.modelHighLODVerts, textureDist, lastRing.at(0).texCoords.y);
            }else{
                createMeshFromRings(lastRing, currRing, newTree.modelHighLODVerts, lastRing.at(0).texCoords.y, textureDist);
            }
        }
    }

    //create VAO and VBO
    glGenVertexArrays(1, &newTree.highLOD_VAO);
    glGenBuffers(1, &newTree.highLOD_VBO);

    glBindVertexArray(newTree.highLOD_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, newTree.highLOD_VBO);
    glBufferData(GL_ARRAY_BUFFER, newTree.modelHighLODVerts.size() * sizeof(Vertex), &newTree.modelHighLODVerts[0], GL_STATIC_DRAW);


    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex texture coords
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
    // normal texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normCoords));

    glBindVertexArray(0);
}


void TreeMaker::drawTree(const Shader& shader, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, const int treeIndex, const glm::vec3 position, unsigned int textureID)
{
    //glDisable(GL_CULL_FACE); //TODO FOR DEBUG, DISABLE FOR RELASE
    shader.use();
     shader.setInt("barkTexture", 4);
    glActiveTexture(GL_TEXTURE4); // set sampler to texture
    glBindTexture(GL_TEXTURE_2D, textureID);

    glm::mat4 pvMatrix = projectionMatrix * viewMatrix;
    shader.setMat4("pvMatrix", pvMatrix);
    shader.setVec3("offsetCoords",position);
    glBindVertexArray(treeList.at(treeIndex).highLOD_VAO);
    glDrawArrays(GL_TRIANGLES, 0, treeList.at(treeIndex).modelHighLODVerts.size()); //send verts as patch to Tessellation shader

    glBindVertexArray(0); // unbind
    //glEnable(GL_CULL_FACE);
}

void TreeMaker::drawTreeColliders(const Shader& shader, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, const int treeIndex, const glm::vec3 position)
{

}
