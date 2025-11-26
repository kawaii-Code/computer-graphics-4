#include "camera.h"

#include <stdlib.h>

#include "scene.h"
#include "third_party/include/raymath.h"

void cameraz_init(CameraZ *camera) {

}

void cameraz_free(CameraZ *camera) {
    free(camera);
}

CameraZ* cameraz_create(Vector3 position, Vector3 target, Vector3 up, float fovy, float aspect, float zoom, float nearPlane, float farPlane, int width, int height, CameraProjectionType projection_type) {
    CameraZ *camera = malloc(sizeof(CameraZ));
    camera->position = position;
    camera->target = target;
    camera->up = up;

    camera->fovy = fovy;
    camera->aspect = aspect;
    camera->zoom = zoom;
    camera->nearPlane = nearPlane;
    camera->farPlane = farPlane;
    camera->projection_type = projection_type;

    camera->width = width;
    camera->height = height;

    camera->view_matrix = MatrixIdentity();
    camera->projection_matrix = MatrixIdentity();

    cameraz_update(camera);
    return camera;
}

Matrix MatrixLookAtIsometric(Vector3 target, float distance, float zoom) {
    // Изометрические углы: 45° по горизонтали и 35.264° по вертикали
    const float angleX = 35.264f * DEG2RAD;  // arctan(1/sqrt(2)) квадрат
    const float angleY = 45.0f * DEG2RAD;

    Vector3 position = {
        target.x + distance * cosf(angleY) * cosf(angleX) / zoom,
        target.y + distance * sinf(angleX) / zoom,
        target.z + distance * sinf(angleY) * cosf(angleX) / zoom
    };

    Vector3 up = {0.0f, 1.0f, 0.0f};

    return MatrixLookAt(position, target, up);
}

void cameraz_update(CameraZ *camera) {
    if (camera->projection_type == PERSPECTIVE_TYPE) {
        camera->view_matrix = MatrixLookAt(camera->position, camera->target, camera->up);
        camera->projection_matrix = MatrixPerspective(camera->fovy * DEG2RAD / camera->zoom, camera->aspect, camera->nearPlane, camera->farPlane);
    }
    else if (camera->projection_type == ISOMETRIC_TYPE) {
        float distance = Vector3Distance(camera->position, camera->target);
        float size = distance / camera->zoom;

        camera->view_matrix = MatrixLookAtIsometric(camera->target, distance, camera->zoom);
        camera->projection_matrix = MatrixOrtho(-size * camera->aspect, size * camera->aspect, -size, size, camera->nearPlane, camera->farPlane);
    }
}

// void cameraz_render(CameraZ *camera, SceneObject *sceneObject) {
//
// }

Vector3 cameraz_world_to_screen(Vector3 worldPos, const CameraZ* camera) {
    Vector4 clipPos = {0};

    Vector4 viewPos = {
        worldPos.x * camera->view_matrix.m0 + worldPos.y * camera->view_matrix.m4 + worldPos.z * camera->view_matrix.m8 + camera->view_matrix.m12,
        worldPos.x * camera->view_matrix.m1 + worldPos.y * camera->view_matrix.m5 + worldPos.z * camera->view_matrix.m9 + camera->view_matrix.m13,
        worldPos.x * camera->view_matrix.m2 + worldPos.y * camera->view_matrix.m6 + worldPos.z * camera->view_matrix.m10 + camera->view_matrix.m14,
        worldPos.x * camera->view_matrix.m3 + worldPos.y * camera->view_matrix.m7 + worldPos.z * camera->view_matrix.m11 + camera->view_matrix.m15
    };

    clipPos.x = viewPos.x * camera->projection_matrix.m0 + viewPos.y * camera->projection_matrix.m4 + viewPos.z * camera->projection_matrix.m8 + viewPos.w * camera->projection_matrix.m12;
    clipPos.y = viewPos.x * camera->projection_matrix.m1 + viewPos.y * camera->projection_matrix.m5 + viewPos.z * camera->projection_matrix.m9 + viewPos.w * camera->projection_matrix.m13;
    clipPos.z = viewPos.x * camera->projection_matrix.m2 + viewPos.y * camera->projection_matrix.m6 + viewPos.z * camera->projection_matrix.m10 + viewPos.w * camera->projection_matrix.m14;
    clipPos.w = viewPos.x * camera->projection_matrix.m3 + viewPos.y * camera->projection_matrix.m7 + viewPos.z * camera->projection_matrix.m11 + viewPos.w * camera->projection_matrix.m15;

    if (clipPos.w != 0.0f) {
        clipPos.x /= clipPos.w;
        clipPos.y /= clipPos.w;
        clipPos.z /= clipPos.w;
    }

    Vector3 screenPos = {
        (clipPos.x + 1.0f) * 0.5f * camera->width,
        (1.0f - clipPos.y) * 0.5f * camera->height,
        clipPos.z,
    };

    return screenPos;
}
