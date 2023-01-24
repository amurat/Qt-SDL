#pragma once

#include "glad/glad_gles32.h"
#include <glm/glm.hpp>

class Hemisphere {
public:
    Hemisphere();
    virtual ~Hemisphere();
    
    void initialize();
    void render(int w, int h);
    
private:
    void subdivideTriangle(float* vertices, int& index, float *v1, float *v2, float *v3, int depth);
    
    void makeGeodesicHemisphereVBO();
    
    void updateMVP(int w, int h);
    
    void renderHemi();
    void renderIco();
    void renderAxis();
    
    void updateHemiScale();
    
    GLuint hemisphereVerticesVBO;
    GLuint hemisphereVerticesVAO;

    GLuint hemiScaleVBO;

    GLuint hemiShader;

    int numTrianglesInHemisphere;
    int numVerticesInHemisphere;

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





