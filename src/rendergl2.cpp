#include "rendergl2.h"
#include <cassert>
#include <iostream>
#include "galogen/gl.h"

#ifndef _WIN32
// backtrace
#include <execinfo.h>
#endif

#include "glesloader.h"
#include "glescontext.h"


#define RENDER_TRIANGLE 1
#define RENDER_LINES 1
//#define RENDER_HEMISPHERE 1
//#define RENDER_ICOSAHEDRON 1
#ifdef RENDER_HEMISPHERE
#include "hemisphere.h"
Hemisphere hemisphere;
#elif defined(RENDER_ICOSAHEDRON)
#include "icosahedron.h"
Icosahedron icosahedron;
#elif defined(RENDER_LINES)
#include "linegen.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "meshline.h"
static MeshLine meshline;
static std::vector<glm::vec4> varray;
#endif


#ifdef RENDER_TRIANGLE
//static GLuint program;
static GLuint vao;
static GLuint vbo;
#endif

namespace {

void stackTrace()
{
#ifndef _WIN32
    void *array[10];
    size_t size = backtrace(array, 10);
    backtrace_symbols_fd(array, size, 2);
#endif
}

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

static GLuint program;



void SetupGL2Renderer(GLESContext* context)
{
    initializeGLES();
    
    std::cout << "GL version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GL extensions: " << glGetString(GL_EXTENSIONS) << std::endl;
    //std::string extensions((char*)glGetString(GL_EXTENSIONS));
    //std::replace(extensions.begin(), extensions.end(), ' ', ',');
    //std::cout << "GL extensions: " << extensions << std::endl;
    
    context->makeSecondaryCurrent();
#ifdef RENDER_HEMISPHERE
    hemisphere.initialize();
#endif
    
#ifdef RENDER_ICOSAHEDRON
    icosahedron.initialize();
#endif
    
#ifdef RENDER_LINES
    generateCircleLineStripTestData(varray);
    convertLineStripToLines(varray);
    meshline.initialize();
#endif

#ifdef RENDER_TRIANGLE
    context->makeCurrent();
    // Load shader program
    constexpr char kVS[] = R"(attribute vec4 vPosition;
  void main()
  {
      gl_Position = vPosition;
  })";

    constexpr char kFS[] = R"(
  void main()
  {
      gl_FragColor = vec4(gl_FragCoord.x / 512.0, gl_FragCoord.y / 512.0, 0.0, 1.0);
  })";

    program = loadProgram(kVS, kFS);
    
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    GLfloat vertices[] = {
        0.0f, 0.5f, 0.0f, -0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f,
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
#endif
}

void RenderGL2Renderer(GLESContext* context, int w, int h)
{
    context->makeCurrent();
    glClearColor(0.2F, 0.2F, 0.2F, 1.F);
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, w, h);

#ifdef RENDER_TRIANGLE
    // Render scene
    glUseProgram(program);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
#endif

    context->makeSecondaryCurrent();

#ifdef RENDER_HEMISPHERE
    hemisphere.render(w, h);
#endif
    
#ifdef RENDER_ICOSAHEDRON
    icosahedron.render(w, h);
#endif
    
#ifdef RENDER_LINES
    float aspect = (float)w/(float)h;
    glm::mat4 project = glm::ortho(-aspect, aspect, -1.0f, 1.0f, -10.0f, 10.0f);
    glm::mat4 modelview1( 1.0f );
    static float angle = 0.0;
    modelview1 = glm::rotate(modelview1, angle, glm::vec3(0.0f, 1.0f, 0.0f) );
    angle += 0.01;
    //modelview1 = glm::translate(modelview1, glm::vec3(-0.6f, 0.0f, 0.0f) );
    modelview1 = glm::scale(modelview1, glm::vec3(0.5f, 0.5f, 1.0f) );
    glm::mat4 mvp1 = project * modelview1;
    static float thickness = 1.0;
    float color[4] = {1.0, 0.0, 0.0, 1.0};
    meshline.draw(varray, w, h, glm::value_ptr(mvp1), color, thickness);
    thickness += 0.1;
    if (thickness > 30) {
        thickness = 1.0;
    }
#endif
    
    context->makeCurrent();
    if (glGetError()) {
        assert(false);
    }
}

void
RenderGL2::setup(GLESContext* context)
{
    SetupGL2Renderer(context);
}

void
RenderGL2::render(GLESContext* context, int w, int h)
{
    RenderGL2Renderer(context, w, h);
}
