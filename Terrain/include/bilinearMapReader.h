#ifndef BILINEARMAPREADER_H
#define BILINEARMAPREADER_H

#include "CustomEnumerators.h"
#include <glad/glad.h>
#include <GL/glfw3.h>
#include <vector>
#include <cmath>


class bilinearMapReader
{
    public:
        bilinearMapReader();
        virtual ~bilinearMapReader();
        Transfer_Status getStatus();
        float normalizedRead(float x, float y, unsigned int channelOffset);
        float read(float x, float y, float gameSize , unsigned int channelOffset);
        void write(unsigned int width, unsigned int height, unsigned int textureID, GLint format, unsigned int channels);


    protected:
        Transfer_Status status;
        unsigned int width;
        unsigned int height;
        unsigned int channels;
        float* MapCopyArray;
    private:
};

#endif // BILINEARMAPREADER_H
