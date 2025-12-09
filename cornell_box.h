#ifndef CORNELL_BOX_H
#define CORNELL_BOX_H

#include "third_party/include/raylib.h"
#include "third_party/include/raymath.h"
#include <stdbool.h>

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

typedef struct {
    Vector3 origin;
    Vector3 direction;
} Ray3D;

typedef struct {
    Color color;
    float reflectivity;
    float transparency;
    float refraction_index;
    float specular;
    bool is_mirror;
    bool is_transparent;
} RT_Material;

typedef enum {
    PRIMITIVE_SPHERE,
    PRIMITIVE_BOX,
    PRIMITIVE_PLANE
} PrimitiveType;

typedef struct {
    PrimitiveType type;
    Vector3 center;
    float radius;
    Vector3 size;
    Vector3 normal;
    float d;
    RT_Material material;
    bool visible;
    int id;
} RT_Primitive;

typedef struct {
    Vector3 position;
    Color color;
    float intensity;
    bool enabled;
} RT_Light;

typedef struct {
    bool hit;
    float t;
    Vector3 point;
    Vector3 normal;
    RT_Material material;
    int object_id;
} HitInfo;

#define MAX_PRIMITIVES 64
#define MAX_LIGHTS 4
#define MAX_BOUNCES 5

typedef struct {
    RT_Primitive primitives[MAX_PRIMITIVES];
    int primitive_count;

    RT_Light lights[MAX_LIGHTS];
    int light_count;

    bool wall_mirror[6];
    int selected_mirror_wall;

    Vector3 camera_pos;
    Vector3 camera_target;
    Vector3 camera_up;
    float fov;

    int max_bounces;
    Color background_color;

    int selected_object;
} RT_Scene;

Ray3D ray_create(Vector3 origin, Vector3 direction);
Vector3 ray_at(Ray3D ray, float t);

bool sphere_intersect(Ray3D ray, RT_Primitive* sphere, float* t_out, Vector3* normal_out);
bool box_intersect(Ray3D ray, RT_Primitive* box, float* t_out, Vector3* normal_out);
bool plane_intersect(Ray3D ray, RT_Primitive* plane, float* t_out);

void rt_scene_init(RT_Scene* scene);
void rt_scene_create_cornell_box(RT_Scene* scene);
int rt_scene_add_sphere(RT_Scene* scene, Vector3 center, float radius, RT_Material material);
int rt_scene_add_box(RT_Scene* scene, Vector3 center, Vector3 size, RT_Material material);
void rt_scene_add_light(RT_Scene* scene, Vector3 position, Color color, float intensity);

RT_Material rt_material_diffuse(Color color);
RT_Material rt_material_mirror(Color color, float reflectivity);
RT_Material rt_material_transparent(Color color, float transparency, float refraction_index);

HitInfo rt_trace_ray(RT_Scene* scene, Ray3D ray);
Color rt_shade(RT_Scene* scene, Ray3D ray, int depth);
void rt_render(RT_Scene* scene, Image* target);

Vector3 reflect_vector(Vector3 incident, Vector3 normal);
Vector3 refract_vector(Vector3 incident, Vector3 normal, float eta);
float fresnel(Vector3 incident, Vector3 normal, float ior);

#endif // CORNELL_BOX_H

