#include "math.h"
#include "linalg.h"

Vector3 vec3_add(Vector3 a, Vector3 b) {
    Vector3 result;

    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;

    return result;
}

Vector3 vec3_negate(Vector3 a) {
    Vector3 result;

    result.x = -a.x;
    result.y = -a.y;
    result.z = -a.z;

    return result;
}

float vec3_dot(Vector3 a, Vector3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vector3 vec3_multiply(Vector3 a, float b) {
    Vector3 result;

    result.x = a.x * b;
    result.y = a.y * b;
    result.z = a.z * b;

    return result;
}

Vector3 vec3_cross(Vector3 a, Vector3 b) {
    Vector3 result = {0};

    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;

    return result;
}

Vector3 vec3_normalize(Vector3 v) {
    float length = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    if (length == 0.0f) {
        length = 1.0f;
    }

    float ilength = 1.0f / length;
    v.x *= ilength;
    v.y *= ilength;
    v.z *= ilength;

    return v;
}

Matrix4x4 mat4_identity() {
    return (Matrix4x4) {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
    };
}

Matrix4x4 mat4_add(Matrix4x4 a, Matrix4x4 b) {
    Matrix4x4 result;

    for (int i = 0; i < 16; i++) {
        result.m[i] = a.m[i] + b.m[i];
    }

    return result;
}

// Vector4 представляется как столбец, не как строка!
Vector4 mat4_multiply_by_vec4(Matrix4x4 a, Vector4 b) {
    Vector4 result = {0};

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result.v[i] += a.m[4 * i + j] * b.v[j];
        }
    }

    return result;
}

Matrix4x4 mat4_multiply(Matrix4x4 a, Matrix4x4 b) {
    Matrix4x4 result = {0};

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                result.m[4 * i + j] += a.m[4 * i + k] * b.m[4 * k + j];
            }
        }
    }

    return result;
}

Matrix4x4 mat4_translation(Vector3 translation) {
    return (Matrix4x4) {
        1, 0, 0, translation.x,
        0, 1, 0, translation.y,
        0, 0, 1, translation.z,
        0, 0, 0, 1
    };
}

Matrix4x4 mat4_scale(Vector3 scale) {
    return (Matrix4x4) {
        scale.x, 0, 0, 0,
        0, scale.y, 0, 0,
        0, 0, scale.z, 0,
        0, 0, 0, 1
    };
}

Matrix4x4 mat4_rotation_x(float angle) {
    float cosA = cosf(angle);
    float sinA = sinf(angle);

    return (Matrix4x4) {
        1, 0, 0, 0,
        0, cosA, -sinA, 0,
        0, sinA, cosA, 0,
        0, 0, 0, 1
    };
}

Matrix4x4 mat4_rotation_y(float angle) {
    float cosA = cosf(angle);
    float sinA = sinf(angle);

    return (Matrix4x4) {
        cosA, 0, sinA, 0,
        0, 1, 0, 0,
        -sinA, 0, cosA, 0,
        0, 0, 0, 1
    };
}

Matrix4x4 mat4_rotation_z(float angle) {
    float cosA = cosf(angle);
    float sinA = sinf(angle);

    return (Matrix4x4) {
        cosA, -sinA, 0, 0,
        sinA, cosA, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
}

Matrix4x4 mat4_world(Vector3 translation, Vector3 rotation, Vector3 scale) {
    Matrix4x4 rx = mat4_rotation_x(rotation.x);
    Matrix4x4 ry = mat4_rotation_y(rotation.y);
    Matrix4x4 rz = mat4_rotation_z(rotation.z);

    Matrix4x4 t = mat4_translation(translation);
    Matrix4x4 r = mat4_multiply(mat4_multiply(rz, ry), rx);
    Matrix4x4 s = mat4_scale(scale);

    return mat4_multiply(mat4_multiply(t, r), s);
}

Matrix4x4 mat4_look_at(Vector3 eye, Vector3 target, Vector3 world_up) {
    Vector3 forward = {
        .x = eye.x - target.x,
        .y = eye.y - target.y,
        .z = eye.z - target.z,
    };
    forward = vec3_normalize(forward);

    Vector3 right = vec3_cross(world_up, forward);
    right = vec3_normalize(right);

    Vector3 up = vec3_cross(forward, right);
    up = vec3_normalize(up);

    float re = -1.0f * vec3_dot(right, eye);
    float ue = -1.0f * vec3_dot(up, eye);
    float fe = -1.0f * vec3_dot(forward, eye);

    return (Matrix4x4) {
        right.x, up.x, forward.x, 0.0f,
        right.y, up.y, forward.y, 0.0f,
        right.z, up.z, forward.z, 0.0f,
        re,      ue,   fe,        1.0f,
    };
}

Matrix4x4 mat4_perspective(float fovy, float aspect, float near, float far) {
    Matrix4x4 result = {0};

    float f = 1.0f / tanf(fovy * 0.5f);
    float fn = 1.0f / (near - far);

    result.m[0] = f / aspect;
    result.m[5] = f;
    result.m[10] = (near + far) * fn;
    result.m[11] = -1.0f;
    result.m[14] = 2.0f * near * far * fn;

    //float top = 1.0f / tanf(fovy * 0.5f);
    //float right = top * aspect;
    //float bottom = -top;
    //float left = -right;

    //float rl = right - left;
    //float tb = top - bottom;
    //float fn = far - near;

    //result.m[0] = (2.0f * near) / rl;
    //result.m[5] = (2.0f * near) / tb;
    //result.m[8] = (right + left) / rl;
    //result.m[9] = (top + bottom) / tb;
    //result.m[10] = -(far + near) / fn;
    //result.m[11] = -1.0f;
    //result.m[14] = -(2.0f * far * near) / fn;

    return result;
}
