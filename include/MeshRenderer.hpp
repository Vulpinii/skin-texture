#ifndef MESHRENDERER_HPP
#define MESHRENDERER_HPP

#include "Camera.hpp"
// Include standard headers
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <fstream>
#include <cfloat>
#include <set>
#include <algorithm>

// Include Glad
#include <glad/glad.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtx/transform.hpp>

#include "Shader.hpp"
#include "Mesh.hpp"
#include "LightSource.hpp"
extern unsigned int SCR_WIDTH;
extern unsigned int SCR_HEIGHT;

class MeshRenderer {
public:
    // constructor
    MeshRenderer() = default;
    MeshRenderer(unsigned int shaderID, unsigned int depthShaderID, Mesh& mesh);
    
    // destructor
    ~MeshRenderer();

    // draw mesh
    void draw(unsigned int ShaderID, Camera & camera, LightSource & lightPosition) ;

    // get depth map from light
    void createDepthMapFromLight(LightSource & light) const;

    // get depth map from eye
    void createDepthMapFromEye(Camera &eye);

    // update mesh's vertices
    void updateBuffers();

    // set model to shader
    void setModelRotation(glm::vec3 rotation) {
        model = glm::rotate(model, (float) rotation.x, glm::vec3(1.0,0.0,0.0));
        model = glm::rotate(model, (float) rotation.y, glm::vec3(0.0,1.0,0.0));
        model = glm::rotate(model, (float) rotation.z, glm::vec3(0.0,0.0,1.0));
    }

    void setModelTranslation(glm::vec3 translate) {
        model = glm::translate(model, translate);
    }

    void setModelNewTranslation(glm::vec3 translate) {
        model = glm::translate(glm::mat4(1.0f), translate);
    }

    void setModelScale(glm::vec3 scale) {
        model = glm::scale(model, scale);
    }

    void setModelColor(glm::vec3 c) {
        color = c;
    }

    void setModelColor(unsigned int shaderID, glm::vec3 c){
        color = c;
        glUseProgram(shaderID);
        glUniform3f(glGetUniformLocation(shaderID, "color"), color.x, color.y, color.z);
        glUseProgram(0);
    }


    // clean all vao / vbo / shader
    void cleanUp();


private:

    GLuint VertexArrayID;
    GLuint programID, depthProgramID;
    
    GLuint MatrixID, ViewMatrixID, ModelMatrixID;
    GLuint LightID;
    
    GLuint vertexbuffer;
    GLuint uvbuffer;
    GLuint normalbuffer;
    GLuint elementbuffer;

    glm::mat4 model;
    glm::vec3 color;

    // mesh to be rendered
    Mesh tridimodel;
};
#endif
