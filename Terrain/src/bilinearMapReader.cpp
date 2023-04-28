#include "bilinearMapReader.h"

bilinearMapReader::bilinearMapReader()
{
    //ctor
    status = NOT_STARTED;
}

bilinearMapReader::~bilinearMapReader()
{
    //dtor
    delete[] MapCopyArray;
}

float bilinearMapReader::normalizedRead(float x, float y, unsigned int channelOffset)
{
    if(x > 0.0 && x < 1.0 &&y > 0.0 && y < 1.0 &&
        status == COMPLETE)
    {
        float yIntComp, xIntComp;
        float yFrac = std::modf(y * (height-1), &yIntComp);
        float xFrac = std::modf(x * (width-1), &xIntComp);
        int xInt = static_cast<unsigned int>(xIntComp);
        int yInt = static_cast<unsigned int>(yIntComp);

        // e = array[(y*rowLength + x)*channels +channelOffset] channelOffset R = 0, G = 1, etc
        float bl = MapCopyArray[((yInt * width) + xInt)*channels + channelOffset];
        float br = MapCopyArray[((yInt * width) + xInt+1)*channels + channelOffset];
        float tl = MapCopyArray[(((yInt+1) * width) + xInt)*channels + channelOffset];
        float tr = MapCopyArray[(((yInt+1) * width) + xInt+1)*channels + channelOffset];
        float topLerp = tl + (tr - tl) * xFrac;
        float botLerp = bl + (br - bl) * xFrac;
        return botLerp + (topLerp - botLerp) * yFrac;
    } else {
        return 0.0;
    }
}

float bilinearMapReader::read(float x, float y, float gameSize , unsigned int channelOffset)
{
    return this->normalizedRead((x / gameSize)+0.5, (y/ gameSize)+0.5, channelOffset);//(0,0) is Bottom left, (1,1) is top right
}


void bilinearMapReader::write(unsigned int width, unsigned int height, unsigned int textureID, GLint format, unsigned int channels)
{
    if(status == COMPLETE){
        delete[] MapCopyArray;
    }
    status = IN_PROGRESS;
    MapCopyArray = new float[height*width*2];
    this->height = height;
    this->width = width;
    this->channels = channels;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glGetTexImage(GL_TEXTURE_2D,0,format, GL_FLOAT, MapCopyArray);
    status = COMPLETE;
}

Transfer_Status bilinearMapReader::getStatus()
{
    return status;
}

