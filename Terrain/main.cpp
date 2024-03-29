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
#include <stdlib.h>
#include <string>
#include <math.h>
#include <cmath>
#include <limits>
#include <vector>
#include <type_traits>
#include <cassert>
#include <utility>

#include "include/shader_s.h"
#include "include/ComputeShader.h"
#include "include/camera.h"
//#include "include/chunk.h"
#include "include/TessChunk.h"
#include "include/CustomEnumerators.h"
#include "include/Ocean.h"
#include "include/Grass.h"
#include "include/bilinearMapReader.h"
#include "include/TreeMaker.h"
#define GLFW_DLL

//Function Delectations
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
ImVec2 dirtyImVec2Convert(glm::vec2 v);
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
    //std::cout << "no error" << " (" << line << ")" << std::endl;
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)


// settings
//---------------------------------
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1027;

// camera and mouse input
Camera camera(glm::vec3(0.0f, 500.0f, 3.0f));
float mouseLastX = SCR_WIDTH / 2.0f;
float mouseLastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true; // if this is the first mouse input
bool captureMouse = false; // if the mouse is currently captured
bool wireframe = false; // if wire frame is on or off
bool keyDown[GLFW_KEY_MENU + 1] = {false}; // if key is currently being pressed



// timing
double deltaTime = 0.0f;
double lastFrame = 0.0f;
double lastSec = 0.0f;
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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_MAXIMIZED , GL_TRUE);//Maximize window

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    int area_x, area_y, area_width, area_height;
    glfwGetMonitorWorkarea(glfwGetPrimaryMonitor(), &area_x, &area_y, &area_width, &area_height);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Terrain Gen", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSwapInterval(0);//framerate cap (1 = cap at framerate, 0 = no cap, 2 = half of monitor framerate)
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

    std::cout << "Compiled on C++:" << __cplusplus << std::endl;
    std::cout << glGetString(GL_VERSION) << std::endl << std::endl;

    // query Compute limitations
	// -----------------

	int max_compute_work_group_count[3];
	int max_compute_work_group_size[3];
	int max_compute_work_group_invocations;

	for (int idx = 0; idx < 3; idx++) {
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, idx, &max_compute_work_group_count[idx]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, idx, &max_compute_work_group_size[idx]);
	}
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &max_compute_work_group_invocations);

	std::cout << "OpenGL Limitations: " << std::endl;
	std::cout << "maximum number of work groups in X dimension " << max_compute_work_group_count[0] << std::endl;
	std::cout << "maximum number of work groups in Y dimension " << max_compute_work_group_count[1] << std::endl;
	std::cout << "maximum number of work groups in Z dimension " << max_compute_work_group_count[2] << std::endl;

	std::cout << "maximum size of a work group in X dimension " << max_compute_work_group_size[0] << std::endl;
	std::cout << "maximum size of a work group in Y dimension " << max_compute_work_group_size[1] << std::endl;
	std::cout << "maximum size of a work group in Z dimension " << max_compute_work_group_size[2] << std::endl;

	std::cout << "Number of invocations in a single local work group that may be dispatched to a compute shader " << max_compute_work_group_invocations << std::endl;
	//TODO add adjustment for work group sizes

	//TODO: add TDR Prevention and Notification of TDR registries [https://learn.microsoft.com/en-us/windows-hardware/drivers/display/tdr-registry-keys]

    // Shader compilation
    //---------------------
    Shader* tessShader = new Shader("shaders/tess_chunk_vert.vs", "shaders/tess_chunk_frag.fs", "shaders/tess_chunk.tcs", "shaders/tess_chunk.tes");
    Shader* mapShader = new Shader("shaders/imgui_shader.vs", "shaders/imgui_shader.fs");
    Shader* screenQuadShader = new Shader("shaders/sky_shader.vs", "shaders/sky_shader.fs");
    Shader* oceanShader = new Shader("shaders/ocean_shader.vs", "shaders/ocean_shader.fs", "shaders/ocean_shader.tcs", "shaders/ocean_shader.tes");
    Shader* grassShader = new Shader("shaders/grass_shader.vs", "shaders/grass_shader.fs");
    Shader* treeShader = new Shader("shaders/tree_shader.vs", "shaders/tree_shader.fs");

    ComputeShader* imageGenShader = new ComputeShader("compute_shaders/image_gen.glsl");
    ComputeShader* windGenShader = new ComputeShader("compute_shaders/wind_gen.glsl");
    ComputeShader* grassHeightGenShader = new ComputeShader("compute_shaders/grass_height_gen.glsl");

    glCheckError();
    glPointSize(10.0f);

    // Enable Depth buffer
    glEnable(GL_DEPTH_TEST);

    // Enable Back Face Culling
    glEnable(GL_CULL_FACE);


    //set zbuffer/clip space to be from 0 to 1 and not -1 to 1 to reduce z fighting
    glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);


    // Set up variables for various shaders and such
    float nearPlane = 0.38f;
    nearPlane = 0.1f; ///TODO REMOVE/ CHANGE WHEN NEAR OBJECTS
    float farPlane = 25000.0f;
    float screenQuadVerts[] = {
    -1.0f, -1.0f, 0.0f,
     1.0f, -1.0f, 0.0f,
     1.0f,  1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f
    };

    //weather/lighting shader variables
    glm::vec3 sunDirection = glm::normalize(glm::vec3(0.0f, 0.5f, 0.2f));
    float windAngle = 0;

    //Set up screenQuad (sky) VBO
    unsigned int screenQuadVBO, screenQuadVAO;
    glGenVertexArrays(1, &screenQuadVAO);
    glGenBuffers(1, &screenQuadVBO);
    glBindVertexArray(screenQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, screenQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(screenQuadVerts), screenQuadVerts, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Texture setup and loading
    //---------------------------
    unsigned int texture1 = loadTexture("textures/wrapTest.png", GL_RGB,  GL_REPEAT);
    //unsigned int heightMap = loadTexture("textures/0.01344305976942091 (4130) (110km) square.png", GL_RGBA, GL_CLAMP_TO_EDGE);
    unsigned int playerIcon = loadTexture("textures/map arrow.png", GL_RGBA, GL_CLAMP_TO_EDGE);

    //Compute Shader Output Texture
    unsigned int computeHightMap;
    unsigned int computeHMWidth= 9600;
    unsigned int computeHMHeight= 9600;
    bilinearMapReader heightMapCopy;
    Transfer_Status heightMapCopied = NOT_STARTED;

	glGenTextures(1, &computeHightMap);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, computeHightMap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, computeHMWidth, computeHMHeight, 0, GL_RG, GL_FLOAT, nullptr);

	glBindImageTexture(0, computeHightMap, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RG32F);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, computeHightMap);

	//compute height map
	int seed = rand();
	seed = rand(); // rand uses UTC time for its seed so it must be called twice
	seed = 5003;/// TODO DELETE  ME
	std::cout << "Seed:" << seed << std::endl;
	imageGenShader->use();
	imageGenShader->setInt("seed", seed);
	imageGenShader->setUVec2("texRes",glm::uvec2(computeHMWidth,computeHMHeight));
    glDispatchCompute((unsigned int)computeHMWidth, (unsigned int)computeHMHeight, 1);

    // make sure writing to image has finished before read
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    //glGenerateMipmap(GL_TEXTURE_2D);
    glCheckError();
    //Generate wind map
    unsigned int windMap;
    unsigned int windMapRes= 256;

	glGenTextures(1, &windMap);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, windMap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glCheckError();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, windMapRes, windMapRes, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    glCheckError();
	glBindImageTexture(1, windMap, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R8);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, windMap);
    glCheckError();
	//compute wind map
	windGenShader->use();
	windGenShader->setInt("seed", seed);
	windGenShader->setInt("imgOutput", 1);
	windGenShader->setUVec2("texRes", glm::uvec2(windMapRes,windMapRes));
    glDispatchCompute((unsigned int)windMapRes, (unsigned int)windMapRes, 1);
    // make sure writing to image has finished before read
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glCheckError();

    //Generate grass height map
    unsigned int grassHeightMap;
    unsigned int grassHeightMapRes= 256;

	glGenTextures(1, &grassHeightMap);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, grassHeightMap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glCheckError();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, grassHeightMapRes, grassHeightMapRes, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    glCheckError();
	glBindImageTexture(2, grassHeightMap, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R8);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, grassHeightMap);
    glCheckError();
	//compute wind map
	grassHeightGenShader->use();
	grassHeightGenShader->setInt("seed", seed);
	grassHeightGenShader->setInt("imgOutput", 2);
	grassHeightGenShader->setUVec2("texRes", glm::uvec2(grassHeightMapRes,grassHeightMapRes));
    glDispatchCompute((unsigned int)grassHeightMapRes, (unsigned int)grassHeightMapRes, 1);
    // make sure writing to image has finished before read
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glCheckError();


    //Object Creation
    //--------------------------------------------------------------------------
    //create chunks
    std::vector<TessChunk*> chunkList;
    const float Max_Height = 1400.0f;// height of brightest pixel in texture
    const int Patchs_Per_Edge = 50;
    const int Divisions = 400/Patchs_Per_Edge; // 800 is 51200
    const float Chunk_Width = 64.0f*Patchs_Per_Edge;
    const float gameSize = (Divisions * Chunk_Width);//length of map in units
    for(int x = 0; x < Divisions; x++){
        for(int z = 0; z < Divisions; z++){// note: -z is forward in coord space
            chunkList.push_back(new TessChunk(glm::vec3((x * Chunk_Width)-(gameSize/2.0f) ,0.0f,(-z * Chunk_Width)+(gameSize/2.0f)) // root of chunk (0,0) with x+ and z-
                                              , Chunk_Width / Patchs_Per_Edge //width of each patch
                                              , Patchs_Per_Edge // number of patches per edge
                                              , texture1, computeHightMap // texture and height map texture
                                              , glm::vec2( (1.0f/Divisions) * x , (1.0f/Divisions) * z ) // UV coord of root
                                              , (1.0f/Divisions))); //uv cord offset (UV coord from bottom to top/ left to right)
        }
    }

    // Settings for use in graphics window
    const float defaultTerrainMinTessDist = 64.0f;
    const float defaultTerrainMaxTessDist = 1000.0f;
    float TerrainMinTessDist = defaultTerrainMinTessDist;
    float TerrainMaxTessDist = defaultTerrainMaxTessDist;

    // set Tess Uniforms
    tessShader->use();
    tessShader->setFloat("minimumTessDist", TerrainMinTessDist);
    tessShader->setFloat("maximumTessDist", TerrainMaxTessDist);

    //Pass height map Variables to Camera
    camera.passHeightMapData(&heightMapCopy, computeHMHeight, computeHMWidth, Max_Height, gameSize);

    // create grass
    const float defaultGrassDensity = 0.35f;
    const float defaultGrassNear = 20.0f;
    const float defaultGrassFar = 240.0f;
    float displayGrassDensity = defaultGrassDensity;
    float displayGrassNear = defaultGrassNear;
    float displayGrassFar = defaultGrassFar;

    Grass grassObj(computeHightMap, glm::uvec2(computeHMWidth,computeHMHeight), windMap, grassHeightMap, glm::vec2(gameSize,gameSize), defaultGrassDensity, defaultGrassNear, defaultGrassFar);

    //set grass shader uniforms
    /// no uniforms to set not in UBI

    // create ocean
    // Settings for use in graphics window
    const unsigned int defaultOceanRes = 32;
    const unsigned int defaultOceanTess = 16;
    unsigned int oceanResHeight = defaultOceanRes;
    unsigned int oceanResWidth = defaultOceanRes;
    unsigned int oceanTessLevel = defaultOceanTess;

    Ocean oceanObj(computeHightMap, glm::vec2(computeHMWidth,computeHMHeight), oceanResWidth, oceanResHeight);
    // set Ocean Shader Uniforms
    oceanShader->use();
    oceanShader->setUInt("tessellationLevel",oceanTessLevel);

    //create tree Manager
    TreeMaker treeObj;
    treeObj.createNewTree();

    // configure a uniform buffer object
    // -------------------------------------------------------------------------------
    struct LowUpdateBufferStruct{
        float heightScale; // height of 1.0 on height map (aka heightest possible point on map)
        float worldSize; // size of map in units/meters
        float uTexelSize; // texel size of hieghtmap or 1.0/(height map resolution)
        float nearPlane;
        float farPlane;
    };

    LowUpdateBufferStruct lowUpdateBufferData;
    lowUpdateBufferData.heightScale = Max_Height;
    lowUpdateBufferData.worldSize = gameSize;
    lowUpdateBufferData.uTexelSize = 1.0f/computeHMWidth;
    lowUpdateBufferData.nearPlane = nearPlane;
    lowUpdateBufferData.farPlane = farPlane;

    // Uniform Block Index for rarely updates shared uniforms
    unsigned int UBIGrassShaderLUS = glGetUniformBlockIndex(grassShader->ID, "lowUpdateShared");
    unsigned int UBIOceanShaderLUS = glGetUniformBlockIndex(oceanShader->ID, "lowUpdateShared");
    unsigned int UBITessShaderLUS = glGetUniformBlockIndex(tessShader->ID, "lowUpdateShared");
    // link each shader's uniform block to this uniform binding point
    glUniformBlockBinding(grassShader->ID, UBIGrassShaderLUS, 0);
    glUniformBlockBinding(oceanShader->ID, UBIOceanShaderLUS, 0);
    glUniformBlockBinding(tessShader->ID, UBITessShaderLUS, 0);
    // Now actually create the buffer
    unsigned int uboLowUpdateShared;
    glGenBuffers(1, &uboLowUpdateShared);
    glBindBuffer(GL_UNIFORM_BUFFER, uboLowUpdateShared);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(LowUpdateBufferStruct), nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    // define the range of the buffer that links to a uniform binding point
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboLowUpdateShared, 0, sizeof(LowUpdateBufferStruct));

    glBindBuffer(GL_UNIFORM_BUFFER, uboLowUpdateShared);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(LowUpdateBufferStruct), &lowUpdateBufferData);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    //freqUpdateShared
    struct FreqUpdateBufferStruct{
        float time;
        float windAngle; // wind Direction in degrees
        float padding1;
        float padding2;
        glm::vec4 sunDirection;
        glm::vec4 playerPos;
    };

    FreqUpdateBufferStruct freqUpdateBufferData;
    freqUpdateBufferData.time = static_cast<float>(glfwGetTime());
    freqUpdateBufferData.windAngle = 0;
    freqUpdateBufferData.sunDirection = glm::vec4(sunDirection,1.0);
    freqUpdateBufferData.playerPos = glm::vec4(camera.Position,1.0);

    // Uniform Block Index for rarely updates shared uniforms
    unsigned int UBIGrassShaderFUS = glGetUniformBlockIndex(grassShader->ID, "freqUpdateShared");
    unsigned int UBIOceanShaderFUS = glGetUniformBlockIndex(oceanShader->ID, "freqUpdateShared");
    unsigned int UBITessShaderFUS = glGetUniformBlockIndex(tessShader->ID, "freqUpdateShared");
    unsigned int UBISkyShaderFUS = glGetUniformBlockIndex(screenQuadShader->ID, "freqUpdateShared");
    // link each shader's uniform block to this uniform binding point
    glUniformBlockBinding(grassShader->ID, UBIGrassShaderFUS, 1);
    glUniformBlockBinding(oceanShader->ID, UBIOceanShaderFUS, 1);
    glUniformBlockBinding(tessShader->ID, UBITessShaderFUS, 1);
    glUniformBlockBinding(screenQuadShader->ID, UBISkyShaderFUS, 1);

    // Now actually create the buffer
    unsigned int uboFreqUpdateShared;
    glGenBuffers(1, &uboFreqUpdateShared);
    glBindBuffer(GL_UNIFORM_BUFFER, uboFreqUpdateShared);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(FreqUpdateBufferStruct), nullptr, GL_STATIC_DRAW); //todo change size
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    // define the range of the buffer that links to a uniform binding point
    glBindBufferRange(GL_UNIFORM_BUFFER, 1, uboFreqUpdateShared, 0, sizeof(FreqUpdateBufferStruct));

    glBindBuffer(GL_UNIFORM_BUFFER, uboFreqUpdateShared);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(FreqUpdateBufferStruct), &freqUpdateBufferData);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    //imgui setup
    //-----------------------------------------------------------------
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

    glCheckError();
    // render loop
    // -------------------------------------------------------------------------------------------------
    while (!glfwWindowShouldClose(window))
    {
        //glCheckError();
        //Delta time Calculation
        double currentFrame = glfwGetTime();
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
        // ---------------------------------------------------------------
        processInput(window);


        // render
        // ----------------------------------------------------------------
        if(!glfwGetWindowAttrib(window, GLFW_ICONIFIED))
        { // if window is minimized do not render (Rendering while at resolution 0x0 cause glm::matix_clip_space.inl assertion to fail)
            //glCheckError();
            //glClearColor(0.3f, 0.4f, 0.5f, 1.0f);
            glClearColor(0.207f, 0.318f, 0.361f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            //get view port info [x, y, width, height]
            static int *viewportData = new int[4];
            static float *depthRangeData = new float[2];
            glGetIntegerv(GL_VIEWPORT, viewportData);
            glGetFloatv(GL_DEPTH_RANGE, depthRangeData);

            //Global -> view
            glm::mat4 view = camera.GetViewMatrix();
            glm::mat4 originView = camera.GetOriginViewMatrix();
            glm::mat4 YCorrectedOriginView = camera.GetOriginYCorrectedViewMatrix();

            //View -> Projection
            glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), static_cast<float>(viewportData[2])/ static_cast<float>(viewportData[3]), nearPlane, farPlane);
            //tessShader.setMat4("projection", projection);

            glm::mat4 projection2 = glm::perspective(glm::radians(40.0f), static_cast<float>(viewportData[2])/ static_cast<float>(viewportData[3]), nearPlane, farPlane);//TODO Remove

            //Set shared uniform buffer
            freqUpdateBufferData.time = static_cast<float>(currentFrame); ///TODO fix potential problems with lack of precision with floats
            freqUpdateBufferData.windAngle = windAngle;
            freqUpdateBufferData.sunDirection = glm::vec4(sunDirection,1.0f);
            freqUpdateBufferData.playerPos = glm::vec4(camera.Position,1.0f);

            glBindBuffer(GL_UNIFORM_BUFFER, uboFreqUpdateShared);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(FreqUpdateBufferStruct), &freqUpdateBufferData);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);

            //ScreenQuad (sky) draw
            //TODO: try stencil mask
            glDepthMask(GL_FALSE);
            screenQuadShader->use();
            float horizionAngle = sqrt((2.0f*camera.Position.y)/(6371000.0f/(1.0f-0.13f)));
            screenQuadShader->setFloat("horizonAngle",horizionAngle);
            screenQuadShader->setMat4("persMatrix",projection * originView);
            screenQuadShader->setMat4("invPersMatrix", glm::inverse(originView) * glm::inverse(projection ));
            screenQuadShader->setVec2("depthrange", glm::vec2(depthRangeData[0], depthRangeData[1]));
            screenQuadShader->setVec4("viewport", glm::vec4(viewportData[0],viewportData[1],viewportData[2],viewportData[3]));
            glBindVertexArray(screenQuadVAO);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            glBindVertexArray(0);
            glDepthMask(GL_TRUE);
            //set up new frame for ImGui
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // Begin Draw
            //-----------------------------------------------------------------------
            //Draw Ground
            chunkList.at(0)->prepShader(*tessShader);
            for(unsigned int i = 0; i < chunkList.size(); i++){
                chunkList.at(i)->draw(*tessShader, view, projection, projection2);
            }

            //Draw Ocean
            oceanObj.setUpVertices(YCorrectedOriginView, projection, camera.Position, 0);
            oceanObj.draw(*oceanShader, YCorrectedOriginView, projection);

            //Draw Grass
            grassObj.draw(*grassShader, view, projection, camera, heightMapCopy, Max_Height, gameSize);
            //glCheckError();

            //Draw Trees
            treeObj.drawTree(*treeShader, view, projection, 0, glm::vec3(0.0f, 500.0f,-5.0f), texture1);

            //Create ImGui windows
            //--------------------------------------------------------

            ImGui::Begin("Pos");
                std::string positionStr = "X:" + std::to_string(camera.Position.x);
                ImGui::Text(positionStr.c_str());
                positionStr = "Y:" + std::to_string(camera.Position.y);
                ImGui::Text(positionStr.c_str());
                positionStr = "Y Feet:" + std::to_string(camera.Position.y -1.7f);
                ImGui::Text(positionStr.c_str());
                positionStr = "Z:" + std::to_string(camera.Position.z);
                ImGui::Text(positionStr.c_str());

                positionStr = "FPS:" + std::to_string(FPS);
                ImGui::Text(positionStr.c_str());

                float yawAngle;
                std::modf(camera.Yaw, &yawAngle);
                yawAngle = static_cast<int>(yawAngle)%360;
                if(yawAngle < 0 ) yawAngle += 360;
                if(yawAngle < 45 || yawAngle > 315){ // East is 0 degrees
                    positionStr = "Yaw:" + std::to_string(camera.Yaw) + "(East)";
                }else if(yawAngle < 135){ //South is 90 degrees
                    positionStr = "Yaw:" + std::to_string(camera.Yaw) + " (South)";
                }else if(yawAngle < 225){ // West is 180 degrees
                    positionStr = "Yaw:" + std::to_string(camera.Yaw) + " (West)";
                } else{ // N is 270 degrees
                    positionStr = "Yaw:" + std::to_string(camera.Yaw) + " (North)";
                }
                ImGui::Text(positionStr.c_str());
                positionStr = "Pitch:" + std::to_string(camera.Pitch);
                ImGui::Text(positionStr.c_str());
                positionStr = "FOV:" + std::to_string(camera.Zoom);
                ImGui::Text(positionStr.c_str());

                glm::vec2 normalizedPlayerPos = glm::vec2(camera.Position.x /gameSize, -camera.Position.z/gameSize); //(0,0) is Bottom left, (1,1) is top right
                positionStr = "Norm X:" + std::to_string(normalizedPlayerPos.x);
                ImGui::Text(positionStr.c_str());
                positionStr = "Norm Y:" + std::to_string(normalizedPlayerPos.y);
                ImGui::Text(positionStr.c_str());


                positionStr = "Elevation of Ground:" + std::to_string(camera.Elevation);
                ImGui::Text(positionStr.c_str());
                positionStr = "Dist to Ground:" + std::to_string(camera.Position.y - camera.Elevation);
                ImGui::Text(positionStr.c_str());
                ImGui::Checkbox("noClip", &camera.fly);

                ImGui::SliderFloat("Speed", &camera.MovementSpeed, 0.1f, 1000.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
            ImGui::End();

            static bool metricsWindowToggle = false;
            static bool demoWindowToggle = false;
            static bool weatherWindowToggle = true;
            static bool graphicsWindowToggle = true;

            ImGui::Begin("Test Image Window");
                ImGui::Image((void*)(intptr_t)computeHightMap, ImVec2(400, 400), ImVec2(0.0f,1.0f),ImVec2(1.0f,0.0f));
                ImGui::Image((void*)(intptr_t)windMap, ImVec2(400, 400), ImVec2(0.0f,1.0f),ImVec2(1.0f,0.0f));
                ImGui::Image((void*)(intptr_t)grassHeightMap, ImVec2(400, 400), ImVec2(0.0f,1.0f),ImVec2(1.0f,0.0f));
            ImGui::End();

            ImGui::Begin("Debug", nullptr, ImGuiWindowFlags_NoScrollbar);
                if (ImGui::Button("Reload Tess Shader")){
                    delete tessShader;
                    tessShader = new Shader("shaders/tess_chunk_vert.vs", "shaders/tess_chunk_frag.fs", "shaders/tess_chunk.tcs", "shaders/tess_chunk.tes");
                    //Set uniforms
                    tessShader->use();
                    tessShader->setFloat("minimumTessDist", TerrainMinTessDist);
                    tessShader->setFloat("maximumTessDist", TerrainMaxTessDist);
                    unsigned int UBITessShaderLUS = glGetUniformBlockIndex(tessShader->ID, "lowUpdateShared");
                    unsigned int UBITessShaderFUS = glGetUniformBlockIndex(tessShader->ID, "freqUpdateShared");
                    // link each shader's uniform block to this uniform binding point
                    glUniformBlockBinding(tessShader->ID, UBITessShaderLUS, 0);
                    glUniformBlockBinding(tessShader->ID, UBITessShaderFUS, 1);
                }
                if (ImGui::Button("Reload Sky Shader")){
                    delete screenQuadShader;
                    screenQuadShader = new Shader("shaders/sky_shader.vs", "shaders/sky_shader.fs", nullptr, nullptr);
                    screenQuadShader->use();
                    unsigned int UBISkyShaderFUS = glGetUniformBlockIndex(screenQuadShader->ID, "freqUpdateShared");
                    // link each shader's uniform block to this uniform binding point
                    glUniformBlockBinding(screenQuadShader->ID, UBISkyShaderFUS, 1);
                }
                if (ImGui::Button("Reload Ocean Shader")){
                    delete oceanShader;
                    oceanShader = new Shader("shaders/ocean_shader.vs", "shaders/ocean_shader.fs", "shaders/ocean_shader.tcs", "shaders/ocean_shader.tes");
                    oceanShader->use();
                    oceanShader->setUInt("tessellationLevel",oceanTessLevel);
                    unsigned int UBIOceanShaderLUS = glGetUniformBlockIndex(oceanShader->ID, "lowUpdateShared");
                    unsigned int UBIOceanShaderFUS = glGetUniformBlockIndex(oceanShader->ID, "fowUpdateShared");
                    // link each shader's uniform block to this uniform binding point
                    glUniformBlockBinding(oceanShader->ID, UBIOceanShaderLUS, 0);
                    glUniformBlockBinding(oceanShader->ID, UBIOceanShaderFUS, 1);
                }
                if (ImGui::Button("Reload Grass Shader")){
                    delete grassShader;
                    grassShader = new Shader("shaders/grass_shader.vs", "shaders/grass_shader.fs", nullptr, nullptr);
                    grassShader->use();
                    unsigned int UBIGrassShaderLUS = glGetUniformBlockIndex(grassShader->ID, "lowUpdateShared");
                     unsigned int UBIGrassShaderFUS = glGetUniformBlockIndex(grassShader->ID, "freqUpdateShared");
                    // link each shader's uniform block to this uniform binding point
                    glUniformBlockBinding(grassShader->ID, UBIGrassShaderLUS, 0);
                    glUniformBlockBinding(grassShader->ID, UBIGrassShaderFUS, 1);
                }
                if (ImGui::Button("Reload Compute Shader")){
                    delete imageGenShader;
                    imageGenShader = new ComputeShader("compute_shaders/image_gen.glsl");

                    //Set uniforms
                    imageGenShader->use();
                    imageGenShader->setInt("seed", seed);
                    imageGenShader->setUVec2("texRes",glm::uvec2(computeHMWidth,computeHMHeight));
                    glDispatchCompute((unsigned int)computeHMWidth, (unsigned int)computeHMHeight, 1);

                    heightMapCopied = NOT_STARTED; //Recopy height map to RAM

                    // make sure writing to image has finished before read
                    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
                }

                if (ImGui::Button("Reload Wind Compute Shader")){
                    delete windGenShader;
                    windGenShader = new ComputeShader("compute_shaders/wind_gen.glsl");

                    //Set uniforms
                    windGenShader->use();
                    windGenShader->setInt("seed", seed);
                    windGenShader->setInt("imgOutput", 1);
                    windGenShader->setUVec2("texRes", glm::uvec2(windMapRes,windMapRes));
                    glDispatchCompute((unsigned int)windMapRes, (unsigned int)windMapRes, 1);

                    // make sure writing to image has finished before read
                    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
                }

                if (ImGui::Button("Reload Grass Height Compute Shader")){
                    delete grassHeightGenShader;
                    grassHeightGenShader = new ComputeShader("compute_shaders/grass_height_gen.glsl");

                    //Set uniforms
                    grassHeightGenShader->use();
                    grassHeightGenShader->setInt("seed", seed);
                    grassHeightGenShader->setInt("imgOutput", 2);
                    grassHeightGenShader->setUVec2("texRes", glm::uvec2(grassHeightMapRes,grassHeightMapRes));
                    glDispatchCompute((unsigned int)grassHeightMapRes, (unsigned int)grassHeightMapRes, 1);

                    // make sure writing to image has finished before read
                    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
                }

                std::string SeedButtonText = "Seed:" + std::to_string(seed);
                if (ImGui::Button(SeedButtonText.c_str())){
                    seed = rand();
                }

                if(ImGui::Button("Print verts")){
                    oceanObj.printLineVerts();
                }
                ImGui::Checkbox("Metrics Window", &metricsWindowToggle);
                ImGui::Checkbox("Demo Window", &demoWindowToggle);
                ImGui::Checkbox("Weather Window", &weatherWindowToggle);
                ImGui::Checkbox("Graphics Window", &graphicsWindowToggle);
            ImGui::End();

            if(weatherWindowToggle){
                ImGui::Begin("Weather and Time");
                    std::string textString;

                    // Longitude of the sun by day (does not account for rotation of earth or position on earth
                    static float Nint = 0.0;
                    static float hourAngle = 0.0;
                    static float latitude = 0.0;
                    // move statics out of imgui if used

                    ImGui::SliderFloat("Days:", &Nint, 0.0f, 365.0f, "%.0f");
                    ImGui::SliderFloat("HourAngle:", &hourAngle, -180.0f, 180.0f, "%.7f");
                    ImGui::SliderFloat("Latitude:", &latitude, -90.0f+0.00001f, 90.0f-0.00001f, "%.5f"); // Math breaks down if exactly on poles

                    float Nfract = hourAngle/180.0f * 0.5f;
                    float minutes = (hourAngle + 180.0f)/15.0f;
                    float hour;
                    minutes = std::modf(minutes, &hour);
                    minutes *= 60.0f;
                    std::modf(minutes, &minutes);
                    std::string minuteString = std::to_string(static_cast<int>(minutes));
                    if(minuteString.size() < 2){minuteString = "0" + minuteString;} // add leading 0

                    textString = "nfract:" + std::to_string(Nfract) + " time:" + std::to_string(static_cast<int>(hour)) + ":" + minuteString;
                    ImGui::Text(textString.c_str());

                    float N = Nint + Nfract; //days since 2000 Jan 1st 12:00 Noon GMT
                    double L = 280.460f + 0.9856474f*N;
                    double g = 357.528+0.9856003*N;
                    double intComponent;
                    // clamp between 0 and 360
                    g = std::modf(g, &intComponent);
                    g += static_cast<int>(intComponent)%360;
                    L = std::modf(L, &intComponent);
                    L += static_cast<int>(intComponent)%360;

                    double a = std::sin(g * (M_PI/180.0d));//calculate sin in degrees
                    double b = std::sin(2.0d*g * (M_PI/180.0d));
                    double eclipticLongitude = L + 1.915d*a + 0.020d*b;

                    double Ecliptic = (23.439 - 0.00000036*N)*(M_PI/180.0d);

                    double RA = std::atan2(cos(Ecliptic),std::cos(eclipticLongitude*(M_PI/180.0d)));
                    double declination = std::asin(std::sin(Ecliptic) * std::sin(eclipticLongitude*(M_PI/180.0d)));

                    double zenith = std::acos( (std::sin(latitude*(M_PI/180.0d))*std::sin(declination)) + (std::cos(latitude*(M_PI/180.0d))*std::cos(declination)*std::cos((hourAngle)*(M_PI/180.0d))) );

                    //double azimuth = std::acos( ((std::sin(declination)*std::cos(latitude*(M_PI/180.0d))) - (std::cos((hourAngle)*(M_PI/180.0d))*std::cos(declination)*std::sin(latitude*(M_PI/180.0d)))) / std::sin(zenith) );
                    //clamped to avoid small rounding errors when hourAngle is close to zero leading to for example acos(1.000...0002) which returns a NaN
                    double azimuth2 = std::acos( glm::clamp((std::sin(declination) - (std::cos(zenith)*std::sin(latitude*(M_PI/180.0d)))) / (std::sin(zenith) * std::cos(latitude*(M_PI/180.0d))),-1.0,1.0) );
                    double azimuth360 = azimuth2;
                    if(hourAngle > 0.0){
                        azimuth360 = (2*M_PI) - azimuth2; //360-azimuth in radians
                    }

                    textString = "g:" + std::to_string(g) + " L:" + std::to_string(L);
                    ImGui::Text(textString.c_str());
                    textString = "Long:" + std::to_string(eclipticLongitude) + " N:" + std::to_string(N);
                    ImGui::Text(textString.c_str());
                    textString = "RA:" + std::to_string(RA * (180.0d/M_PI)) + " d:" + std::to_string(declination*(180.0d/M_PI));
                    ImGui::Text(textString.c_str());
                    textString = "Zenith: " + std::to_string(zenith*(180.0d/M_PI)) + " Azimuth:" + std::to_string(azimuth2*(180.0d/M_PI)) + " Azimuth 360:" + std::to_string(azimuth360*(180.0d/M_PI));
                    ImGui::Text(textString.c_str());
                    textString = "angle:" + std::to_string(90.0-(zenith *(180.0d/M_PI)));
                    ImGui::Text(textString.c_str());

                    //double sunElevation = (M_PI/2)-zenith;

                    sunDirection = glm::vec3(std::sin(zenith) * std::sin(azimuth360), std::cos(zenith) , std::sin(zenith) *  -std::cos(azimuth360)); //todo fix sun dipping south at noon
                    sunDirection = glm::normalize(sunDirection);
                    textString = "Sun Dir:" + std::to_string(sunDirection.x) + ", " + std::to_string(sunDirection.y) + ", " + std::to_string(sunDirection.z) + ", ";
                    ImGui::Text(textString.c_str());

                    //windDirection
                    ImGui::SliderFloat("Wind Direction:", &windAngle, 0, 360, "%.5f");
                    float cardinalAngle;
                    std::modf(windAngle, &cardinalAngle);
                    cardinalAngle = static_cast<int>(cardinalAngle)%360;
                    if(cardinalAngle < 0 ) cardinalAngle += 360;
                    if(cardinalAngle < 45 || cardinalAngle > 315){ // North is 0 degrees
                        textString = "(North)";
                    }else if(cardinalAngle < 135){ //West is 90 degrees
                        textString = "(West)";
                    }else if(cardinalAngle < 225){ // Sound is 180 degrees
                        textString = "South)";
                    } else{ // E is 270 degrees
                        textString ="(East)";
                    }

                    ImGui::Text(textString.c_str());

                    tessShader->use();
                    tessShader->setFloat("latitude", latitude);
                ImGui::End();
            }

            if(graphicsWindowToggle){
                ImGui::Begin("Graphics");
                    static unsigned int oceanTessDisplay = oceanTessLevel;
                    static float terrainMinDisplay = TerrainMinTessDist;
                    static float terrainMaxDisplay = TerrainMaxTessDist;
                    if(ImGui::Button("Clear All to Current Value")){
                        //ocean Super Resolution
                        oceanObj.getResolution(oceanResWidth,oceanResHeight);
                        //Ocean Tessellation
                        oceanTessDisplay = oceanTessLevel;
                        //Terrain Tessellation
                        terrainMinDisplay = TerrainMinTessDist;
                        terrainMaxDisplay = TerrainMaxTessDist;
                        //Grass
                        grassObj.getSettings(displayGrassDensity, displayGrassNear, displayGrassFar);
                    }
                    if(ImGui::Button("Reset All to Default")){
                        //ocean Super Resolution
                        oceanResWidth = defaultOceanRes;
                        oceanResHeight = defaultOceanRes;
                        oceanObj.changeResolution(oceanResWidth,oceanResHeight);
                        //Ocean Tessellation
                        oceanTessLevel = defaultOceanTess;
                        oceanTessDisplay = oceanTessLevel;
                        oceanShader->use();
                        oceanShader->setUInt("tessellationLevel",oceanTessLevel);
                        //Terrain Tessellation
                        TerrainMinTessDist = defaultTerrainMinTessDist;
                        TerrainMaxTessDist = defaultTerrainMaxTessDist;
                        terrainMinDisplay = TerrainMinTessDist;
                        terrainMaxDisplay = TerrainMaxTessDist;
                        tessShader->use();
                        tessShader->setFloat("minimumTessDist", TerrainMinTessDist);
                        tessShader->setFloat("maximumTessDist", TerrainMaxTessDist);
                        //Grass
                        displayGrassDensity = defaultGrassDensity;
                        displayGrassFar = defaultGrassFar;
                        displayGrassNear = defaultGrassNear;
                        grassObj.changeSettings(displayGrassDensity, displayGrassNear, displayGrassFar);

                    }

                    const unsigned int superResLower = 0;
                    const unsigned int superResUpper = 100;
                    ImGui::Text("Ocean Super Resolution:");
                    ImGui::SameLine();
                    ImGui::TextDisabled("(?)");
                    if (ImGui::IsItemHovered()){
                        ImGui::BeginTooltip();
                        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                        ImGui::TextUnformatted(("The number of division the screen \n"
                                               "is broken into for rendering the Ocean. \n"
                                               "(Expensive but accurate) Default:" + std::to_string(defaultOceanRes) + 'x' + std::to_string(defaultOceanRes)).c_str());
                        ImGui::PopTextWrapPos();
                        ImGui::EndTooltip();
                    }
                    if(ImGui::Button("Apply##oceanSupRes")){
                        oceanObj.changeResolution(oceanResWidth,oceanResHeight);
                    }
                    ImGui::SameLine();
                    ImGui::Dummy(ImVec2(5.0,0.0));
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 3);
                    ImGui::DragScalar("##oceanSupResWidth", ImGuiDataType_U32, &oceanResWidth, 1, &superResLower, &superResUpper, "%d%", ImGuiSliderFlags_AlwaysClamp);
                    ImGui::SameLine();
                    ImGui::Text("X");
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 3);
                    ImGui::DragScalar("##oceanSupResHeight", ImGuiDataType_U32, &oceanResHeight, 1, &superResLower, &superResUpper, "%d%", ImGuiSliderFlags_AlwaysClamp);


                    const unsigned int oceanTessLower = 0;
                    const unsigned int oceanTessUpper = 64;
                    ImGui::Text("Ocean Tessellation Level:");
                    ImGui::SameLine();
                    ImGui::TextDisabled("(?)");
                    if (ImGui::IsItemHovered()){
                        ImGui::BeginTooltip();
                        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                        ImGui::TextUnformatted(("The number of division the divsions of\n"
                                               "the super resolution is broken into for\n"
                                               "rendering the Ocean. \n"
                                               "(Cheap but non-linear) Default:" + std::to_string(defaultOceanTess)).c_str());
                        ImGui::PopTextWrapPos();
                        ImGui::EndTooltip();
                    }
                    if(ImGui::Button("Apply##oceanTess")){
                        oceanTessLevel = oceanTessDisplay;
                        oceanShader->use();
                        oceanShader->setUInt("tessellationLevel",oceanTessLevel);
                    }
                    ImGui::SameLine();
                    ImGui::Dummy(ImVec2(5.0,0.0));
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 3);
                    ImGui::DragScalar("##oceanTess", ImGuiDataType_U32, &oceanTessDisplay, 1, &oceanTessLower, &oceanTessUpper, "%d%", ImGuiSliderFlags_AlwaysClamp);

                    const float terrainTessLower = 0.0;
                    const float terrainTessUpper = farPlane;
                    ImGui::Text("Terrain Tessellation Distance:");
                    ImGui::SameLine();
                    ImGui::TextDisabled("(?)");
                    if (ImGui::IsItemHovered()){
                        ImGui::BeginTooltip();
                        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                        ImGui::TextUnformatted(("How close to the player should the\n"
                                                "highest/lowest terrain resolution be set.\n"
                                                "(Larger = more expensive)\n"
                                                "Default: 0 -> " + std::to_string(static_cast<int>(defaultTerrainMinTessDist)) +
                                                " -> " + std::to_string(static_cast<int>(defaultTerrainMaxTessDist)) + " -> Horizon" ).c_str());
                        ImGui::PopTextWrapPos();
                        ImGui::EndTooltip();
                    }
                    if(ImGui::Button("Apply##terrainTessMin")){
                        TerrainMinTessDist = terrainMinDisplay;
                        TerrainMaxTessDist = terrainMaxDisplay;
                        tessShader->use();
                        tessShader->setFloat("minimumTessDist", TerrainMinTessDist);
                        tessShader->setFloat("maximumTessDist", TerrainMaxTessDist);
                    }
                    ImGui::SameLine();
                    ImGui::Dummy(ImVec2(5.0,0.0));
                    ImGui::SameLine();
                    ImGui::Text("Near:");
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 4);
                    ImGui::DragScalar("##terrainTessMin", ImGuiDataType_Float, &terrainMinDisplay, 1.0, &terrainTessLower, &terrainMaxDisplay, "%.0f", ImGuiSliderFlags_AlwaysClamp);
                    ImGui::SameLine();
                    ImGui::Text("Far:");
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 4);
                    ImGui::DragScalar("##terrainTessMzn", ImGuiDataType_Float, &terrainMaxDisplay, 1.0, &terrainMinDisplay, &terrainTessUpper, "%.0f", ImGuiSliderFlags_AlwaysClamp);


                    const float grassLower = 0.0f;
                    const float grassUpper = farPlane;
                    const float grassDensityLower = 0.1f;
                    const float grassDensityUpper = 2.0f;

                    ImGui::Text("Grass Render Distance:");
                    ImGui::SameLine();
                    ImGui::TextDisabled("(?)");
                    if (ImGui::IsItemHovered()){
                        ImGui::BeginTooltip();
                        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                        ImGui::TextUnformatted(("How close to the player should the\n"
                                                "High/Low poly resolution be set.\n"
                                                "(Larger = more expensive)\n"
                                                "Default: 0 -> " + std::to_string(static_cast<int>(defaultGrassNear)) +
                                                " (Low Poly) -> " + std::to_string(static_cast<int>(defaultGrassFar)) ).c_str());
                        ImGui::PopTextWrapPos();
                        ImGui::EndTooltip();
                    }
                    if(ImGui::Button("Apply##GrassRenderDist")){
                        grassObj.changeSettings(displayGrassDensity, displayGrassNear, displayGrassFar);
                    }
                    ImGui::SameLine();
                    ImGui::Dummy(ImVec2(5.0,0.0));
                    ImGui::SameLine();
                    ImGui::Text("Near:");
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 4);
                    ImGui::DragScalar("##GrassRenderDistNear", ImGuiDataType_Float, &displayGrassNear, 10.0f, &grassLower, &displayGrassFar, "%.0f", ImGuiSliderFlags_AlwaysClamp);
                    ImGui::SameLine();
                    ImGui::Text("Far:");
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 4);
                    ImGui::DragScalar("##GrassRenderDistFar", ImGuiDataType_Float, &displayGrassFar, 10.0f, &displayGrassNear, &grassUpper, "%.0f", ImGuiSliderFlags_AlwaysClamp);

                    ImGui::Text("Average Speration in Meters:");
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 4);
                    ImGui::DragScalar("##GrassDensity", ImGuiDataType_Float, &displayGrassDensity, 0.01f, &grassDensityLower, &grassDensityUpper, "%.02f", ImGuiSliderFlags_AlwaysClamp);
                    displayGrassNear = static_cast<float>( static_cast<int>(displayGrassNear) - static_cast<int>(displayGrassNear)%5); // round to nearest mul of 5
                    displayGrassFar = static_cast<float>( static_cast<int>(displayGrassFar) - static_cast<int>(displayGrassFar)%5); // round to nearest mul of 5


                ImGui::End();
            }


            ImGui::SetNextWindowSizeConstraints(ImVec2(168,264), ImVec2(FLT_MAX,FLT_MAX));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0,0.0));
            ImGui::Begin("Map", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
                ImDrawList* draw_list = ImGui::GetWindowDrawList();

                // Toggles for color channel filter
                static bool redChannel = true;
                static bool greenChannel = true;
                static bool blueChannel = true;
                static bool alphaChannel = true;
                static bool alphaOnly = false;
                static bool showMap = true;

                float window_visible_x = ImGui::GetContentRegionAvail().x;
                unsigned int elementNum = 1; // number of elements from the left + next/current element
                const float elementSize = 75.0f;
                ImGui::Dummy(ImVec2(2.0,0.0));
                ImGui::Dummy(ImVec2(5.0,5.0)); ImGui::SameLine();
                if (alphaOnly || showMap)
                    ImGui::BeginDisabled();
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
                if (alphaOnly || showMap)
                    ImGui::EndDisabled();

                if (showMap)
                    ImGui::BeginDisabled();
                ImGui::Checkbox("Render Alpha Only", &alphaOnly);
                if (showMap)
                    ImGui::EndDisabled();

                ImGui::Dummy(ImVec2(5.0,5.0)); ImGui::SameLine();

                ImGui::Checkbox("Show Map", &showMap);

                ImGui::Dummy(ImVec2(2.0,0.0));

                //Create a struct to pass into callback function
                struct mapCallbackArguments{
                    Shader* shaderPtr;
                    float uTexelSize;
                    float heightScale;
                    glm::vec4 channels;
                    bool alphaOnly;
                    bool showMap;
                };

                static struct mapCallbackArguments *callback_Args_Ptr, callback_Args;
                callback_Args_Ptr = &callback_Args; // set struct pointer to point to a struct
                //set up arguments to be passed to callback function
                callback_Args.shaderPtr = mapShader;
                callback_Args.uTexelSize = 1.0f/computeHMWidth;
                callback_Args.heightScale = Max_Height;
                callback_Args.channels = glm::vec4(redChannel, greenChannel, blueChannel, alphaChannel);
                callback_Args.alphaOnly = alphaOnly;
                callback_Args.showMap = showMap;

                draw_list->AddCallback([]( [[maybe_unused]] const ImDrawList* parent_list, const ImDrawCmd* cmd)
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
                        arguments->shaderPtr->setFloat("uTexelSize", arguments->uTexelSize);
                        arguments->shaderPtr->setFloat("heightScale", arguments->heightScale);
                        arguments->shaderPtr->setBool("alphaOnly", arguments->alphaOnly);
                        arguments->shaderPtr->setBool("showMap", arguments->showMap);
                        arguments->shaderPtr->setVec4("channels", arguments->channels);
                        glUniformMatrix4fv(glGetUniformLocation(arguments->shaderPtr->ID, "ProjMtx"), 1, GL_FALSE, &ortho_projection[0][0]);
                    }, (void*)callback_Args_Ptr);

                float mapSize = ImGui::GetContentRegionAvail().x;

                //Resize window to remove extra window space to fit map
                ImGui::SetWindowSize(ImVec2(ImGui::GetWindowWidth(), ImGui::GetItemRectMax().y - ImGui::GetWindowPos().y + mapSize + 1));

                //TODO Change computeHightMap for some mipmaped or low res variant
                ImGui::Image((void*)(intptr_t)computeHightMap, ImVec2(mapSize, mapSize), ImVec2(0.0f,1.0f),ImVec2(1.0f,0.0f));
                draw_list->AddCallback(ImDrawCallback_ResetRenderState, nullptr); // reset shader

                // Draw playericon on mini map

                const float halfIconSize = std::max(16.0f, std::min(mapSize/15.0f, 30.0f))/2.0f; // map icon size between 16-30 pixels scaled by dividing size of map by some factor that looks good
                // rotation matrix (camera -90 is north and rotates the wrong way so it must be corrected by subtracting 90 and taking the negitive
                glm::mat2 rotMatrix = glm::mat2(glm::vec2(cos(glm::radians(-camera.Yaw - 90)),sin(glm::radians(-camera.Yaw - 90))),glm::vec2(-sin(glm::radians(-camera.Yaw - 90)),cos(glm::radians(-camera.Yaw - 90))));
                glm::vec2 normalizedIconPos = glm::vec2((camera.Position.x /gameSize)+0.5, ((camera.Position.z+gameSize)/gameSize)-0.5); //(0,0) is top left, (1,1) is bottom right
                glm::vec2 iconCenter = glm::vec2(ImGui::GetItemRectMin().x + (mapSize * normalizedIconPos.x), ImGui::GetItemRectMin().y + (mapSize * normalizedIconPos.y));
                glm::vec2 a = glm::vec2(-halfIconSize,-halfIconSize) * rotMatrix + iconCenter;
                glm::vec2 b = glm::vec2(-halfIconSize, halfIconSize) * rotMatrix + iconCenter;
                glm::vec2 c = glm::vec2( halfIconSize, halfIconSize) * rotMatrix + iconCenter;
                glm::vec2 d = glm::vec2( halfIconSize,-halfIconSize) * rotMatrix + iconCenter;

                draw_list->PushClipRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), true); //Clip player icon to only with in map
                // draw quad order: a>b>c  a>c>d
                // a is top left, b is bottom left, c is bottom right, d is top right
                draw_list->AddImageQuad((void*)(intptr_t)playerIcon, dirtyImVec2Convert(a), dirtyImVec2Convert(b), dirtyImVec2Convert(c), dirtyImVec2Convert(d), ImVec2(0.0f,1.0f),ImVec2(0.0f,0.0f), ImVec2(1.0f,0.0f), ImVec2(1.0f,1.0f));
                draw_list->PopClipRect();
            ImGui::End();
            ImGui::PopStyleVar();

            if(metricsWindowToggle){ImGui::ShowMetricsWindow();}

            if(demoWindowToggle){ImGui::ShowDemoWindow();}

            //render ImGui
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }
        //glCheckError();
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();

        //TODO MOVE ME and add in menu integration (or multi threading?)
        if(heightMapCopied == NOT_STARTED){
            heightMapCopied = IN_PROGRESS;
            heightMapCopy.write(computeHMWidth, computeHMHeight, computeHightMap, GL_RG, 2);
            glCheckError();
            heightMapCopied = COMPLETE;
        }
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
        chunkList.at(i) = nullptr;
    }

    glDeleteVertexArrays(1, &screenQuadVAO);
    glDeleteBuffers(1, &screenQuadVBO);

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
        camera.ProcessKeyboard(LEFT, deltaTime);
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
void framebuffer_size_callback([[maybe_unused]] GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback([[maybe_unused]] GLFWwindow* window, double xposIn, double yposIn)
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
void scroll_callback([[maybe_unused]] GLFWwindow* window, [[maybe_unused]] double xoffset, double yoffset)
{
    if(!ImGui::GetIO().WantCaptureMouse){
        camera.ProcessMouseScroll(static_cast<float>(yoffset));
    }
}

ImVec2 dirtyImVec2Convert(glm::vec2 v){
    // a dirty way to go from GLM vec2 to ImVec2 because I can be assed to remember how to add operators in another namespace
    return ImVec2(v.x,v.y);
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
