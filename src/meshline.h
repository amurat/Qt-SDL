#ifndef MESHLINE_H
#define MESHLINE_H

#include <glm/glm.hpp>

#include <vector>
#include <string>

class MeshLine {
public:
    MeshLine();

    virtual ~MeshLine();
    
    void initialize(unsigned int program, std::vector<glm::vec4>& varray, float thickness = 20.0, bool linestrip=false);

    void initialize(std::vector<glm::vec4>& varray, float thickness = 10.0, bool linestrip=false);

    void draw(int w, int h, float* mvp);
    
    static const std::string& vertexShader();
    static const std::string& fragmentShader();
    
private:
    struct MeshLineImpl;
    std::unique_ptr< MeshLineImpl > impl_;
};

#endif
