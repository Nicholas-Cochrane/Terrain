#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "CustomEnumerators.h"

#include <vector>
#include <cmath>
#include <cassert>
#include <limits>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Default camera values
const float YAW         = -90.0f;
const float PITCH       =  0.0f;
const float SPEED       =  1000.0f;
const float SENSITIVITY =  0.08f;
const float ZOOM        =  95.0f;

static_assert(std::numeric_limits<float>::is_iec559, "IEEE 754 (ISO IEC559) required"); // Need IEEE 754 for Infinity float
const float ELEVATION = -std::numeric_limits<float>::infinity();

// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
    // camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // euler Angles
    float Yaw;
    float Pitch;
    // camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
    // Player World Interaction
    float Elevation;
    bool fly = true;

    // constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM), Elevation(ELEVATION)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }
    // constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM), Elevation(ELEVATION)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    glm::mat4 GetOriginViewMatrix()
    {
        return glm::lookAt(glm::vec3(0,0,0), Front, Up);
    }

    void passHeightMapData(float *heightMapPtr, unsigned int *height, unsigned int *width, const int* maxHeight, const float* inGameSize, Transfer_Status *status){
        heightMapArray = heightMapPtr;
        heightMapHeight = height;
        heightMapWidth = width;
        heightMapMaxHeight = maxHeight;
        gameSize = inGameSize;
        heightMapStatus = status;
    }

    // processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {

        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;

        glm::vec2 normalizedPlayerPos = glm::vec2(Position.x / *gameSize, -Position.z/ *gameSize); //(0,0) is Bottom left, (1,1) is top right
        if(normalizedPlayerPos.x > 0.0 && normalizedPlayerPos.x < 1.0 &&
           normalizedPlayerPos.y > 0.0 && normalizedPlayerPos.y < 1.0 &&
           heightMapStatus != NULL && *heightMapStatus == COMPLETE)
        {
            float yIntComp, xIntComp;
            float yFrac = std::modf(normalizedPlayerPos.y * (*heightMapHeight-1), &yIntComp);
            float xFrac = std::modf(normalizedPlayerPos.x * (*heightMapWidth-1), &xIntComp);
            int xInt = static_cast<unsigned int>(xIntComp);
            int yInt = static_cast<unsigned int>(yIntComp);

            // e = array[(y*rowLength + x)*2] (2 skips over green channel
            float bl = heightMapArray[((yInt * *heightMapWidth) + xInt)*2] * *heightMapMaxHeight;
            float br = heightMapArray[((yInt * *heightMapWidth) + xInt+1)*2] * *heightMapMaxHeight;
            float tl = heightMapArray[(((yInt+1) * *heightMapWidth) + xInt)*2] * *heightMapMaxHeight;
            float tr = heightMapArray[(((yInt+1) * *heightMapWidth) + xInt+1)*2] * *heightMapMaxHeight;
            float topLerp = tl + (tr - tl) * xFrac;
            float botLerp = bl + (br - bl) * xFrac;
            Elevation = botLerp + (topLerp - botLerp) * yFrac;

        }else{
            Elevation = ELEVATION; //Set Elevation to Default
        }

        if(!fly && !std::isinf(Elevation)){
            Position.y = Elevation + 1.7;
        }
    }

    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw   += xoffset;
        Pitch += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        // update Front, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();
    }

    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
        Zoom -= (float)yoffset;
        if (Zoom < 40.0f)
            Zoom = 40.0f;
        if (Zoom > 110.0f)
            Zoom = 110.0f;
    }

private:
    unsigned int* heightMapHeight;
    unsigned int* heightMapWidth;
    const int* heightMapMaxHeight;
    const float* gameSize;
    float* heightMapArray;
    Transfer_Status* heightMapStatus = NULL;

    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        // calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        // also re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up    = glm::normalize(glm::cross(Right, Front));
    }
};
#endif // CAMERA_H_INCLUDED
