#pragma once

#include "glad/glad_gles32.h"
#include <glm/glm.hpp>

class Icosahedron {
public:
    Icosahedron();
    virtual ~Icosahedron();
    
    void initialize();
    void render(int w, int h);
    
private:
    void makeIcoVBO();
    
    void updateMVP(int w, int h);
    
    void renderHemi();
    void renderIco();
    void renderAxis();
    
    void updateIcoScale();
    
    GLuint icoVerticesVBO;
    GLuint icoVAO;
    GLuint icoIndicesVBO;
    GLuint icoScaleVBO;

    GLuint hemiShader;
    GLuint axisShader;

    int numTrianglesInIco;
    int numVerticesInIco;

    float lookPhi;
    float lookTheta;
    float lookZoom;

    float NEAR_PLANE;
    float FAR_PLANE;
    float FOV_Y;
    
    glm::mat4 projectionMatrix;
    glm::mat4 modelViewMatrix;
    int frame_;
};





