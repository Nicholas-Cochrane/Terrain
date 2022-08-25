#include <glad/glad.h>
#include <GL/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glm/gtx/string_cast.hpp>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "include/stb_image.h"


#include <iostream>
#include <string>
#include <math.h>
#include <vector>
#include<type_traits>
#include<utility>

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


GLenum glCheckError_(const char *file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        std::string error;
        switch (errorCode)
        {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            case 0x0503:                           error = "STACK_OVERFLOW"; break;
            case 0x0504:                           error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)


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
bool firstMouse = true; // if this is the first mouse input
bool captureMouse = false; // if the mouse is currently captured
bool wireframe = false; // if wire frame is on or off
bool keyDown[GLFW_KEY_MENU + 1] = {false}; // if key is currently being pressed



// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float lastSec = 0.0f;
int frameNumber = 0;
int FPS = 0;

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
    captureMouse = true;
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
    Shader* tessShader = new Shader("shaders/tess_chunk_vert.vs", "shaders/tess_chunk_frag.fs", "shaders/tess_chunk.tcs", "shaders/tess_chunk.tes");
    Shader* mapShader = new Shader("shaders/imgui_shader.vs", "shaders/imgui_shader.fs", NULL, NULL);

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
    unsigned int playerIcon = loadTexture("textures/map arrow.png", GL_RGBA, GL_CLAMP_TO_EDGE);

    //mainShader.use();
    //mainShader.setInt("texture1",0);

    //Object Creation
    //----------------
    //create chunks
    std::vector<TessChunk*> chunkList;
    const int Meter_Scale = 110000; // scale of texture in meters
    const int Max_Height = 3997;// height of brightest pixel in texture
    const int Patchs_Per_Edge = 50;
    const int Divisions = 200/Patchs_Per_Edge;
    const float Chunk_Width = 64.0f*Patchs_Per_Edge;
    for(int x = 0; x < Divisions; x++){
        for(int z = 0; z < Divisions; z++){// note: -z is forward in coord space
            chunkList.push_back(new TessChunk(glm::vec3(x * Chunk_Width ,0.0f,-z * Chunk_Width) // root of chunk (0,0) with x+ and z-
                                              , Chunk_Width / Patchs_Per_Edge //width of each patch
                                              , Patchs_Per_Edge // number of patches per edge
                                              , texture1, heightMap // texture and height map texture
                                              , glm::vec2( (1.0f/Divisions) * x , (1.0f/Divisions) * z ) // UV coord of root
                                              , (1.0f/Divisions))); //uv cord offset (UV coord from bottom to top/ left to right)
        }
    }
    // set Tess Uniforms
    tessShader->use();
    tessShader->setFloat("uTexelSize", 1.0f/ (Divisions * Chunk_Width)); // 1/total size of terrain
    tessShader->setFloat("heightScale", ((Divisions*Chunk_Width)/Meter_Scale) * Max_Height); // (number of units (or maximum tiles) / texture size in meters) * maximum height of height map from 0

    std::cout << glGetString(GL_VERSION) << std::endl;

    //imgui
    //-----
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.FrameRounding = 4.0f; //Ex: Slider Bar, Window, Check Box
    style.GrabRounding = 3.0f;  //Ex: Slider handle
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
    style.WindowBorderSize = 0.0f;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");

    // render loop
    // -------------------------------------------------------------------------------------------------
    while (!glfwWindowShouldClose(window))
    {
        //Delta time Calculation
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        frameNumber++;
        // If a second has passed.
        if ( currentFrame - lastSec >= 1.0 )
            {
                lastSec = currentFrame;
                FPS = frameNumber;
                frameNumber = 0;
            }

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

            //set up new frame for ImGui
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // bind textures
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture1);
            //tessShader.use();
            tessShader->use();

            //Global -> view
            glm::mat4 view = camera.GetViewMatrix();
            //tessShader.setMat4("view", view);

            //View -> Projection
            glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), static_cast<float>(curr_scr_width)/ static_cast<float>(curr_scr_height), 0.1f, 50000.0f);
            //tessShader.setMat4("projection", projection);

            glm::mat4 projection2 = glm::perspective(glm::radians(40.0f), static_cast<float>(curr_scr_width)/ static_cast<float>(curr_scr_height), 0.1f, 50000.0f);

            //send player position to shader
            glUniform4fv(glGetUniformLocation(tessShader->ID, "camPos"), 1, glm::value_ptr(glm::vec4(camera.Position, 1.0f)));

            // Begin Draw

            for(unsigned int i = 0; i < chunkList.size(); i++){
                chunkList.at(i)->draw(*tessShader, view, projection, projection2);
            }


            //Create ImGui windows
            //--------------------

            ImGui::Begin("Pos");
                std::string positionStr = "X:" + std::to_string(camera.Position.x);
                ImGui::Text(positionStr.c_str());
                positionStr = "Y:" + std::to_string(camera.Position.y);
                ImGui::Text(positionStr.c_str());
                positionStr = "Z:" + std::to_string(camera.Position.z);
                ImGui::Text(positionStr.c_str());
                positionStr = "FPS:" + std::to_string(FPS);
                ImGui::Text(positionStr.c_str());
                positionStr = "Pitch:" + std::to_string(camera.Pitch);
                ImGui::Text(positionStr.c_str());
            ImGui::End();

            static bool metricsWindowToggle = false;
            static bool demoWindowToggle = false;

            ImGui::Begin("Debug", NULL, ImGuiWindowFlags_NoScrollbar);
                if (ImGui::Button("Reload Tess Shader")){
                    delete tessShader;
                    tessShader = new Shader("shaders/tess_chunk_vert.vs", "shaders/tess_chunk_frag.fs", "shaders/tess_chunk.tcs", "shaders/tess_chunk.tes");
                    //Set uniforms
                    tessShader->use();
                    tessShader->setFloat("uTexelSize", 1.0f/ (Divisions * Chunk_Width)); // 1/total size of terrain
                    tessShader->setFloat("heightScale", ((Divisions * Chunk_Width)/110000) * 3997); // (number of units (or maximum tiles) / texture size in meters) * maximum height of height map from 0
                }
                ImGui::Checkbox("Metrics Window", &metricsWindowToggle);
                ImGui::Checkbox("Demo Window", &demoWindowToggle);
            ImGui::End();

            ImGui::SetNextWindowSizeConstraints(ImVec2(168,264), ImVec2(FLT_MAX,FLT_MAX));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0,0.0));
            ImGui::Begin("Map", NULL, ImGuiWindowFlags_NoScrollbar);
                ImDrawList* draw_list = ImGui::GetWindowDrawList();

                // Toggles for color channel filter
                static bool redChannel = true;
                static bool greenChannel = true;
                static bool blueChannel = true;
                static bool alphaChannel = true;
                static bool alphaOnly = false;

                float window_visible_x = ImGui::GetContentRegionAvail().x;
                unsigned int elementNum = 1; // number of elements from the left + next element
                const float elementSize = 75.0f;
                const float padding = 00.0f;
                ImGui::Dummy(ImVec2(2.0,0.0));
                ImGui::Dummy(ImVec2(5.0,5.0)); ImGui::SameLine();
                ImGui::Checkbox("Red", &redChannel);
                if(elementSize*(elementNum+1) < window_visible_x ){ // is next element past window edge
                    ImGui::SameLine(elementSize*elementNum);
                    elementNum++;
                }else{
                    elementNum = 1;
                    ImGui::Dummy(ImVec2(5.0,5.0)); ImGui::SameLine();
                }

                ImGui::Checkbox("Green", &greenChannel);
                if(elementSize*(elementNum+1) < window_visible_x ){// is next element past window edge
                    ImGui::SameLine(elementSize*elementNum);
                    elementNum++;
                }else{
                    elementNum = 1;
                    ImGui::Dummy(ImVec2(5.0,5.0)); ImGui::SameLine();
                }

                ImGui::Checkbox("Blue", &blueChannel);
                if(elementSize*(elementNum+1) < window_visible_x ){// is next element past window edge
                    ImGui::SameLine(elementSize*elementNum);
                    elementNum++;
                }else{
                    elementNum = 1;
                    ImGui::Dummy(ImVec2(5.0,5.0)); ImGui::SameLine();
                }

                ImGui::Checkbox("Alpha", &alphaChannel);
                if(elementSize*(elementNum+2) < window_visible_x ){// is next element past window edge (next element is double size)
                    ImGui::SameLine(elementSize*elementNum);
                    elementNum++;
                }else{
                    elementNum = 1;
                    ImGui::Dummy(ImVec2(5.0,5.0)); ImGui::SameLine();
                }

                ImGui::Checkbox("Render Alpha Only", &alphaOnly);

                ImGui::Dummy(ImVec2(2.0,0.0));
                //Create a struct to pass into callback function
                struct mapCallbackArguments{
                    Shader* shaderPtr;
                    glm::vec4 channels;
                    bool alphaOnly;
                };

                static struct mapCallbackArguments *callback_Args_Ptr, callback_Args;
                callback_Args_Ptr = &callback_Args; // set struct pointer to point to a struct
                //set up arguments to be passed to callback function
                callback_Args.shaderPtr = mapShader;
                callback_Args.channels = glm::vec4(redChannel, blueChannel, greenChannel, alphaChannel);
                callback_Args.alphaOnly = alphaOnly;

                draw_list->AddCallback([](const ImDrawList* parent_list, const ImDrawCmd* cmd)
                    {
                        mapCallbackArguments* arguments = (mapCallbackArguments*)cmd->UserCallbackData; // retrieve arguments from callback

                        ImDrawData* draw_data = ImGui::GetDrawData();
                        float L = draw_data->DisplayPos.x;
                        float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
                        float T = draw_data->DisplayPos.y;
                        float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;

                        const float ortho_projection[4][4] =
                        {
                           { 2.0f / (R - L),   0.0f,         0.0f,   0.0f },
                           { 0.0f,         2.0f / (T - B),   0.0f,   0.0f },
                           { 0.0f,         0.0f,        -1.0f,   0.0f },
                           { (R + L) / (L - R),  (T + B) / (B - T),  0.0f,   1.0f },
                        };

                        arguments->shaderPtr->use(); // If I remove this line, it works
                        arguments->shaderPtr->setBool("alphaOnly", arguments->alphaOnly);
                        glUniform4fv(glGetUniformLocation(arguments->shaderPtr->ID, "channels"), 1, glm::value_ptr(arguments->channels));
                        glUniformMatrix4fv(glGetUniformLocation(arguments->shaderPtr->ID, "ProjMtx"), 1, GL_FALSE, &ortho_projection[0][0]);
                    }, (void*)callback_Args_Ptr);

                float mapSize = ImGui::GetContentRegionAvail().x;

                ImGui::SetWindowSize(ImVec2(ImGui::GetWindowWidth(), ImGui::GetItemRectMax().y - ImGui::GetWindowPos().y + mapSize + 1));

                ImGui::Image((void*)(intptr_t)heightMap, ImVec2(mapSize, mapSize), ImVec2(0.0f,1.0f),ImVec2(1.0f,0.0f));
                draw_list->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
                ImGui::Image((void*)(intptr_t)playerIcon, ImVec2(16,16), ImVec2(0.0f,1.0f),ImVec2(1.0f,0.0f));
            ImGui::End();
            ImGui::PopStyleVar();

            if(metricsWindowToggle){ImGui::ShowMetricsWindow();}

            if(demoWindowToggle){ImGui::ShowDemoWindow();}

            //render ImGui
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    // ImGui: Terminate
    //-----------------
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Delete objects
    //---------------
    for(unsigned int i = 0; i < chunkList.size(); i++){
        delete chunkList.at(i);
        chunkList.at(i) = NULL;
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

    if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS){
        if(!keyDown[GLFW_KEY_LEFT_CONTROL]){// if this is the fist frame ctrl is pressed
            keyDown[GLFW_KEY_LEFT_CONTROL] = true;
            if(captureMouse){ // if mouse is currently being captured toggle
                captureMouse = false;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);// tell GLFW to STOP capturing our mouse
                ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse; // enable ImGui mouse interaction
            }else{
                captureMouse = true;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);// tell GLFW to capture mouse
                ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse; // disable ImGui mouse interaction
            }
        }
    }else{ // if key is not currenlty pressed reset
        keyDown[GLFW_KEY_LEFT_CONTROL] = false;
    }

    if(glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS){ // toggle wireframe
        if(!keyDown[GLFW_KEY_K]){ // if this is the fist frame ctrl is pressed
            keyDown[GLFW_KEY_K] = true;
            if(wireframe){ // if in wireframe
                wireframe = false;
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // render wire frame
            }else{
                wireframe = true;
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);// render solid
            }
        }
    }else{
        keyDown[GLFW_KEY_K] = false;


    }
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
    if(captureMouse){ // only run if mouse is being t (ImGui does this in it's own func for menues)
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
