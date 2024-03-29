#ifndef SHADER_S_H
#define SHADER_S_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
    unsigned int ID;
    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    Shader(const char* vertexPath, const char* fragmentPath, const char* tessCtrlPath = nullptr, const char* tessEvalPath = nullptr)
    {
        // 1. retrieve the vertex/fragment source code from filePath
        std::string vertexCode;
        std::string fragmentCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;

        std::string tessCtrlCode;
        std::string tessEvalCode;
        std::ifstream tessCtrlFile;
        std::ifstream tessEvalFile;
        // ensure ifstream objects can throw exceptions:
        vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        tessCtrlFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        tessEvalFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            // open files
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vShaderStream, fShaderStream;
            // read file's buffer contents into streams
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            // close file handlers
            vShaderFile.close();
            fShaderFile.close();
            // convert stream into string
            vertexCode   = vShaderStream.str();
            fragmentCode = fShaderStream.str();

            if(tessCtrlPath != nullptr && tessEvalPath != nullptr){ // if shader has Tessellation shaders
                // open files
                tessCtrlFile.open(tessCtrlPath);
                tessEvalFile.open(tessEvalPath);
                std::stringstream tessCtrlStream, tessEvalStream;
                // read file's buffer contents into streams
                tessCtrlStream << tessCtrlFile.rdbuf();
                tessEvalStream << tessEvalFile.rdbuf();
                // close file handlers
                tessCtrlFile.close();
                tessEvalFile.close();
                // convert stream into string
                tessCtrlCode   = tessCtrlStream.str();
                tessEvalCode = tessEvalStream.str();
            }
        }
        catch (std::ifstream::failure& e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
        }
        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();
        const char* tcShaderCode = nullptr;
        const char* teShaderCode = nullptr;
        if(tessCtrlPath != nullptr && tessEvalPath != nullptr){
            tcShaderCode = tessCtrlCode.c_str();
            teShaderCode = tessEvalCode.c_str();
        }

        // 2. compile shaders
        unsigned int vertex, fragment, tessControl, tessEvaluation;
        // vertex shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, nullptr);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");
        // fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, nullptr);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");
        // Tessellation Shader
        if(tessCtrlPath != nullptr && tessEvalPath != nullptr){
            tessControl = glCreateShader(GL_TESS_CONTROL_SHADER);
            glShaderSource(tessControl, 1, &tcShaderCode, nullptr);
            glCompileShader(tessControl);
            checkCompileErrors(tessControl, "TESSELLATION_CONTROL");

            tessEvaluation = glCreateShader(GL_TESS_EVALUATION_SHADER);
            glShaderSource(tessEvaluation, 1, &teShaderCode, nullptr);
            glCompileShader(tessEvaluation);
            checkCompileErrors(tessControl, "TESSELLATION_EVALUATION");

        }
        // shader Program
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        if(tessCtrlPath && tessEvalPath){
            glAttachShader(ID, tessControl);
            glAttachShader(ID, tessEvaluation);
        }
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");
        // delete the shaders as they're linked into our program now and no longer necessary
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        if(tessCtrlPath && tessEvalPath){
            glDeleteShader(tessControl);
            glDeleteShader(tessEvaluation);
        }
    }

    virtual ~Shader(){
        glDeleteProgram(ID);
    }
    // activate the shader
    // ------------------------------------------------------------------------
    void use() const
    {
        glUseProgram(ID);
    }
    // utility uniform functions
    // ------------------------------------------------------------------------
    void setBool(const std::string &name, const bool &value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }
    // ------------------------------------------------------------------------
    void setInt(const std::string &name, const int &value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
    //-------------------------------------------------------------------------
    void setUInt(const std::string &name, const unsigned int &value) const
    {
        glUniform1ui(glGetUniformLocation(ID, name.c_str()), value);
    }
    // ------------------------------------------------------------------------
    void setFloat(const std::string &name, const float &value) const
    {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }
    // ------------------------------------------------------------------------
    void setMat4(const std::string &name, const glm::mat4 &matrix) const
    {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(matrix));
    }
    //-------------------------------------------------------------------------
    void setVec2(const std::string &name, const glm::vec2 &floatVec) const
    {
        glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(floatVec));

    }
    //-------------------------------------------------------------------------
    void setVec3(const std::string &name, const glm::vec3 &floatVec) const
    {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(floatVec));

    }
    //-------------------------------------------------------------------------
    void setVec4(const std::string &name, const glm::vec4 &floatVec) const
    {
        glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(floatVec));

    }
    //-------------------------------------------------------------------------
    void setUVec2(const std::string &name, const glm::uvec2 &unsignedVec) const
    {
        glUniform2uiv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(unsignedVec));

    }

private:
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    void checkCompileErrors(unsigned int shader, std::string type)
    {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
};
#endif // SHADER_S_H
