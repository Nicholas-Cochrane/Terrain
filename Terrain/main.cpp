#include <glad/glad.h>
#include <GL/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include "include/stb_image.h"


#include <iostream>
#include <string>
#include <math.h>
#include <vector>

#include "include/shader_s.h"
#include "include/camera.h"
//#include "include/chunk.h"
#include "include/TessChunk.h"
#define GLFW_DLL


//Function Delectations
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(char const* filepath, GLint formatColor, GLint wrappingParam = GL_REPEAT);


// settings
//---------------------------------
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1027;

unsigned int curr_scr_width= SCR_WIDTH;
unsigned int curr_scr_height = SCR_HEIGHT;

// camera and mouse input
Camera camera(glm::vec3(0.0f, 2.0f, 3.0f));
float mouseLastX = SCR_WIDTH / 2.0f;
float mouseLastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;



// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

//main
//---------------------------------------------------------------------------------------------------
int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_MAXIMIZED , GL_TRUE);//Maximize window

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    int area_x, area_y, area_width, area_height;
    glfwGetMonitorWorkarea(glfwGetPrimaryMonitor(), &area_x, &area_y, &area_width, &area_height);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Terrain Gen", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);// tell GLFW to capture our mouse
    glfwSetWindowPos(window,
        area_x + area_width / 2 - area_width / 2,
        (area_y + area_height / 2 - area_height / 2)+25); // +25 lowers window 25px to account for size of windows title bar
    //glfwMaximizeWindow(window);
    // Old code for maximizing window (but glfwMaximizeWindow() causes crash



    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Shader compilation
    //---------------------
    Shader tessShader("shaders/tess_chunk_vert.vs", "shaders/tess_chunk_frag.fs", "shaders/tess_chunk.tcs", "shaders/tess_chunk.tes");

    //SET DRAW MODE TO WIREFRAME
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glPointSize(10.0f);

    // Enable Depth buffer
    glEnable(GL_DEPTH_TEST);

    // Enable Back Face Culling
    glEnable(GL_CULL_FACE);

    // Texture setup and loading
    //---------------------------
    unsigned int texture1 = loadTexture("textures/uv.png", GL_RGB);
    unsigned int heightMap = loadTexture("textures/0.01344305976942091 (4130) (110km) square.png", GL_RGBA, GL_CLAMP_TO_EDGE);

    //Set uniform to sampler
    tessShader.use();
    //tessShader.setInt("texture1",0);
    tessShader.setInt("heightMap", 0);

    //Object Creation
    //----------------
    //create chunks
    //TessChunk chunk(glm::vec3(0.0f,0.0f,0.0f), 64, texture1, heightMap, glm::vec2(0.0f, 0.0f), 1.0f);
    std::vector<TessChunk*> chunkList;
    const int divisions = 40;
    for(int x = 0; x < divisions; x++){
        for(int z = 0; z < divisions; z++){// note: -z is forward in coord space
            chunkList.push_back(new TessChunk(glm::vec3(x * 64.0f ,0.0f,-z * 64.0f)
                                              , 64
                                              , texture1, heightMap
                                              , glm::vec2( (1.0f/divisions) * x
                                                          , (1.0f/divisions) * z )
                                              , (1.0f/divisions)));
        }
    }

    //chunkList.push_back(new TessChunk(glm::vec3(0.0f,0.0f,0.0f), 64, texture1, heightMap, glm::vec2(0.0f, 0.0f), 1.0f));


    std::cout << glGetString(GL_VERSION) << std::endl;

    // render loop
    // -------------------------------------------------------------------------------------------------
    while (!glfwWindowShouldClose(window))
    {
        //Delta time Calculation
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);


        // render
        // ------
        if(!glfwGetWindowAttrib(window, GLFW_ICONIFIED))
        { // if window is minimized do not render (Rendering while at resolution 0x0 cause glm::matix_clip_space.inl assertion to fail)

            //glClearColor(0.3f, 0.4f, 0.5f, 1.0f);
            glClearColor(0.207f, 0.318f, 0.361f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // bind textures
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture1);
            tessShader.use();

            //Model -> Global
            // Begin Draw

            //chunk.draw(tessShader);
            for(int i = 0; i < chunkList.size(); i++){
                chunkList.at(i)->draw(tessShader);
            }

            //Global -> view
            glm::mat4 view = camera.GetViewMatrix();
            tessShader.setMat4("view", view);

            //View -> Projection
            glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), static_cast<float>(curr_scr_width)/ static_cast<float>(curr_scr_height), 0.1f, 5000.0f);
            tessShader.setMat4("projection", projection);
        }
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        std::cout << camera.Position.x << ',' << camera.Position.y << ',' << camera.Position.z << std::endl;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    curr_scr_width = width;
    curr_scr_height = height;
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        mouseLastX = xpos;
        mouseLastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - mouseLastX;
    float yoffset = mouseLastY - ypos; // reversed since y-coordinates go from bottom to top

    mouseLastX = xpos;
    mouseLastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

unsigned int loadTexture(char const* filepath, GLint formatColor, GLint wrappingParam){
    //NOTE wrappingParam defaults to GL_REPEAT due to first declaration
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    // set texture wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrappingParam);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrappingParam);
    // set texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //load with stbi
    stbi_set_flip_vertically_on_load(true);  // make bottom of image 0.0
    int texWidth, texHeight, nrChannels;
    unsigned char *texdData = stbi_load(filepath, &texWidth, &texHeight, &nrChannels, 0);
    if(texdData){
        //load data into texture
        glTexImage2D(GL_TEXTURE_2D, 0, formatColor, texWidth, texHeight, 0, formatColor, GL_UNSIGNED_BYTE, texdData);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
       std::cout << "ERROR::TEXTURE_FAILED_TO_LOAD" << std::endl;
    }
    stbi_image_free(texdData);// free data

    return textureID;
}
