
#include "rendergl.h"
#include <cassert>
#include <iostream>
#include "glad/glad_gles32.h"


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

static GLuint program;

void SetupGLES2Renderer()
{
    int gles_version = gladLoaderLoadGLES2();
    if (!gles_version) {
        std::cout << "Unable to load GLES." << std::endl;
        return;
    }
    
    std::cout << "GL version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GL extensions: " << glGetString(GL_EXTENSIONS) << std::endl;
    
    // Load shader program
    constexpr char kVS[] = R"(attribute vec4 vPosition;
  void main()
  {
      gl_Position = vPosition;
  })";

    constexpr char kFS[] = R"(precision mediump float;
  void main()
  {
      gl_FragColor = vec4(gl_FragCoord.x / 512.0, gl_FragCoord.y / 512.0, 0.0, 1.0);
  })";

    program = loadProgram(kVS, kFS);
}

void RenderGLES2Renderer(int w, int h)
{
      // Clear
      glClearColor(0.2F, 0.2F, 0.2F, 1.F);
      glClear(GL_COLOR_BUFFER_BIT);
      glViewport(0, 0, w, h);

      // Render scene
      GLfloat vertices[] = {
          0.0f, 0.5f, 0.0f, -0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f,
      };
      glUseProgram(program);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices);
      glEnableVertexAttribArray(0);
#if 0
      glDrawArrays(GL_TRIANGLES, 0, 3);
#else
      GLuint indices[] = {0, 1, 2};
      glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, indices);
#endif
      if (glGetError()) {
          assert(false);
      }
}

void
RenderGLES2::setup()
{
    SetupGLES2Renderer();
}

void
RenderGLES2::render(int w, int h)
{
    RenderGLES2Renderer(w, h);
}
