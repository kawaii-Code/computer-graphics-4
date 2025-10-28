#ifndef POLYHEDRON_H
#define POLYHEDRON_H

#include "third_party/include/raylib.h"
#include "third_party/include/raymath.h"
#include "vector.h"  

// �������
typedef struct {
    Vector3 position;
    Vector3 normal;
} Vertex;

// �����
typedef struct {
    vector(int) vertexIndices;
} Face;

// ������������
typedef struct {
    vector(Vertex) vertices;
    vector(Face) faces;
    Color color;
    Vector3 center;
} Polyhedron;

void Polyhedron_init(Polyhedron* poly);
void Polyhedron_free(Polyhedron* poly);
void Polyhedron_addVertex(Polyhedron* poly, Vector3 position);
void Polyhedron_addFace(Polyhedron* poly, int* indices, int count);
void Polyhedron_updateCenter(Polyhedron* poly);

Matrix CreateTranslationMatrix(Vector3 translation);
Matrix CreateRotationX(float angle);
Matrix CreateRotationY(float angle);
Matrix CreateRotationZ(float angle);
Matrix CreateScaleMatrix(Vector3 scale);
Matrix CreateTransformMatrix(Polyhedron* poly, Vector3 translation, float rotation_angles[3], Vector3 scale);

typedef enum {
    TETRAHEDRON,
    HEXAHEDRON,
    OCTAHEDRON,
    ICOSAHEDRON,
    DODECAHEDRON
} PolyhedronType;

void Polyhedron_createTetrahedron(Polyhedron* poly);
void Polyhedron_createHexahedron(Polyhedron* poly);
void Polyhedron_createOctahedron(Polyhedron* poly);
void Polyhedron_createIcosahedron(Polyhedron* poly);
void Polyhedron_createDodecahedron(Polyhedron* poly);

void Polyhedron_draw(Polyhedron* poly, Matrix transform);

#endif