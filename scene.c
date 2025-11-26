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

Vector3 TransformNormal(Vector3 normal, Matrix mat) {
    Vector3 result = {
        normal.x * mat.m0 + normal.y * mat.m4 + normal.z * mat.m8,
        normal.x * mat.m1 + normal.y * mat.m5 + normal.z * mat.m9,
        normal.x * mat.m2 + normal.y * mat.m6 + normal.z * mat.m10
    };
    return result;
}

Color calculate_lambert_lighting(Vector3 world_pos, Vector3 world_normal, Color base_color, Light* light) {
    Vector3 light_dir = Vector3Normalize(Vector3Subtract(light->position, world_pos));

    Vector3 normal = Vector3Normalize(world_normal);

    float diffuse = fmaxf(Vector3DotProduct(normal, light_dir), 0.0f);

    diffuse *= light->intensity;

    float ambient = 0.2f;
    float total_light = fminf(ambient + diffuse, 1.0f);

    Color result;
    result.r = (unsigned char)(base_color.r * total_light);
    result.g = (unsigned char)(base_color.g * total_light);
    result.b = (unsigned char)(base_color.b * total_light);
    result.a = base_color.a;

    return result;
}

float calculate_phong_lighting(Vector3 position, Vector3 normal, Vector3 camera, Light *light_source) {
    Vector3 view = Vector3Normalize(Vector3Subtract(camera, position));
    Vector3 light = Vector3Normalize(Vector3Subtract(light_source->position, position));
    normal = Vector3Normalize(normal);

    float dot = Vector3DotProduct(normal, Vector3Negate(light));
    Vector3 reflected_light = Vector3Subtract(light, Vector3Scale(normal, 2 * dot));

    float diffuse = light_source->intensity * 0.5f * fmaxf(Vector3DotProduct(normal, light), 0.0f);
    float specular = light_source->intensity * 0.2f * fmaxf(Vector3DotProduct(view, reflected_light), 0.0f);
    float ambient = 0.2f;

    float total_light = fminf(ambient + diffuse + specular, 1.0f);
    return total_light;
}

float calculate_lambert_lighting2(Vector3 world_pos, Vector3 world_normal, Color base_color, Light* light) {
    Vector3 light_dir = Vector3Normalize(Vector3Subtract(light->position, world_pos));

    Vector3 normal = Vector3Normalize(world_normal);

    float diffuse = fmaxf(Vector3DotProduct(normal, light_dir), 0.0f);

    diffuse *= light->intensity;

    float ambient = 0.2f;
    float total_light = fminf(ambient + diffuse, 1.0f);

    return total_light;
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

void draw_triangle_gouraud(ZBuffer *zbuffer, Vector3 a, Vector3 b, Vector3 c,
                           Color color_a, Color color_b, Color color_c);

void draw_triangle(ZBuffer *zbuffer, Vector3 a, Vector3 b, Vector3 c, Color color) {
    draw_triangle_gouraud(zbuffer, a, b, c, color, color, color);
}

// Шейдинг Гуро - интерполяция цветов вершин
void draw_triangle_gouraud(ZBuffer *zbuffer, Vector3 a, Vector3 b, Vector3 c,
                           Color color_a, Color color_b, Color color_c) {
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
                    Color pixel_color;
                    pixel_color.r = (unsigned char)(bary.x * color_a.r + bary.y * color_b.r + bary.z * color_c.r);
                    pixel_color.g = (unsigned char)(bary.x * color_a.g + bary.y * color_b.g + bary.z * color_c.g);
                    pixel_color.b = (unsigned char)(bary.x * color_a.b + bary.y * color_b.b + bary.z * color_c.b);
                    pixel_color.a = (unsigned char)(bary.x * color_a.a + bary.y * color_b.a + bary.z * color_c.a);

                    DrawPixel(x, y, pixel_color);
                    zbuffer->buffer[y * zbuffer->width + x] = z;
                }
            }
        }
    }
}

void draw_textured_triangle_with_gouraud_lighting(ZBuffer* zbuffer,
    Vector3 a_pos, Vector2 a_tex, float light_a,
    Vector3 b_pos, Vector2 b_tex, float light_b,
    Vector3 c_pos, Vector2 c_tex, float light_c,
    TextureZ* tex) {

    Vector2 v1 = { a_pos.x, a_pos.y };
    Vector2 v2 = { b_pos.x, b_pos.y };
    Vector2 v3 = { c_pos.x, c_pos.y };

    int min_x, max_x, min_y, max_y;
    get_triangle_bounding_box(v1, v2, v3, &min_x, &max_x, &min_y, &max_y);

    min_x = fmax(min_x, 0);
    max_x = fmin(max_x, zbuffer->width - 1);
    min_y = fmax(min_y, 0);
    max_y = fmin(max_y, zbuffer->height - 1);

    for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
            Vector2 p = { (float)x + 0.5f, (float)y + 0.5f };
            Vector3 bary = barycentric_coordinates(p, v1, v2, v3);

            if (bary.x >= 0 && bary.y >= 0 && bary.z >= 0) {
                float inv_z_A = 1.0f / a_pos.z;
                float inv_z_B = 1.0f / b_pos.z;
                float inv_z_C = 1.0f / c_pos.z;

                float depth = 1.0f / (bary.x * inv_z_A + bary.y * inv_z_B + bary.z * inv_z_C);

                // Интерполируем u/z и v/z
                float u_over_z = bary.x * (a_tex.x * inv_z_A) +
                    bary.y * (b_tex.x * inv_z_B) +
                    bary.z * (c_tex.x * inv_z_C);
                float v_over_z = bary.x * (a_tex.y * inv_z_A) +
                    bary.y * (b_tex.y * inv_z_B) +
                    bary.z * (c_tex.y * inv_z_C);

                // Восстанавливаем u, v
                float u = u_over_z * depth;
                float v = v_over_z * depth;

                v = 1.0f - v;  // Инвертируем V

                if (depth < zbuffer->buffer[y * zbuffer->width + x]) {
                    Color pixel_color = Texture_sample(tex, u, v);

                    pixel_color.r *= bary.x * light_a + bary.y * light_b + bary.z * light_c;
                    pixel_color.g *= bary.x * light_a + bary.y * light_b + bary.z * light_c;
                    pixel_color.b *= bary.x * light_a + bary.y * light_b + bary.z * light_c;

                    DrawPixel(x, y, pixel_color);
                    zbuffer->buffer[y * zbuffer->width + x] = depth;
                }
            }
        }
    }
}

void draw_textured_triangle_with_phong_lighting(ZBuffer *zbuffer,
                Vector3 a_pos, Vector2 a_tex, Vector3 a_world, Vector3 a_normal,
                Vector3 b_pos, Vector2 b_tex, Vector3 b_world, Vector3 b_normal,
                Vector3 c_pos, Vector2 c_tex, Vector3 c_world, Vector3 c_normal,
                TextureZ *tex, Light *light_source, Vector3 camera)
{
    Vector2 v1 = { a_pos.x, a_pos.y };
    Vector2 v2 = { b_pos.x, b_pos.y };
    Vector2 v3 = { c_pos.x, c_pos.y };

    int min_x, max_x, min_y, max_y;
    get_triangle_bounding_box(v1, v2, v3, &min_x, &max_x, &min_y, &max_y);

    min_x = fmax(min_x, 0);
    max_x = fmin(max_x, zbuffer->width - 1);
    min_y = fmax(min_y, 0);
    max_y = fmin(max_y, zbuffer->height - 1);

    for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
            Vector2 p = { (float)x + 0.5f, (float)y + 0.5f };
            Vector3 bary = barycentric_coordinates(p, v1, v2, v3);

            if (bary.x >= 0 && bary.y >= 0 && bary.z >= 0) {
                float inv_z_A = 1.0f / a_pos.z;
                float inv_z_B = 1.0f / b_pos.z;
                float inv_z_C = 1.0f / c_pos.z;

                float depth = 1.0f / (bary.x * inv_z_A + bary.y * inv_z_B + bary.z * inv_z_C);

                // Интерполируем u/z и v/z
                float u_over_z = bary.x * (a_tex.x * inv_z_A) +
                    bary.y * (b_tex.x * inv_z_B) +
                    bary.z * (c_tex.x * inv_z_C);
                float v_over_z = bary.x * (a_tex.y * inv_z_A) +
                    bary.y * (b_tex.y * inv_z_B) +
                    bary.z * (c_tex.y * inv_z_C);

                // Восстанавливаем u, v
                float u = u_over_z * depth;
                float v = v_over_z * depth;

                v = 1.0f - v;  // Инвертируем V

                if (depth < zbuffer->buffer[y * zbuffer->width + x]) {
                    Color pixel_color = Texture_sample(tex, u, v);

                    Vector3 normal;
                    normal.x = bary.x * a_normal.x + bary.y * b_normal.x + bary.z * c_normal.x;
                    normal.y = bary.x * a_normal.y + bary.y * b_normal.y + bary.z * c_normal.y;
                    normal.z = bary.x * a_normal.z + bary.y * b_normal.z + bary.z * c_normal.z;

                    Vector3 worldpos;
                    worldpos.x = bary.x * a_world.x + bary.y * b_world.x + bary.z * c_world.x;
                    worldpos.y = bary.x * a_world.y + bary.y * b_world.y + bary.z * c_world.y;
                    worldpos.z = bary.x * a_world.z + bary.y * b_world.z + bary.z * c_world.z;

                    float light = calculate_phong_lighting(worldpos, normal, camera, light_source);

                    pixel_color.r *= light;
                    pixel_color.g *= light;
                    pixel_color.b *= light;

                    DrawPixel(x, y, pixel_color);
                    zbuffer->buffer[y * zbuffer->width + x] = depth;
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
        Vector3 world_positions[8];
        Vector3 normals[8];
        for (size_t v = 0; v < indices.len; v++) {
            Vector3 worldVert = worldVerts->head[indices.head[v]];
            screenVerts[v] = cameraz_world_to_screen(worldVert, scene->camera);
            texCoords[v] = obj->mesh->vertices.head[indices.head[v]].texCoord;
            world_positions[v] = worldVert;
            normals[v] = obj->mesh->vertices.head[indices.head[v]].normal;
        }

        if (scene->lighting_mode == LIGHTING_GOURAUD) {
            float lights[3];
            for (int v = 0; v < indices.len; v++) {
                Vector3 worldVert = worldVerts->head[indices.head[v]];
                Vector3 worldNormal = TransformNormal(obj->mesh->vertices.head[indices.head[v]].normal, worldMatrix);

                float normalLength = Vector3Length(worldNormal);
                if (normalLength > 0.0001f) {
                    worldNormal = Vector3Normalize(worldNormal);
                } else {
                    // Если нормаль нулевая, вычисляем нормаль грани
                    Vector3 v0 = worldVerts->head[indices.head[0]];
                    Vector3 v1 = worldVerts->head[indices.head[(v + 1) % indices.len]];
                    Vector3 v2 = worldVerts->head[indices.head[(v + 2) % indices.len]];
                    Vector3 edge1 = Vector3Subtract(v1, v0);
                    Vector3 edge2 = Vector3Subtract(v2, v0);
                    worldNormal = Vector3Normalize(Vector3CrossProduct(edge1, edge2));
                }

                lights[v] = calculate_lambert_lighting2(worldVert, worldNormal, obj->mesh->color, &scene->light);
            }

            // Текстурированная отрисовка - веером из первой вершины
            for (int i = 1; i < indices.len - 1; i++) {
                draw_textured_triangle_with_gouraud_lighting(&scene->zbuffer,
                    screenVerts[0], texCoords[0], lights[0],
                    screenVerts[i], texCoords[i], lights[i],
                    screenVerts[i + 1], texCoords[i + 1], lights[i + 1],
                    obj->texture);
            }
        } else {
            for (int i = 1; i < indices.len - 1; i++) {
                draw_textured_triangle_with_phong_lighting(&scene->zbuffer,
                    screenVerts[0], texCoords[0], world_positions[0], normals[0],
                    screenVerts[i], texCoords[i], world_positions[i], normals[1],
                    screenVerts[i + 1], texCoords[i + 1], world_positions[i + 1], normals[2],
                    obj->texture, &scene->light, scene->camera->position);
            }
        }

        //// Вычисляем цвета вершин по модели Ламберта для шейдинга Гуро
        //Color *vertexColors = calloc(indices.len, sizeof(Color));
        //for (size_t v = 0; v < indices.len; v++) {
        //    Vector3 worldVert = worldVerts->head[indices.head[v]];
        //    Vector3 worldNormal = TransformNormal(obj->mesh->vertices.head[indices.head[v]].normal, worldMatrix);

        //    float normalLength = Vector3Length(worldNormal);
        //    if (normalLength > 0.0001f) {
        //        worldNormal = Vector3Normalize(worldNormal);
        //    } else {
        //        // Если нормаль нулевая, вычисляем нормаль грани
        //        Vector3 v0 = worldVerts->head[indices.head[0]];
        //        Vector3 v1 = worldVerts->head[indices.head[(v + 1) % indices.len]];
        //        Vector3 v2 = worldVerts->head[indices.head[(v + 2) % indices.len]];
        //        Vector3 edge1 = Vector3Subtract(v1, v0);
        //        Vector3 edge2 = Vector3Subtract(v2, v0);
        //        worldNormal = Vector3Normalize(Vector3CrossProduct(edge1, edge2));
        //    }

        //    vertexColors[v] = calculate_lambert_lighting(worldVert, worldNormal, obj->mesh->color, &scene->light);
        //}

        //// Рисуем контуры
        //for (size_t v = 0; v < indices.len; v++) {
        //    int next = (v + 1) % indices.len;
        //    plotLine(&scene->zbuffer, screenVerts[v], screenVerts[next]);
        //}

        //// Рисуем треугольники с шейдингом Гуро (интерполяция цветов вершин)
        //for (int i = 1; i < indices.len - 1; i++) {
        //    draw_triangle_gouraud(&scene->zbuffer,
        //        screenVerts[0], screenVerts[i], screenVerts[i + 1],
        //        vertexColors[0], vertexColors[i], vertexColors[i + 1]);
        //}

        //free(vertexColors);
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

    scene->light.position = (Vector3){ 5.0f, 5.0f, 5.0f };
    scene->light.color = WHITE;
    scene->light.intensity = 1.0f;

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

void draw_light_source(Light* light, CameraZ* camera) {
    Vector3 screen_pos = cameraz_world_to_screen(light->position, camera);

    Vector2 light_screen = { screen_pos.x, screen_pos.y };
    DrawCircleV(light_screen, 8, YELLOW);
    DrawCircleV(light_screen, 6, light->color);

    DrawLineV((Vector2){light_screen.x - 10, light_screen.y},
              (Vector2){light_screen.x + 10, light_screen.y}, YELLOW);
    DrawLineV((Vector2){light_screen.x, light_screen.y - 10},
              (Vector2){light_screen.x, light_screen.y + 10}, YELLOW);
}

void scene_draw(Scene* scene) {
    VECTOR_PTR_TYPE(SceneObject) objs = scene->objs;
    CameraZ* camera = scene->camera;

    draw_coordinate_axes(camera);

    draw_light_source(&scene->light, camera);

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
