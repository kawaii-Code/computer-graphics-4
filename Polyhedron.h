#ifndef POLYHEDRON_H
#define POLYHEDRON_H

#include "camera.h"
#include "third_party/include/raylib.h"
#include "third_party/include/raymath.h"
#include "vector.h"

// Текстурные координаты
typedef struct {
    float u, v;
} TexCoord;

// Вершина
typedef struct {
    Vector3 position;
    Vector3 normal;
    Vector2 texCoord;
} Vertex;

vector(int);

// Грань
typedef struct {
    VECTOR_TYPE(int) vertexIndices;
} Face;

vector(Vertex);
vector(Face);
vector(Vector3);

// Многогранник
typedef struct {
    VECTOR_TYPE(Vertex) vertices;
    VECTOR_TYPE(Face) faces;
    Color color;
    Vector3 center;
} Polyhedron;

void Polyhedron_init(Polyhedron* poly);
void Polyhedron_free(Polyhedron* poly);
void Polyhedron_addVertex(Polyhedron* poly, Vector3 position);
void Polyhedron_addVertexEx(Polyhedron* poly, Vector3 position, Vector3 normal, Vector2 texCoord);
void Polyhedron_addFace(Polyhedron* poly, int* indices, int count);
void Polyhedron_updateCenter(Polyhedron* poly);

Vector3 Face_calculateNormal(Polyhedron* poly, Face* face);
void Polyhedron_calculateNormals(Polyhedron* poly);
bool Face_isFrontFacing(Polyhedron* poly, Face* face, CameraZ* camera);

Matrix CreateTranslationMatrix(Vector3 translation);
Matrix CreateRotationX(float angle);
Matrix CreateRotationY(float angle);
Matrix CreateRotationZ(float angle);
Matrix CreateScaleMatrix(Vector3 scale);
Matrix CreateTransformMatrix(Polyhedron* poly, Vector3 translation, Vector3 rotation_angles, Vector3 scale, char reflection_plane, Vector3 line_p1, Vector3 line_p2, float line_angle);
Matrix CreateReflectionMatrix(char plane);

typedef enum {
    TETRAHEDRON,
    HEXAHEDRON,
    OCTAHEDRON,
    ICOSAHEDRON,
    DODECAHEDRON
} PolyhedronType;

Polyhedron* Polyhedron_create();
Polyhedron* Polyhedron_createTetrahedron();
Polyhedron* Polyhedron_createHexahedron();
Polyhedron* Polyhedron_createOctahedron();
Polyhedron* Polyhedron_createIcosahedron();
Polyhedron* Polyhedron_createDodecahedron();

void Polyhedron_draw(Polyhedron* poly, Matrix transform);
VECTOR_TYPE(Vector3)* Polyhedron_transform(Polyhedron* poly, Matrix transform);


bool Polyhedron_loadFromObj(Polyhedron* poly, const char* filename);
bool Polyhedron_saveToObj(Polyhedron* poly, const char* filename);


float Polyhedron_bounding_radius(Polyhedron* poly);

void free_positions();

#endif