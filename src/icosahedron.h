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
    void subdivideTriangle(float* vertices, int& index, float *v1, float *v2, float *v3, int depth);
    
    void makeGeodesicHemisphereVBO();
    void makeIcoVBO();
    
    void updateMVP(int w, int h);
    
    void renderHemi();
    void renderIco();
    void renderAxis();
    
    void updateIcoScale();
    
    GLuint hemisphereVerticesVBO;
    GLuint hemisphereVerticesVAO;

    GLuint icoVerticesVBO;
    GLuint icoVAO;
    GLuint icoIndicesVBO;
    GLuint icoScaleVBO;

    GLuint hemiShader;
    GLuint axisShader;

    int numTrianglesInHemisphere;
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





