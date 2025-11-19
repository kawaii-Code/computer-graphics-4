#ifndef SCENE_H
#define SCENE_H

#include "Polyhedron.h"
#include "vector.h"
#include "camera.h"

// TODO
// Сделать для любых объектов и определить эти самые объекты
// Перенести смещение и все действия над объектами сюда
typedef struct {
     Polyhedron* mesh;

     size_t render_layer;
     bool visible;
     Vector3 position;
     Vector3 rotation;
     Vector3 scale;

     char reflection_plane;
     Vector3 line_p1;
     Vector3 line_p2;
     float line_angle;

     float bounding_radius;

     TextureZ* texture; 
     bool has_texture;
} SceneObject;

void scene_obj_free(SceneObject* obj);

SceneObject* scene_obj_create(Polyhedron* mesh, size_t render_layer, bool visible, Vector3 position, Vector3 rotation, Vector3 scale);

void scene_obj_toggle_texture(SceneObject* obj, TextureZ* texture);

vector_ptr(SceneObject);

typedef struct {
     float buffer[1920*1080];
     int width;
     int height;
} ZBuffer;

typedef struct {
     VECTOR_PTR_TYPE(SceneObject) objs;
     CameraZ* camera;
     ZBuffer zbuffer;
} Scene;

void scene_obj_draw(Scene* scene, SceneObject* obj);

void scene_init(Scene* scene);
void scene_free(Scene* scene);

Scene* scene_create(CameraZ* camera);

void scene_draw(Scene* scene);
void scene_update(Scene* scene);

void scene_add_obj(Scene* scene, SceneObject* obj);
void scene_remove_obj(Scene* scene, size_t index);

#endif //SCENE_H
