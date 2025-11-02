#include "polyhedron.h"

static vector(Vector3) originalPositions;

void Polyhedron_init(Polyhedron* poly) {
    vector_init(poly->vertices);
    vector_init(poly->faces);
    poly->color = WHITE;
    poly->center = (Vector3){ 0, 0, 0 };
    vector_init(originalPositions);
}

void Polyhedron_free(Polyhedron* poly) {
    for (size_t i = 0; i < poly->faces.len; i++) {
        vector_free(poly->faces.head[i].vertexIndices);
    }
    vector_free(poly->faces);
    vector_free(poly->vertices);
    vector_free(originalPositions);
}

void Polyhedron_addVertex(Polyhedron* poly, Vector3 position) {
    Vertex v = { position };
    vector_append(poly->vertices, v);
    v.normal = (Vector3){ 0, 0, 0 };
    vector_append(originalPositions, position);
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


void Polyhedron_createTetrahedron(Polyhedron* poly) {
    Polyhedron_init(poly);

    Polyhedron_addVertex(poly, (Vector3) { 1.0f, 1.0f, 1.0f });
    Polyhedron_addVertex(poly, (Vector3) { 1.0f, -1.0f, -1.0f });
    Polyhedron_addVertex(poly, (Vector3) { -1.0f, 1.0f, -1.0f });
    Polyhedron_addVertex(poly, (Vector3) { -1.0f, -1.0f, 1.0f });

    int faces[4][3] = {
        {0, 1, 2}, {0, 2, 3}, {0, 3, 1}, {1, 3, 2}
    };

    for (int i = 0; i < 4; i++) {
        Polyhedron_addFace(poly, faces[i], 3);
    }

    Polyhedron_updateCenter(poly);
    poly->color = RED;
}

void Polyhedron_createHexahedron(Polyhedron* poly) {
    Polyhedron_init(poly);

    float s = 1.0f;
    Polyhedron_addVertex(poly, (Vector3) { -s, -s, -s });
    Polyhedron_addVertex(poly, (Vector3) { s, -s, -s });
    Polyhedron_addVertex(poly, (Vector3) { s, s, -s });
    Polyhedron_addVertex(poly, (Vector3) { -s, s, -s });
    Polyhedron_addVertex(poly, (Vector3) { -s, -s, s });
    Polyhedron_addVertex(poly, (Vector3) { s, -s, s });
    Polyhedron_addVertex(poly, (Vector3) { s, s, s });
    Polyhedron_addVertex(poly, (Vector3) { -s, s, s });

    int faces[6][4] = {
        {0, 1, 2, 3},
        {4, 7, 6, 5}, 
        {0, 4, 5, 1}, 
        {3, 2, 6, 7}, 
        {0, 3, 7, 4}, 
        {1, 5, 6, 2} 
    };

    for (int i = 0; i < 6; i++) {
        Polyhedron_addFace(poly, faces[i], 4);
    }

    Polyhedron_updateCenter(poly);
    poly->color = GREEN;
}

void Polyhedron_createOctahedron(Polyhedron* poly) {
    Polyhedron_init(poly);

    float s = 1.0f;
    Polyhedron_addVertex(poly, (Vector3) { s, 0.0f, 0.0f });
    Polyhedron_addVertex(poly, (Vector3) { -s, 0.0f, 0.0f });
    Polyhedron_addVertex(poly, (Vector3) { 0.0f, s, 0.0f });
    Polyhedron_addVertex(poly, (Vector3) { 0.0f, -s, 0.0f });
    Polyhedron_addVertex(poly, (Vector3) { 0.0f, 0.0f, s });
    Polyhedron_addVertex(poly, (Vector3) { 0.0f, 0.0f, -s });

    int faces[8][3] = {
        {0, 2, 4}, {0, 4, 3}, {0, 3, 5}, {0, 5, 2},
        {1, 2, 5}, {1, 5, 3}, {1, 3, 4}, {1, 4, 2}
    };

    for (int i = 0; i < 8; i++) {
        Polyhedron_addFace(poly, faces[i], 3);
    }

    Polyhedron_updateCenter(poly);
    poly->color = BLUE;
}

void Polyhedron_createIcosahedron(Polyhedron* poly) {
    Polyhedron_init(poly);

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
    poly->color = YELLOW;
}

void Polyhedron_createDodecahedron(Polyhedron* poly) {
    Polyhedron_init(poly);

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
    poly->color = PINK;
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
            DrawTriangle3D(first, v2, v3, poly->color);
        }

        for (int j = 0; j < vertexCount; j++) {
            int next = (j + 1) % vertexCount;
            int idx1 = face.vertexIndices.head[j];
            int idx2 = face.vertexIndices.head[next];
            DrawLine3D(transformedVertices[idx1], transformedVertices[idx2], BLACK);
        }
    }

    for (size_t i = 0; i < poly->vertices.len; i++) {
        DrawSphere(transformedVertices[i], 0.03f, poly->color);
    }

    free(transformedVertices);
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
    Vector3 axis = Vector3Normalize(Vector3Subtract(p2,p1));
    float x = axis.x, y = axis.y, z = axis.z;
    float c = cosf(angle), s = sinf(angle);
    Matrix R = {
        c+x*x*(1-c), x*y*(1-c)-z*s, x*z*(1-c)+y*s, 0,
        y*x*(1-c)+z*s, c+y*y*(1-c), y*z*(1-c)-x*s, 0,
        z*x*(1-c)-y*s, z*y*(1-c)+x*s, c+z*z*(1-c), 0,
        0,0,0,1
    };
    Matrix T1 = CreateTranslationMatrix((Vector3){-p1.x,-p1.y,-p1.z});
    Matrix T2 = CreateTranslationMatrix(p1);
    return MatrixMultiply(MatrixMultiply(T2,R),T1);
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

    transform = MatrixMultiply(transform, CreateRotationX(rotation_angles.x*DEG2RAD));
    transform = MatrixMultiply(transform, CreateRotationY(rotation_angles.y*DEG2RAD));
    transform = MatrixMultiply(transform, CreateRotationZ(rotation_angles.z*DEG2RAD));

    if (!Vector3Equals(line_p1,line_p2)) transform = MatrixMultiply(transform, CreateRotationAroundLine(line_p1,line_p2,line_angle));

    transform = MatrixMultiply(transform, CreateTranslationMatrix(translation));
    return transform;
}