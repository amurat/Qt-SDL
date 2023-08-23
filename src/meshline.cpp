#include "meshline.h"

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glad/glad_gles32.h"

#define LOCAL_SHADER

struct MeshLine::MeshLineImpl {
    
    MeshLineImpl() :
        program_(0), vao_(0), num_vertices_(0), inited_(false)
    {}
    
    ~MeshLineImpl() {
        if (inited_) {
            glDeleteBuffers(2, v_buffer_);
        }
    }

#ifdef LOCAL_SHADER
    static void printProgramLog(GLuint f_programId) {
      if (glIsProgram(f_programId)) {
        int logLen = 0;
        glGetProgramiv(f_programId, GL_INFO_LOG_LENGTH, &logLen);

        char* infoLog_a = new char[logLen];
        int infoLogLen = 0;
        glGetProgramInfoLog(f_programId, logLen, &infoLogLen, infoLog_a);

        std::cout << infoLog_a << std::endl;
        delete[] infoLog_a;
      }
    }

    static void printShaderLog(GLuint f_shaderId) {
      if (glIsShader(f_shaderId)) {
        int logLen = 0;
        glGetShaderiv(f_shaderId, GL_INFO_LOG_LENGTH, &logLen);

        char* infoLog_a = new char[logLen];
        int infoLogLen = 0;
        glGetShaderInfoLog(f_shaderId, logLen, &infoLogLen, infoLog_a);

        std::cout << infoLog_a << std::endl;
        delete[] infoLog_a;
      }
    }

    static GLuint loadShader(const GLchar* f_source_p, GLenum f_type) {
      GLuint shaderId = glCreateShader(f_type);
      glShaderSource(shaderId, 1, &f_source_p, nullptr);
      glCompileShader(shaderId);

      GLint compileStatus = GL_FALSE;
      glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileStatus);

      if (!compileStatus) {
        printShaderLog(shaderId);
        glDeleteShader(shaderId);
        shaderId = 0;
      }

      return shaderId;
    }

    static GLuint loadProgram(const GLchar* f_vertSource_p, const GLchar* f_fragSource_p) {
      GLuint vertShader = loadShader(f_vertSource_p, GL_VERTEX_SHADER);
      GLuint fragShader = loadShader(f_fragSource_p, GL_FRAGMENT_SHADER);

      if (!glIsShader(vertShader) || !glIsShader(fragShader)) {
        glDeleteShader(vertShader);
        glDeleteShader(fragShader);
        return 0;
      }

      GLuint programId = glCreateProgram();
      glAttachShader(programId, vertShader);
      glAttachShader(programId, fragShader);

      glLinkProgram(programId);
      GLint linkStatus = GL_FALSE;
      glGetProgramiv(programId, GL_LINK_STATUS, &linkStatus);

      if (!linkStatus) {
        printProgramLog(programId);
        glDeleteShader(vertShader);
        glDeleteShader(fragShader);
        glDeleteProgram(programId);
        return 0;
      }

      glDeleteShader(vertShader);
      glDeleteShader(fragShader);
      return programId;
    }
#endif
    
    void draw(int w, int h, float* mvp, float* color, float thickness)
    {
        // placeholder state xfer
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_DEPTH_TEST);
        //glEnable(GL_POLYGON_OFFSET_FILL);
        //glPolygonOffset(1, 1);
        //glPolygonOffset(-10,-10);

        // end state xfr

        GLint  loc_mvp  = glGetUniformLocation(program_, "u_mvp");
        GLint  loc_res  = glGetUniformLocation(program_, "u_resolution");
        GLint  loc_thi  = glGetUniformLocation(program_, "u_thickness");
        GLint  loc_color = glGetUniformLocation(program_, "u_color");

        glViewport(0, 0, w, h);
        glUseProgram(program_);
        glUniform1f(loc_thi, thickness);
        glUniform2f(loc_res, (float)w, (float)h);
        glUniformMatrix4fv(loc_mvp, 1, GL_FALSE, mvp);
        glUniform4fv(loc_color, 1, color);
        glBindVertexArray(vao_);
        glDrawArrays(GL_TRIANGLES, 0, num_vertices_/2);
        //glDrawArrays(GL_TRIANGLES, 0, num_vertices_);
        glBindVertexArray(0);
    }

    void initialize(GLuint program)
    {
        program_ = program;
        setupVertexArrays();
        inited_ = true;
    }

    void setupVertexArrays()
    {
        glGenVertexArrays(1, &vao_);
        glBindVertexArray(vao_);
        glGenBuffers(2, v_buffer_);
        glBindVertexArray(0);
    }

    void updateVertexArrays(std::vector<glm::vec4>& varray, bool linestrip)
    {
        glBindVertexArray(vao_);
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
        glBindVertexArray(0);
    }

    GLuint program_;
    GLuint vao_;
    GLuint v_buffer_[2];
    GLsizei num_vertices_;
    bool inited_;
};

    
MeshLine::MeshLine() : impl_(new MeshLineImpl())
{
}

MeshLine::~MeshLine()
{
}

void MeshLine::draw(std::vector<glm::vec4>& varray, int w, int h, float* mvp, float* color, float thickness)
{
    bool linestrip = false;
    impl_->updateVertexArrays(varray, linestrip);
    impl_->draw(w, h, mvp, color, thickness);
}

void MeshLine::initialize()
{
    if (impl_->inited_)
        return;
    GLuint program = MeshLineImpl::loadProgram(vertexShader().c_str(), fragmentShader().c_str());
    impl_->initialize(program);
}

void MeshLine::initialize(unsigned int program)
{
    if (impl_->inited_)
        return;
    impl_->initialize(program);
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
    // bump z to emulate polygon offset
    pos.z += -0.1;
    gl_Position = pos;
}
)";

static std::string fragShader = R"(#version 300 es
precision highp float;

uniform vec4 u_color;
out vec4 fragColor;

void main()
{
    fragColor = u_color;
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
