#include <glad/glad.h>
#include <GL/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include "include/stb_image.h"


#include <iostream>
#include <string>
#include <math.h>

#include "include/shader_s.h"
#include "include/camera.h"
#include "include/chunk.h"
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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
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
    Shader mainShader("shaders/vertex_shader_projection.vs","shaders/frag_shader_texture.fs");


    // Set up Vertex Data and buffers
    //--------------------------------
    // Set up vertices array
    float vertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };

    glm::vec3 cubePositions[] = {
        glm::vec3( 0.0f,  0.0f,  0.0f),
        glm::vec3( 2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3( 2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3( 1.3f, -2.0f, -2.5f),
        glm::vec3( 1.5f,  2.0f, -2.5f),
        glm::vec3( 1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
    };

    //Set up Vertex Buffer
    unsigned int VBO, VAO; // Vertex Buffer Object, Vertex Array Object, Element Buffer Object
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    //glGenBuffers(1, &EBO);

    glBindVertexArray(VAO); // bind VAO

    glBindBuffer(GL_ARRAY_BUFFER, VBO);// bind VBO
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // Load vertices in to buffer

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //glBindBuffer(GL_ARRAY_BUFFER, 0); // set current array buffer to 0
    //glBindVertexArray(0); // Unbind vertex array
    //NOTE: Do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO

    //SET DRAW MODE TO WIREFRAME
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Enable Depth buffer
    glEnable(GL_DEPTH_TEST);

    // Texture setup and loading
    //---------------------------
    unsigned int texture1 = loadTexture("textures/grass.png", GL_RGB);

    //Set uniform to sampler
    mainShader.use();
    mainShader.setInt("texture1",0);

    //create chunks
    Chunk chunk(glm::vec3(0.0f,0.0f,0.0f), 3000);

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
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // bind textures
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture1);
            mainShader.use();

            //Model -> Global
            // Begin Draw
            /*glBindVertexArray(VAO);
            for(unsigned int i = 0; i < 10; i++)
            {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            float angle = 20.0f * i;

            model = glm::rotate(model, (float)glfwGetTime() * glm::radians(angle), glm::vec3(0.5f, 1.0f, 0.0f));
            glDrawArrays(GL_TRIANGLES, 0, 36);

            mainShader.setMat4("model", model);
            }*/


            chunk.draw(mainShader);


            //Global -> view
            glm::mat4 view = camera.GetViewMatrix();
            mainShader.setMat4("view", view);

            //View -> Projection
            glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), static_cast<float>(curr_scr_width)/ static_cast<float>(curr_scr_height), 0.1f, 5000.0f);
            mainShader.setMat4("projection", projection);
        }
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // De-allocate all GL resources
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

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
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    curr_scr_width = width;
    curr_scr_height = height;
    glViewport(0, 0, width, height);
    std::cout << width << "X" << height << std::endl;
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
    int texWidth, texHeight, nrChannels;
    unsigned char *texdData = stbi_load(filepath, &texWidth, &texHeight, &nrChannels, 0);
    stbi_set_flip_vertically_on_load(true);  // make bottom of image 0.0
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
