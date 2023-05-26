#include "TreeMaker.h"
#include <iostream>
#include <math.h>
#include <vector>
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
    float startRadius = 1.5;
    treeModel newTree;
    //root node
    treeNode currNode(glm::vec3(0.0f,0.0f,0.0f), startRadius);
    newTree.branchTree.push_back(currNode);
    size_t lastNodeIndex = 0;

    //next node
    currNode.nodeCoords = glm::vec3(0.7f,3.0f,1.2f);
    currNode.radius = startRadius - 1.0f;
    newTree.branchTree.push_back(currNode);
    //link parent to child (lastNode -> currNode)
    newTree.branchTree.at(lastNodeIndex).children.push_back(newTree.branchTree.size()-1);

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


void TreeMaker::createTreeMesh(treeModel& newTree)
{
    treeNode currNode = newTree.branchTree.at(0);
    std::vector<Vertex> currRing;
    unsigned int points = 20; ///TODO ADD LOGIC
    float angle = (M_PI*2.0f)/points; // in radians
    glm::vec3 groundSpike{0.0f, 1.0f, 0.0f}; // spike into ground to hide possible floating on hills
    Vertex groundSpikeVertex(glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(-1.0f, 0.5f), glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f)));

    //push first point
    currRing.push_back(Vertex(glm::vec3(currNode.radius * std::sin(0.0f), 0.0f, currNode.radius * std::cos(0.0f)), // position
                       glm::vec2(0.0f, 0.0f), //texture
                       glm::normalize(glm::vec3((currNode.radius+1.0f) * std::sin(0.0f), 0.0f, (currNode.radius+1.0f) * std::cos(0.0f)) - glm::vec3(currNode.radius * std::sin(0.0f), 0.0f, currNode.radius * std::cos(0.0f))))); // normal

    for(unsigned int i = 1; i < points; i++){
        currRing.push_back(Vertex(glm::vec3(currNode.radius * std::sin(angle * i), 0.0f, currNode.radius * std::cos(angle * i)),
                           glm::vec2(0.0f, 1.0f/points * i),
                           glm::normalize(glm::vec3((currNode.radius+1.0f) * std::sin(angle * i), 0.0f, (currNode.radius+1.0f) * std::cos(angle * i))- glm::vec3(currNode.radius * std::sin(angle * i), 0.0f, currNode.radius * std::cos(angle * i)))));
        // create end cap/spike (clockwise order)
        //printf("new: %.3f, %.3f, %.3f \n",currRing.back().posCoords.x, currRing.back().posCoords.y, currRing.back().posCoords.z);
        newTree.modelHighLODVerts.push_back(currRing.at(currRing.size()-2)); // previous point
        newTree.modelHighLODVerts.push_back(groundSpikeVertex);
        newTree.modelHighLODVerts.push_back(currRing.at(currRing.size()-1)); // current point
    }
    newTree.modelHighLODVerts.push_back(currRing.at(currRing.size()-1)); // previous point
    newTree.modelHighLODVerts.push_back(groundSpikeVertex);
    newTree.modelHighLODVerts.push_back(Vertex(currRing.at(0).posCoords, glm::vec2(0.0f,1.0f), currRing.at(0).normCoords)); // back to start


    if(currNode.children.empty()){
        throw std::invalid_argument( "TreeModel had no children"); // if only root node, then no tree

    }else if(currNode.children.size() == 1){
        std::vector<Vertex> lastRing = currRing;
        currRing.clear();
        treeNode lastNode = currNode;
        currNode = newTree.branchTree.at(currNode.children.front());

        //first ring point
        glm::mat3 rotationMatrix = rotateToVec(lastNode.nodeCoords-currNode.nodeCoords);
        glm::vec3 ringPoint = (glm::vec3(currNode.radius * std::sin(0.0f), 0.0f, currNode.radius * std::cos(0.0f)) + currNode.nodeCoords) * rotationMatrix;
        glm::vec3 ringPointExpanded = (glm::vec3((currNode.radius+1.0f) * std::sin(0.0f), 0.0f, (currNode.radius+1.0f) * std::cos(0.0f)) + currNode.nodeCoords) * rotationMatrix;
        currRing.push_back(Vertex(ringPoint,
                           glm::vec2(1.0f, 0),
                           glm::normalize(ringPointExpanded-ringPoint)));

        for(unsigned int i = 1; i < points; i++){

            glm::mat3 rotationMatrix = rotateToVec(lastNode.nodeCoords-currNode.nodeCoords);
            glm::vec3 ringPoint = (glm::vec3(currNode.radius * std::sin(angle * i), 0.0f, currNode.radius * std::cos(angle * i)) + currNode.nodeCoords) * rotationMatrix;
            glm::vec3 ringPointExpanded = (glm::vec3((currNode.radius+1.0f) * std::sin(angle * i), 0.0f, (currNode.radius+1.0f) * std::cos(angle * i)) + currNode.nodeCoords) * rotationMatrix;
            currRing.push_back(Vertex( ringPoint,
                               glm::vec2(1.0f, 1.0f/points * i),
                               glm::normalize(ringPointExpanded-ringPoint)));

            // create end cap/spike (clockwise order)
            //printf("new: %.3f, %.3f, %.3f \n",currRing.back().posCoords.x, currRing.back().posCoords.y, currRing.back().posCoords.z);
            newTree.modelHighLODVerts.push_back(currRing.at(i-1)); // previous point
            newTree.modelHighLODVerts.push_back(lastRing.at(i-1)); // corresponding point on last ring
            newTree.modelHighLODVerts.push_back(currRing.at(i)); // current point

            newTree.modelHighLODVerts.push_back(currRing.at(i)); // current point
            newTree.modelHighLODVerts.push_back(lastRing.at(i-1)); // prev point on last ring
            newTree.modelHighLODVerts.push_back(lastRing.at(i)); // corresponding point on last ring
        }
        newTree.modelHighLODVerts.push_back(Vertex(currRing.at(0).posCoords, glm::vec2(1.0f,1.0f), currRing.at(0).normCoords)); // start
        newTree.modelHighLODVerts.push_back(currRing.at(currRing.size()-1)); // current point
        newTree.modelHighLODVerts.push_back(Vertex(lastRing.at(0).posCoords, glm::vec2(0.0f,1.0f), lastRing.at(0).normCoords)); // start point on last ring


        newTree.modelHighLODVerts.push_back(currRing.at(currRing.size()-1)); // current point
        newTree.modelHighLODVerts.push_back(lastRing.at(lastRing.size()-1)); // corresponding point on last ring
        newTree.modelHighLODVerts.push_back(Vertex(lastRing.at(0).posCoords, glm::vec2(0.0f,1.0f), lastRing.at(0).normCoords)); // start on last ring


    }else{
    ///nextnode(s)
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
    glDisable(GL_CULL_FACE); //TODO FOR DEBUG, DISABLE FOR RELASE
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
    glEnable(GL_CULL_FACE);
}

void TreeMaker::drawTreeColliders(const Shader& shader, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, const int treeIndex, const glm::vec3 position)
{

}
