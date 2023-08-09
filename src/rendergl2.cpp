#include "rendergl2.h"
#include <cassert>
#include <iostream>
#include "galogen/gl.h"
#include <execinfo.h>

//#define RENDER_LINES 1
//#define RENDER_HEMISPHERE 1
//#define RENDER_ICOSAHEDRON 1
#ifdef RENDER_HEMISPHERE
#include "hemisphere.h"
Hemisphere hemisphere;
#elif defined(RENDER_ICOSAHEDRON)
#include "icosahedron.h"
Icosahedron icosahedron;
#elif defined(RENDER_LINES)
#include "meshline.h"
MeshLine meshline;
#endif


namespace {

void stackTrace()
{
    void *array[10];
    size_t size = backtrace(array, 10);
    backtrace_symbols_fd(array, size, 2);
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


#ifdef RENDER_LINES
void generateLineStripTestData(std::vector<glm::vec4>& varray)
{
    varray.clear();
    varray.emplace_back(glm::vec4(1.0f, -1.0f, 0.0f, 1.0f));
    for (int u=0; u <= 90; u += 10)
    {
        double a = u*M_PI/180.0;
        double c = cos(a), s = sin(a);
        varray.emplace_back(glm::vec4((float)c, (float)s, 0.0f, 1.0f));
    }
    varray.emplace_back(glm::vec4(-1.0f, 1.0f, 0.0f, 1.0f));
    for (int u = 90; u >= 0; u -= 10)
    {
        double a = u * M_PI / 180.0;
        double c = cos(a), s = sin(a);
        varray.emplace_back(glm::vec4((float)c-1.0f, (float)s-1.0f, 0.0f, 1.0f));
    }
    varray.emplace_back(glm::vec4(1.0f, -1.0f, 0.0f, 1.0f));
}

void convertLineStripToLines(std::vector<glm::vec4>& varray)
{
    std::vector<glm::vec4> result;
    const size_t num_lines = varray.size()-1;
    for (auto i = 0; i < num_lines; i++) {
        result.push_back(varray[i]);
        result.push_back(varray[i+1]);
    }
    varray = result;
}
#endif

void SetupGL2Renderer()
{
    std::cout << "GL version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GL extensions: " << glGetString(GL_EXTENSIONS) << std::endl;

#ifdef RENDER_HEMISPHERE
    hemisphere.initialize();

#elif defined(RENDER_ICOSAHEDRON)
    icosahedron.initialize();
#elif defined(RENDER_LINES)
    program = loadProgram(MeshLine::vertexShader().c_str(), MeshLine::fragmentShader().c_str());
    std::vector<glm::vec4> varray;
    generateLineStripTestData(varray);
    convertLineStripToLines(varray);
    meshline.initialize(program, varray);
#else
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
#endif
}

void RenderGL2Renderer(int w, int h)
{
#ifdef RENDER_HEMISPHERE
    hemisphere.render(w, h);
#elif defined(RENDER_ICOSAHEDRON)
    icosahedron.render(w, h);
#elif defined(RENDER_LINES)
    glClearColor(0.2F, 0.2F, 0.2F, 1.F);
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, w, h);

    float aspect = (float)w/(float)h;
    glm::mat4 project = glm::ortho(-aspect, aspect, -1.0f, 1.0f, -10.0f, 10.0f);
    glm::mat4 modelview1( 1.0f );
    static float angle = 0.0;
    modelview1 = glm::rotate(modelview1, angle, glm::vec3(0.0f, 1.0f, 0.0f) );
    angle += 0.01;
    //modelview1 = glm::translate(modelview1, glm::vec3(-0.6f, 0.0f, 0.0f) );
    modelview1 = glm::scale(modelview1, glm::vec3(0.5f, 0.5f, 1.0f) );
    glm::mat4 mvp1 = project * modelview1;
    
    meshline.draw(w, h, glm::value_ptr(mvp1));
#else
    // Clear
    //auto level = (double)rand()/(double)RAND_MAX; 
    //glClearColor(level, level, level, 1.F);
    //glClearColor(0.184F, 0.46F, 0.77F, 1.0F);
    glClearColor(0.2F, 0.2F, 0.2F, 0.F);
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, w, h);

#if 0
    GLuint indexVBO;
    glGenBuffers(1, &indexVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBO);
    GLuint indices[] = {0, 1, 2};
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*3, indices, GL_STATIC_DRAW);
#endif
    
    // Render scene
    GLfloat vertices[] = {
        0.0f, 0.5f, 0.0f, -0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f,
    };
    glUseProgram(program);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(0);
#if 1
      glDrawArrays(GL_TRIANGLES, 0, 3);
#else
      glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
#endif
#if 0
    glDeleteBuffers(1, &indexVBO);
#endif
    if (glGetError()) {
        assert(false);
    }
#endif
}

void
RenderGL2::setup()
{
    SetupGL2Renderer();
}

void
RenderGL2::render(int w, int h)
{
    RenderGL2Renderer(w, h);
}
