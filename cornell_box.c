#include "cornell_box.h"

Ray3D ray_create(Vector3 origin, Vector3 direction) {
    Ray3D ray;
    ray.origin = origin;
    ray.direction = Vector3Normalize(direction);
    return ray;
}

Vector3 ray_at(Ray3D ray, float t) {
    return Vector3Add(ray.origin, Vector3Scale(ray.direction, t));
}

RT_Material rt_material_diffuse(Color color) {
    RT_Material mat = {0};
    mat.color = color;
    mat.reflectivity = 0.0f;
    mat.transparency = 0.0f;
    mat.refraction_index = 1.0f;
    mat.specular = 0.1f;
    mat.is_mirror = false;
    mat.is_transparent = false;
    return mat;
}

RT_Material rt_material_mirror(Color color, float reflectivity) {
    RT_Material mat = {0};
    mat.color = color;
    mat.reflectivity = reflectivity;
    mat.transparency = 0.0f;
    mat.refraction_index = 1.0f;
    mat.specular = 1.0f;
    mat.is_mirror = true;
    mat.is_transparent = false;
    return mat;
}

RT_Material rt_material_transparent(Color color, float transparency, float refraction_index) {
    RT_Material mat = {0};
    mat.color = color;
    mat.reflectivity = 0.1f;
    mat.transparency = transparency;
    mat.refraction_index = refraction_index;
    mat.specular = 0.5f;
    mat.is_mirror = false;
    mat.is_transparent = true;
    return mat;
}

bool sphere_intersect(Ray3D ray, RT_Primitive* sphere, float* t_out, Vector3* normal_out) {
    Vector3 oc = Vector3Subtract(ray.origin, sphere->center);
    float a = Vector3DotProduct(ray.direction, ray.direction);
    float b = 2.0f * Vector3DotProduct(oc, ray.direction);
    float c = Vector3DotProduct(oc, oc) - sphere->radius * sphere->radius;
    float discriminant = b * b - 4 * a * c;

    if (discriminant < 0) return false;

    float sqrt_d = sqrtf(discriminant);
    float t1 = (-b - sqrt_d) / (2.0f * a);
    float t2 = (-b + sqrt_d) / (2.0f * a);

    float t = t1;
    if (t < 0.001f) {
        t = t2;
        if (t < 0.001f) return false;
    }

    *t_out = t;
    Vector3 hit_point = ray_at(ray, t);
    *normal_out = Vector3Normalize(Vector3Subtract(hit_point, sphere->center));
    return true;
}

bool box_intersect(Ray3D ray, RT_Primitive* box, float* t_out, Vector3* normal_out) {
    Vector3 min_pt = Vector3Subtract(box->center, box->size);
    Vector3 max_pt = Vector3Add(box->center, box->size);

    float tmin = -FLT_MAX;
    float tmax = FLT_MAX;
    Vector3 tmin_normal = {0, 0, 0};
    Vector3 tmax_normal = {0, 0, 0};

    // X axis
    if (fabsf(ray.direction.x) > 0.0001f) {
        float t1 = (min_pt.x - ray.origin.x) / ray.direction.x;
        float t2 = (max_pt.x - ray.origin.x) / ray.direction.x;
        Vector3 n1 = {-1, 0, 0};
        Vector3 n2 = {1, 0, 0};
        if (t1 > t2) { float tmp = t1; t1 = t2; t2 = tmp; Vector3 tmpn = n1; n1 = n2; n2 = tmpn; }
        if (t1 > tmin) { tmin = t1; tmin_normal = n1; }
        if (t2 < tmax) { tmax = t2; tmax_normal = n2; }
    } else if (ray.origin.x < min_pt.x || ray.origin.x > max_pt.x) {
        return false;
    }

    // Y axis
    if (fabsf(ray.direction.y) > 0.0001f) {
        float t1 = (min_pt.y - ray.origin.y) / ray.direction.y;
        float t2 = (max_pt.y - ray.origin.y) / ray.direction.y;
        Vector3 n1 = {0, -1, 0};
        Vector3 n2 = {0, 1, 0};
        if (t1 > t2) { float tmp = t1; t1 = t2; t2 = tmp; Vector3 tmpn = n1; n1 = n2; n2 = tmpn; }
        if (t1 > tmin) { tmin = t1; tmin_normal = n1; }
        if (t2 < tmax) { tmax = t2; tmax_normal = n2; }
    } else if (ray.origin.y < min_pt.y || ray.origin.y > max_pt.y) {
        return false;
    }

    // Z axis
    if (fabsf(ray.direction.z) > 0.0001f) {
        float t1 = (min_pt.z - ray.origin.z) / ray.direction.z;
        float t2 = (max_pt.z - ray.origin.z) / ray.direction.z;
        Vector3 n1 = {0, 0, -1};
        Vector3 n2 = {0, 0, 1};
        if (t1 > t2) { float tmp = t1; t1 = t2; t2 = tmp; Vector3 tmpn = n1; n1 = n2; n2 = tmpn; }
        if (t1 > tmin) { tmin = t1; tmin_normal = n1; }
        if (t2 < tmax) { tmax = t2; tmax_normal = n2; }
    } else if (ray.origin.z < min_pt.z || ray.origin.z > max_pt.z) {
        return false;
    }

    if (tmin > tmax) return false;

    float t = tmin;
    Vector3 n = tmin_normal;
    if (t < 0.001f) {
        t = tmax;
        n = tmax_normal;  // Не инвертируем нормаль - она должна указывать наружу
        if (t < 0.001f) return false;
    }

    *t_out = t;
    *normal_out = n;
    return true;
}

bool plane_intersect(Ray3D ray, RT_Primitive* plane, float* t_out) {
    float denom = Vector3DotProduct(plane->normal, ray.direction);
    if (fabsf(denom) < 0.0001f) return false;

    float t = -(Vector3DotProduct(plane->normal, ray.origin) + plane->d) / denom;
    if (t < 0.001f) return false;

    *t_out = t;
    return true;
}

Vector3 reflect_vector(Vector3 incident, Vector3 normal) {
    float dot = Vector3DotProduct(incident, normal);
    return Vector3Subtract(incident, Vector3Scale(normal, 2.0f * dot));
}

Vector3 refract_vector(Vector3 incident, Vector3 normal, float eta) {
    float cos_i = -Vector3DotProduct(incident, normal);
    float sin_t2 = eta * eta * (1.0f - cos_i * cos_i);

    if (sin_t2 > 1.0f) {
        // Total internal reflection
        return reflect_vector(incident, normal);
    }

    float cos_t = sqrtf(1.0f - sin_t2);
    return Vector3Add(
        Vector3Scale(incident, eta),
        Vector3Scale(normal, eta * cos_i - cos_t)
    );
}

float fresnel(Vector3 incident, Vector3 normal, float ior) {
    float cos_i = fabsf(Vector3DotProduct(incident, normal));
    float sin_t2 = ior * ior * (1.0f - cos_i * cos_i);

    if (sin_t2 > 1.0f) return 1.0f;

    float cos_t = sqrtf(1.0f - sin_t2);
    float r_orth = (ior * cos_i - cos_t) / (ior * cos_i + cos_t);
    float r_para = (cos_i - ior * cos_t) / (cos_i + ior * cos_t);

    return (r_orth * r_orth + r_para * r_para) / 2.0f;
}

void rt_scene_init(RT_Scene* scene) {
    memset(scene, 0, sizeof(RT_Scene));
    scene->camera_pos = (Vector3){0, 0, 5};
    scene->camera_target = (Vector3){0, 0, 0};
    scene->camera_up = (Vector3){0, 1, 0};
    scene->fov = 60.0f;
    scene->max_bounces = MAX_BOUNCES;
    scene->background_color = BLACK;
    scene->selected_object = -1;
    scene->selected_mirror_wall = -1;

    for (int i = 0; i < 6; i++) {
        scene->wall_mirror[i] = false;
    }
}

int rt_scene_add_sphere(RT_Scene* scene, Vector3 center, float radius, RT_Material material) {
    if (scene->primitive_count >= MAX_PRIMITIVES) return -1;

    int id = scene->primitive_count;
    RT_Primitive* p = &scene->primitives[id];
    p->type = PRIMITIVE_SPHERE;
    p->center = center;
    p->radius = radius;
    p->material = material;
    p->visible = true;
    p->id = id;
    scene->primitive_count++;
    return id;
}

int rt_scene_add_box(RT_Scene* scene, Vector3 center, Vector3 size, RT_Material material) {
    if (scene->primitive_count >= MAX_PRIMITIVES) return -1;

    int id = scene->primitive_count;
    RT_Primitive* p = &scene->primitives[id];
    p->type = PRIMITIVE_BOX;
    p->center = center;
    p->size = size;
    p->material = material;
    p->visible = true;
    p->id = id;
    scene->primitive_count++;
    return id;
}

void rt_scene_add_light(RT_Scene* scene, Vector3 position, Color color, float intensity) {
    if (scene->light_count >= MAX_LIGHTS) return;

    RT_Light* light = &scene->lights[scene->light_count];
    light->position = position;
    light->color = color;
    light->intensity = intensity;
    light->enabled = true;
    scene->light_count++;
}

void rt_scene_create_cornell_box(RT_Scene* scene) {
    rt_scene_init(scene);

    float room_size = 5.0f;
    float wall_thickness = 0.1f;

    scene->camera_pos = (Vector3){0, 0, room_size * 1.8f};
    scene->camera_target = (Vector3){0, 0, 0};
    scene->fov = 50.0f;

    RT_Material left_mat = rt_material_diffuse(RED);
    rt_scene_add_box(scene, (Vector3){-room_size - wall_thickness, 0, 0},
                     (Vector3){wall_thickness, room_size, room_size}, left_mat);

    RT_Material right_mat = rt_material_diffuse(GREEN);
    rt_scene_add_box(scene, (Vector3){room_size + wall_thickness, 0, 0},
                     (Vector3){wall_thickness, room_size, room_size}, right_mat);

    RT_Material floor_mat = rt_material_diffuse(WHITE);
    rt_scene_add_box(scene, (Vector3){0, -room_size - wall_thickness, 0},
                     (Vector3){room_size, wall_thickness, room_size}, floor_mat);

    RT_Material ceiling_mat = rt_material_diffuse(WHITE);
    rt_scene_add_box(scene, (Vector3){0, room_size + wall_thickness, 0},
                     (Vector3){room_size, wall_thickness, room_size}, ceiling_mat);

    RT_Material back_mat = rt_material_diffuse(WHITE);
    rt_scene_add_box(scene, (Vector3){0, 0, -room_size - wall_thickness},
                     (Vector3){room_size, room_size, wall_thickness}, back_mat);

    RT_Material front_mat = rt_material_diffuse((Color){200, 200, 200, 255});
    rt_scene_add_box(scene, (Vector3){0, 0, room_size * 2 + wall_thickness},
                     (Vector3){room_size, room_size, wall_thickness}, front_mat);

    RT_Material cube1_mat = rt_material_diffuse((Color){200, 200, 255, 255});
    rt_scene_add_box(scene, (Vector3){-2.0f, -3.0f, -1.5f},
                     (Vector3){1.5f, 2.0f, 1.5f}, cube1_mat);

    RT_Material cube2_mat = rt_material_diffuse((Color){255, 200, 150, 255});
    rt_scene_add_box(scene, (Vector3){2.0f, -4.0f, 1.0f},
                     (Vector3){1.0f, 1.0f, 1.0f}, cube2_mat);

    RT_Material sphere1_mat = rt_material_diffuse((Color){255, 100, 100, 255});
    rt_scene_add_sphere(scene, (Vector3){-2.0f, -0.5f, -1.5f}, 1.0f, sphere1_mat);

    RT_Material sphere2_mat = rt_material_mirror((Color){230, 230, 255, 255}, 0.8f);
    rt_scene_add_sphere(scene, (Vector3){2.5f, -2.5f, -2.0f}, 1.2f, sphere2_mat);

    rt_scene_add_light(scene, (Vector3){0, room_size - 0.5f, 0}, WHITE, 1.0f);

    rt_scene_add_light(scene, (Vector3){3, 2, 3}, (Color){255, 220, 180, 255}, 0.6f);
}

HitInfo rt_trace_ray(RT_Scene* scene, Ray3D ray) {
    HitInfo closest = {0};
    closest.hit = false;
    closest.t = FLT_MAX;

    for (int i = 0; i < scene->primitive_count; i++) {
        RT_Primitive* prim = &scene->primitives[i];
        if (!prim->visible) continue;

        float t;
        Vector3 normal;
        bool hit = false;

        switch (prim->type) {
            case PRIMITIVE_SPHERE:
                hit = sphere_intersect(ray, prim, &t, &normal);
                break;
            case PRIMITIVE_BOX:
                hit = box_intersect(ray, prim, &t, &normal);
                break;
            case PRIMITIVE_PLANE:
                hit = plane_intersect(ray, prim, &t);
                if (hit) normal = prim->normal;
                break;
        }

        if (hit && t < closest.t) {
            closest.hit = true;
            closest.t = t;
            closest.point = ray_at(ray, t);
            closest.normal = normal;
            closest.material = prim->material;
            closest.object_id = prim->id;

            if (i < 6 && scene->wall_mirror[i]) {
                closest.material.is_mirror = true;
                closest.material.reflectivity = 0.9f;
            }
        }
    }

    return closest;
}

Color rt_shade(RT_Scene* scene, Ray3D ray, int depth) {
    if (depth <= 0) {
        return scene->background_color;
    }

    HitInfo hit = rt_trace_ray(scene, ray);

    if (!hit.hit) {
        return scene->background_color;
    }

    Color result = {0, 0, 0, 255};
    RT_Material mat = hit.material;

    float ambient = 0.1f;
    result.r = (unsigned char)(mat.color.r * ambient);
    result.g = (unsigned char)(mat.color.g * ambient);
    result.b = (unsigned char)(mat.color.b * ambient);

    for (int i = 0; i < scene->light_count; i++) {
        RT_Light* light = &scene->lights[i];
        if (!light->enabled) continue;

        Vector3 to_light = Vector3Subtract(light->position, hit.point);
        float light_distance = Vector3Length(to_light);
        Vector3 light_dir = Vector3Normalize(to_light);

        Ray3D shadow_ray = ray_create(Vector3Add(hit.point, Vector3Scale(hit.normal, 0.001f)), light_dir);
        HitInfo shadow_hit = rt_trace_ray(scene, shadow_ray);

        if (shadow_hit.hit && shadow_hit.t < light_distance) {
            continue;
        }

        float diff = fmaxf(0, Vector3DotProduct(hit.normal, light_dir));
        float attenuation = light->intensity / (1.0f + 0.05f * light_distance);

        float dr = (mat.color.r / 255.0f) * diff * attenuation * (light->color.r / 255.0f);
        float dg = (mat.color.g / 255.0f) * diff * attenuation * (light->color.g / 255.0f);
        float db = (mat.color.b / 255.0f) * diff * attenuation * (light->color.b / 255.0f);

        Vector3 view_dir = Vector3Negate(ray.direction);
        Vector3 reflect_dir = reflect_vector(Vector3Negate(light_dir), hit.normal);
        float spec = powf(fmaxf(0, Vector3DotProduct(view_dir, reflect_dir)), 32) * mat.specular;

        float sr = spec * attenuation * (light->color.r / 255.0f);
        float sg = spec * attenuation * (light->color.g / 255.0f);
        float sb = spec * attenuation * (light->color.b / 255.0f);

        result.r = (unsigned char)fminf(255, result.r + (dr + sr) * 255);
        result.g = (unsigned char)fminf(255, result.g + (dg + sg) * 255);
        result.b = (unsigned char)fminf(255, result.b + (db + sb) * 255);
    }

    if (mat.is_mirror && mat.reflectivity > 0 && depth > 1) {
        Vector3 reflect_dir = reflect_vector(ray.direction, hit.normal);
        Ray3D reflect_ray = ray_create(Vector3Add(hit.point, Vector3Scale(hit.normal, 0.001f)), reflect_dir);
        Color reflected = rt_shade(scene, reflect_ray, depth - 1);

        float r = mat.reflectivity;
        result.r = (unsigned char)(result.r * (1 - r) + reflected.r * r);
        result.g = (unsigned char)(result.g * (1 - r) + reflected.g * r);
        result.b = (unsigned char)(result.b * (1 - r) + reflected.b * r);
    }

    if (mat.is_transparent && mat.transparency > 0 && depth > 1) {
        float n1 = 1.0f;
        float n2 = mat.refraction_index;
        Vector3 n = hit.normal;
        float cos_i = Vector3DotProduct(ray.direction, hit.normal);
        bool entering = cos_i < 0;

        if (!entering) {
            // Exiting material - луч выходит изнутри
            n = Vector3Negate(hit.normal);
            float tmp = n1; n1 = n2; n2 = tmp;
        }

        float eta = n1 / n2;
        float fr = fresnel(ray.direction, n, eta);

        Vector3 refract_dir = refract_vector(ray.direction, n, eta);
        // Смещаем в направлении, противоположном нормали (проходим сквозь поверхность)
        Vector3 offset = Vector3Scale(entering ? Vector3Negate(hit.normal) : hit.normal, 0.001f);
        Ray3D refract_ray = ray_create(Vector3Add(hit.point, offset), refract_dir);
        Color refracted = rt_shade(scene, refract_ray, depth - 1);

        // Reflected ray для Fresnel эффекта
        if (fr > 0.01f) {
            Vector3 reflect_dir = reflect_vector(ray.direction, hit.normal);
            Ray3D reflect_ray = ray_create(Vector3Add(hit.point, Vector3Scale(hit.normal, 0.001f)), reflect_dir);
            Color reflected = rt_shade(scene, reflect_ray, depth - 1);

            // Смешиваем отражение и преломление по Френелю
            refracted.r = (unsigned char)((1 - fr) * refracted.r + fr * reflected.r);
            refracted.g = (unsigned char)((1 - fr) * refracted.g + fr * reflected.g);
            refracted.b = (unsigned char)((1 - fr) * refracted.b + fr * reflected.b);
        }

        // Blend with transparency
        float t = mat.transparency;
        result.r = (unsigned char)(result.r * (1 - t) + refracted.r * t);
        result.g = (unsigned char)(result.g * (1 - t) + refracted.g * t);
        result.b = (unsigned char)(result.b * (1 - t) + refracted.b * t);
    }

    return result;
}

void rt_render(RT_Scene* scene, Image* target) {
    int width = target->width;
    int height = target->height;

    float aspect = (float)width / (float)height;
    float fov_rad = scene->fov * DEG2RAD;
    float half_height = tanf(fov_rad / 2.0f);
    float half_width = aspect * half_height;

    Vector3 w = Vector3Normalize(Vector3Subtract(scene->camera_pos, scene->camera_target));
    Vector3 u = Vector3Normalize(Vector3CrossProduct(scene->camera_up, w));
    Vector3 v = Vector3CrossProduct(w, u);

    Color* pixels = (Color*)target->data;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float px = (2.0f * ((x + 0.5f) / width) - 1.0f) * half_width;
            float py = (1.0f - 2.0f * ((y + 0.5f) / height)) * half_height;

            Vector3 direction = Vector3Add(
                Vector3Add(
                    Vector3Scale(u, px),
                    Vector3Scale(v, py)
                ),
                Vector3Negate(w)
            );

            Ray3D ray = ray_create(scene->camera_pos, direction);
            Color color = rt_shade(scene, ray, scene->max_bounces);

            pixels[y * width + x] = color;
        }
    }
}
