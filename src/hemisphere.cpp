#include "hemisphere.h"
#include "datatransfer.h"
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
    hemiMarkerVBO(-1),
    hemiShader(-1),
    axisShader(-1),
    markerShader(-1),
    numVerticesInHemisphere(0),
    numTrianglesInHemisphere(0)
{
    lookPhi = 0;
    lookTheta = 0.785398163;
    lookZoom = 3.0;
    
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
      in float vScale;
      void main()
      {
          float offset = 0.01;
          vec4 position = vec4((vScale+offset) * vPosition, 1.0);
          //vec4 position = vec4(vPosition, 1.0);
          gl_Position = projectionmatrix * modelviewmatrix * position;
          gl_PointSize = 5.0;
      }
    )";

    constexpr char kAxisFS[] = R"(#version 300 es
  precision mediump float;
  out vec4 outColor;
  void main()
  {
          outColor = vec4(0.2, 0.2, 0.2, 1.0);
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
    
    constexpr char kMarkerVS[] = R"(#version 300 es
      uniform mat4 modelviewmatrix;
      uniform mat4 projectionmatrix;
      in vec3 vPosition;
      void main()
      {
          float offset = 0.2;
          vec4 position = vec4((1.0+offset) * vPosition, 1.0);
          gl_Position = projectionmatrix * modelviewmatrix * position;
          gl_PointSize = 5.0;
      }
    )";

    constexpr char kMarkerFS[] = R"(#version 300 es
  precision mediump float;
  uniform vec4 inColor;
  out vec4 outColor;
  void main()
  {
          outColor = inColor;
  })";

    markerShader = loadProgram(kMarkerVS, kMarkerFS);

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
    vertices.clear();
    int index = 0;
    std::vector<unsigned int> face_indices;
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
            vertices.push_back(vertex);
            face_indices.push_back(index);
            index++;
        }
    }
    numIndexedTrianglesInHemisphere = face_indices.size() / 3;
    numIndexedVerticesInHemisphere = vertices.size();
    std::cout << "indexed vertices size : " << vertices.size() << std::endl;
    std::cout << "face indices size : " << face_indices.size() << std::endl;
    std::cout << "codegen of vertex struct" << std::endl;
    std::cout << "{ ";
    bool first = true;
    for(auto v : vertices) {
        if (first) {
            first = false;
        } else {
            std::cout << ", ";
        }
        std::cout << v.x << ", " << v.y << ", " << v.z;
    }
    std::cout << " }" << std::endl;
#endif
    
    // copy the data into a buffer on the GPU
//    glBufferData(GL_ARRAY_BUFFER, numTrianglesInHemisphere*sizeof(float)*9, hemisphereVertices, GL_STATIC_DRAW);

    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float)*9, &vertices[0], GL_STATIC_DRAW);

    glGenBuffers( 1, &hemiScaleVBO );
    glBindBuffer( GL_ARRAY_BUFFER, hemiScaleVBO );
    
    std::vector<float> vScale;
    vScale.resize(numIndexedVerticesInHemisphere);
    std::fill(vScale.begin(), vScale.end(), 1.0);

    glBufferData(GL_ARRAY_BUFFER, numIndexedVerticesInHemisphere*sizeof(float), &vScale[0], GL_DYNAMIC_DRAW);

    // marker
    float marker[6] = { 0.0, 1.0, 0.0, 0.0, 0.0, 0.0 };
    glGenBuffers( 1, &hemiMarkerVBO );
    glBindBuffer( GL_ARRAY_BUFFER, hemiMarkerVBO );
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(marker)*sizeof(float), marker, GL_DYNAMIC_DRAW);

    // vertices finished
    glBindVertexArray(0);
    
    glGenBuffers(1, &hemiIndicesVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, hemiIndicesVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndexedTrianglesInHemisphere * sizeof(GLuint) * 3, &face_indices[0], GL_STATIC_DRAW);

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
    static float angleX = 0.0;
    model = glm::rotate(model, glm::radians(angleX), glm::vec3(1, 0, 0));
    //angleX += 1.0;
    static float angleY = 0.0;
    model = glm::rotate(model, glm::radians(angleY), glm::vec3(0, 1, 0));
    //angleY += 1.0;
    static float angleZ = 0.0;
    model = glm::rotate(model, glm::radians(angleZ), glm::vec3(0, 0, 1));
    angleZ += 1.0;
    modelViewMatrix = glm::lookAt(lookVec, glm::vec3(0, 0, 0), glm::vec3(0, 0, 1)) * model;
}

void Hemisphere::updateHemiScale()
{
#if 0
    float lengths[] = { 559.652, 536.744, 561.849, 522.372, 533.371, 568.492, 506.582, 513.921, 468.156, 494.854, 510.036, 535.212, 578.381, 544.74, 589.253, 533.371, 232.491, 246.289, 290.179, 269.734, 310.973, 342.969, 384.66, 324.42, 435.666, 359.897, 396.097, 360.4, 413.526, 432.888, 685.602, 664.531, 637.927, 632.053, 619.274, 567.959, 603.57, 577.44, 557.861, 532.03, 566.987, 487.381, 501.825, 474.403, 454.698, 518.048, 589.623, 594.401, 600.795, 600.605, 608.463, 616.969, 607.815, 620.719, 625.947, 643.123, 1012.33, 986.493, 671.617, 997.315, 676.18, 1008.71, 719.443, 651.153, 637.52, 620.395, 631.929, 658.789, 1045.91, 666.972, 642.689, 641.187, 580.514, 598.016, 611.798, 621.757, 633.041, 622.926, 614.502, 615.599, 603.586, 596.432, 549.419, 570.922, 546.681, 602.996, 615.354, 596.525, 579.152, 552.221, 582.215, 559.268, 595.324, 585.174, 621.489, 576.761, 609.971, 625.354, 561.579, 600.095, 593.637, 611.483, 619.343, 631.785, 610.644, 624.515, 602.036, 615.519, 599.871, 618.027, 599.763, 597.688, 533.455, 540.367, 537.405, 518.475, 524.912, 495.135, 547.46, 492.307, 529.552, 447.827, 488.113, 395.947, 457.728, 494.419, 524.006, 530.956, 277.157, 317.241, 312.514, 371.949, 444.379, 420.321, 354.408, 349.024, 400.527, 507.321, 463.223, 473.754, 626.323, 631.816, 637.464, 644.673, 642.008, 659.712, 652.817, 658.968, 674.323, 981.993, 798.818, 657.315, 574.324, 741.481, 552.715, 593.953, 839.917, 556.32, 809.948, 610.742, 566.454, 581.352, 593.95, 594.88, 596.432, 981.173, 923.932, 891.503, 927.049, 877.806, 872.202, 648.567, 889.87, 643.123, 617.273, 612.155, 850.465, 859.846, 801.151, 603.586, 600.668, 622.926, 577.148, 607.159, 633.041, 1025.28, 1037.71, 980.888, 1017.54, 1033.51, 1008.71, 1045.91, 666.972, 941.241, 641.187, 599.312, 599.192, 608.24, 603.402, 602.036, 615.519, 612.033, 625.273, 618.027, 624.515, 865.49, 889.411, 875.125, 635.058, 643.891, 932.818, 898.696, 934.164, 960.515, 950.172, 1004.2, 959.879, 990.231, 977.822, 963.233, 982.776, 981.811, 977.218, 996.46, 974.454, 962.285, 986.995, 981.993, 642.008, 659.712, 657.052, 674.323, 1020.03, 997.504, 930.215, 943.538, 1017.49, 974.106, 1009.71, 439.923, 1021.86, 406.968, 410.764, 380.009, 943.538, 623.26, 533.016, 472.534, 500.098, 562.338, 530.896, 134.407, 281.382, 167.464, 348.189, 314.995, 380.752, 361.947, 297.774, 407.977, 456.775, 433.58, 416.288, 940.459, 1005.54, 382.372, 973.595, 967.706, 955.682, 148.749, 133.369, 97.4643, 143.277, 85.7733, 372.571, 359.114, 321.187, 306.17, 339.719, 488.405, 999.156, 446.255, 978.912, 995.858, 417.46, 427.188, 413.667, 400.41, 369.404, 363.777, 402.13, 589.87, 626.083, 585.571, 647.257, 677.154, 667.958, 634.69, 599.287, 644.6, 588.835, 1004.62, 573.9, 999.918, 595.047, 981.587, 647.873, 964.75, 999.556, 976.768, 955.653, 940.957, 949.652, 969.861, 948.35, 683.744, 535.704, 524.311, 535.811, 462.668, 477.39, 407.163, 504.281, 468.267, 392.15, 457.694, 511.882, 516.807, 204.087, 305.837, 345.392, 284.464, 194.904, 199.905, 427.969, 376.298, 343.142, 961.697, 949.077, 955.653, 940.957, 968.395, 969.861};
    int minLengthIndex = 273;
    float seedNormal[] = { -0.148068, -0.132639, 0.980042 };
    float orientNormal[] = { 0.181733, -0.295244, 0.937979 };
#else
    float lengths[] = { 494.951, 477.956, 494.283, 442.532, 476.54, 503.391, 390.164, 446.089, 311.834, 389.527, 454.982, 479.108, 513.08, 480.385, 538.866, 476.54, 144.474, 235.136, 196.79, 363.554, 243.528, 228.071, 393.647, 289.177, 460.636, 337.557, 326.337, 271.733, 265.589, 308.849, 1011.32, 672.948, 667.829, 616.903, 628.085, 614.261, 574.309, 556.904, 514.902, 502.305, 571.587, 534.844, 455.952, 392.525, 394.683, 463.807, 525.111, 534.203, 570.043, 546.286, 566.114, 585.036, 576.69, 593, 600.438, 622.447, 1001.11, 986.244, 674.258, 996.947, 669.276, 1010.51, 673.206, 637.354, 626.87, 599.301, 606.07, 644.43, 1049.81, 650.739, 622.253, 634.642, 511.812, 547.822, 530.43, 585.492, 607.369, 601.76, 573.849, 561.103, 615.964, 605.623, 488.306, 501.195, 477.309, 520.213, 570.64, 522.615, 500.703, 460.98, 486.817, 448.349, 464.96, 443.794, 516.774, 431.673, 485.008, 550.874, 433.805, 475.552, 480.986, 541.873, 542.651, 589.449, 579.783, 597.988, 600.106, 597.871, 570.415, 600.578, 577.629, 524.941, 459.64, 438.181, 414.699, 387.152, 414.948, 355.724, 410.852, 348.915, 399.873, 259.143, 338.125, 208.385, 272.271, 344.643, 390.65, 397.184, 82.703, 157.915, 42.5014, 218.362, 267.321, 224.873, 199.189, 91.4889, 226.495, 358.659, 269.612, 295.988, 522.131, 555.373, 569.095, 578.08, 604.299, 612.002, 585.594, 599.663, 621.267, 638.056, 636.592, 624.047, 632.087, 610.477, 631.084, 625.595, 614.883, 626.01, 866.978, 620.97, 630.774, 624.669, 623.967, 626.338, 605.623, 996.028, 952.496, 921.001, 744.551, 947.01, 914.397, 642.954, 652.981, 622.447, 653.072, 640.623, 878.925, 847.717, 627.572, 615.964, 633.603, 601.76, 639.401, 645.349, 607.369, 1054.29, 1048.76, 976.101, 1024.51, 1004.35, 1010.51, 1049.81, 650.739, 652.914, 634.642, 641.138, 634.161, 629.613, 608.257, 600.106, 597.871, 603.609, 614.562, 600.578, 597.988, 656.982, 655.305, 954.13, 639.695, 633.556, 661.191, 677.777, 976.5, 984.711, 1038.94, 993.96, 954.945, 999.376, 968.666, 953.023, 1028.72, 1007.86, 1051.86, 1022.52, 980.627, 964.481, 669.326, 638.056, 604.299, 612.002, 641.057, 621.267, 645.107, 681.282, 854.83, 926.636, 1014.23, 947, 1016.36, 1090.64, 1036.9, 1085.33, 1095.35, 494.793, 926.636, 676.56, 604.684, 538.886, 591.084, 1031.23, 1048.9, 286.288, 407.735, 351.549, 467.098, 444.977, 487.019, 503.589, 420.83, 476.353, 1103.26, 544.521, 529.589, 717.352, 1039.28, 1064.71, 1019.76, 850.346, 900.75, 138.49, 290.893, 336.637, 352.322, 400.443, 424.294, 438.31, 455.887, 438.41, 476.099, 1031.63, 960.27, 507.266, 950.009, 903.679, 1054.18, 505.782, 465.264, 467.731, 503.954, 475.773, 1044.35, 436.155, 511.739, 425.46, 579.917, 618.867, 598.596, 523.628, 455.517, 549.564, 491.036, 1025.9, 576.159, 1011.9, 546.148, 648.61, 609.562, 955.844, 1004.18, 974.843, 947.272, 935.95, 645.277, 968.501, 937.64, 633.892, 377.862, 353.498, 385.424, 288.5, 310.558, 237.065, 527.33, 489.179, 425.903, 436.788, 505.851, 437.59, 101.392, 95.5167, 198.016, 162.865, 149.385, 190.12, 294.055, 241.186, 247.592, 949.58, 945.434, 947.272, 935.95, 958.366, 968.501};
    int minLengthIndex = 285;
    float seedNormal[] = { -0.119659, 0.685701, 0.71798 };
    float orientNormal[] = { 0.485137, -0.515679, 0.706199 };
#endif
    
    
    unsigned int numElements = sizeof(lengths) / sizeof(float);
    assert(numElements == numIndexedVerticesInHemisphere);

#if 1
    std::vector<float> buf(numIndexedVerticesInHemisphere + 6);
    readDataBytes((char*)buf.data(), buf.size()*sizeof(float));
    std::copy(buf.begin(), buf.begin()+numIndexedVerticesInHemisphere, lengths);
    float* fSeed = buf.data() + numIndexedVerticesInHemisphere;
    float* fOrient = fSeed + 3;
    std::copy(fSeed, fSeed+3, seedNormal);
    std::copy(fOrient, fOrient+3, orientNormal);
#endif

      markerOrient[0] = orientNormal[0];
      markerOrient[1] = orientNormal[1];
      markerOrient[2] = orientNormal[2];

      markerSeed[0] = seedNormal[0];
      markerSeed[1] = seedNormal[1];
      markerSeed[2] = seedNormal[2];


    float maxLen = 0;
    float minLen = FLT_MAX;
    int maxIndex = 0;
    int minIndex = 0;
    const float maxDotProduct = 0.85;
    const float maxRadians = acos(maxDotProduct);
    const float maxDegrees =  glm::degrees(maxRadians);
    for (int i = 0; i < numElements; i++)
    {
        auto vertex = vertices[i];
        float dotproduct = glm::dot(vertex, markerSeed);
        if (dotproduct > maxDotProduct) {
            continue;
        }
        if (lengths[i] > maxLen) {
            maxLen = lengths[i];
            maxIndex = i;
        }
        if (lengths[i] < minLen) {
            minIndex = i;
            minLen = lengths[i];
        }
    }

    for (auto& l : lengths) {
        l /= maxLen;
    }
    glBindBuffer(GL_ARRAY_BUFFER, hemiScaleVBO);
    glBufferData(GL_ARRAY_BUFFER, numIndexedVerticesInHemisphere*sizeof(float), lengths, GL_DYNAMIC_DRAW);

    //std::cout << "minIndex : " << minIndex << std::endl;
    //std::cout << "minLen : " << minLen << std::endl;
    
    auto markerMinVtx = vertices[minIndex];
    markerMin[0] = markerMinVtx[0];
    markerMin[1] = markerMinVtx[1];
    markerMin[2] = markerMinVtx[2];


    /*
     float scale = (frame_++ % 100) / 100.0;
     scale = 1.0;
     std::vector<float> vScale;
     vScale.resize(numIndexedVerticesInHemisphere);
     std::fill(vScale.begin(), vScale.end(), scale);
     

    int index = 0;
    for(auto&v : vScale) {
        float val = index++ / (float)vScale.size();
        val = std::max<float>(val, 0.5);
        v *= val;
    }
     glBindBuffer(GL_ARRAY_BUFFER, hemiScaleVBO);
     glBufferData(GL_ARRAY_BUFFER, numIndexedVerticesInHemisphere*sizeof(float), &vScale[0], GL_DYNAMIC_DRAW);
     glBufferData(GL_ARRAY_BUFFER, numIndexedVerticesInHemisphere*sizeof(float), lengths, GL_DYNAMIC_DRAW);
     */
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

    
    int scale_loc = glGetAttribLocation(hemiShader, "vScale");
    if(scale_loc>=0){
        glEnableVertexAttribArray(scale_loc);
        glVertexAttribPointer(scale_loc, 1, GL_FLOAT, GL_FALSE, 0, 0);
    }
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, hemiIndicesVBO);
    glDrawElements(GL_TRIANGLES, numIndexedTrianglesInHemisphere*3, GL_UNSIGNED_INT, 0);

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

    glBindBuffer(GL_ARRAY_BUFFER, hemiScaleVBO);
    
    int scale_loc = glGetAttribLocation(hemiShader, "vScale");
    if(scale_loc>=0){
        glEnableVertexAttribArray(scale_loc);
        glVertexAttribPointer(scale_loc, 1, GL_FLOAT, GL_FALSE, 0, 0);
    }

    glDrawArrays(GL_POINTS, 0, numVerticesInHemisphere);

}

void Hemisphere::renderMarker(glm::vec4 color, glm::vec3 position)
{
    glUseProgram(markerShader);

    // assume matrices updated
    int mvLoc = glGetUniformLocation(markerShader, "modelviewmatrix");
    glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(modelViewMatrix));

    int projLoc = glGetUniformLocation(markerShader, "projectionmatrix");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    int colorLoc = glGetUniformLocation(markerShader, "inColor");
    glUniform4f(colorLoc, color[0], color[1], color[2], color[3]);

    glBindVertexArray(hemisphereVerticesVAO);

    // setup to draw the VBO
    glBindBuffer(GL_ARRAY_BUFFER, hemiMarkerVBO);

    float marker[6] = { position[0], position[1], position[2], 0.0, 0.0, 0.0 };
    glBufferData(GL_ARRAY_BUFFER, sizeof(marker)*sizeof(float), marker, GL_DYNAMIC_DRAW);

    int vertex_loc = glGetAttribLocation(markerShader, "vPosition");
    if(vertex_loc>=0){
        glEnableVertexAttribArray(vertex_loc);
        glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }
/*
    glBindBuffer(GL_ARRAY_BUFFER, hemiScaleVBO);
    
    int scale_loc = glGetAttribLocation(markerShader, "vScale");
    if(scale_loc>=0){
        glEnableVertexAttribArray(scale_loc);
        glVertexAttribPointer(scale_loc, 1, GL_FLOAT, GL_FALSE, 0, 0);
    }
*/

    glDrawArrays(GL_LINES, 0, 2);
    glDrawArrays(GL_POINTS, 0, 1);
}

void Hemisphere::render(int w, int h)
{
    glClearColor( 0.2, 0.2, 0.2, 0.2 );
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    updateMVP(w, h);

    updateHemiScale();

    renderHemi();
    
    renderAxis();

    glm::vec4 orientColor(0, 1, 0, 1);
    renderMarker(orientColor, markerOrient);

    glm::vec4 seedColor(0, 0, 1, 1);
    renderMarker(seedColor, markerSeed);

    /*
    glm::vec4 minColor(0, 1, 1, 1);
    renderMarker(minColor, markerMin);
    */
    
    glm::vec3 horiz(0, 1, 0);
    glm::vec4 horizColor(1, 1, 0, 1);
    renderMarker(horizColor, horiz);

    /*
    glm::vec3 vert(0, 1, 0);
    glm::vec4 vertColor(0, 1, 1, 1);
    renderMarker(vertColor, vertColor);
     */
}
