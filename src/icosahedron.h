#pragma once

#include "glad/glad_gles32.h"
#include <glm/glm.hpp>
#include <vector>

// adapted from https://github.com/libigl/eigen/blob/master/demos/opengl/icosphere.cpp
class IcoSphere
{
  public:
    IcoSphere(unsigned int levels=1);
    const std::vector<glm::vec3>& vertices() const { return mVertices; }
    const std::vector<int>& indices(int level) const;
  protected:
    void _subdivide();
    std::vector<glm::vec3> mVertices;
    std::vector<std::vector<int>*> mIndices;
    std::vector<int> mListIds;
};


class Icosahedron {
public:
    Icosahedron();
    virtual ~Icosahedron();
    
    void initialize();
    void render(int w, int h);
    
private:
    void makeIcoVBO();
    
    void updateMVP(int w, int h);
    
    void renderIco();
    
    GLuint icoVerticesVBO;
    GLuint icoVAO;
    GLuint icoIndicesVBO;

    GLuint hemiShader;

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
    
    IcoSphere icosphere_;
};





