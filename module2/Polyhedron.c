#include <assert.h>
#include "Polyhedron.h"
#include "camera.h"

void Polyhedron_init(Polyhedron* poly) {
    vector_init(poly->vertices);
    vector_init(poly->faces);
    poly->color = WHITE;
    poly->center = (Vector3){ 0, 0, 0 };
}

void Polyhedron_free(Polyhedron* poly) {
    for (size_t i = 0; i < poly->faces.len; i++) {
        vector_free(poly->faces.head[i].vertexIndices);
    }
    vector_free(poly->faces);
    vector_free(poly->vertices);
}

void Polyhedron_addVertex(Polyhedron* poly, Vector3 position) {
    Polyhedron_addVertexEx(poly, position, (Vector3) { 0, 0, 0 }, (Vector2) { 0, 0 });
}

void Polyhedron_addVertexEx(Polyhedron* poly, Vector3 position, Vector3 normal, Vector2 texCoord) {
    Vertex v = {
        .position = position,
        .normal = normal,
        .texCoord = texCoord
    };
    vector_append(poly->vertices, v);
}

void Polyhedron_addFace(Polyhedron* poly, int* indices, int count) {
    Face face;
    vector_init(face.vertexIndices);
    for (int i = 0; i < count; i++) {
        vector_append(face.vertexIndices, indices[i]);
    }
    vector_append(poly->faces, face);
}

void Polyhedron_updateCenter(Polyhedron* poly) {
    if (poly->vertices.len == 0) {
        poly->center = (Vector3){ 0, 0, 0 };
        return;
    }

    Vector3 sum = { 0, 0, 0 };
    for (size_t i = 0; i < poly->vertices.len; i++) {
        sum = Vector3Add(sum, poly->vertices.head[i].position);
    }
    poly->center = Vector3Scale(sum, 1.0f / poly->vertices.len);
}

Polyhedron* Polyhedron_create() {
    Polyhedron* poly = calloc(1, sizeof(Polyhedron));
    poly->vertices.head = 0;
    poly->vertices.len = 0;
    poly->vertices.cap = 0;

    poly->faces.head = 0;
    poly->faces.len = 0;
    poly->faces.cap = 0;
    // vector_init(poly->vertices);
    // vector_init(poly->faces);

    return poly;
}

Vector3 Face_calculateNormal(Polyhedron* poly, Face* face) {
    if (face->vertexIndices.len < 3) return (Vector3) { 0, 0, 0 };

    Vector3 v0 = poly->vertices.head[face->vertexIndices.head[0]].position;
    Vector3 v1 = poly->vertices.head[face->vertexIndices.head[1]].position;
    Vector3 v2 = poly->vertices.head[face->vertexIndices.head[2]].position;

    Vector3 edge1 = Vector3Subtract(v1, v0);
    Vector3 edge2 = Vector3Subtract(v2, v0);
    return Vector3Normalize(Vector3CrossProduct(edge1, edge2));
}

Vector3 Face_calculateNormal_World(Polyhedron* poly, Face* face, Matrix world) {
    if (face->vertexIndices.len < 3) return (Vector3){ 0,0,0 };

    Vector3 v0 = Vector3Transform(poly->vertices.head[face->vertexIndices.head[0]].position, world);
    Vector3 v1 = Vector3Transform(poly->vertices.head[face->vertexIndices.head[1]].position, world);
    Vector3 v2 = Vector3Transform(poly->vertices.head[face->vertexIndices.head[2]].position, world);

    Vector3 edge1 = Vector3Subtract(v1, v0);
    Vector3 edge2 = Vector3Subtract(v2, v0);
    return Vector3Normalize(Vector3CrossProduct(edge1, edge2));
}

void Polyhedron_calculateNormals(Polyhedron* poly) {
    for (size_t i = 0; i < poly->vertices.len; i++) {
        poly->vertices.head[i].normal = (Vector3){ 0, 0, 0 };
    }

    for (size_t i = 0; i < poly->faces.len; i++) {
        Face* face = &poly->faces.head[i];

        if (face->vertexIndices.len >= 3) {
            Vector3 faceNormal = Face_calculateNormal(poly, face);

            for (size_t j = 0; j < face->vertexIndices.len; j++) {
                int vertexIndex = face->vertexIndices.head[j];
                poly->vertices.head[vertexIndex].normal =
                    Vector3Add(poly->vertices.head[vertexIndex].normal, faceNormal);
            }
        }
    }

    for (size_t i = 0; i < poly->vertices.len; i++) {
        poly->vertices.head[i].normal = Vector3Normalize(poly->vertices.head[i].normal);
    }
}

Polyhedron *Polyhedron_splitToTriangles(Polyhedron* poly) {
    Polyhedron *result = Polyhedron_create();
    result->color = poly->color;

    for (int i = 0; i < poly->vertices.len; i++) {
        Polyhedron_addVertexEx(
            result,
            poly->vertices.head[i].position,
            poly->vertices.head[i].normal,
            poly->vertices.head[i].texCoord
        );
    }

    int max_index = 0;
    int indices[3] = {0};
    for (int i = 0; i < poly->faces.len; i++) {
        Face face = vector_get(poly->faces, i);
        for (int j = 1; j < face.vertexIndices.len - 1; j++) {
            indices[0] = vector_get(face.vertexIndices, 0);
            indices[1] = vector_get(face.vertexIndices, j);
            indices[2] = vector_get(face.vertexIndices, j + 1);
            Polyhedron_addFace(result, indices, 3);
        }
    }

    return result;
}

Vector3 Face_getCenter_World(Polyhedron* poly, Face* face, Matrix world) {
    Vector3 faceCenter = { 0, 0, 0 };
    for (size_t i = 0; i < face->vertexIndices.len; i++) {
        Vector3 worldVert = Vector3Transform(poly->vertices.head[face->vertexIndices.head[i]].position, world);
        faceCenter = Vector3Add(faceCenter, worldVert);
    }
    faceCenter = Vector3Scale(faceCenter, 1.0f / face->vertexIndices.len);
    return faceCenter;
}

bool Face_isFrontFacing(Polyhedron* poly, Face* face, CameraZ* camera, Matrix world) {
    Vector3 worldNormal = Face_calculateNormal_World(poly, face, world);

    if (camera->projection_type == PERSPECTIVE_TYPE) {
        Vector3 faceCenter = Face_getCenter_World(poly, face,world);

        Vector3 viewDirection = Vector3Normalize(Vector3Subtract(camera->position, faceCenter));

        return Vector3DotProduct(worldNormal, viewDirection) > 0;
    }
    else {
        Vector3 cameraForward = (Vector3){ -1, -1, -1 };

        float dot = Vector3DotProduct(worldNormal, cameraForward);

        return dot < 0;
    }
}

Polyhedron* Polyhedron_createTetrahedron() {
    Polyhedron* poly = Polyhedron_create();

    /*Polyhedron_addVertex(poly, (Vector3) { 1.0f, 1.0f, 1.0f });
    Polyhedron_addVertex(poly, (Vector3) { 1.0f, -1.0f, -1.0f });
    Polyhedron_addVertex(poly, (Vector3) { -1.0f, 1.0f, -1.0f });
    Polyhedron_addVertex(poly, (Vector3) { -1.0f, -1.0f, 1.0f });*/

    Vector3 positions[4] = {
        { 1.0f, 1.0f, 1.0f },
        { 1.0f, -1.0f, -1.0f },
        { -1.0f, 1.0f, -1.0f },
        { -1.0f, -1.0f, 1.0f }
    };

    for (int i = 0; i < 4; i++) {
        Vector3 pos = positions[i];
        Vector3 normal = Vector3Normalize(pos);

        float u = 0.5f + atan2f(pos.z, pos.x) / (2.0f * PI);
        float v = 0.5f - asinf(pos.y) / PI;

        Polyhedron_addVertexEx(poly, pos, normal, (Vector2) { u, v });
    }

    int faces[4][3] = {
        {0, 1, 2}, {0, 2, 3}, {0, 3, 1}, {1, 3, 2}
    };

    for (int i = 0; i < 4; i++) {
        Polyhedron_addFace(poly, faces[i], 3);
    }

    Polyhedron_updateCenter(poly);
    Polyhedron_calculateNormals(poly);
    poly->color = RED;

    return poly;
}

Polyhedron* Polyhedron_createHexahedron() {
    Polyhedron* poly = Polyhedron_create();

    /*float s = 1.0f;
    Polyhedron_addVertex(poly, (Vector3) { -s, -s, -s });
    Polyhedron_addVertex(poly, (Vector3) { s, -s, -s });
    Polyhedron_addVertex(poly, (Vector3) { s, s, -s });
    Polyhedron_addVertex(poly, (Vector3) { -s, s, -s });
    Polyhedron_addVertex(poly, (Vector3) { -s, -s, s });
    Polyhedron_addVertex(poly, (Vector3) { s, -s, s });
    Polyhedron_addVertex(poly, (Vector3) { s, s, s });
    Polyhedron_addVertex(poly, (Vector3) { -s, s, s });*/
    float s = 1.0f;
    Polyhedron_addVertexEx(poly, (Vector3) { -s, -s, -s }, (Vector3) { 0, 0, -1 }, (Vector2) { 0.0f, 0.0f });
    Polyhedron_addVertexEx(poly, (Vector3) { s, -s, -s }, (Vector3) { 0, 0, -1 }, (Vector2) { 1.0f, 0.0f });
    Polyhedron_addVertexEx(poly, (Vector3) { s, s, -s }, (Vector3) { 0, 0, -1 }, (Vector2) { 1.0f, 1.0f });
    Polyhedron_addVertexEx(poly, (Vector3) { -s, s, -s }, (Vector3) { 0, 0, -1 }, (Vector2) { 0.0f, 1.0f });

    Polyhedron_addVertexEx(poly, (Vector3) { -s, -s, s }, (Vector3) { 0, 0, 1 }, (Vector2) { 0.0f, 0.0f });
    Polyhedron_addVertexEx(poly, (Vector3) { s, -s, s }, (Vector3) { 0, 0, 1 }, (Vector2) { 1.0f, 0.0f });
    Polyhedron_addVertexEx(poly, (Vector3) { s, s, s }, (Vector3) { 0, 0, 1 }, (Vector2) { 1.0f, 1.0f });
    Polyhedron_addVertexEx(poly, (Vector3) { -s, s, s }, (Vector3) { 0, 0, 1 }, (Vector2) { 0.0f, 1.0f });


    int faces[6][4] = {
        {3, 2, 1, 0},
        { 5, 6, 7, 4},
        {1, 5, 4, 0},
        {7, 6, 2, 3},
        {4, 7, 3, 0},
        {2, 6, 5, 1}
    };

    for (int i = 0; i < 6; i++) {
        Polyhedron_addFace(poly, faces[i], 4);
    }

    Polyhedron_updateCenter(poly);
    Polyhedron_calculateNormals(poly);
    poly->color = GREEN;

    return poly;
}

Polyhedron* Polyhedron_createOctahedron() {
    Polyhedron* poly = Polyhedron_create();

    /*float s = 1.0f;
    Polyhedron_addVertex(poly, (Vector3) { s, 0.0f, 0.0f });
    Polyhedron_addVertex(poly, (Vector3) { -s, 0.0f, 0.0f });
    Polyhedron_addVertex(poly, (Vector3) { 0.0f, s, 0.0f });
    Polyhedron_addVertex(poly, (Vector3) { 0.0f, -s, 0.0f });
    Polyhedron_addVertex(poly, (Vector3) { 0.0f, 0.0f, s });
    Polyhedron_addVertex(poly, (Vector3) { 0.0f, 0.0f, -s });*/
    float s = 1.0f;
    Polyhedron_addVertexEx(poly, (Vector3) { s, 0.0f, 0.0f }, (Vector3) { 1, 0, 0 }, (Vector2) { 0.5f, 0.0f });
    Polyhedron_addVertexEx(poly, (Vector3) { -s, 0.0f, 0.0f }, (Vector3) { -1, 0, 0 }, (Vector2) { 0.5f, 1.0f });
    Polyhedron_addVertexEx(poly, (Vector3) { 0.0f, s, 0.0f }, (Vector3) { 0, 1, 0 }, (Vector2) { 1.0f, 0.5f });
    Polyhedron_addVertexEx(poly, (Vector3) { 0.0f, -s, 0.0f }, (Vector3) { 0, -1, 0 }, (Vector2) { 0.0f, 0.5f });
    Polyhedron_addVertexEx(poly, (Vector3) { 0.0f, 0.0f, s }, (Vector3) { 0, 0, 1 }, (Vector2) { 0.5f, 0.5f });
    Polyhedron_addVertexEx(poly, (Vector3) { 0.0f, 0.0f, -s }, (Vector3) { 0, 0, -1 }, (Vector2) { 0.5f, 0.5f });

    int faces[8][3] = {
        {0, 2, 4}, {0, 4, 3}, {0, 3, 5}, {0, 5, 2},
        {1, 2, 5}, {1, 5, 3}, {1, 3, 4}, {1, 4, 2}
    };

    for (int i = 0; i < 8; i++) {
        Polyhedron_addFace(poly, faces[i], 3);
    }

    Polyhedron_updateCenter(poly);
    Polyhedron_calculateNormals(poly);
    poly->color = BLUE;

    return poly;
}

Polyhedron* Polyhedron_createIcosahedron() {
    Polyhedron* poly = Polyhedron_create();

    float t = (1.0f + sqrtf(5.0f)) / 2.0f;

    Vector3 vertices[12] = {
        {-1,  t,  0}, {1,  t,  0}, {-1, -t,  0}, {1, -t,  0},
        {0, -1,  t}, {0,  1,  t}, {0, -1, -t}, {0,  1, -t},
        {t,  0, -1}, {t,  0,  1}, {-t, 0, -1}, {-t, 0,  1}
    };

    for (int i = 0; i < 12; i++) {
        vertices[i] = Vector3Normalize(vertices[i]);
        Polyhedron_addVertex(poly, vertices[i]);
    }

    int faces[20][3] = {
        {0, 11, 5}, {0, 5, 1}, {0, 1, 7}, {0, 7, 10}, {0, 10, 11},
        {1, 5, 9}, {5, 11, 4}, {11, 10, 2}, {10, 7, 6}, {7, 1, 8},
        {3, 9, 4}, {3, 4, 2}, {3, 2, 6}, {3, 6, 8}, {3, 8, 9},
        {4, 9, 5}, {2, 4, 11}, {6, 2, 10}, {8, 6, 7}, {9, 8, 1}
    };

    for (int i = 0; i < 20; i++) {
        Polyhedron_addFace(poly, faces[i], 3);
    }

    Polyhedron_updateCenter(poly);
    Polyhedron_calculateNormals(poly);
    poly->color = YELLOW;

    return poly;
}

Polyhedron* Polyhedron_createDodecahedron() {
    Polyhedron* poly = Polyhedron_create();

    float phi = (1.0f + sqrtf(5.0f)) / 2.0f; // золотое сечение ≈ 1.618

    Vector3 vertices[20] = {
        // (±1, ±1, ±1)
        { 1,  1,  1}, { 1,  1, -1}, { 1, -1,  1}, { 1, -1, -1},
        {-1,  1,  1}, {-1,  1, -1}, {-1, -1,  1}, {-1, -1, -1},

        // (0, ±1/φ, ±φ)
        {0,  1 / phi,  phi}, {0,  1 / phi, -phi}, {0, -1 / phi,  phi}, {0, -1 / phi, -phi},

        // (±1/φ, ±φ, 0)
        { 1 / phi,  phi, 0}, { 1 / phi, -phi, 0}, {-1 / phi,  phi, 0}, {-1 / phi, -phi, 0},

        // (±φ, 0, ±1/φ)
        { phi, 0,  1 / phi}, { phi, 0, -1 / phi}, {-phi, 0,  1 / phi}, {-phi, 0, -1 / phi}
    };

    for (int i = 0; i < 20; i++) {
        Vector3 normalized = Vector3Normalize(vertices[i]);
        Polyhedron_addVertex(poly, normalized);
    }

    int faces[12][5] = {
        {0, 8, 10, 2, 16},
        {0, 16, 17, 1, 12},
        {0, 12, 14, 4, 8},
        {17, 3, 11, 9, 1},
        {2, 10, 6, 15, 13},
        {13, 15, 7, 11, 3},
        {17, 16, 2, 13, 3},
        {14, 5, 19, 18, 4},
        {9, 11, 7, 19, 5},
        {18, 19, 7, 15,6},
        {12, 1, 9, 5, 14},
        {4, 18, 6, 10, 8}
    };

    for (int i = 0; i < 12; i++) {
        Polyhedron_addFace(poly, faces[i], 5);
    }

    Polyhedron_updateCenter(poly);
    Polyhedron_calculateNormals(poly);
    poly->color = PINK;

    return poly;
}

void Polyhedron_draw(Polyhedron* poly, Matrix transform) {
    if (poly->vertices.len == 0 || poly->faces.len == 0) return;

    Vector3* transformedVertices = malloc(poly->vertices.len * sizeof(Vector3));
    for (size_t i = 0; i < poly->vertices.len; i++) {
        transformedVertices[i] = Vector3Transform(poly->vertices.head[i].position, transform);
    }

    for (size_t i = 0; i < poly->faces.len; i++) {
        Face face = poly->faces.head[i];
        int vertexCount = face.vertexIndices.len;

        if (vertexCount < 3) continue;

        Vector3 first = transformedVertices[face.vertexIndices.head[0]];
        for (int j = 1; j < vertexCount - 1; j++) {
            Vector3 v2 = transformedVertices[face.vertexIndices.head[j]];
            Vector3 v3 = transformedVertices[face.vertexIndices.head[j + 1]];
            DrawTriangle((Vector2){first.x / first.z, first.y / first.z}, (Vector2){v2.x / v2.z, v2.y / v2.z}, (Vector2){v3.x / v3.z, v3.y / v3.z}, poly->color);
        }

        for (int j = 0; j < vertexCount; j++) {
            int next = (j + 1) % vertexCount;
            int idx1 = face.vertexIndices.head[j];
            int idx2 = face.vertexIndices.head[next];
            DrawLine(transformedVertices[idx1].x / transformedVertices[idx1].z, transformedVertices[idx1].y / transformedVertices[idx1].z, transformedVertices[idx2].x / transformedVertices[idx2].z, transformedVertices[idx2].y / transformedVertices[idx2].z, BLACK);
        }
    }

    /*for (size_t i = 0; i < poly->vertices.len; i++) {
        DrawSphere(transformedVertices[i], 0.03f, poly->color);
    }*/

    free(transformedVertices);
}

VECTOR_TYPE(Vector3)* Polyhedron_transform(Polyhedron* poly, Matrix transform) {
    VECTOR_TYPE(Vector3)* verts = malloc(sizeof(VECTOR_TYPE(Vector3)));
    vector_init(*verts);

    if (poly->vertices.len == 0 || poly->faces.len == 0) return verts;

    for (size_t i = 0; i < poly->vertices.len; i++) {
        vector_append(*verts, Vector3Transform(poly->vertices.head[i].position, transform));
    }

    return verts;
}

float Polyhedron_bounding_radius(Polyhedron* poly) {
    VECTOR_TYPE(Vertex) vert = poly->vertices;

    float max_dist = 0;

    for (size_t i = 0; i < vert.len; i++) {
        Vector3 pos = vector_get(vert, i).position;
        float tmp = pos.x * pos.x + pos.y * pos.y + pos.z * pos.z;
        if (tmp > max_dist) {
            max_dist = tmp;
        }
    }

    return sqrtf(max_dist);
}

// Матрица смещения
Matrix CreateTranslationMatrix(Vector3 translation) {
    return (Matrix) {
        1, 0, 0, translation.x,
        0, 1, 0, translation.y,
        0, 0, 1, translation.z,
        0, 0, 0, 1
    };
}

// Матрица масштабирования
Matrix CreateScaleMatrix(Vector3 scale) {
    return (Matrix) {
        scale.x, 0, 0, 0,
        0, scale.y, 0, 0,
        0, 0, scale.z, 0,
        0, 0, 0, 1
    };
}

// Матрица поворота по X
Matrix CreateRotationX(float angle) {
    float cosA = cosf(angle);
    float sinA = sinf(angle);
    return (Matrix) {
        1, 0, 0, 0,
        0, cosA, -sinA, 0,
        0, sinA, cosA, 0,
        0, 0, 0, 1
    };
}

// Матрица поворота по Y
Matrix CreateRotationY(float angle) {
    float cosA = cosf(angle);
    float sinA = sinf(angle);
    return (Matrix) {
        cosA, 0, sinA, 0,
        0, 1, 0, 0,
        -sinA, 0, cosA, 0,
        0, 0, 0, 1
    };
}

// Матрица поворота по Z
Matrix CreateRotationZ(float angle) {
    float cosA = cosf(angle);
    float sinA = sinf(angle);
    return (Matrix) {
        cosA, -sinA, 0, 0,
        sinA, cosA, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
}

Matrix CreateReflectionMatrix(char plane) {
    switch (plane) {
        case 'X': // x -> -x
            return (Matrix){
                -1, 0, 0, 0,
                 0, 1, 0, 0,
                 0, 0, 1, 0,
                 0, 0, 0, 1
            };
        case 'Y': // y -> -y
            return (Matrix){
                1, 0, 0, 0,
                0,-1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1
           };
        case 'Z': // z -> -z
            return (Matrix){
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0,-1, 0,
                0, 0, 0, 1
           };
        default:
            return MatrixIdentity();
    }
}

Matrix CreateRotationAroundLine(Vector3 p1, Vector3 p2, float angle) {
    Vector3 axis = Vector3Normalize(Vector3Subtract(p2, p1));

    Matrix T1 = MatrixTranslate(-p1.x, -p1.y, -p1.z);
    Matrix R = MatrixRotate(axis, angle);
    Matrix T2 = MatrixTranslate(p1.x, p1.y, p1.z);

    return MatrixMultiply(MatrixMultiply(T1, R), T2);
}

Matrix CreateTransformMatrix(Polyhedron* poly, Vector3 translation, Vector3 rotation_angles, Vector3 scale, char reflection_plane, Vector3 line_p1, Vector3 line_p2, float line_angle) {
    Matrix transform = MatrixIdentity();

    if (reflection_plane != 0) {
        Matrix refl = CreateReflectionMatrix(reflection_plane);
        transform = MatrixMultiply(refl, transform);
    }
    Matrix toOrigin = CreateTranslationMatrix(Vector3Negate(poly->center));
    Matrix fromOrigin = CreateTranslationMatrix(poly->center);

    Matrix scaleM = CreateScaleMatrix(scale);
    transform = MatrixMultiply(transform, MatrixMultiply(MatrixMultiply(toOrigin, scaleM), fromOrigin));

    transform = MatrixMultiply(transform, CreateRotationX(rotation_angles.x * DEG2RAD));
    transform = MatrixMultiply(transform, CreateRotationY(rotation_angles.y * DEG2RAD));
    transform = MatrixMultiply(transform, CreateRotationZ(rotation_angles.z * DEG2RAD));

    if (!Vector3Equals(line_p1,line_p2)) transform = MatrixMultiply(transform, CreateRotationAroundLine(line_p1,line_p2,line_angle));

    transform = MatrixMultiply(transform, CreateTranslationMatrix(translation));

    return transform;
}

bool Polyhedron_loadFromObj(Polyhedron* poly, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) { return false; }

    char line[256];

    Vector3* tempPositions = NULL;
    Vector2* tempTexCoords = NULL;
    Vector3* tempNormals = NULL;

    size_t tempPositionsCount = 0;
    size_t tempTexCoordsCount = 0;
    size_t tempNormalsCount = 0;

    size_t tempPositionsCapacity = 0;
    size_t tempTexCoordsCapacity = 0;
    size_t tempNormalsCapacity = 0;

    typedef struct {
        int* v_indices;
        int* vt_indices;
        int* vn_indices;
        size_t count;
    } TempFace;

    TempFace* tempFaces = NULL;
    size_t tempFacesCount = 0;
    size_t tempFacesCapacity = 0;

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '\n') continue;

        // Вершины (v)
        if (line[0] == 'v' && line[1] == ' ') {
            Vector3 pos;
            if (sscanf(line, "v %f %f %f", &pos.x, &pos.y, &pos.z) == 3) {
                if (tempPositionsCount >= tempPositionsCapacity) {
                    tempPositionsCapacity = tempPositionsCapacity == 0 ? 16 : tempPositionsCapacity * 2;
                    tempPositions = realloc(tempPositions, tempPositionsCapacity * sizeof(Vector3));
                }
                tempPositions[tempPositionsCount++] = pos;
            }
        }
        // Текстурные координаты (vt)
        else if (line[0] == 'v' && line[1] == 't' && line[2] == ' ') {
            Vector2 texCoord;
            float dummy;
            int components = sscanf(line, "vt %f %f %f", &texCoord.x, &texCoord.y, &dummy);

            if (components >= 2) { 
                if (tempTexCoordsCount >= tempTexCoordsCapacity) {
                    tempTexCoordsCapacity = tempTexCoordsCapacity == 0 ? 16 : tempTexCoordsCapacity * 2;
                    tempTexCoords = realloc(tempTexCoords, tempTexCoordsCapacity * sizeof(Vector2));
                }
                tempTexCoords[tempTexCoordsCount++] = texCoord;
            }
        }
        // Нормали (vn)
        else if (line[0] == 'v' && line[1] == 'n' && line[2] == ' ') {
            Vector3 normal;
            if (sscanf(line, "vn %f %f %f", &normal.x, &normal.y, &normal.z) == 3) {
                if (tempNormalsCount >= tempNormalsCapacity) {
                    tempNormalsCapacity = tempNormalsCapacity == 0 ? 16 : tempNormalsCapacity * 2;
                    tempNormals = realloc(tempNormals, tempNormalsCapacity * sizeof(Vector3));
                }
                tempNormals[tempNormalsCount++] = normal;
            }
        }
        // Грани (f)
        else if (line[0] == 'f' && line[1] == ' ') {
            if (tempFacesCount >= tempFacesCapacity) {
                tempFacesCapacity = tempFacesCapacity == 0 ? 16 : tempFacesCapacity * 2;
                tempFaces = realloc(tempFaces, tempFacesCapacity * sizeof(TempFace));
            }

            TempFace* face = &tempFaces[tempFacesCount];
            face->v_indices = NULL;
            face->vt_indices = NULL;
            face->vn_indices = NULL;
            face->count = 0;

            size_t faceCapacity = 0;

            char* token = strtok(line + 2, " \t\n");
            while (token) {
                int v = -1, vt = -1, vn = -1;

                if (sscanf(token, "%d/%d/%d", &v, &vt, &vn) == 3) {
                    // v/vt/vn 
                }
                else if (sscanf(token, "%d//%d", &v, &vn) == 2) {
                    vt = -1; 
                }
                else if (sscanf(token, "%d/%d", &v, &vt) == 2) {
                    vn = -1; 
                }
                else if (sscanf(token, "%d", &v) == 1) {
                    vt = -1;  
                    vn = -1;  
                }

                if (v != -1) {
                    if (v < 0) v = tempPositionsCount + v + 1;
                    if (vt < 0) vt = tempTexCoordsCount + vt + 1;
                    if (vn < 0) vn = tempNormalsCount + vn + 1;

                    if (face->count >= faceCapacity) {
                        faceCapacity = faceCapacity == 0 ? 4 : faceCapacity * 2;
                        face->v_indices = realloc(face->v_indices, faceCapacity * sizeof(int));
                        face->vt_indices = realloc(face->vt_indices, faceCapacity * sizeof(int));
                        face->vn_indices = realloc(face->vn_indices, faceCapacity * sizeof(int));
                    }

                    face->v_indices[face->count] = v;
                    face->vt_indices[face->count] = vt;
                    face->vn_indices[face->count] = vn;
                    face->count++;
                }

                token = strtok(NULL, " \t\n");
            }

            if (face->count >= 3) {
                tempFacesCount++;
            }
            else {
                free(face->v_indices);
                free(face->vt_indices);
                free(face->vn_indices);
            }
        }
    }
    fclose(file);

    for (size_t f = 0; f < tempFacesCount; f++) {
        TempFace* tempFace = &tempFaces[f];
        Face face;
        vector_init(face.vertexIndices);

        for (size_t v = 0; v < tempFace->count; v++) {
            Vertex vertex = { 0 };

            int v_idx = tempFace->v_indices[v];
            if (v_idx > 0 && v_idx <= (int)tempPositionsCount) {
                vertex.position = tempPositions[v_idx - 1];
            }

            int vt_idx = tempFace->vt_indices[v];
            if (vt_idx > 0 && vt_idx <= (int)tempTexCoordsCount) {
                vertex.texCoord = tempTexCoords[vt_idx - 1];
            }
            else {
                vertex.texCoord = (Vector2){ 0, 0 };
            }

            int vn_idx = tempFace->vn_indices[v];
            if (vn_idx > 0 && vn_idx <= (int)tempNormalsCount) {
                vertex.normal = tempNormals[vn_idx - 1];
            }
            else {
                vertex.normal = (Vector3){ 0, 0, 0 };
            }

            vector_append(poly->vertices, vertex);
            vector_append(face.vertexIndices, poly->vertices.len - 1);
        }

        if (face.vertexIndices.len >= 3) {
            vector_append(poly->faces, face);
        }
        else {
            vector_free(face.vertexIndices);
        }
    }

    if (tempNormalsCount == 0) {
        Polyhedron_calculateNormals(poly);
    }

    poly->color = WHITE;

    free(tempPositions);
    free(tempTexCoords);
    free(tempNormals);

    for (size_t i = 0; i < tempFacesCount; i++) {
        free(tempFaces[i].v_indices);
        free(tempFaces[i].vt_indices);
        free(tempFaces[i].vn_indices);
    }
    free(tempFaces);

    return (poly->vertices.len > 0 && poly->faces.len > 0);
}

bool Polyhedron_saveToObj(Polyhedron* poly, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) { return false; }

    for (size_t i = 0; i < poly->vertices.len; i++) {
        Vertex v = poly->vertices.head[i];
        fprintf(file, "v %.6f %.6f %.6f\n", v.position.x, v.position.y, v.position.z);
    }

    for (size_t i = 0; i < poly->vertices.len; i++) {
        Vertex v = poly->vertices.head[i];
        fprintf(file, "vn %.6f %.6f %.6f\n", v.normal.x, v.normal.y, v.normal.z);
    }

    for (size_t i = 0; i < poly->vertices.len; i++) {
        Vertex v = poly->vertices.head[i];
        fprintf(file, "vt %.6f %.6f\n", v.texCoord.x, v.texCoord.y);
    }

    for (size_t i = 0; i < poly->faces.len; i++) {
        Face face = poly->faces.head[i];
        fprintf(file, "f");

        for (size_t j = 0; j < face.vertexIndices.len; j++) {
            int idx = face.vertexIndices.head[j] + 1;
            fprintf(file, " %d/%d/%d", idx, idx, idx);
        }
        fprintf(file, "\n");
    }

    fclose(file);
    return true;
}

TextureZ* Texture_sh() {
    TextureZ* tex = malloc(sizeof(TextureZ));
    if (!tex) return NULL;

    tex->width = 64;
    tex->height = 64;
    tex->pixels = malloc(tex->width * tex->height * sizeof(unsigned int));

    if (!tex->pixels) {
        free(tex);
        return NULL;
    }

    for (int y = 0; y < tex->height; y++) {
        for (int x = 0; x < tex->width; x++) {
            if ((x / 8 + y / 8) % 2 == 0) {
                tex->pixels[y * tex->width + x] = 0xFFFF0000;
            }
            else {
                tex->pixels[y * tex->width + x] = 0xFFFFFF00; 
            }
        }
    }
    return tex;
}

Color Texture_sample(TextureZ* tex, float u, float v) {
    if (!tex || !tex->pixels) {
        return WHITE;
    }

    u = fmodf(u, 1.0f);
    v = fmodf(v, 1.0f);
    if (u < 0) u += 1.0f;
    if (v < 0) v += 1.0f;

    int x = (int)(u * (tex->width - 1) + 0.5f);
    int y = (int)(v * (tex->height - 1) + 0.5f);

    x = Clamp(x, 0, tex->width - 1);
    y = Clamp(y, 0, tex->height - 1);

    unsigned int pixel = tex->pixels[y * tex->width + x];

    return (Color) {
        .a = (pixel >> 24) & 0xFF,  
            .r = (pixel >> 16) & 0xFF,  
            .g = (pixel >> 8) & 0xFF,     
            .b = pixel & 0xFF          
    };
}

TextureZ* load_texture_from_file(const char* filename) {
    Image image = LoadImage(filename);
    if (image.data == NULL) {
        printf("Failed to load texture: %s\n", filename);
        return NULL;
    }

    TextureZ* tex = malloc(sizeof(TextureZ));

    tex->width = image.width;
    tex->height = image.height;
    tex->pixels = malloc(tex->width * tex->height * sizeof(unsigned int));

    for (int y = 0; y < tex->height; y++) {
        for (int x = 0; x < tex->width; x++) {
            Color pixel = GetImageColor(image, x, y);
            tex->pixels[y * tex->width + x] =
                (pixel.a << 24) | (pixel.r << 16) | (pixel.g << 8) | pixel.b;
        }
    }

    UnloadImage(image);
    return tex;
}
