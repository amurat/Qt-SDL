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
    hemiShader(-1),
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

    constexpr char kHemiVS[] = R"(#version 300 es
      uniform mat4 modelviewmatrix;
      uniform mat4 projectionmatrix;
      in vec3 vPosition;
      out vec4 eyeSpaceVert;
      void main()
      {
          eyeSpaceVert = modelviewmatrix * vec4(vPosition, 1.0);
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

    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, icoIndicesVBO);
    glDrawElements(GL_TRIANGLES, numTrianglesInIco*3, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

}

void Icosahedron::render(int w, int h)
{
    glEnable(GL_CULL_FACE);
    updateMVP(w, h);
    renderIco();
}
