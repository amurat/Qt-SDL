#include "rendergles2.h"
#include <cassert>
#include <iostream>
#include <vector>
#include "glad/glad_gles32.h"
#include "glesloader.h"
#include "glescontext.h"
#include "hemisphere.h"
#include "icosahedron.h"
#include "linegen.h"
#include "meshline.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// default scene for RenderGLES2()
#define RENDER_LINES 1
//#define RENDER_HEMISPHERE 1
//#define RENDER_ICOSAHEDRON 1
//#define RENDER_TRIANGLE 1

struct LinesScene {
    MeshLine meshline;
    std::vector<glm::vec4> varray;
};

namespace {
void printProgramLog(GLuint f_programId) {
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

void printShaderLog(GLuint f_shaderId) {
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

GLuint loadShader(const GLchar* f_source_p, GLenum f_type) {
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

GLuint loadProgram(const GLchar* f_vertSource_p, const GLchar* f_fragSource_p) {
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
}  // namespace

Scene RenderGLES2::defaultScene()
{
#if defined(RENDER_HEMISPHERE)
    return Scene::Hemisphere;
#elif defined(RENDER_ICOSAHEDRON)
    return Scene::Icosahedron;
#elif defined(RENDER_TRIANGLE)
    return Scene::Triangle;
#else
    return Scene::Lines;
#endif
}

RenderGLES2::RenderGLES2(Scene scene) :
    scene_(scene),
    frame_(0),
    triangleProgram_(0),
    triangleVao_(0),
    triangleVbo_(0)
{
}

RenderGLES2::~RenderGLES2()
{
}

void RenderGLES2::setFrame(int frame)
{
    frame_ = frame;
    if (icosahedron_) {
        icosahedron_->setFrame(frame);
    }
}

void RenderGLES2::setupTriangle(GLESContext* context)
{
    context->makeCurrent();
    // Load shader program
    constexpr char kVS[] = R"(#version 300 es
  layout (location = 0) in vec3 vPos;
  void main()
  {
      gl_Position = vec4(vPos.x, vPos.y, vPos.z, 1.0);
  })";

    constexpr char kFS[] = R"(#version 300 es
  precision mediump float;
  out vec4 FragColor;
  void main()
  {
      FragColor = vec4(gl_FragCoord.x / 512.0, gl_FragCoord.y / 512.0, 0.0, 1.0);
  })";
    triangleProgram_ = loadProgram(kVS, kFS);

    glGenVertexArrays(1, &triangleVao_);
    glBindVertexArray(triangleVao_);
    glGenBuffers(1, &triangleVbo_);
    glBindBuffer(GL_ARRAY_BUFFER, triangleVbo_);
    GLfloat vertices[] = {
        0.0f, 0.5f, 0.0f, -0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f,
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void
RenderGLES2::setup(GLESContext* context)
{
    initializeGLES();
    std::cout << "GL version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GL extensions: " << glGetString(GL_EXTENSIONS) << std::endl;

    switch (scene_) {
    case Scene::Triangle:
        setupTriangle(context);
        break;
    case Scene::Hemisphere:
        hemisphere_.reset(new Hemisphere());
        hemisphere_->initialize();
        break;
    case Scene::Icosahedron:
        icosahedron_.reset(new Icosahedron());
        icosahedron_->initialize();
        icosahedron_->setFrame(frame_);
        break;
    case Scene::Lines:
        lines_.reset(new LinesScene());
//        generateCircleLineStripTestData(lines_->varray);
        generateLineStripTestData(lines_->varray);
        convertLineStripToLines(lines_->varray);
        lines_->meshline.initialize();
        break;
    }
}

void
RenderGLES2::render(GLESContext* context, int w, int h)
{
    context->makeCurrent();
    // Clear
    glClearColor(0.2F, 0.2F, 0.2F, 1.F);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, w, h);

    switch (scene_) {
    case Scene::Triangle:
        // Render scene
        glUseProgram(triangleProgram_);
        glBindVertexArray(triangleVao_);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);
        break;
    case Scene::Hemisphere:
        hemisphere_->render(w, h);
        break;
    case Scene::Icosahedron:
        icosahedron_->render(w, h);
        break;
    case Scene::Lines: {
        float aspect = (float)w/(float)h;
        glm::mat4 project = glm::ortho(-aspect, aspect, -1.0f, 1.0f, -10.0f, 10.0f);
        glm::mat4 modelview1( 1.0f );
        float angle = 0.01f * frame_;
        modelview1 = glm::rotate(modelview1, angle, glm::vec3(0.0f, 1.0f, 0.0f) );
        //modelview1 = glm::translate(modelview1, glm::vec3(-0.6f, 0.0f, 0.0f) );
        modelview1 = glm::scale(modelview1, glm::vec3(0.5f, 0.5f, 1.0f) );
        glm::mat4 mvp1 = project * modelview1;
        // grows by 0.1 per frame from 1 to 30, then starts over
        float thickness = 1.0f + 0.1f * (frame_ % 291);
        float color[4] = {1.0, 0.0, 0.0, 1.0};
        lines_->meshline.draw(lines_->varray, w, h, glm::value_ptr(mvp1), color, thickness);
        break;
    }
    }
    ++frame_;
}
