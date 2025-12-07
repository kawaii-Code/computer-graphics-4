#ifndef LINALG_H_
#define LINALG_H_


#define PI 3.141592653589793
#define DEG2RAD (2 * PI / 360.0f)

typedef struct {
    float x;
    float y;
} Vector2;

typedef struct {
    float x;
    float y;
    float z;
} Vector3;

typedef union {
    struct {
        float x;
        float y;
        float z;
        float w;
    } Vector4AsCoords;

    float v[4];
} Vector4;

//
// Матрица хранится построчно, т.е.
//
// m[0],  m[1],  m[2],  m[3]
// m[4],  m[5],  m[6],  m[7]
// m[8],  m[9],  m[10], m[11]
// m[12], m[13], m[14], m[15]
//
typedef struct {
    float m[16];
} Matrix4x4;

Vector3 vec3_add(Vector3 a, Vector3 b);
Vector3 vec3_subtract(Vector3 a, Vector3 b);
Vector3 vec3_multiply(Vector3 a, float b);
float   vec3_dot(Vector3 a, Vector3 b);
float   vec3_length(Vector3 v);
Vector3 vec3_cross(Vector3 a, Vector3 b);
Vector3 vec3_normalize(Vector3 v);
Vector3 vec3_negate(Vector3 a);

Matrix4x4 mat4_identity();
Matrix4x4 mat4_add(Matrix4x4 a, Matrix4x4 b);
Vector4   mat4_multiply_by_vec4(Matrix4x4 a, Vector4 b);
Matrix4x4 mat4_multiply(Matrix4x4 a, Matrix4x4 b);
Matrix4x4 mat4_translation(Vector3 translation);
Matrix4x4 mat4_scale(Vector3 scale);
Matrix4x4 mat4_rotation_x(float angle);
Matrix4x4 mat4_rotation_y(float angle);
Matrix4x4 mat4_rotation_z(float angle);
Matrix4x4 mat4_world(Vector3 translation, Vector3 rotation, Vector3 scale);
Matrix4x4 mat4_look_at(Vector3 eye, Vector3 target, Vector3 up);
Matrix4x4 mat4_perspective(float fov, float aspect, float near, float far);

#endif // LINALG_H_
