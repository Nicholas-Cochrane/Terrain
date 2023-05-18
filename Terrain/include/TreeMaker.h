#ifndef TREEMAKER_H
#define TREEMAKER_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "stb_image.h"
#include <glad/glad.h>
#include <GL/glfw3.h>

#include "shader_s.h"
#include "Collision.h"

#include <iostream>
#include <math.h>
#include <vector>
namespace Trees{
    struct Vertex {
        Vertex(glm::vec3 inPos, glm::vec2 inTex) : posCoords(inPos), texCoords(inTex) {};
        Vertex() = default;
        glm::vec3 posCoords;
        glm::vec2 texCoords;
    };

    struct treeNode{
        treeNode(glm::vec3 inCoord, float inRadius) : nodeCoords(inCoord), radius(inRadius) {};
        treeNode() : nodeCoords{0.0f, 0.0f, 0.0f}, radius(0.0f) {};
        glm::vec3 nodeCoords;
        float radius;
        std::vector<size_t> children;
    };

    struct treeModel
    {
        std::vector<treeNode> branchTree;
        std::vector<Vertex> modelHighLODVerts;
        std::vector<Vertex> modelLowLODVerts;
        std::vector<unsigned int> billBoardImages;
        AABB boundingBox;
        std::vector<Capsual> treeColliderList;
        unsigned int highLOD_VAO, highLOD_VBO, lowLOD_VAO, lowLOD_VBO; // VAO/VBO ID's
    };
}
using namespace Trees;
class TreeMaker
{
    public:
        TreeMaker();
        virtual ~TreeMaker();
        void createNewTree();
        //treeModel createNewTree();
        std::vector<treeModel> treeList;

        void drawTree(const Shader &shader, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, const int treeIndex, const glm::vec3 position);
        void drawTreeColliders(const Shader &shader, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, const int treeIndex, const glm::vec3 position);

    protected:

    private:
        void createTreeMesh(treeModel &newTree);
};

#endif // TREEMAKER_H
