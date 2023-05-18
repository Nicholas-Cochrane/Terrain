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
    float startRadius = 2.5;
    treeModel newTree;
    //root node
    treeNode currNode(glm::vec3(0.0f,0.0f,0.0f), startRadius);
    newTree.branchTree.push_back(currNode);
    size_t lastNodeIndex = 0;

    //next node
    currNode.nodeCoords = glm::vec3(0.0f,5.0f,0.0f);
    currNode.radius = startRadius - 0.1f;
    newTree.branchTree.push_back(currNode);
    //link parent to child (lastNode -> currNode)
    newTree.branchTree.at(lastNodeIndex).children.push_back(newTree.branchTree.size()-1);

    createTreeMesh(newTree); // create mesh, vao, vbo
    treeList.push_back(newTree);// add finished tree to list
}

void TreeMaker::createTreeMesh(treeModel& newTree)
{
    treeNode currNode = newTree.branchTree.at(0);
    std::vector<Vertex> currRing;
    unsigned int points = 20; ///TODO ADD LOGIC
    float angle = (M_PI*2.0f)/points; // in radians
    glm::vec3 groundSpike{0.0f, 1.0f, 0.0f}; // spike into ground to hide possible floating on hills
    Vertex groundSpikeVertex(glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(-1.0f, 0.5f));

    //push first point
    currRing.push_back(Vertex(glm::vec3(currNode.radius * std::sin(0.0f), 0.0f, currNode.radius * std::cos(0.0f)),
                      glm::vec2(0.0f, 0)) );

    for(unsigned int i = 1; i < points; i++){
        currRing.push_back(Vertex(glm::vec3(currNode.radius * std::sin(angle * i), 0.0f, currNode.radius * std::cos(angle * i)),
                      glm::vec2(0.0f, 1.0f/points * i)) );
        // create end cap/spike (clockwise order)
        //printf("new: %.3f, %.3f, %.3f \n",currRing.back().posCoords.x, currRing.back().posCoords.y, currRing.back().posCoords.z);
        newTree.modelHighLODVerts.push_back(currRing.at(currRing.size()-2)); // previous point
        newTree.modelHighLODVerts.push_back(groundSpikeVertex);
        newTree.modelHighLODVerts.push_back(currRing.at(currRing.size()-1)); // current point
    }
    newTree.modelHighLODVerts.push_back(currRing.at(currRing.size()-1)); // previous point
    newTree.modelHighLODVerts.push_back(groundSpikeVertex);
    newTree.modelHighLODVerts.push_back(currRing.at(0)); // back to start


    if(currNode.children.empty()){
        throw std::invalid_argument( "TreeModel had no children"); // if only root node, then no tree

    }else if(currNode.children.size() == 1){
        std::vector<Vertex> lastRing = currRing;
        currRing.clear();
        currNode = newTree.branchTree.at(currNode.children.front());

        currRing.push_back(Vertex(glm::vec3(currNode.radius * std::sin(0.0f), 0.0f, currNode.radius * std::cos(0.0f)) + currNode.nodeCoords, //todo add rotation
                      glm::vec2(1.0f, 0)) );

        for(unsigned int i = 1; i < points; i++){
        currRing.push_back(Vertex(glm::vec3(currNode.radius * std::sin(angle * i), 0.0f, currNode.radius * std::cos(angle * i)) + currNode.nodeCoords,
                      glm::vec2(1.0f, 1.0f/points * i)) );
        // create end cap/spike (clockwise order)
        //printf("new: %.3f, %.3f, %.3f \n",currRing.back().posCoords.x, currRing.back().posCoords.y, currRing.back().posCoords.z);
        newTree.modelHighLODVerts.push_back(currRing.at(i-1)); // previous point
        newTree.modelHighLODVerts.push_back(lastRing.at(i-1)); // corresponding point on last ring
        newTree.modelHighLODVerts.push_back(currRing.at(i)); // current point

        newTree.modelHighLODVerts.push_back(currRing.at(i)); // current point
        newTree.modelHighLODVerts.push_back(lastRing.at(i-1)); // prev point on last ring
        newTree.modelHighLODVerts.push_back(lastRing.at(i)); // corresponding point on last ring
        }
        newTree.modelHighLODVerts.push_back(currRing.at(0)); // start
        newTree.modelHighLODVerts.push_back(currRing.at(currRing.size()-1)); // current point
        newTree.modelHighLODVerts.push_back(lastRing.at(0)); // start point on last ring


        newTree.modelHighLODVerts.push_back(currRing.at(currRing.size()-1)); // current point
        newTree.modelHighLODVerts.push_back(lastRing.at(lastRing.size()-1)); // corresponding point on last ring
        newTree.modelHighLODVerts.push_back(lastRing.at(0)); // start on last ring


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

    glBindVertexArray(0);
}


void TreeMaker::drawTree(const Shader& shader, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, const int treeIndex, const glm::vec3 position)
{
    shader.use();
    glm::mat4 pvMatrix = projectionMatrix * viewMatrix;
    shader.setMat4("pvMatrix", pvMatrix);
    shader.setVec3("offsetCoords",position);
    glBindVertexArray(treeList.at(treeIndex).highLOD_VAO);
    glDrawArrays(GL_TRIANGLES, 0, treeList.at(treeIndex).modelHighLODVerts.size()); //send verts as patch to Tessellation shader

    glBindVertexArray(0); // unbind
}

void TreeMaker::drawTreeColliders(const Shader& shader, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, const int treeIndex, const glm::vec3 position)
{

}
