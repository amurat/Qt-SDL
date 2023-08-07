#include "meshline.h"

#include <glm/glm.hpp>
#include "glad/glad_gles32.h"

struct MeshLine::MeshLineImpl {
    
    MeshLineImpl() :
        program_(0), num_vertices_(0), inited_(false)
    {}
    
    ~MeshLineImpl() {
        if (inited_) {
            glDeleteBuffers(2, v_buffer_);
        }
    }
    
    void draw(int w, int h, float* mvp)
    {
        GLint  loc_mvp  = glGetUniformLocation(program_, "u_mvp");
        GLint  loc_res  = glGetUniformLocation(program_, "u_resolution");
        GLint  loc_thi  = glGetUniformLocation(program_, "u_thickness");
        
        glUseProgram(program_);
        glUniform1f(loc_thi, thickness_);
        glUniform2f(loc_res, (float)w, (float)h);
        glUniformMatrix4fv(loc_mvp, 1, GL_FALSE, mvp);
        
        glDrawArrays(GL_TRIANGLES, 0, num_vertices_);
        
    }

    void initialize(GLuint program, std::vector<glm::vec4>& varray, float thickness, bool linestrip)
    {
        program_ = program;
        thickness_ = thickness;
        glGenBuffers(2, v_buffer_);
        buildVertexArrays(varray, linestrip);
        inited_ = true;
    }

    void buildVertexArrays(std::vector<glm::vec4>& varray, bool linestrip)
    {
        GLuint vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        
        const GLsizei N = (GLsizei)varray.size();
        const GLsizei num_vertices = 6*(N-1);
        std::vector<glm::vec4> v[2];
        for (int i = 0; i < num_vertices; i++)
        {
            int line_id = i / 6;
            if (!linestrip) {
                line_id *= 2;
            }
            // start point
            v[0].emplace_back(varray[line_id+0]);
            // end point
            v[1].emplace_back(varray[line_id+1]);
        }
        
        glGenBuffers(2, v_buffer_);
        
        GLint loc_v[2];
        loc_v[0] = glGetAttribLocation(program_, "v_0");
        loc_v[1] = glGetAttribLocation(program_, "v_1");
        
        for (int i = 0; i < 2; i++) {
            glBindBuffer(GL_ARRAY_BUFFER, v_buffer_[i]);
            glBufferData(GL_ARRAY_BUFFER, v[i].size() * sizeof(glm::vec4), &(v[i][0]), GL_STATIC_DRAW);
            glEnableVertexAttribArray(loc_v[i]);
            glVertexAttribPointer(loc_v[i] , 4, GL_FLOAT, 0, 0, 0);
        }
        
        num_vertices_ = num_vertices;
    }

    GLuint program_;
    GLuint v_buffer_[2];
    GLsizei num_vertices_;
    float thickness_;
    bool inited_;
};

    
MeshLine::MeshLine() : impl_(new MeshLineImpl())
{
}

MeshLine::~MeshLine()
{
}


void MeshLine::draw(int w, int h, float* mvp)
{
    impl_->draw(w, h, mvp);
}

void MeshLine::initialize(unsigned int program, std::vector<glm::vec4>& varray, float thickness, bool linestrip)
{
    impl_->initialize(program, varray, thickness, linestrip);
}

static std::string vertShader = R"(#version 300 es
precision highp float;

uniform mat4  u_mvp;
uniform vec2  u_resolution;
uniform float u_thickness;

in vec4 v_0;
in vec4 v_1;

void main()
{
    int line_i = gl_VertexID / 6;
    int tri_i  = gl_VertexID % 6;

    vec4 va[2];
    va[0] = u_mvp * v_0;
    va[1] = u_mvp * v_1;
    for (int i=0; i<2; ++i)
    {
        va[i].xyz /= va[i].w;
        va[i].xy = (va[i].xy + 1.0) * 0.5 * u_resolution;
    }


    vec4 basis = va[1] - va[0];
    vec2 perpendicular = normalize(vec2(-basis.y, basis.x));

    vec4 pos;
    if (tri_i == 0 || tri_i == 1 || tri_i == 3)
    {
        pos = va[0];
        pos.xy += u_thickness * perpendicular * (tri_i == 1 ? -0.5 : 0.5);
    }
    else
    {
        pos = va[1];
        pos.xy += u_thickness * perpendicular * (tri_i == 5 ? 0.5 : -0.5);
    }
    pos.xy = pos.xy / u_resolution * 2.0 - 1.0;
    pos.xyz *= pos.w;
    gl_Position = pos;
}
)";

static std::string fragShader = R"(#version 300 es
precision highp float;

out vec4 fragColor;

void main()
{
    fragColor = vec4(1.0);
}
)";

const std::string& MeshLine::vertexShader()
{
    return vertShader;
    
}

const std::string& MeshLine::fragmentShader()
{
    return fragShader;
}
