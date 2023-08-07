#ifndef MESHLINE_H
#define MESHLINE_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <string>
#ifdef USE_GLEW
#include <GL/glew.h>
#else
#include "glad/glad_gles32.h"
//#include "galogen/gl.h"
#endif

class MeshLine {
public:
    MeshLine();

    virtual ~MeshLine();
    
    void initialize(GLuint program, std::vector<glm::vec4>& varray, bool linestrip=false);
    void draw(int w, int h, float* mvp);
    
    static const std::string& vertexShader();
    static const std::string& fragmentShader();
    
private:
    void buildVertexArrays(std::vector<glm::vec4>& varray, bool linestrip);
    
    GLuint program_;
    GLuint v_buffer_[2];
    GLsizei num_vertices_;
    bool inited_;
};

#endif
