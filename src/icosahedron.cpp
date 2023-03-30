#include "icosahedron.h"

#include "glad/glad_gles32.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <vector>



Icosahedron::Icosahedron() :
    icoVerticesVBO(-1),
    icoVAO(-1),
    icoIndicesVBO(-1),
    icoScaleVBO(-1),
    hemiShader(-1),
    axisShader(-1),
    numVerticesInIco(0),
    numTrianglesInIco(0)
{
    lookPhi = 0;
    lookTheta = 0.785398163;
    lookZoom = 4.0;
    
    NEAR_PLANE = 0.5f;
    FAR_PLANE = 100.0f;
    FOV_Y = 45.0f;
    
    frame_ = 0;
}

Icosahedron::~Icosahedron()
{
    glDeleteVertexArrays(1, &icoVAO);
    glDeleteBuffers(1, &icoVerticesVBO);
    glDeleteBuffers(1, &icoIndicesVBO);
    glDeleteBuffers(1, &icoScaleVBO);
}

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
void Icosahedron::initialize()
{
    makeIcoVBO();
    
    constexpr char kAxisVS[] = R"(#version 300 es
      uniform mat4 modelviewmatrix;
      uniform mat4 projectionmatrix;
      in vec3 vPosition;
      void main()
      {
        //gl_Position = vec4(0, 0, 0, 1);
          gl_Position = projectionmatrix * modelviewmatrix * vec4(vPosition, 1.0);
          gl_PointSize = 20.0;
      }
    )";

    constexpr char kAxisFS[] = R"(#version 300 es
  precision mediump float;
  out vec4 outColor;
  void main()
  {
          outColor = vec4(0.0, 1.0, 0.0, 1.0);
  })";
    
    axisShader = loadProgram(kAxisVS, kAxisFS);

    constexpr char kHemiVS[] = R"(#version 300 es
      uniform mat4 modelviewmatrix;
      uniform mat4 projectionmatrix;
      in vec3 vPosition;
      in float vScale;
      out vec4 eyeSpaceVert;
      void main()
      {
          vec4 position = vec4(vScale * vPosition, 1.0);
          eyeSpaceVert = modelviewmatrix * position;
          gl_Position = projectionmatrix * eyeSpaceVert;
      }
    )";

    
    constexpr char kHemiFS[] = R"(#version 300 es
  precision mediump float;
  in vec4 eyeSpaceVert;
  out vec4 outColor;
  void main()
  {
    vec3 drawColor = vec3(1.0, 0.0, 0.0);
    vec3 viewVec = -normalize(eyeSpaceVert.xyz);
    // since we're distorting the sphere all over the place, can't really use the sphere normal.
    // instead compute a per-pixel normal based on the derivative of the eye-space vertex position.
    // It ain't perfect but it works OK.
    vec3 normal = normalize( cross( dFdx(eyeSpaceVert.xyz), dFdy(eyeSpaceVert.xyz) ) );
    vec3 ref = reflect( -viewVec, normal );

    // simple phong shading
    vec3 q = drawColor * dot( normal, viewVec );
    q += vec3(0.4) * pow( max( dot( ref, viewVec ), 0.0 ), 10.0 );
    outColor = vec4( q, 1 );
  })";
    
    hemiShader = loadProgram(kHemiVS, kHemiFS);
}

void Icosahedron::makeIcoVBO()
{
    const float X = .525731112119133606;
    const float Z = .850650808352039932;
#if 0
    static float vdata[12][3] = {
       {-X, 0.0, Z}, {X, 0.0, Z}, {-X, 0.0, -Z}, {X, 0.0, -Z},
       {0.0, Z, X}, {0.0, Z, -X}, {0.0, -Z, X}, {0.0, -Z, -X},
       {Z, X, 0.0}, {-Z, X, 0.0}, {Z, -X, 0.0}, {-Z, -X, 0.0}
    };
    
    static unsigned int tindices[20][3] = {
       {0,4,1}, {0,9,4}, {9,5,4}, {4,5,8}, {4,8,1},
       {8,10,1}, {8,3,10}, {5,3,8}, {5,2,3}, {2,7,3},
       {7,10,3}, {7,6,10}, {7,11,6}, {11,0,6}, {0,1,6},
       {6,1,10}, {9,0,11}, {9,11,2}, {9,2,5}, {7,2,11} };

#else
    const float t = ( 1.0 + sqrtf(5.0) ) / 2.0;

    const float vdata[] = {
        - 1, t, 0,     1, t, 0,     - 1, - t, 0,     1, - t, 0,
        0, - 1, t,     0, 1, t,    0, - 1, - t,     0, 1, - t,
        t, 0, - 1,     t, 0, 1,     - t, 0, - 1,     - t, 0, 1
    };
    
    const unsigned int tindices[] = {
        0, 11, 5,     0, 5, 1,     0, 1, 7,     0, 7, 10,     0, 10, 11,
        1, 5, 9,     5, 11, 4,    11, 10, 2,    10, 7, 6,    7, 1, 8,
        3, 9, 4,     3, 4, 2,    3, 2, 6,    3, 6, 8,    3, 8, 9,
        4, 9, 5,     2, 4, 11,    6, 2, 10,    8, 6, 7,    9, 8, 1
    };
#endif
    numVerticesInIco = 12;
    numTrianglesInIco = 20;
    glGenVertexArrays(1, &icoVAO);
    glBindVertexArray(icoVAO);

    // Generate and bind the vertex buffer object
    glGenBuffers( 1, &icoVerticesVBO );
    glBindBuffer( GL_ARRAY_BUFFER, icoVerticesVBO );

    // recursively divide the hemisphere triangles to get a nicely tessellated hemisphere
    float* icoVertices = (float*)vdata;
/*
    for (int i = 0; i < numVertices*3; i += 3) {
        if (icoVertices[i+1] < 0) {
            icoVertices[i+1] = 0;
        }
    }
*/
    
    // copy the data into a buffer on the GPU
    
    glBufferData(GL_ARRAY_BUFFER, numVerticesInIco*3*sizeof(float), icoVertices, GL_STATIC_DRAW);
    
    // scale
    glGenBuffers( 1, &icoScaleVBO );
    glBindBuffer( GL_ARRAY_BUFFER, icoScaleVBO );
    
    std::vector<float> vScale;
    vScale.resize(numVerticesInIco);
    std::fill(vScale.begin(), vScale.end(), 1.0);

    glBufferData(GL_ARRAY_BUFFER, numVerticesInIco*sizeof(float), &vScale[0], GL_DYNAMIC_DRAW);

    glBindVertexArray(0);


    glGenBuffers(1, &icoIndicesVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, icoIndicesVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numTrianglesInIco * sizeof(GLuint) * 3, tindices, GL_STATIC_DRAW);
}

void Icosahedron::updateMVP(int w, int h)
{
    float nearPlane = std::min<float>( lookZoom*0.1, 0.5 );
    float farPlane =  std::min<float>( lookZoom*10.0, 100.0 );

    float fWidth = w;
    float fHeight = h;

    glViewport(0, 0, fWidth, fHeight);
    projectionMatrix = glm::perspective(FOV_Y, fWidth / fHeight, nearPlane, farPlane);

    glm::vec3 lookVec;
    lookVec[0] = sin(lookTheta) * cos(lookPhi);
    lookVec[1] = sin(lookTheta) * sin(lookPhi);
    lookVec[2] = cos(lookTheta);

    lookVec[0] *= lookZoom;
    lookVec[1] *= lookZoom;
    lookVec[2] *= lookZoom;

    glm::mat4 model = glm::mat4(1.0);
    static float angle = 45.0f;
    model = glm::rotate(model, glm::radians(angle), glm::vec3(1, 0, 0));
    angle += 1.0;
    
    modelViewMatrix = glm::lookAt(lookVec, glm::vec3(0, 0, 0), glm::vec3(0, 0, 1)) * model;
}

void Icosahedron::updateIcoScale()
{
    float scale = (frame_++ % 100) / 100.0;
    std::vector<float> vScale;
    vScale.resize(numVerticesInIco);
    std::fill(vScale.begin(), vScale.end(), scale);

    glBufferData(GL_ARRAY_BUFFER, numVerticesInIco*sizeof(float), &vScale[0], GL_DYNAMIC_DRAW);
}
void Icosahedron::renderIco()
{
    glUseProgram(hemiShader);

    // assume matrices updated
    int mvLoc = glGetUniformLocation(hemiShader, "modelviewmatrix");
    glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(modelViewMatrix));

    int projLoc = glGetUniformLocation(hemiShader, "projectionmatrix");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    // draw hemisphere
    glBindVertexArray(icoVAO);

    // setup to draw the VBO
    glBindBuffer(GL_ARRAY_BUFFER, icoVerticesVBO);
    
    int vertex_loc = glGetAttribLocation(hemiShader, "vPosition");
    if(vertex_loc>=0){
        glEnableVertexAttribArray(vertex_loc);
        glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }

    // setup to draw the VBO
    glBindBuffer(GL_ARRAY_BUFFER, icoScaleVBO);

    updateIcoScale();
    
    int scale_loc = glGetAttribLocation(hemiShader, "vScale");
    if(scale_loc>=0){
        glEnableVertexAttribArray(scale_loc);
        glVertexAttribPointer(scale_loc, 1, GL_FLOAT, GL_FALSE, 0, 0);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, icoIndicesVBO);
    glDrawElements(GL_TRIANGLES, numTrianglesInIco*3, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

}

void Icosahedron::renderAxis()
{
    glUseProgram(axisShader);

    // assume matrices updated
    int mvLoc = glGetUniformLocation(axisShader, "modelviewmatrix");
    glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(modelViewMatrix));

    int projLoc = glGetUniformLocation(axisShader, "projectionmatrix");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    glBindVertexArray(icoVAO);

    // setup to draw the VBO
    glBindBuffer(GL_ARRAY_BUFFER, icoVerticesVBO);
    
    int vertex_loc = glGetAttribLocation(axisShader, "vPosition");
    if(vertex_loc>=0){
        glEnableVertexAttribArray(vertex_loc);
        glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, icoIndicesVBO);
    glDrawElements(GL_POINTS, 12*3, GL_UNSIGNED_INT, 0);

}

void Icosahedron::render(int w, int h)
{
    glClearColor( 0.2, 0.2, 0.2, 0.2 );
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    updateMVP(w, h);

    //renderHemi();
    renderIco();

    //glDisable(GL_DEPTH_TEST);
    renderAxis();
}
