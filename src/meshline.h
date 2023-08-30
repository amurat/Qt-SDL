#ifndef MESHLINE_H
#define MESHLINE_H

#include <glm/glm.hpp>

#include <vector>
#include <string>
#include <memory>

/*
 
 Each segment of the line consist of a quad represented by 2 triangle primitives respectively 6 vertices.
 
0        2   5
 +-------+  +
 |     /  / |
 |   /  /   |
 | /  /     |
 +  +-------+
1   3        4

 The coordinates have to be transformed from model space to window space. For that the resolution of the viewport has to be known.
 With the normal vector to the line segment and the line thickness (u_thickness), the vertex coordinate can be computed.
 */

class MeshLine {
public:
    MeshLine();
    
    virtual ~MeshLine();
    
    void initialize();
 
    void draw(std::vector<glm::vec4>& varray, int w, int h, float* mvp, float* color, float thickness=20.0);
    
    static const std::string& vertexShader();
    static const std::string& fragmentShader();
    
private:
    struct MeshLineImpl;
    std::unique_ptr< MeshLineImpl > impl_;

    void initialize(unsigned int program);

};

#endif
