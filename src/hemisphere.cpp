#include "hemisphere.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glad/glad_gles32.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec3.hpp>
#include <glm/gtx/hash.hpp>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <vector>
#include <unordered_map>

struct Vertex {
    float x;
    float y;
    float z;
    /*
    bool operator==(const Vertex &other) const {
        return (this->x == other.x) && (this->y == other.y) && (this->z == other.z);
    }
    */
};

struct VertexHasher
 {
     size_t operator()(const Vertex& k) const
     {
       size_t h1 = std::hash<int64_t>()(k.x);
       size_t h2 = std::hash<int64_t>()(k.y);
       size_t h3 = std::hash<int64_t>()(k.z);
       return (h1 ^ (h2 << 1)) ^ h3;
     }
 };

// Coordinates for a "unit" 40-face geodesic hemisphere. This can be tessellated as finely as required
// to turn it into a unit hemisphere with (fairly) even sampling.
static float geodesicHemisphereVerts[40][3][3] =
{
    { {-0.276393, 0.850651, 0.447214},  {-0.162460, 0.500000, 0.850651},  {-0.688191, 0.500000, 0.525731} },
    { {-0.000000, 0.000000, 1.000000},  {-0.525731, 0.000000, 0.850651},  {-0.162460, 0.500000, 0.850651} },
    { {-0.894427, 0.000000, 0.447214},  {-0.688191, 0.500000, 0.525731},  {-0.525731, 0.000000, 0.850651} },
    { {-0.162460, 0.500000, 0.850651},  {-0.525731, 0.000000, 0.850651},  {-0.688191, 0.500000, 0.525731} },
    { {-0.276393, 0.850651, 0.447214},  {-0.688191, 0.500000, 0.525731},  {-0.587785, 0.809017, -0.000000} },
    { {-0.894427, 0.000000, 0.447214},  {-0.951056, 0.309017, -0.000000},  {-0.688191, 0.500000, 0.525731} },
    { {-0.688191, 0.500000, 0.525731},  {-0.951056, 0.309017, -0.000000},  {-0.587785, 0.809017, -0.000000} },
    { {-0.276393, 0.850651, 0.447214},  {-0.587785, 0.809017, -0.000000},  {0.000000, 1.000000, 0.000000} },
    { {-0.276393, 0.850651, 0.447214},  {0.000000, 1.000000, 0.000000},  {0.262866, 0.809017, 0.525731} },
    { {0.723607, 0.525731, 0.447214},  {0.262866, 0.809017, 0.525731},  {0.587785, 0.809017, 0.000000} },
    { {0.000000, 1.000000, 0.000000},  {0.587785, 0.809017, 0.000000},  {0.262866, 0.809017, 0.525731} },
    { {-0.276393, 0.850651, 0.447214},  {0.262866, 0.809017, 0.525731},  {-0.162460, 0.500000, 0.850651} },
    { {0.723607, 0.525731, 0.447214},  {0.425325, 0.309017, 0.850651},  {0.262866, 0.809017, 0.525731} },
    { {-0.000000, 0.000000, 1.000000},  {-0.162460, 0.500000, 0.850651},  {0.425325, 0.309017, 0.850651} },
    { {0.262866, 0.809017, 0.525731},  {0.425325, 0.309017, 0.850651},  {-0.162460, 0.500000, 0.850651} },
    { {0.723607, 0.525731, 0.447214},  {0.587785, 0.809017, 0.000000},  {0.951056, 0.309017, 0.000000} },
    { {-0.276393, -0.850651, 0.447214},  {0.262866, -0.809017, 0.525731},  {0.000000, -1.000000, 0.000000} },
    { {0.723607, -0.525731, 0.447214},  {0.587785, -0.809017, 0.000000},  {0.262866, -0.809017, 0.525731} },
    { {0.000000, -1.000000, 0.000000},  {0.262866, -0.809017, 0.525731},  {0.587785, -0.809017, 0.000000} },
    { {0.723607, -0.525731, 0.447214},  {0.951056, -0.309017, 0.000000},  {0.587785, -0.809017, 0.000000} },
    { {-0.276393, -0.850651, 0.447214},  {0.000000, -1.000000, 0.000000},  {-0.587785, -0.809017, -0.000000} },
    { {-0.276393, -0.850651, 0.447214},  {-0.587785, -0.809017, -0.000000},  {-0.688191, -0.500000, 0.525731} },
    { {-0.894427, 0.000000, 0.447214},  {-0.688191, -0.500000, 0.525731},  {-0.951056, -0.309017, -0.000000} },
    { {-0.587785, -0.809017, -0.000000},  {-0.951056, -0.309017, -0.000000},  {-0.688191, -0.500000, 0.525731} },
    { {-0.276393, -0.850651, 0.447214},  {-0.688191, -0.500000, 0.525731},  {-0.162460, -0.500000, 0.850651} },
    { {-0.894427, 0.000000, 0.447214},  {-0.525731, 0.000000, 0.850651},  {-0.688191, -0.500000, 0.525731} },
    { {-0.000000, 0.000000, 1.000000},  {-0.162460, -0.500000, 0.850651},  {-0.525731, 0.000000, 0.850651} },
    { {-0.688191, -0.500000, 0.525731},  {-0.525731, 0.000000, 0.850651},  {-0.162460, -0.500000, 0.850651} },
    { {-0.276393, -0.850651, 0.447214},  {-0.162460, -0.500000, 0.850651},  {0.262866, -0.809017, 0.525731} },
    { {-0.000000, 0.000000, 1.000000},  {0.425325, -0.309017, 0.850651},  {-0.162460, -0.500000, 0.850651} },
    { {0.723607, -0.525731, 0.447214},  {0.262866, -0.809017, 0.525731},  {0.425325, -0.309017, 0.850651} },
    { {-0.162460, -0.500000, 0.850651},  {0.425325, -0.309017, 0.850651},  {0.262866, -0.809017, 0.525731} },
    { {0.723607, 0.525731, 0.447214},  {0.951056, 0.309017, 0.000000},  {0.850651, 0.000000, 0.525731} },
    { {0.723607, -0.525731, 0.447214},  {0.850651, 0.000000, 0.525731},  {0.951056, -0.309017, 0.000000} },
    { {0.951056, 0.309017, 0.000000},  {0.951056, -0.309017, 0.000000},  {0.850651, 0.000000, 0.525731} },
    { {0.723607, 0.525731, 0.447214},  {0.850651, 0.000000, 0.525731},  {0.425325, 0.309017, 0.850651} },
    { {0.723607, -0.525731, 0.447214},  {0.425325, -0.309017, 0.850651},  {0.850651, 0.000000, 0.525731} },
    { {-0.000000, 0.000000, 1.000000},  {0.425325, 0.309017, 0.850651},  {0.425325, -0.309017, 0.850651} },
    { {0.850651, 0.000000, 0.525731},  {0.425325, -0.309017, 0.850651},  {0.425325, 0.309017, 0.850651} },
    { {-0.894427, 0.000000, 0.447214},  {-0.951056, -0.309017, -0.000000},  {-0.951056, 0.309017, -0.000000} }
};

Hemisphere::Hemisphere() :
    hemisphereVerticesVBO(-1),
    hemisphereVerticesVAO(-1),
    hemiShader(-1),
    axisShader(-1),
    numVerticesInHemisphere(0),
    numTrianglesInHemisphere(0)
{
    lookPhi = 0;
    lookTheta = 0.785398163;
    lookZoom = 4.0;
    
    NEAR_PLANE = 0.5f;
    FAR_PLANE = 100.0f;
    FOV_Y = 45.0f;
    
    frame_ = 0;

#if 0
    std::unordered_map<glm::vec3, int> vertex_indices;
    int index = 0;
    std::vector<int> face_indices;
    for( int i = 0; i < 40; i++ )
    {
        for (int k = 0; k < 3; k++) {
            float *v = geodesicHemisphereVerts[i][k];
            glm::vec3 vertex = { v[0], v[1], v[2] };
            if (auto it = vertex_indices.find(vertex); it != vertex_indices.end()) {
                // vertex already found
                // ..
                face_indices.push_back(it->second);
            } else {
                vertex_indices[vertex] = index;
                face_indices.push_back(index);
                index++;
            }
        }
    }
    std::cout << "indexed vertices size : " << index << std::endl;
    std::cout << "face indices size : " << face_indices.size() << std::endl;
#endif

}

Hemisphere::~Hemisphere()
{
    glDeleteVertexArrays(1, &hemisphereVerticesVAO);
    glDeleteBuffers(1, &hemisphereVerticesVBO);
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
void Hemisphere::initialize()
{
    makeGeodesicHemisphereVBO();
    
    constexpr char kAxisVS[] = R"(#version 300 es
      uniform mat4 modelviewmatrix;
      uniform mat4 projectionmatrix;
      in vec3 vPosition;
      void main()
      {
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

void Hemisphere::subdivideTriangle(float* vertices, int& index, float *v1, float *v2, float *v3, int depth)
{
    float v12[3];
    float v13[3];
    float v23[3];
    
    if( depth == 0 )
    {
        vertices[index++] = v1[0];
        vertices[index++] = v1[1];
        vertices[index++] = v1[2];
        
        vertices[index++] = v2[0];
        vertices[index++] = v2[1];
        vertices[index++] = v2[2];

        vertices[index++] = v3[0];
        vertices[index++] = v3[1];
        vertices[index++] = v3[2];

        return;
    }
    
    for( int i = 0; i < 3; i++ )
    {
        v12[i] = (v2[i] - v1[i])*0.5 + v1[i];
        v13[i] = (v3[i] - v1[i])*0.5 + v1[i];
        v23[i] = (v3[i] - v2[i])*0.5 + v2[i];
    }
    
    subdivideTriangle(vertices, index, v1, v12, v13, depth - 1);
    subdivideTriangle(vertices, index, v12, v2, v23, depth - 1);
    subdivideTriangle(vertices, index, v13, v23, v3, depth - 1);
    subdivideTriangle(vertices, index, v13, v12, v23, depth - 1);
}

void Hemisphere::makeGeodesicHemisphereVBO()
{
    float* hemisphereVertices;
    int numSubdivisions = 2;
    int memIndex = 0;

    // allocate enough memory for all the vertices in the hemisphere
    numTrianglesInHemisphere = 40 * int(std::powf(4.0, float(numSubdivisions)));
    numVerticesInHemisphere =  numTrianglesInHemisphere * 3;
    std::vector<float> vertexCoords;
    vertexCoords.resize(numVerticesInHemisphere * 3);
    hemisphereVertices = &vertexCoords[0];
    //printf( "numTrianglesInHemisphere: %d\n", numTrianglesInHemisphere );

    glGenVertexArrays(1, &hemisphereVerticesVAO);
    glBindVertexArray(hemisphereVerticesVAO);

    // Generate and bind the vertex buffer object
    glGenBuffers( 1, &hemisphereVerticesVBO );
    glBindBuffer( GL_ARRAY_BUFFER, hemisphereVerticesVBO );

    // recursively divide the hemisphere triangles to get a nicely tessellated hemisphere
    for( int i = 0; i < 40; i++ )
    {
        subdivideTriangle( hemisphereVertices, memIndex,
                        geodesicHemisphereVerts[i][0],
                        geodesicHemisphereVerts[i][1],
                        geodesicHemisphereVerts[i][2], numSubdivisions );
    }

    // indice calculation
#if 1
    std::unordered_map<glm::vec3, int> vertex_indices;
    int index = 0;
    std::vector<int> face_indices;
    for( int i = 0; i < numVerticesInHemisphere; i++ )
    {
        float *v = hemisphereVertices + i*3;
        glm::vec3 vertex = { v[0], v[1], v[2] };
        if (auto it = vertex_indices.find(vertex); it != vertex_indices.end()) {
            // vertex already found
            // ..
            face_indices.push_back(it->second);
        } else {
            vertex_indices[vertex] = index;
            face_indices.push_back(index);
            index++;
        }
    }
    std::cout << "indexed vertices size : " << index << std::endl;
    std::cout << "face indices size : " << face_indices.size() << std::endl;
#endif
    
    // copy the data into a buffer on the GPU
    glBufferData(GL_ARRAY_BUFFER, numTrianglesInHemisphere*sizeof(float)*9, hemisphereVertices, GL_STATIC_DRAW);
    
    glGenBuffers( 1, &hemiScaleVBO );
    glBindBuffer( GL_ARRAY_BUFFER, hemiScaleVBO );
    
    std::vector<float> vScale;
    vScale.resize(numVerticesInHemisphere);
    std::fill(vScale.begin(), vScale.end(), 1.0);

    glBufferData(GL_ARRAY_BUFFER, numVerticesInHemisphere*sizeof(float), &vScale[0], GL_DYNAMIC_DRAW);

    glBindVertexArray(0);
}

/*
// omit 2,3,4,6,
*/



void Hemisphere::updateMVP(int w, int h)
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

void Hemisphere::updateHemiScale()
{
    float scale = (frame_++ % 100) / 100.0;
    std::vector<float> vScale;
    vScale.resize(numVerticesInHemisphere);
    std::fill(vScale.begin(), vScale.end(), scale);

    glBufferData(GL_ARRAY_BUFFER, numVerticesInHemisphere*sizeof(float), &vScale[0], GL_DYNAMIC_DRAW);
}

void Hemisphere::renderHemi()
{
    glUseProgram(hemiShader);

    // assume matrices updated
    int mvLoc = glGetUniformLocation(hemiShader, "modelviewmatrix");
    glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(modelViewMatrix));

    int projLoc = glGetUniformLocation(hemiShader, "projectionmatrix");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    // draw hemisphere
    glBindVertexArray(hemisphereVerticesVAO);

    // setup to draw the VBO
    glBindBuffer(GL_ARRAY_BUFFER, hemisphereVerticesVBO);
    
    int vertex_loc = glGetAttribLocation(hemiShader, "vPosition");
    if(vertex_loc>=0){
        glEnableVertexAttribArray(vertex_loc);
        glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, hemiScaleVBO);

    updateHemiScale();
    
    int scale_loc = glGetAttribLocation(hemiShader, "vScale");
    if(scale_loc>=0){
        glEnableVertexAttribArray(scale_loc);
        glVertexAttribPointer(scale_loc, 1, GL_FLOAT, GL_FALSE, 0, 0);
    }

    glDrawArrays(GL_TRIANGLES, 0, numTrianglesInHemisphere*3);
    glBindVertexArray(0);

}

void Hemisphere::renderAxis()
{
    glUseProgram(axisShader);

    // assume matrices updated
    int mvLoc = glGetUniformLocation(axisShader, "modelviewmatrix");
    glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(modelViewMatrix));

    int projLoc = glGetUniformLocation(axisShader, "projectionmatrix");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    glBindVertexArray(hemisphereVerticesVAO);

    // setup to draw the VBO
    glBindBuffer(GL_ARRAY_BUFFER, hemisphereVerticesVBO);
    
    int vertex_loc = glGetAttribLocation(axisShader, "vPosition");
    if(vertex_loc>=0){
        glEnableVertexAttribArray(vertex_loc);
        glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }

    glDrawArrays(GL_POINTS, 0, numVerticesInHemisphere);

}

void Hemisphere::render(int w, int h)
{
    glClearColor( 0.2, 0.2, 0.2, 0.2 );
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    updateMVP(w, h);

    renderHemi();
    
    renderAxis();

}
