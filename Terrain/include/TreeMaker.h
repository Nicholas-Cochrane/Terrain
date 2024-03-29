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
        Vertex(glm::vec3 inPos, glm::vec2 inTex, glm::vec3 inNorm) : posCoords(inPos), texCoords(inTex), normCoords(inNorm) {};
        Vertex() = default;
        glm::vec3 posCoords;
        glm::vec2 texCoords;
        glm::vec3 normCoords;
    };

    struct treeNode{
        treeNode(glm::vec3 inCoord, float inRadius) : nodeCoords(inCoord), radius(inRadius) {};
        treeNode() : nodeCoords{0.0f, 0.0f, 0.0f}, radius(0.0f) {};
        glm::vec3 nodeCoords;
        float radius;
        glm::mat3 rotationMatrix;
        std::vector<size_t> children;
        std::vector<size_t> minorChildren;
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

        void drawTree(const Shader &shader, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, const int treeIndex, const glm::vec3 position, unsigned int textureID);
        void drawTreeColliders(const Shader &shader, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, const int treeIndex, const glm::vec3 position);

    protected:

    private:
        void createTreeMesh(treeModel &newTree);
        void createMeshFromRings(std::vector<Trees::Vertex> &currRing, std::vector<Trees::Vertex> &lastRing, std::vector<Trees::Vertex> &mesh, float currTexY, float lastTexY);
        glm::mat3 rotateToVec(const glm::vec3 target);
};

#endif // TREEMAKER_H
