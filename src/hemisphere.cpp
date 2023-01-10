#include "hemisphere.h"

#include "glad/glad_gles32.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <algorithm>

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
    hemisphereVerticesVBO(0),
    hemisphereVerticesVAO(0),
    numTrianglesInHemisphere(0)
{
    lookPhi = 0;
    lookTheta = 0.785398163;
    lookZoom = 4.0;
    
    NEAR_PLANE = 0.5f;
    FAR_PLANE = 100.0f;
    FOV_Y = 45.0f;
}

Hemisphere::~Hemisphere()
{
    glDeleteVertexArrays(1, &hemisphereVerticesVAO);
    glDeleteBuffers(1, &hemisphereVerticesVBO);
}

void Hemisphere::initialize()
{
    makeGeodesicHemisphereVBO();
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
    int numSubdivisions = 1; //6;
    int memIndex = 0;

    // allocate enough memory for all the vertices in the hemisphere
    numTrianglesInHemisphere = 40 * int(std::powf(4.0, float(numSubdivisions)));
    hemisphereVertices = new float[ numTrianglesInHemisphere * 3 * 3 ];
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

    // copy the data into a buffer on the GPU
    glBufferData(GL_ARRAY_BUFFER, numTrianglesInHemisphere*sizeof(float)*9, hemisphereVertices, GL_STATIC_DRAW);
    glBindVertexArray(0);

    // now that the hemisphere vertices are on the GPU, we're done with the local copy
    delete[] hemisphereVertices;
}

void Hemisphere::render(int w, int h)
{
    glClearColor( 0.2, 0.2, 0.2, 0.2 );
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
    
    modelViewMatrix = glm::lookAt(lookVec, glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));

    // draw hemisphere
    glBindVertexArray(hemisphereVerticesVAO);

    // setup to draw the VBO
    glBindBuffer(GL_ARRAY_BUFFER, hemisphereVerticesVBO);
    /*
    int vertex_loc = shader->getAttribLocation("vtx_position");
    if(vertex_loc>=0){
        glf->glEnableVertexAttribArray(vertex_loc);
        glf->glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }
    */
    glDrawArrays(GL_TRIANGLES, 0, numTrianglesInHemisphere*3);
    glBindVertexArray(0);
}
