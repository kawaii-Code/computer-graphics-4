#ifndef CAMERA_H
#define CAMERA_H

#include "third_party/include/raylib.h"

typedef enum {
    PERSPECTIVE_TYPE = 0,
    ISOMETRIC_TYPE
} CameraProjectionType;

typedef struct {
    Vector3 position;
    Vector3 target;
    Vector3 up;

    float fovy;
    float aspect;
    float zoom;
    float nearPlane;
    float farPlane;
    CameraProjectionType projection_type;

    int width;
    int height;

    Matrix view_matrix;
    Matrix projection_matrix;
} CameraZ;

void cameraz_init(CameraZ *camera);
void cameraz_free(CameraZ *camera);

CameraZ* cameraz_create(Vector3 position, Vector3 target, Vector3 up, float fovy, float aspect, float zoom, float nearPlane, float farPlane, int width, int height, CameraProjectionType projection_type);

void cameraz_update(CameraZ *camera);
Vector3 cameraz_world_to_screen(Vector3 worldPos, const CameraZ* camera);

#endif //CAMERA_H
