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
#include "srgb.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// default scene for RenderGLES2()
#define RENDER_LINES 1
//#define RENDER_HEMISPHERE 1
//#define RENDER_ICOSAHEDRON 1
//#define RENDER_TRIANGLE 1
//#define RENDER_COLORCHECKER 1

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
#elif defined(RENDER_COLORCHECKER)
    return Scene::ColorChecker;
#else
    return Scene::Lines;
#endif
}

RenderGLES2::RenderGLES2(Scene scene) :
    scene_(scene),
    frame_(0),
    triangleProgram_(0),
    triangleVao_(0),
    triangleVbo_(0),
    colorCheckerProgram_(0),
    colorCheckerVao_(0),
    colorCheckerResolutionLoc_(-1),
    linearFbo_(0),
    linearColor_(0),
    linearDepth_(0),
    linearWidth_(0),
    linearHeight_(0),
    linearFailed_(false),
    srgbProgram_(0),
    srgbVao_(0)
{
}

RenderGLES2::~RenderGLES2()
{
    if (colorCheckerProgram_) {
        glDeleteProgram(colorCheckerProgram_);
    }
    if (colorCheckerVao_) {
        glDeleteVertexArrays(1, &colorCheckerVao_);
    }
    releaseLinearTarget();
    if (srgbProgram_) {
        glDeleteProgram(srgbProgram_);
    }
    if (srgbVao_) {
        glDeleteVertexArrays(1, &srgbVao_);
    }
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
  precision highp float;
  )" SRGB_TO_LINEAR_GLSL R"(
  out vec4 FragColor;
  void main()
  {
      FragColor = vec4(srgbToLinear(vec3(gl_FragCoord.x / 512.0, gl_FragCoord.y / 512.0, 0.0)), 1.0);
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

// The 24-patch Macbeth ColorChecker, drawn entirely by the fragment shader over a fullscreen triangle
void RenderGLES2::setupColorChecker(GLESContext* context)
{
    context->makeCurrent();
    constexpr char kVS[] = R"(#version 300 es
  void main()
  {
      // (-1,-1), (3,-1), (-1,3) covers the viewport
      vec2 pos = vec2(float((gl_VertexID & 1) << 2) - 1.0, float((gl_VertexID & 2) << 1) - 1.0);
      gl_Position = vec4(pos, 0.0, 1.0);
  })";

    constexpr char kFS[] = R"(#version 300 es
  precision highp float;
  precision highp int;
  )" SRGB_TO_LINEAR_GLSL R"(
  uniform ivec2 uResolution;
  out vec4 FragColor;

  // sRGB 8-bit values, row-major from the top left
  const vec3 kPatches[24] = vec3[24](
      vec3(115.0,  82.0,  68.0),  // dark skin
      vec3(194.0, 150.0, 130.0),  // light skin
      vec3( 98.0, 122.0, 157.0),  // blue sky
      vec3( 87.0, 108.0,  67.0),  // foliage
      vec3(133.0, 128.0, 177.0),  // blue flower
      vec3(103.0, 189.0, 170.0),  // bluish green
      vec3(214.0, 126.0,  44.0),  // orange
      vec3( 80.0,  91.0, 166.0),  // purplish blue
      vec3(193.0,  90.0,  99.0),  // moderate red
      vec3( 94.0,  60.0, 108.0),  // purple
      vec3(157.0, 188.0,  64.0),  // yellow green
      vec3(224.0, 163.0,  46.0),  // orange yellow
      vec3( 56.0,  61.0, 150.0),  // blue
      vec3( 70.0, 148.0,  73.0),  // green
      vec3(175.0,  54.0,  60.0),  // red
      vec3(231.0, 199.0,  31.0),  // yellow
      vec3(187.0,  86.0, 149.0),  // magenta
      vec3(  8.0, 133.0, 161.0),  // cyan
      vec3(243.0, 243.0, 242.0),  // white
      vec3(200.0, 200.0, 200.0),  // neutral 8
      vec3(160.0, 160.0, 160.0),  // neutral 6.5
      vec3(122.0, 122.0, 121.0),  // neutral 5
      vec3( 85.0,  85.0,  85.0),  // neutral 3.5
      vec3( 52.0,  52.0,  52.0)   // black
  );
  const vec3 kFrame = vec3(20.0);

  void main()
  {
      // patches of 10 units with 2-unit gaps and border: 74 x 50 units, 90% of the fitting size
      float unit = 0.9 * min(float(uResolution.x) / 74.0, float(uResolution.y) / 50.0);
      int patchSize = int(floor(10.0 * unit));
      int gap = max(1, int(floor(2.0 * unit)));
      ivec2 chart = ivec2(6 * patchSize + 7 * gap, 4 * patchSize + 5 * gap);
      ivec2 origin = (uResolution - chart) / 2;

      // whole pixels, row 0 at the top
      ivec2 pixel = ivec2(gl_FragCoord.xy);
      pixel.y = uResolution.y - 1 - pixel.y;
      ivec2 local = pixel - origin;
      if (any(lessThan(local, ivec2(0))) || any(greaterThanEqual(local, chart))) {
          discard;
      }

      int cellSize = patchSize + gap;
      ivec2 cell = local / cellSize;
      ivec2 offset = local - cell * cellSize;
      vec3 color = kFrame;
      if (cell.x < 6 && cell.y < 4 && all(greaterThanEqual(offset, ivec2(gap)))) {
          color = kPatches[cell.y * 6 + cell.x];
      }
      FragColor = vec4(srgbToLinear(color / 255.0), 1.0);
  })";
    colorCheckerProgram_ = loadProgram(kVS, kFS);
    colorCheckerResolutionLoc_ = glGetUniformLocation(colorCheckerProgram_, "uResolution");

    // GLES 3 draws need a bound vertex array, even without attributes
    glGenVertexArrays(1, &colorCheckerVao_);
}

// Final pass: encodes the linear scene buffer to sRGB into the context's target
void RenderGLES2::setupSrgbPass()
{
    constexpr char kVS[] = R"(#version 300 es
  void main()
  {
      // (-1,-1), (3,-1), (-1,3) covers the viewport
      vec2 pos = vec2(float((gl_VertexID & 1) << 2) - 1.0, float((gl_VertexID & 2) << 1) - 1.0);
      gl_Position = vec4(pos, 0.0, 1.0);
  })";

    constexpr char kFS[] = R"(#version 300 es
  precision highp float;
  uniform highp sampler2D uLinear;
  out vec4 FragColor;
  void main()
  {
      vec4 linear = clamp(texelFetch(uLinear, ivec2(gl_FragCoord.xy), 0), 0.0, 1.0);
      vec3 c = linear.rgb;
      vec3 srgb = mix(1.055 * pow(c, vec3(1.0 / 2.4)) - 0.055, 12.92 * c, vec3(lessThanEqual(c, vec3(0.0031308))));
      FragColor = vec4(srgb, linear.a);
  })";
    srgbProgram_ = loadProgram(kVS, kFS);
    glGenVertexArrays(1, &srgbVao_);
}

bool RenderGLES2::ensureLinearTarget(int w, int h)
{
    if (linearFailed_) {
        return false;
    }
    if (linearFbo_ && w == linearWidth_ && h == linearHeight_) {
        return true;
    }
    releaseLinearTarget();

    glGenTextures(1, &linearColor_);
    glBindTexture(GL_TEXTURE_2D, linearColor_);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, w, h);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenRenderbuffers(1, &linearDepth_);
    glBindRenderbuffer(GL_RENDERBUFFER, linearDepth_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glGenFramebuffers(1, &linearFbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, linearFbo_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, linearColor_, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, linearDepth_);

    // RGBA16F needs EXT_color_buffer_half_float or EXT_color_buffer_float
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "linear RGBA16F FBO incomplete: 0x" << std::hex << status << std::dec
                  << ", rendering without the sRGB pass" << std::endl;
        releaseLinearTarget();
        linearFailed_ = true;
        return false;
    }
    linearWidth_ = w;
    linearHeight_ = h;
    return true;
}

void RenderGLES2::releaseLinearTarget()
{
    if (linearFbo_) {
        glDeleteFramebuffers(1, &linearFbo_);
    }
    if (linearColor_) {
        glDeleteTextures(1, &linearColor_);
    }
    if (linearDepth_) {
        glDeleteRenderbuffers(1, &linearDepth_);
    }
    linearFbo_ = 0;
    linearColor_ = 0;
    linearDepth_ = 0;
    linearWidth_ = 0;
    linearHeight_ = 0;
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
    case Scene::ColorChecker:
        setupColorChecker(context);
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

    context->makeCurrent();
    setupSrgbPass();
}

void
RenderGLES2::render(GLESContext* context, int w, int h)
{
    context->makeCurrent();
    // the context's FBO, or 0 for a window surface
    GLint output = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &output);

    if (!ensureLinearTarget(w, h)) {
        drawScene(w, h);
        ++frame_;
        return;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, linearFbo_);
    drawScene(w, h);

    glBindFramebuffer(GL_FRAMEBUFFER, output);
    glViewport(0, 0, w, h);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glUseProgram(srgbProgram_);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, linearColor_);
    glBindVertexArray(srgbVao_);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    ++frame_;
}

// draws frame_ in linear color into the bound framebuffer
void RenderGLES2::drawScene(int w, int h)
{
    // Clear
    const float gray = srgbToLinear(0.2f);
    glClearColor(gray, gray, gray, 1.F);
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
    case Scene::ColorChecker:
        glUseProgram(colorCheckerProgram_);
        glUniform2i(colorCheckerResolutionLoc_, w, h);
        glBindVertexArray(colorCheckerVao_);
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
}
