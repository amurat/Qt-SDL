#pragma once

#include "glad/glad_gles32.h"
#include <glm/glm.hpp>
#include <vector>

class Hemisphere {
public:
    Hemisphere();
    virtual ~Hemisphere();
    
    void initialize();
    void render(int w, int h);
    
private:
    void subdivideTriangle(float* vertices, int& index, float *v1, float *v2, float *v3, int depth);
    
    void makeGeodesicHemisphereVBO();
    void setupShadersES3();
    void setupShadersGL2();

    void updateMVP(int w, int h);
    
    void renderHemi();
    void renderIco();
    void renderAxis();
    void renderMarker(glm::vec4 color, glm::vec3 position);
    
    void updateHemiScale();
    //void updateMarkers();
    
    GLuint hemisphereVerticesVBO;
    GLuint hemisphereVerticesVAO;

    GLuint hemiScaleVBO;
    GLuint hemiMarkerVBO;
    GLuint hemiIndicesVBO;
    
    GLuint hemiShader;
    GLuint axisShader;
    GLuint markerShader;

    int numTrianglesInHemisphere;
    int numVerticesInHemisphere;

    int numIndexedTrianglesInHemisphere;
    int numIndexedVerticesInHemisphere;


    float lookPhi;
    float lookTheta;
    float lookZoom;

    float NEAR_PLANE;
    float FAR_PLANE;
    float FOV_Y;
    
    glm::mat4 projectionMatrix;
    glm::mat4 modelViewMatrix;
    int frame_;
    
    std::vector<glm::vec3> vertices;

    glm::vec3 markerMin;
    glm::vec3 markerSeed;
    glm::vec3 markerFitSeed;
    glm::vec3 markerOrient;
    
    bool inited_;
};





