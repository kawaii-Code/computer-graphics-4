#include "scene.h"
#include "Polyhedron.h"

void scene_obj_free(SceneObject* obj) {
    free(obj);
}

SceneObject* scene_obj_create(Polyhedron* mesh, size_t render_layer, bool visible, Vector3 position, Vector3 rotation, Vector3 scale) {
    SceneObject* scene = calloc(1, sizeof(SceneObject));
    scene->mesh = mesh;
    scene->render_layer = render_layer;
    scene->visible = visible;
    scene->position = position;
    scene->rotation = rotation;
    scene->scale = scale;
    scene->bounding_radius = Polyhedron_bounding_radius(mesh);

    return scene;
}

void scene_obj_draw(Scene* scene, SceneObject* obj) {
    Matrix worldMatrix = CreateTransformMatrix(obj->mesh, obj->position, obj->rotation, obj->scale, obj->reflection_plane, obj->line_p1, obj->line_p2, obj->line_angle);

    VECTOR_TYPE(Vector3)* worldVerts = Polyhedron_transform(obj->mesh, worldMatrix);

    for (size_t f = 0; f < obj->mesh->faces.len; f++) {
        Face* face = &obj->mesh->faces.head[f];
        VECTOR_TYPE(int) indices = face->vertexIndices;

        if (indices.len < 3) {
            continue;
        }

        if (!Face_isFrontFacing(obj->mesh, face, scene->camera)) {
            continue; 
        }

        Vector2 *screenVerts = calloc(1, sizeof(Vector2) * indices.len);
        for (size_t v = 0; v < indices.len; v++) {
            Vector3 worldVert = worldVerts->head[indices.head[v]];
            screenVerts[v] = cameraz_world_to_screen(worldVert, scene->camera);
        }

        DrawTriangleFan(screenVerts, indices.len, obj->mesh->color);

        for (size_t v = 0; v < indices.len; v++) {
            int next = (v + 1) % indices.len;
            DrawLineV(screenVerts[v], screenVerts[next], BLACK);
        }
    }

    vector_free(*worldVerts);
    free(worldVerts);
}

void scene_init(Scene* scene) {

}

void scene_free(Scene* scene) {
    vector_free(scene->objs);
    free(scene);
}

Scene* scene_create(CameraZ* camera) {
    Scene* scene = calloc(1, sizeof(Scene));
    scene->camera = camera;

    return scene;
}

void draw_coordinate_axes(CameraZ* camera) {
    Vector2 origin = cameraz_world_to_screen((Vector3){0, 0, 0}, camera);
    Vector2 x_axis = cameraz_world_to_screen((Vector3){5, 0, 0}, camera);
    Vector2 y_axis = cameraz_world_to_screen((Vector3){0, 5, 0}, camera);
    Vector2 z_axis = cameraz_world_to_screen((Vector3){0, 0, 5}, camera);

    DrawLineV(origin, x_axis, RED);
    DrawLineV(origin, y_axis, GREEN);
    DrawLineV(origin, z_axis, BLUE);

    DrawText("X", x_axis.x + 5, x_axis.y, 10, RED);
    DrawText("Y", y_axis.x, y_axis.y - 15, 10, GREEN);
    DrawText("Z", z_axis.x + 5, z_axis.y, 10, BLUE);
}

void scene_draw(Scene* scene) {
    VECTOR_PTR_TYPE(SceneObject) objs = scene->objs;
    CameraZ* camera = scene->camera;

    draw_coordinate_axes(camera);

    for (size_t i = 0; i < objs.len; i++) {
        SceneObject* obj = vector_get(objs, i);
        if (!obj->visible) continue;

        float distance = Vector3Distance(obj->position, camera->position);
        if (distance < camera->nearPlane || distance > camera->farPlane) {
            continue;
        }

        scene_obj_draw(scene, obj);
    }
}

void scene_update(Scene* scene) {

}

void scene_add_obj(Scene* scene, SceneObject* obj) {
    VECTOR_PTR_TYPE(SceneObject)* objs = &scene->objs;

    vector_append(*objs, obj);
}

void scene_remove_obj(Scene* scene, size_t index) {

}
