#ifndef BILINEARMAPREADER_H
#define BILINEARMAPREADER_H

#include "CustomEnumerators.h"
#include <glad/glad.h>
#include <GL/glfw3.h>
#include <vector>
#include <cmath>
#include <memory>


class bilinearMapReader
{
    public:
        bilinearMapReader();
        virtual ~bilinearMapReader();
        Transfer_Status getStatus() const;
        float normalizedRead(float x, float y, unsigned int channelOffset) const;
        float read(float x, float y, float gameSize , unsigned int channelOffset) const;
        void write(unsigned int width, unsigned int height, unsigned int textureID, GLint format, unsigned int channels);


    protected:
        Transfer_Status status;
        unsigned int width;
        unsigned int height;
        unsigned int channels;
        std::unique_ptr<float[]> MapCopyArray;
    private:
};

#endif // BILINEARMAPREADER_H
