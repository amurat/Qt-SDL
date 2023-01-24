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
      in float vScale;
      void main()
      {
          float offset = 0.01;
          vec4 position = vec4((vScale+offset) * vPosition, 1.0);
          gl_Position = projectionmatrix * modelviewmatrix * position;
          gl_PointSize = 5.0;
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
    std::vector<glm::vec3> vertices;
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
    float lengths[] = { 582.754, 580.61, 596.587, 655.306, 654.085, 688.126, 380.172, 791.791, 446.326, 818.416, 870.923, 744.339, 762.192, 903.766, 850.499, 654.085, 1154.87, 1125.2, 1125.09, 663.25, 1060.89, 1036.72, 618.154, 589.847, 1083.37, 1026.04, 952.992, 965.923, 864.148, 570.392, 1001.76, 993.881, 1015.22, 932.862, 994.37, 1048.63, 880.392, 960.646, 918.482, 981.584, 1019.53, 1070, 1019.67, 932.73, 973.512, 955.31, 564.151, 600.41, 569.772, 689.968, 784.089, 697.73, 614.512, 579.28, 634.686, 590.134, 984.064, 980.533, 961.697, 982.983, 956.772, 1001.79, 974.582, 919.822, 917.756, 891.366, 862.047, 925.398, 980.521, 900.872, 775.938, 749.689, 597.921, 590.03, 604.141, 576.062, 571.004, 613.309, 618.703, 674.483, 694.034, 731.877, 586.261, 566.268, 449.361, 617.211, 750.887, 701.208, 534.635, 652.325, 646.681, 611.883, 460.959, 476.394, 473.63, 497.104, 489.881, 486.131, 590.711, 551.313, 621.77, 650.994, 550.187, 546.382, 617.216, 605.027, 745.159, 678.522, 683.81, 670.27, 649.781, 640.131, 421.015, 396.024, 651.474, 377.489, 371.052, 360.49, 488.679, 523.516, 472.192, 713.343, 511.995, 972.683, 574.565, 517.741, 499.039, 649.276, 1124.38, 1100.42, 1037.64, 962.952, 508.851, 1015.84, 1070.96, 1038.74, 946.562, 706.24, 736.79, 433.859, 459.433, 464.303, 459.648, 488.51, 536.487, 495.79, 457.171, 460.301, 453.131, 463.636, 740.439, 736.305, 747.367, 772.93, 758.15, 769.794, 749.798, 751.826, 761.263, 758.505, 743.956, 767.009, 790.239, 724.437, 731.877, 988.051, 889.983, 890.968, 785.796, 824.022, 905.888, 606.049, 597.626, 590.134, 580.407, 604.957, 619.884, 722.798, 714.035, 694.034, 709.587, 613.309, 711.514, 610.597, 571.004, 1035.57, 1037.61, 944.309, 1013.7, 990.587, 1001.79, 980.521, 900.872, 829.379, 749.689, 725.565, 722.09, 699.76, 726.499, 745.159, 678.522, 684.36, 678.361, 670.27, 605.027, 741.434, 719.401, 723.862, 650.453, 599.159, 596.635, 655.034, 660.918, 594.916, 598.771, 1002.65, 983.667, 519.096, 501.675, 911.313, 544.842, 510.301, 535.154, 506.07, 480.197, 473.26, 470.559, 463.636, 536.487, 495.79, 533.057, 453.131, 503.674, 532.443, 728.208, 748.291, 717.141, 728.056, 672.137, 726.07, 741.165, 712.069, 732.417, 711.792, 748.291, 1031.19, 576.984, 597.272, 600.704, 558.004, 602.926, 1189.27, 1164.63, 1188.62, 715.273, 724.566, 725.169, 691.603, 676.963, 651.907, 674.951, 673.06, 708.709, 736.049, 739.005, 744.266, 792.261, 777.825, 757.315, 1184.83, 1145.7, 1208.89, 1144.49, 1168.3, 1147.11, 1130.11, 1168.58, 1167.78, 754.495, 1008.95, 1007.46, 1048.32, 973.221, 763.942, 1045.51, 1072.33, 1129.91, 1118.85, 812.672, 1092.16, 759.583, 494.71, 492.211, 738.811, 492.083, 496.368, 816.956, 770.532, 848.587, 893.95, 949.687, 1078.28, 1059.97, 1057.17, 1004.91, 1024.23, 973.166, 1000.99, 1026.42, 1006.81, 564.022, 944.637, 901.72, 977.761, 959.909, 928.318, 524.635, 801.621, 915.384, 853.963, 771.821, 832.262, 1094.71, 1102.97, 1160.58, 1059.78, 1063.28, 997.066, 1096.69, 1083.69, 1055.36, 1150.03, 1128.47, 1147.47, 983.294, 1148.43, 937.758, 971.323, 939.05, 564.022, 944.637, 972.825, 977.761};
    
    unsigned int numElements = sizeof(lengths) / sizeof(float);
    assert(numElements == numIndexedVerticesInHemisphere);
    float maxLen = 0;
    for (auto l : lengths) {
        if (l > maxLen) {
            maxLen = l;
        }
    }

    for (auto& l : lengths) {
        l /= maxLen;
    }
    glBindBuffer(GL_ARRAY_BUFFER, hemiScaleVBO);
    glBufferData(GL_ARRAY_BUFFER, numIndexedVerticesInHemisphere*sizeof(float), lengths, GL_DYNAMIC_DRAW);


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

void Hemisphere::render(int w, int h)
{
    glClearColor( 0.2, 0.2, 0.2, 0.2 );
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    updateMVP(w, h);

    updateHemiScale();

    renderHemi();
    
    renderAxis();

}
