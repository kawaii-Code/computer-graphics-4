#include "scene.h"
#include "Polyhedron.h"
#include "common.h"

#define Z_BUFFER_MAX 1e9

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
    scene->texture = NULL;       
    scene->has_texture = false;

    return scene;
}

void scene_obj_toggle_texture(SceneObject* obj, TextureZ* texture) {
    if (obj->has_texture) {
        obj->has_texture = false;
        obj->texture = NULL;
    }
    else {
        obj->has_texture = true;
        obj->texture = texture;
    }
}


Vector3 barycentric_coordinates(Vector2 p, Vector2 a, Vector2 b, Vector2 c) {
    // переходим в систему координат с началом в точке A
    float v0x = b.x - a.x, v0y = b.y - a.y;
    float v1x = c.x - a.x, v1y = c.y - a.y;
    float v2x = p.x - a.x, v2y = p.y - a.y;

    /*d00 - квадрат длины стороны AB
      d11 - квадрат длины стороны AC
      d01 - показывает, насколько векторы AB и AC перпендикулярны
      d20, d21 - проекции точки P на базисные векторы*/
    float d00 = v0x * v0x + v0y * v0y; 
    float d01 = v0x * v1x + v0y * v1y; 
    float d11 = v1x * v1x + v1y * v1y; 
    float d20 = v2x * v0x + v2y * v0y; 
    float d21 = v2x * v1x + v2y * v1y; 

    float denom = d00 * d11 - d01 * d01; // определитель матрицы Грама
    float inv_denom = 1.0f / denom;

    float v = (d11 * d20 - d01 * d21) * inv_denom;
    float w = (d00 * d21 - d01 * d20) * inv_denom;

    return (Vector3) { 1.0f - v - w, v, w };
}

// Функция для вычисления ограничивающего прямоугольника
void get_triangle_bounding_box(Vector2 v1, Vector2 v2, Vector2 v3,
    int* min_x, int* max_x, int* min_y, int* max_y) {
    *min_x = (int)fminf(v1.x, fminf(v2.x, v3.x));
    *max_x = (int)fmaxf(v1.x, fmaxf(v2.x, v3.x));
    *min_y = (int)fminf(v1.y, fminf(v2.y, v3.y));
    *max_y = (int)fmaxf(v1.y, fmaxf(v2.y, v3.y));
}

//Точка P принадлежит треугольнику 𝑃𝑎𝑃𝑏𝑃𝑐 — все барицентрические координаты не отрицательны
//Точка P совпадает с одной из вершин треугольника 𝑃𝑎𝑃𝑏𝑃𝑐 — две барицентрические координаты равны нулю
//Точка P лежит на стороне треугольника 𝑃𝑎𝑃𝑏𝑃𝑐 или лежит на ее продолжении — одна из барицентрических координат равна нулю.
//Точка P не принадлежит треугольнику 𝑃𝑎𝑃𝑏𝑃𝑐 — по крайней мере одна барицентрическая sкоордината отрицательная.
bool point_in_triangle(Vector3 bary) {
    return bary.x >= 0 && bary.y >= 0 && bary.z >= 0;
}


// Из второй лабы. А как нам быть, в наше то время?
static void DrawBrush(ZBuffer *zbuffer, int cx, int cy, float cz) {
    float *zbuf = &zbuffer->buffer[cy * zbuffer->width + cx];
    if (cz < *zbuf) {
        DrawPixel(cx, cy, BLACK);
        *zbuf = cz;
    }
}

static void plotLineLow(ZBuffer *zbuffer, Vector3 beg, Vector3 end) {
    int dx = (int)end.x - (int)beg.x;
    int dy = (int)end.y - (int)beg.y;
    int yi = 1;

    if (dy < 0) {
        yi = -1;
        dy *= -1;
    }

    int D = (2 * dy) - dx;
    int y = (int)beg.y;
    float z = beg.z;

    for (int x = (int)beg.x; x <= (int)end.x; x++) {
        DrawBrush(zbuffer, x, y, z);
        if (D > 0) {
            y += yi;
            D += 2 * (dy - dx);
        } else {
            D += 2 * dy;
        }
        float t = (float)(x - (int)beg.x) / (float)((int)end.x - (int)beg.x);
        z = beg.z + t * (end.z - beg.z);
    }
}

static void plotLineHigh(ZBuffer *zbuffer, Vector3 from, Vector3 to) {
    int dx = (int)to.x - (int)from.x;
    int dy = (int)to.y - (int)from.y;
    int xi = 1;

    if (dx < 0) {
        xi = -1;
        dx *= -1;
    }

    int D = (2 * dx) - dy;
    int x = (int)from.x;
    float z = from.z;

    for (int y = (int)from.y; y <= (int)to.y; y++) {
        DrawBrush(zbuffer, x, y, z);
        if (D > 0) {
            x += xi;
            D += 2 * (dx - dy);
        } else {
            D += 2 * dx;
        }
        //z = from.z + ((float)y / (float)to.y) * (to.z - from.z);
        float t = (float)(x - (int)from.x) / (float)((int)to.x - (int)from.x);
        z = from.z + t * (to.z - from.z);
    }
}

static void plotLine(ZBuffer *zbuffer, Vector3 beg, Vector3 end) {
    if (abs(end.y - beg.y) < abs(end.x - beg.x)) {
        if (beg.x > end.x) {
            plotLineLow(zbuffer, end, beg);
        } else {
            plotLineLow(zbuffer, beg, end);
        }
    } else {
        if (beg.y > end.y) {
            plotLineHigh(zbuffer, end, beg);
        } else {
            plotLineHigh(zbuffer, beg, end);
        }
    }
}


void draw_triangle(ZBuffer *zbuffer, Vector3 a, Vector3 b, Vector3 c, Color color) {
    Vector2 v1 = (Vector2) { .x = a.x, .y = a.y };
    Vector2 v2 = (Vector2) { .x = b.x, .y = b.y };
    Vector2 v3 = (Vector2) { .x = c.x, .y = c.y };

    int min_x, max_x, min_y, max_y;
    get_triangle_bounding_box(v1, v2, v3, &min_x, &max_x, &min_y, &max_y);

    for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
            Vector2 p = { (float)x, (float)y };

            Vector3 bary = barycentric_coordinates(p, v1, v2, v3);
            if (point_in_triangle(bary)) {
                float z = bary.x * a.z + bary.y * b.z + bary.z * c.z;
                if (z < zbuffer->buffer[y * zbuffer->width + x]) {
                    Color pixel_color = color;
                    DrawPixel(x, y, pixel_color);
                    zbuffer->buffer[y * zbuffer->width + x] = z;
                }
            }
        }
    }
}

void draw_textured_triangle(ZBuffer* zbuffer, Vector3 a_pos, Vector2 a_tex,
    Vector3 b_pos, Vector2 b_tex, Vector3 c_pos, Vector2 c_tex, TextureZ* tex) {

    Vector2 v1 = (Vector2){ .x = a_pos.x, .y = a_pos.y };
    Vector2 v2 = (Vector2){ .x = b_pos.x, .y = b_pos.y };
    Vector2 v3 = (Vector2){ .x = c_pos.x, .y = c_pos.y };

    int min_x, max_x, min_y, max_y;
    get_triangle_bounding_box(v1, v2, v3, &min_x, &max_x, &min_y, &max_y);

    for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
            Vector2 p = { (float)x, (float)y };
            Vector3 bary = barycentric_coordinates(p, v1, v2, v3);

            if (point_in_triangle(bary)) {
                float z = bary.x * a_pos.z + bary.y * b_pos.z + bary.z * c_pos.z;

                // Интерполируем обратную глубину
                float w = bary.x * (1.0f / a_pos.z) +
                    bary.y * (1.0f / b_pos.z) +
                    bary.z * (1.0f / c_pos.z);

                // Интерполируем u/z и v/z
                float u_over_w = bary.x * (a_tex.x / a_pos.z) +
                    bary.y * (b_tex.x / b_pos.z) +
                    bary.z * (c_tex.x / c_pos.z);
                float v_over_w = bary.x * (a_tex.y / a_pos.z) +
                    bary.y * (b_tex.y / b_pos.z) +
                    bary.z * (c_tex.y / c_pos.z);

                // Восстанавливаем u, v
                float u = u_over_w / w;
                float v = v_over_w / w;

                if (z < zbuffer->buffer[y * zbuffer->width + x]) {
                    Color pixel_color = Texture_sample(tex, u, v);
                    DrawPixel(x, y, pixel_color);
                    zbuffer->buffer[y * zbuffer->width + x] = z;
                }
            }
        }
    }
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

        if (!Face_isFrontFacing(obj->mesh, face, scene->camera, worldMatrix)) {
            continue; 
        }

        Vector3 *screenVerts = calloc(1, sizeof(Vector3) * indices.len);
        Vector2 texCoords[8];
        for (size_t v = 0; v < indices.len; v++) {
            Vector3 worldVert = worldVerts->head[indices.head[v]];
            screenVerts[v] = cameraz_world_to_screen(worldVert, scene->camera);
            texCoords[v] = obj->mesh->vertices.head[indices.head[v]].texCoord;
        }

        if (obj->has_texture) {
            // Текстурированная отрисовка - веером из первой вершины
            for (int i = 1; i < indices.len - 1; i++) {
                draw_textured_triangle(&scene->zbuffer,
                    screenVerts[0], texCoords[0],
                    screenVerts[i], texCoords[i],
                    screenVerts[i + 1], texCoords[i + 1],
                    obj->texture);
            }
        }
        else {
            for (size_t v = 0; v < indices.len; v++) {
                int next = (v + 1) % indices.len;
                plotLine(&scene->zbuffer, screenVerts[v], screenVerts[next]);
            }

            for (int i = 0; i < indices.len - 1; i++) {
                draw_triangle(&scene->zbuffer, screenVerts[0], screenVerts[i], screenVerts[i + 1], obj->mesh->color);
            }
        }
        free(screenVerts);
    }

    //for (size_t f = 0; f < obj->mesh->faces.len; f++) {
    //    Face* face = &obj->mesh->faces.head[f];
    //    VECTOR_TYPE(int) indices = face->vertexIndices;

    //    if (indices.len < 3) {
    //        continue;
    //    }

    //    /*Vector3 faceCenter = Face_getCenter(obj->mesh, face);
    //    faceCenter = Vector3Transform(faceCenter, worldMatrix);
    //    Vector3 faceNormal = Face_calculateNormal(obj->mesh, face);
    //    Vector3 normalEnd = Vector3Add(faceCenter, faceNormal);
    //    Vector2 faceScreen = cameraz_world_to_screen(faceCenter, scene->camera);
    //    Vector2 normalScreen = cameraz_world_to_screen(normalEnd, scene->camera);
    //    DrawLineV(faceScreen, normalScreen, BLUE);*/
    //
    //    Vector3 worldNormal = Face_calculateNormal_World(obj->mesh, face, worldMatrix);
    //    Vector3 faceCenter = Face_getCenter_World(obj->mesh, face, worldMatrix);

    //    Vector3 normalEnd = Vector3Add(faceCenter, Vector3Scale(worldNormal, 0.5f));
    //    Vector2 faceScreen = cameraz_world_to_screen(faceCenter, scene->camera);
    //    Vector2 normalScreen = cameraz_world_to_screen(normalEnd, scene->camera);

    //    bool isVisible = Face_isFrontFacing(obj->mesh, face, scene->camera,worldMatrix);
    //    DrawLineV(faceScreen, normalScreen, isVisible ? GREEN : RED);
    //}

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
    Vector3 _origin = cameraz_world_to_screen((Vector3){0, 0, 0}, camera);
    Vector3 _x_axis = cameraz_world_to_screen((Vector3){5, 0, 0}, camera);
    Vector3 _y_axis = cameraz_world_to_screen((Vector3){0, 5, 0}, camera);
    Vector3 _z_axis = cameraz_world_to_screen((Vector3){0, 0, 5}, camera);

    Vector2 origin = { .x = _origin.x, .y = _origin.y };
    Vector2 x_axis = { .x = _x_axis.x, .y = _x_axis.y };
    Vector2 y_axis = { .x = _y_axis.x, .y = _y_axis.y };
    Vector2 z_axis = { .x = _z_axis.x, .y = _z_axis.y };

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

    for (int i = 0; i < scene->zbuffer.width * scene->zbuffer.height; i++) {
        scene->zbuffer.buffer[i] = Z_BUFFER_MAX;
    }

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
