#include "common.h"
#include <time.h>
#include "Polyhedron.h"
#include "camera.h"
#include "scene.h"

#define RAYGUI_IMPLEMENTATION
#include "third_party/include/raygui.h"

// Свалка лично моя! Не трогать! 🦝
typedef struct {
    bool do_epic_function_task;
    bool do_epic_rotate_task;
    bool hide;
    int mode;

    float min_x;
    float max_x;
    float min_z;
    float max_z;
    int function;

    float x1, x2, y1, y2, z1, z2;
    float number_of_rotations;
    int axis;

    int      points_count;
    Vector3  points[128];
} My_Epic_Tasks_Info;

Font fonts[FONT_COUNT];

void draw_epic_ui_for_my_EPIC_tasks(My_Epic_Tasks_Info *epic_data);
void change_polyhedron(int new_selection);

int selected_polygon = 0;
bool show_dropdown = false;

Vector3 translation = { 0 };
Vector3 scale = { 1, 1, 1 };

char reflection_plane = 0; // 'X', 'Y', 'Z', или 0
Vector3 line_p1 = {0,0,0};
Vector3 line_p2 = {0,0,0};
float line_angle = 0;

const char* polyhedron_names[] = {
    "Тетраэдр",
    "Гексаэдр (Куб)",
    "Октаэдр",
    "Икосаэдр",
    "Додекаэдр",
    "Свой"
};
#define polyhedron_count 6

SceneObject* objs[polyhedron_count];
int prev_obj = 0;
int selected = 0;

Vector3 user_translation = {0, 0, 0};
Vector3 user_rotation = {0, 0, 0};
Vector3 user_scale = {1, 1, 1};
float* axis_translation = &user_translation.x;
float* axis_rotation = &user_rotation.x;

bool update_camera = true;

Polyhedron* current_poly;

TextureZ* global_texture = NULL;
TextureZ* chess_texture = NULL;
bool texture_loaded = false;
bool texture_enabled = false;
bool use_custom_texture = true; 

static char textureFilePath[256] = "textures/skull.jpg";
static bool editTextureFile = false;

void init_textures() {
    chess_texture = Texture_sh();

    global_texture = load_texture_from_file(textureFilePath);

    if (chess_texture) {
        texture_loaded = true;
        printf("Chess texture loaded successfully\n");

        for (int i = 0; i < polyhedron_count; i++) {
            if (objs[i] != NULL) {
                objs[i]->has_texture = texture_enabled;
                objs[i]->texture = texture_enabled ? chess_texture : NULL;
            }
            else {
                printf("WARNING: objs[%d] is NULL!\n", i);
            }
        }
    }

    if (global_texture) {
        printf("External texture loaded from %s\n", textureFilePath);
    }
    else {
        printf("Failed to load external texture from %s\n", textureFilePath);
    }
}

Matrix CreateRotationAroundLine(Vector3 p1, Vector3 p2, float angle);

float function5(float x, float y) {
    float r = x * x + y * y + 1;
    return 5 * (cos(r) / r + 0.1f);
}

float function4(float x, float y) {
    return cos(x) * cos(x) - sin(y) * sin(y);
}

float function3(float x, float y) {
    return atan2(y, x);
}

float function2(float x, float y) {
    return 1.0f;
}

float function1(float x, float y) {
    return x + y;
}

float function0(float x, float y) {
    return cosf(PI * x) * cosf(PI * y);
}

float (*funcs[])(float x, float y) = {
    function0,
    function1,
    function2,
    function3,
    function4,
    function5,
};

int main(int argc, char **argv) {
    init();
    SetWindowSize(menu_window_width, menu_window_height);
    srand(time(NULL));

    init_textures();

    CameraZ* camera = cameraz_create((Vector3) {0, 0, 10},
        (Vector3) {0, 0, 0},
        (Vector3) {0, 1, 0},
        45, 1, 1, 0.1, 1000, menu_window_width, menu_window_height, PERSPECTIVE_TYPE);
    Scene* scene = scene_create(camera);

    My_Epic_Tasks_Info epic_data = {0};
    epic_data.min_x = -5;
    epic_data.max_x = 5;
    epic_data.min_z = -5;
    epic_data.max_z = 5;
    epic_data.axis = 0;
    epic_data.y1 = 1;
    epic_data.y2 = 1;
    epic_data.x1 = -1;
    epic_data.x2 = 1;
    epic_data.number_of_rotations = 12;
    epic_data.hide = true;

    Polyhedron* tetra = Polyhedron_createTetrahedron();
    Polyhedron* hexa = Polyhedron_createHexahedron();
    Polyhedron* ico = Polyhedron_createIcosahedron();
    epic_data.points_count = 2;//ico->vertices.len;
    epic_data.points[0] = (Vector3) { 1, 1, 0 };
    epic_data.points[1] = (Vector3) { -1, 1, 0 };
    Polyhedron* dodeca = Polyhedron_createDodecahedron();
    Polyhedron* octa = Polyhedron_createOctahedron();
    objs[0] = scene_obj_create(tetra, 0, 1, (Vector3) {0, 0, 0}, (Vector3) {0, 0, 0}, (Vector3) {1, 1, 1});
    objs[1] = scene_obj_create(hexa, 0, 0, (Vector3) {0, 0, 0}, (Vector3) {0, 0, 0}, (Vector3) {1, 1, 1});
    objs[3] = scene_obj_create(ico, 0, 0, (Vector3) {0, 0, 0}, (Vector3) {0, 0, 0}, (Vector3) {1, 1, 1});
    objs[4] = scene_obj_create(dodeca, 0, 0, (Vector3) {0, 0, 0}, (Vector3) {0, 0, 0}, (Vector3) {1, 1, 1});
    objs[2] = scene_obj_create(octa, 0, 0,(Vector3) {0, 0, 0}, (Vector3) {0, 0, 0}, (Vector3) {1, 1, 1});
    objs[5] = scene_obj_create(tetra, 0, 0, (Vector3) {0, 0, 0}, (Vector3) {0, 0, 0}, (Vector3) {1, 1, 1});
    current_poly = Polyhedron_create();

    for (int i = 0; i < polyhedron_count; i++) {
        scene_add_obj(scene, objs[i]);
    }

    static char loadFilePath[256] = "Mash/Skull.obj";
    static char saveFilePath[256] = "Mash/result.obj";
    static bool editLoadFile = false;
    static bool editSaveFile = false;

    while (!WindowShouldClose()) {
        Window_Info window = get_window_info();
        SetWindowTitle("Лаба 5");
        scene->zbuffer.width = window.width;
        scene->zbuffer.height = window.height;

        typedef enum {
            MODE_NONE,
            MODE_TRANSLATE,
            MODE_ROTATE,
            MODE_SCALE,
            MODE_REFLECT,
            MODE_ARBITRARY_ROT,
            MODE_LIGHT,
        } TransformMode;

        static TransformMode current_mode = MODE_NONE;

                // Управление камерой - исправленная версия
        float camera_speed = 0.5f;
        float rotation_speed = 0.03f; // скорость вращения

        // Для перспективной проекции - вращение вокруг цели
        if (camera->projection_type == PERSPECTIVE_TYPE) {
            if (IsKeyDown(KEY_W)) {
                // Приближение к цели
                Vector3 direction = Vector3Normalize(Vector3Subtract(camera->target, camera->position));
                camera->position = Vector3Add(camera->position, Vector3Scale(direction, camera_speed));
                update_camera = true;
            }
            if (IsKeyDown(KEY_S)) {
                // Отдаление от цели
                Vector3 direction = Vector3Normalize(Vector3Subtract(camera->target, camera->position));
                camera->position = Vector3Subtract(camera->position, Vector3Scale(direction, camera_speed));
                update_camera = true;
            }

            // Вращение вокруг цели (влево-вправо)
            if (IsKeyDown(KEY_A)) {
                // Вращение камеры влево вокруг цели
                Matrix rotation = MatrixRotateY(rotation_speed);
                Vector3 offset = Vector3Subtract(camera->position, camera->target);
                offset = Vector3Transform(offset, rotation);
                camera->position = Vector3Add(camera->target, offset);
                update_camera = true;
            }
            if (IsKeyDown(KEY_D)) {
                // Вращение камеры вправо вокруг цели
                Matrix rotation = MatrixRotateY(-rotation_speed);
                Vector3 offset = Vector3Subtract(camera->position, camera->target);
                offset = Vector3Transform(offset, rotation);
                camera->position = Vector3Add(camera->target, offset);
                update_camera = true;
            }

            // Вращение вокруг цели (вверх-вниз)
            if (IsKeyDown(KEY_Q)) {
                camera->position.y += camera_speed;
                update_camera = true;
            }
            if (IsKeyDown(KEY_E)) {
                camera->position.y -= camera_speed;
                update_camera = true;
            }
            if (IsKeyPressed(KEY_L)) {
                scene->lighting_mode = 1 - scene->lighting_mode;
            }
        }
        // Для изометрической проекции - обычное движение
        else if (camera->projection_type == ISOMETRIC_TYPE) {
            // Вычисляем направление взгляда камеры
            Vector3 camera_forward = Vector3Normalize(Vector3Subtract(camera->target, camera->position));
            Vector3 camera_right = Vector3Normalize(Vector3CrossProduct(camera_forward, camera->up));

            if (Vector3Length(camera_forward) < 0.001f) {
                camera_forward = (Vector3){0, 0, -1};
                camera_right = (Vector3){1, 0, 0};
            }

            if (IsKeyDown(KEY_W)) {
                camera->position = Vector3Add(camera->position, Vector3Scale(camera_forward, camera_speed));
                camera->target = Vector3Add(camera->target, Vector3Scale(camera_forward, camera_speed));
                update_camera = true;
            }
            if (IsKeyDown(KEY_S)) {
                camera->position = Vector3Subtract(camera->position, Vector3Scale(camera_forward, camera_speed));
                camera->target = Vector3Subtract(camera->target, Vector3Scale(camera_forward, camera_speed));
                update_camera = true;
            }
            if (IsKeyDown(KEY_A)) {
                camera->position = Vector3Subtract(camera->position, Vector3Scale(camera_right, camera_speed));
                camera->target = Vector3Subtract(camera->target, Vector3Scale(camera_right, camera_speed));
                update_camera = true;
            }
            if (IsKeyDown(KEY_D)) {
                camera->position = Vector3Add(camera->position, Vector3Scale(camera_right, camera_speed));
                camera->target = Vector3Add(camera->target, Vector3Scale(camera_right, camera_speed));
                update_camera = true;
            }
            if (IsKeyDown(KEY_Q)) {
                camera->position.y += camera_speed;
                camera->target.y += camera_speed;
                update_camera = true;
            }
            if (IsKeyDown(KEY_E)) {
                camera->position.y -= camera_speed;
                camera->target.y -= camera_speed;
                update_camera = true;
            }
        }

        if (IsKeyDown(KEY_Z)) {
            camera->zoom *= 1.02f;
            update_camera = true;
        }
        if (IsKeyDown(KEY_X)) {
            camera->zoom = fmaxf(0.3, camera->zoom / 1.02f);
            update_camera = true;
        }

        if (IsKeyPressed(KEY_N)) {
            texture_enabled = !texture_enabled;

            TextureZ* current_texture = use_custom_texture ? chess_texture : global_texture;

            for (int i = 0; i < polyhedron_count; i++) {
                if (objs[i] != NULL) {
                    objs[i]->has_texture = texture_enabled;
                    objs[i]->texture = texture_enabled ? current_texture : NULL;
                }
            }

            printf("Texturing %s (%s) for all objects\n",
                texture_enabled ? "ENABLED" : "DISABLED",
                use_custom_texture ? "CHESS" : "LOADED");
        }

        if (IsKeyPressed(KEY_M)) {
            if (!global_texture) {
                printf("No external texture loaded, using chess texture\n");
                use_custom_texture = true;
            }
            else {
                use_custom_texture = !use_custom_texture;

                if (texture_enabled) {
                    TextureZ* current_texture = use_custom_texture ? chess_texture : global_texture;
                    for (int i = 0; i < polyhedron_count; i++) {
                        objs[i]->texture = current_texture;
                    }
                }

                printf("Using %s texture\n", use_custom_texture ? "CHESS" : "LOADED");
            }
        }

        if (epic_data.do_epic_rotate_task) {
            epic_data.do_epic_rotate_task = false;

            Polyhedron *p = Polyhedron_create();
            p->color = YELLOW;

            int points_count = epic_data.points_count;
            Vector3 *points = calloc(1, points_count * sizeof *points);
            for (int i = 0; i < points_count; i++) {
                points[i] = epic_data.points[i];
            }

            int number_of_rotations = (int)epic_data.number_of_rotations;
            Vector3 line;
            if (epic_data.axis == 0) {
                line = (Vector3) { 1, 0, 0 };
            } else if (epic_data.axis == 1) {
                line = (Vector3) { 0, 1, 0 };
            } else if (epic_data.axis == 2) {
                line = (Vector3) { 0, 0, 1 };
            }

            float angle = (2 * PI) / (float)number_of_rotations;
            float current_angle = angle;
            int max_index = 0;
            int indices_len = points_count * 2;
            int *indices = calloc(1, sizeof(int) * indices_len);
            for (int j = 0; j < points_count; j++) {
                Polyhedron_addVertex(p, points[j]);
            }
            for (int i = 1; i < number_of_rotations; i++) {
                for (int j = 0; j < points_count; j++) {
                    Matrix transform = CreateRotationAroundLine((Vector3){0, 0, 0}, line, current_angle);
                    Vector3 v = Vector3Transform(points[j], transform);
                    Polyhedron_addVertex(p, v);
                }

                for (int j = 0; j < (points_count - 1); j++) {
                    int temp_indices[3] = {0};
                    temp_indices[0] = max_index + j;
                    temp_indices[1] = max_index + j + 1;
                    temp_indices[2] = max_index + points_count + j;
                    Polyhedron_addFace(p, temp_indices, 3);

                    temp_indices[0] = max_index + points_count + j;
                    temp_indices[1] = max_index + j + 1;
                    temp_indices[2] = max_index + points_count + j + 1;
                    Polyhedron_addFace(p, temp_indices, 3);
                }

                max_index += points_count;

                current_angle += angle;
            }
            for (int j = 0; j < (points_count - 1); j++) {
                int temp_indices[3] = {0};
                temp_indices[0] = max_index + j;
                temp_indices[1] = max_index + j + 1;
                temp_indices[2] = j;
                Polyhedron_addFace(p, temp_indices, 3);

                temp_indices[0] = j;
                temp_indices[1] = max_index + j + 1;
                temp_indices[2] = j + 1;
                Polyhedron_addFace(p, temp_indices, 3);
            }

            int *top_indices = calloc(1, number_of_rotations * sizeof *indices);
            for (int i = 0; i < number_of_rotations; i++) {
                top_indices[i] = i * points_count;
            }
            Polyhedron_addFace(p, top_indices, number_of_rotations);
            for (int i = 0; i < number_of_rotations; i++) {
                top_indices[i] = (points_count - 1) + (number_of_rotations - 1 - i) * points_count;
            }
            Polyhedron_addFace(p, top_indices, number_of_rotations);

            objs[selected]->visible = false;
            p = Polyhedron_splitToTriangles(p);
            *objs[5] = *scene_obj_create(p, 0, 1, (Vector3) {0, 0, 0}, (Vector3) {0, 0, 0}, (Vector3) {1, 1, 1});
            selected = 5;
            current_poly = p;
        }

        if (epic_data.do_epic_function_task) {
            epic_data.do_epic_function_task = false;

            Polyhedron *p = Polyhedron_create();
            p->color = YELLOW;

            int min_x = epic_data.min_x;
            int max_x = epic_data.max_x;
            int min_y = epic_data.min_z;
            int max_y = epic_data.max_z;

            int indices[3] = {0};

            int max_index = 0;
            float step = 0.25f;
            float x = min_x;
            float y = min_y;
            while (y <= max_y) {
                float (*f)(float x, float y) = funcs[epic_data.function];

                float tx = x;
                float ty = y;
                Polyhedron_addVertex(p, (Vector3) { .x = tx, .y = f(tx, ty), .z = ty });
                tx += step;
                Polyhedron_addVertex(p, (Vector3) { .x = tx, .y = f(tx, ty), .z = ty });
                tx = x;
                ty += step;
                Polyhedron_addVertex(p, (Vector3) { .x = tx, .y = f(tx, ty), .z = ty });
                tx += step;
                Polyhedron_addVertex(p, (Vector3) { .x = tx, .y = f(tx, ty), .z = ty });

                indices[0] = max_index + 0;
                indices[1] = max_index + 2;
                indices[2] = max_index + 1;
                Polyhedron_addFace(p, indices, 3);

                indices[0] = max_index + 1;
                indices[1] = max_index + 2;
                indices[2] = max_index + 3;
                Polyhedron_addFace(p, indices, 3);

                max_index += 4;

                x += step;
                if (x > max_x) {
                    x = min_x;
                    y += step;
                }
            }

            objs[selected]->visible = false;
            p = Polyhedron_splitToTriangles(p);
            *objs[5] = *scene_obj_create(p, 0, 1, (Vector3) {0, 0, 0}, (Vector3) {0, 0, 0}, (Vector3) {1, 1, 1});
            selected = 5;
            current_poly = p;
        }

        // Обработка переключения проекций
        if (IsKeyPressed(KEY_P)) {
            camera->projection_type = PERSPECTIVE_TYPE;
            camera->target = (Vector3){0, 0, 0};
            update_camera = true;
        }
        if (IsKeyPressed(KEY_I)) {
            camera->projection_type = ISOMETRIC_TYPE;
            update_camera = true;
        }

        BeginDrawing();
        ClearBackground(ui_background_color);

        Rectangle dropdown_rect = { 20, 20, 200, 40 };
        DropdownMenu(dropdown_rect, &selected_polygon, &show_dropdown, polyhedron_count, polyhedron_names, change_polyhedron);

        int button_x = 240;
        int button_y = 20;
        int button_width = 200;
        int button_height = 40;
        int button_spacing = 10;

        Rectangle translate_btn = { button_x, button_y, button_width, button_height };
        if (Button(translate_btn, "Смещение")) {
            current_mode = MODE_TRANSLATE;
        }

        Rectangle rotate_btn = { button_x, button_y + button_height + button_spacing, button_width, button_height };
        if (Button(rotate_btn, "Поворот")) {
            current_mode = MODE_ROTATE;
        }

        Rectangle scale_btn = { button_x, button_y + 2 * (button_height + button_spacing), button_width, button_height };
        if (Button(scale_btn, "Масштаб")) current_mode = MODE_SCALE;

        Rectangle reflect_btn = {button_x, button_y+3*(button_height+button_spacing), button_width, button_height};
        if (Button(reflect_btn, "Отражение")) current_mode = MODE_REFLECT;

        Rectangle arb_rotate_btn = {button_x, button_y+4*(button_height+button_spacing), button_width, button_height};
        if (Button(arb_rotate_btn, "Вращение по линии")) current_mode = MODE_ARBITRARY_ROT;

        Rectangle light_btn = {button_x, button_y+5*(button_height+button_spacing), button_width, button_height};
        if (Button(light_btn, "Источник света")) current_mode = MODE_LIGHT;

        Rectangle load_panel = { button_x + 220, button_y, button_width, button_height };
        DrawTextEx(fonts[FONT_MAIN], "Загрузка OBJ:", (Vector2) { load_panel.x + 5, load_panel.y + 5 }, 14, 0, BLACK);
        Rectangle load_file_input = { load_panel.x + 10, load_panel.y + 25, load_panel.width - 20, 25 };
        if (GuiTextBox(load_file_input, loadFilePath, 256, editLoadFile)) {
            editLoadFile = !editLoadFile;
        }
        Rectangle load_btn = { load_panel.x + 10, load_panel.y + 55, load_panel.width - 20, 20 };
        if (Button(load_btn, "Загрузить")) {
            Polyhedron* new_poly = Polyhedron_create();

            if (Polyhedron_loadFromObj(new_poly, loadFilePath)) {
                new_poly = Polyhedron_splitToTriangles(new_poly);

                printf("Модель успешно загружена из %s\n", loadFilePath);

                if (objs[5]->mesh != NULL) {
                    Polyhedron_free(objs[5]->mesh);
                }

                objs[5]->mesh = new_poly;
                objs[5]->has_texture = texture_enabled;
                objs[5]->texture = texture_enabled ? (use_custom_texture ? chess_texture : global_texture) : NULL;
                objs[5]->bounding_radius = Polyhedron_bounding_radius(new_poly);

                prev_obj = selected;
                selected = 5;
            }
            else {
                printf("Ошибка загрузки модели из %s\n", loadFilePath);
            }
        }

        Rectangle save_panel = { button_x + 220, button_y + (button_height + button_spacing) + 30, button_width,  button_height };

        DrawTextEx(fonts[FONT_MAIN], "Сохранение OBJ:", (Vector2) { save_panel.x + 5, save_panel.y + 5 }, 14, 0, BLACK);

        Rectangle save_file_input = { save_panel.x + 10, save_panel.y + 25, save_panel.width - 20, 25 };
        if (GuiTextBox(save_file_input, saveFilePath, 256, editSaveFile)) {
            editSaveFile = !editSaveFile;
        }

        Rectangle save_btn = { save_panel.x + 10, save_panel.y + 55, save_panel.width - 20, 20 };
        if (Button(save_btn, "Сохранить")) {
            if (Polyhedron_saveToObj(current_poly, saveFilePath)) {
                printf("Модель успешно сохранена в %s\n", saveFilePath);
            }
            else {
                printf("Ошибка сохранения модели в %s\n", saveFilePath);
            }
        }

        Rectangle texture_panel = { button_x + 220, button_y + 2 * (button_height + button_spacing) + 60, button_width, button_height };

        DrawTextEx(fonts[FONT_MAIN], "Загрузка текстуры:", (Vector2) { texture_panel.x + 5, texture_panel.y + 5 }, 14, 0, BLACK);

        Rectangle texture_file_input = { texture_panel.x + 10, texture_panel.y + 25, texture_panel.width - 20, 25 };
        if (GuiTextBox(texture_file_input, textureFilePath, 256, editTextureFile)) {
            editTextureFile = !editTextureFile;
        }

        Rectangle texture_load_btn = { texture_panel.x + 10, texture_panel.y + 55, (texture_panel.width - 30) / 2, 20 };
        if (Button(texture_load_btn, "Загрузить")) {
            TextureZ* new_texture = load_texture_from_file(textureFilePath);
            if (new_texture) {
                if (global_texture) {
                    free(global_texture->pixels);
                    free(global_texture);
                }
                global_texture = new_texture;
                use_custom_texture = false; 

                if (texture_enabled) {
                    for (int i = 0; i < polyhedron_count; i++) {
                        objs[i]->texture = global_texture;
                    }
                }
                printf("Texture successfully loaded from %s\n", textureFilePath);
            }
            else {
                printf("Failed to load texture from %s\n", textureFilePath);
            }
        }

        Rectangle texture_chess_btn = { texture_panel.x + 20 + (texture_panel.width - 30) / 2, texture_panel.y + 55, (texture_panel.width - 30) / 2, 20 };
        if (Button(texture_chess_btn, "Шахматная")) {
            use_custom_texture = true;

            if (texture_enabled) {
                for (int i = 0; i < polyhedron_count; i++) {
                    objs[i]->texture = chess_texture;
                }
            }
            printf("Switched to chess texture\n");
        }

        Rectangle reset_btn = { button_x, button_y + 6 * (button_height + button_spacing), button_width, button_height };
        if (Button(reset_btn, "Сбросить всё")) {
            user_translation = (Vector3) {0, 0, 0};
            user_rotation = (Vector3) {0, 0, 0};
            user_scale = (Vector3) {1, 1, 1};
            line_p1 =  (Vector3) {0, 0, 0};
            line_p2 =  (Vector3) {0, 0, 0};

            current_mode = MODE_NONE;
        }

        int panel_x = 240;
        int panel_y = button_y + 7 * (button_height + button_spacing) + 10;
        int panel_width = 300;
        int panel_height = 200;

        DrawRectangle(panel_x, panel_y, panel_width, panel_height, Fade(LIGHTGRAY, 0.8f));
        DrawRectangleLines(panel_x, panel_y, panel_width, panel_height, DARKGRAY);

        int param_y = panel_y + 10;

        switch (current_mode) {
        case MODE_TRANSLATE: {
            DrawTextEx(fonts[FONT_MAIN], "Ось смещения:",(Vector2) {.x = panel_x + 10, .y = param_y},16, 0, BLACK);
            param_y += 25;

            Rectangle axis_x_btn = { panel_x + 20, param_y, 60, 25 };
            Rectangle axis_y_btn = { panel_x + 90, param_y, 60, 25 };
            Rectangle axis_z_btn = { panel_x + 160, param_y, 60, 25 };

            if (Button(axis_x_btn, "X")) axis_translation = &user_translation.x;
            if (Button(axis_y_btn, "Y")) axis_translation = &user_translation.y;
            if (Button(axis_z_btn, "Z")) axis_translation = &user_translation.z;

            param_y += 40;

            const char* axis_names[] = { "X", "Y", "Z" };
            DrawTextEx(fonts[FONT_MAIN], "Смещение по оси:",(Vector2) {.x = panel_x + 10, .y = param_y},16, 0, BLACK);

            GuiSliderBar((Rectangle) { panel_x + 40, param_y + 25, 120, 20 }, "-20", "20", axis_translation, -20.0f, 20.0f);

            param_y += 50;

        } break;

        case MODE_ROTATE: {
            DrawTextEx(fonts[FONT_MAIN], "Ось вращения:",(Vector2) {.x = panel_x + 10, .y = param_y},16, 0, BLACK);
            param_y += 25;

            Rectangle axis_x_btn = { panel_x + 20, param_y, 60, 25 };
            Rectangle axis_y_btn = { panel_x + 90, param_y, 60, 25 };
            Rectangle axis_z_btn = { panel_x + 160, param_y, 60, 25 };

            if (Button(axis_x_btn, "X")) axis_rotation = &user_rotation.x;
            if (Button(axis_y_btn, "Y")) axis_rotation = &user_rotation.y;
            if (Button(axis_z_btn, "Z")) axis_rotation = &user_rotation.z;

            param_y += 40;

            DrawTextEx(fonts[FONT_MAIN], "Угол поворота:",(Vector2) {.x = panel_x + 10, .y = param_y},16, 0, BLACK);
            param_y += 25;

            GuiSliderBar((Rectangle) { panel_x + 20, param_y, 150, 20 }, "0", "360", axis_rotation, 0.0f, 360.0f);
        } break;

        case MODE_SCALE: {
            DrawTextEx(fonts[FONT_MAIN], "Масштабирование:",(Vector2) {.x = panel_x + 10, .y = param_y},16, 0, BLACK);
            param_y += 25;

            DrawTextEx(fonts[FONT_MAIN], "Масштаб X:",(Vector2) {.x = panel_x + 10, .y = param_y},14, 0, BLACK);
            GuiSliderBar((Rectangle) { panel_x + 100, param_y, 120, 20 }, "0.1", "10", &user_scale.x, 0.1f, 10.0f);
            param_y += 25;

            DrawTextEx(fonts[FONT_MAIN], "Масштаб Y:",(Vector2) {.x = panel_x + 10, .y = param_y },14, 0, BLACK);
            GuiSliderBar((Rectangle) { panel_x + 100, param_y, 120, 20 }, "0.1", "10", &user_scale.y, 0.1f, 10.0f);
            param_y += 25;

            DrawTextEx(fonts[FONT_MAIN], "Масштаб Z:",(Vector2) {.x = panel_x + 10, .y = param_y},14, 0, BLACK);
            GuiSliderBar((Rectangle) { panel_x + 100, param_y, 120, 20 }, "0.1", "10", &user_scale.z, 0.1f, 10.0f);
            param_y += 25;

            DrawTextEx(fonts[FONT_MAIN], "Общий масштаб:", (Vector2) { panel_x + 10, param_y }, 14, 0, BLACK);
            static float uniform_scale = 1.0f;  
            static float previous_scale = 1.0f;
            if (GuiSliderBar((Rectangle) { panel_x + 120, param_y, 120, 20 }, "0.1", "10", &uniform_scale, 0.1f, 10.0f)) {
                float scale_factor = uniform_scale / previous_scale;

                user_scale.x = Clamp(user_scale.x * scale_factor, 0.1f, 10);
                user_scale.y = Clamp(user_scale.y * scale_factor, 0.1f, 10);
                user_scale.z = Clamp(user_scale.z * scale_factor, 0.1f, 10);
            }
            else {
                uniform_scale = (user_scale.x + user_scale.y + user_scale.z) / 3.0f;
                previous_scale = uniform_scale;
            }
            param_y += 35;

        } break;
        case MODE_REFLECT: {
            DrawTextEx(fonts[FONT_MAIN],"Плоскость отражения:",(Vector2){panel_x+10,param_y},16,0,BLACK); param_y+=25;
            Rectangle btnX={panel_x+20,param_y,60,25};
            Rectangle btnY={panel_x+90,param_y,60,25};
            Rectangle btnZ={panel_x+160,param_y,60,25};
            Rectangle btnF={panel_x+230,param_y,60,25};
            if (Button(btnX, "X")) reflection_plane='X';
            if (Button(btnY, "Y")) reflection_plane='Y';
            if (Button(btnZ, "Z")) reflection_plane='Z';
            if (Button(btnF, "Вернуть")) reflection_plane=0;
        } break;

        case MODE_ARBITRARY_ROT: {
            DrawTextEx(fonts[FONT_MAIN],"Вращение вокруг линии", (Vector2){panel_x+10,param_y},16,0,BLACK);
            param_y+=25;
            GuiSliderBar((Rectangle){panel_x+20,param_y,120,20},"-5","5",&line_p1.x,-20,20);
            GuiSliderBar((Rectangle){panel_x+150,param_y,120,20},"-5","5",&line_p2.x,-20,20);
            param_y+=25;
            GuiSliderBar((Rectangle){panel_x+20,param_y,120,20},"-5","5",&line_p1.y,-20,20);
            GuiSliderBar((Rectangle){panel_x+150,param_y,120,20},"-5","5",&line_p2.y,-20,20);
            param_y+=25;
            GuiSliderBar((Rectangle){panel_x+20,param_y,120,20},"-5","5",&line_p1.z,-20,20);
            GuiSliderBar((Rectangle){panel_x+150,param_y,120,20},"-5","5",&line_p2.z,-20,20);
            param_y+=25;
            GuiSliderBar((Rectangle){panel_x+20,param_y,250,20},"0","360",&line_angle,0,360);
        } break;
        case MODE_LIGHT: {
            DrawTextEx(fonts[FONT_MAIN], "Положение источника света:", (Vector2){panel_x+10, param_y}, 16, 0, BLACK);
            param_y += 25;

            DrawTextEx(fonts[FONT_MAIN], "X:", (Vector2){panel_x+10, param_y}, 14, 0, BLACK);
            GuiSliderBar((Rectangle){panel_x+30, param_y, 200, 20}, "-20", "20", &scene->light.position.x, -20.0f, 20.0f);
            param_y += 25;

            DrawTextEx(fonts[FONT_MAIN], "Y:", (Vector2){panel_x+10, param_y}, 14, 0, BLACK);
            GuiSliderBar((Rectangle){panel_x+30, param_y, 200, 20}, "-20", "20", &scene->light.position.y, -20.0f, 20.0f);
            param_y += 25;

            DrawTextEx(fonts[FONT_MAIN], "Z:", (Vector2){panel_x+10, param_y}, 14, 0, BLACK);
            GuiSliderBar((Rectangle){panel_x+30, param_y, 200, 20}, "-20", "20", &scene->light.position.z, -20.0f, 20.0f);
            param_y += 30;

            DrawTextEx(fonts[FONT_MAIN], "Интенсивность:", (Vector2){panel_x+10, param_y}, 14, 0, BLACK);
            param_y += 20;
            GuiSliderBar((Rectangle){panel_x+30, param_y, 200, 20}, "0.0", "2.0", &scene->light.intensity, 0.0f, 2.0f);
        } break;
        case MODE_NONE:
        default: break;
        }

        if (update_camera) {
            cameraz_update(camera);
            update_camera = false;
        }

        if (reflection_plane != 0) {
            Polyhedron* current_poly = objs[selected]->mesh;

            Vector3 v = current_poly->vertices.head[0].position;  // первая вершина
            Matrix reflMat = CreateReflectionMatrix(reflection_plane);

            // отражение относительно центра
            Matrix toOrigin = CreateTranslationMatrix((Vector3){ -current_poly->center.x, -current_poly->center.y, -current_poly->center.z });
            Matrix fromOrigin = CreateTranslationMatrix(current_poly->center);
            Matrix fullRefl = MatrixMultiply(fromOrigin, MatrixMultiply(reflMat, toOrigin));

            Vector3 v_after = Vector3Transform(v, fullRefl);

            printf("[DEBUG] plane=%c | before=(%.2f, %.2f, %.2f) -> after=(%.2f, %.2f, %.2f)\n", reflection_plane, v.x, v.y, v.z, v_after.x, v_after.y, v_after.z);
        }

        for (int i = 0; i < 6; i++) {
            objs[i]->visible = false;
        }
        //objs[selected]->texture = global_texture;
        objs[selected]->visible = true;
        objs[selected]->position = user_translation;
        objs[selected]->rotation = (Vector3){user_rotation.x, user_rotation.y, user_rotation.z};
        objs[selected]->scale = user_scale;
        objs[selected]->reflection_plane = reflection_plane;
        objs[selected]->line_p1 = line_p1;
        objs[selected]->line_p2 = line_p2;
        objs[selected]->line_angle = line_angle * DEG2RAD;
        scene_draw(scene);

        // ЗДЕСЬ Я НАРИСУЮ СВОЙ, ПО НАСТОЯЩЕМУ КРУТОЙ UI!
        if (epic_data.hide && Button((Rectangle) { .x = window.width - 190, .y = 10, .width = 180, .height = 60 }, "Нажми Меня!")) {
            epic_data.hide = false;
        }
        draw_epic_ui_for_my_EPIC_tasks(&epic_data);

        //Vector2 p1 = cameraz_world_to_screen(line_p1, camera);
        //Vector2 p2 = cameraz_world_to_screen(line_p2, camera);

        //DrawLineV(p1, p2, BLACK);
        EndDrawing();
    }
    Polyhedron_free(tetra);
    Polyhedron_free(hexa);
    Polyhedron_free(octa);
    Polyhedron_free(ico);
    Polyhedron_free(dodeca);
    if (chess_texture) {
        free(chess_texture->pixels);
        free(chess_texture);
    }
    if (global_texture) {
        free(global_texture->pixels);
        free(global_texture);
    }
}

void init() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(menu_window_width, menu_window_height, "Лаба 4");

    uint64_t total_memory_size = Clay_MinMemorySize();
    Clay_Arena arena = Clay_CreateArenaWithCapacityAndMemory(total_memory_size, malloc(total_memory_size));
    Clay_Initialize(arena, (Clay_Dimensions) { task_window_width, task_window_height }, (Clay_ErrorHandler){ NULL });

    fonts[FONT_FOR_DEBUG_WINDOW] = LoadFontEx("fonts/jetbrains-mono.ttf", 16, 0, 0);

    int codepoints[512] = {0};
    int count = 0;
    // ASCII
    for (int i = 0x00; i <= 0x7F; i++) {
        codepoints[count++] = i;
    }
    // UTF8 Кириллица
    for (int i = 0x400; i <= 0x4FF; i++) {
        codepoints[count++] = i;
    }
    fonts[FONT_MAIN] = LoadFontEx("fonts/jetbrains-mono.ttf", 32, codepoints, count);

    Clay_SetMeasureTextFunction(Raylib_MeasureText, fonts);
}

bool Button(Rectangle bounds, const char *text) {
    Vector2 mouse = GetMousePosition();
    bool hover = CheckCollisionPointRec(mouse, bounds);
    bool pressed = hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    Color color = hover ? LIGHTGRAY : GRAY;

    DrawRectangleRec(bounds, color);
    DrawRectangleLinesEx(bounds, 2, BLACK);

    int fontSize = 20;
    Vector2 textSize = MeasureTextEx(fonts[FONT_MAIN], text, fontSize, 1);
    DrawTextEx(fonts[FONT_MAIN], text,
        (Vector2){ .x = bounds.x + (bounds.width - textSize.x) / 2,.y = bounds.y + (bounds.height - textSize.y) / 2},
        fontSize,
        0,
        BLACK);

    return pressed;
}

void DropdownMenu(Rectangle bounds, int* selectedOption, bool* showDropdown, int optionsCount, const char** options, void (*func)(int)) {
    if (Button((Rectangle){bounds.x, bounds.y, bounds.width, bounds.height}, options[*selectedOption]))
    {
        *showDropdown = !*showDropdown;
    }

    // Show dropdown if button pressed
    if (*showDropdown)
    {
        int button_cnt = 1;
        for (int i = 0; i < optionsCount; i++)
        {
            if (i == *selectedOption) continue;
            if (Button((Rectangle){bounds.x, bounds.y + button_cnt*50, bounds.width, bounds.height}, options[i]))
            {
                *selectedOption = i;
                *showDropdown = false;
                func(*selectedOption);
            }
            button_cnt++;
        }
    }
}

void draw_epic_ui_for_my_EPIC_tasks(My_Epic_Tasks_Info *epic_data) {
    if (epic_data->hide) {
        return;
    }
    Window_Info window = get_window_info();

    int ui_area_width = 260;
    Rectangle area = { .x = window.width - ui_area_width, .y = 0, .width = ui_area_width, .height = window.height };
    DrawRectangleRec(area, WHITE);

    int margin_left = 10;
    int margin_right = margin_left;
    int margin_top = 20;
    int button_y_pad = 5;
    int button_height = 30;

    int all_buttons_x = area.x + margin_left;
    int all_buttons_width = area.width - margin_left - margin_right;
    float y = area.y + margin_top;
    Rectangle function_button = {
        .x = all_buttons_x,
        .y = y,
        .width = all_buttons_width,
        .height = button_height,
    };
    y += button_height + button_y_pad;
    Rectangle rotate_body_button = {
        .x = all_buttons_x,
        .y = y,
        .width = all_buttons_width,
        .height = button_height,
    };
    y += button_height + button_y_pad;
    if (Button(function_button, "Создать функцию")) {
        epic_data->do_epic_function_task = true;
        epic_data->mode = 1;
    }
    if (Button(rotate_body_button, "Создать тело вращения")) {
        epic_data->do_epic_rotate_task = true;
        epic_data->mode = 2;
    }

    if (epic_data->mode == 1) {
        //GuiSliderBar((Rectangle){panel_x+20,param_y,250,20},"0","360",&line_angle,0,360);

        int button_x = area.x + margin_left;
        int button_width = all_buttons_width;
        Rectangle slider = {
            .x = button_x + 20,
            .y = y,
            .width = button_width - 40,
            .height = button_height,
        };

        DrawTextEx(fonts[FONT_MAIN],"Min_X", (Vector2){slider.x, slider.y}, 16, 0, BLACK);
        slider.y += 20;
        if (GuiSliderBar(slider, "-10", "10", &epic_data->min_x, -10.0f, 10.0f)) {
            epic_data->do_epic_function_task = true;
        }

        slider.y += button_height;
        DrawTextEx(fonts[FONT_MAIN],"Max_X", (Vector2){slider.x, slider.y}, 16, 0, BLACK);
        slider.y += 20;
        if (GuiSliderBar(slider, "-10", "10", &epic_data->max_x, -10.0f, 10.0f)) {
            epic_data->do_epic_function_task = true;
        }

        slider.y += button_height;
        DrawTextEx(fonts[FONT_MAIN],"Min_Z", (Vector2){slider.x, slider.y}, 16, 0, BLACK);
        slider.y += 20;
        if (GuiSliderBar(slider, "-10", "10", &epic_data->min_z, -10.0f, 10.0f)) {
            epic_data->do_epic_function_task = true;
        }

        slider.y += button_height;
        DrawTextEx(fonts[FONT_MAIN],"Max_Z", (Vector2){slider.x, slider.y}, 16, 0, BLACK);
        slider.y += 20;
        if (GuiSliderBar(slider, "-10", "10", &epic_data->max_z, -10.0f, 10.0f)) {
            epic_data->do_epic_function_task = true;
        }

        slider.y += slider.height + 30;

        const char *func_names[ARRAY_LEN(funcs)] = {
            "f(x) = cos(x) + cos(y)",
            "f(x) = x + y",
            "f(x) = 1",
            "f(x) = atan2(y, x)",
            "f(x) = cos(x)^2 - sin(x)^2",
            "TOP 1 f(x)",
        };
        for (int i = 0; i < ARRAY_LEN(funcs); i++) {
            if (GuiButton(slider, func_names[i])) {
                epic_data->function = i;
                epic_data->do_epic_function_task = true;
            }
            slider.y += button_height + button_y_pad;
        }
    } else if (epic_data->mode == 2) {
        int button_x = area.x + margin_left;
        int button_width = all_buttons_width;
        Rectangle slider = {
            .x = button_x + 20,
            .y = y,
            .width = button_width - 40,
            .height = button_height,
        };

        if (Button(slider, "Добавить Точку")) {
            epic_data->points_count += 1;
        }
        slider.y += slider.height + 10;
        if (epic_data->points_count > 0 && Button(slider, "Удалить Точку")) {
            epic_data->points_count -= 1;
        }
        slider.y += slider.height + 10;

        for (int i = 0; i < epic_data->points_count; i++) {
            DrawTextEx(fonts[FONT_MAIN], TextFormat("Точка №%d", i + 1), (Vector2) {slider.x, slider.y}, 16, 0, BLACK);
            slider.y += 20;
            Rectangle rec = slider;
            rec.x = button_x;
            rec.width /= 3;
            rec.height *= 0.75f;
            if (GuiSliderBar(rec, "-1", "1", &epic_data->points[i].x, -5.0f, 5.0f)) {
                epic_data->do_epic_rotate_task = true;
            }
            rec.x += rec.width + 20;
            if (GuiSliderBar(rec, "-1", "1", &epic_data->points[i].y, -5.0f, 5.0f)) {
                epic_data->do_epic_rotate_task = true;
            }
            rec.x += rec.width + 20;
            if (GuiSliderBar(rec, "-1", "1", &epic_data->points[i].z, -5.0f, 5.0f)) {
                epic_data->do_epic_rotate_task = true;
            }
            slider.y += rec.height + 5;
        }

        DrawTextEx(fonts[FONT_MAIN],"Количество разбиений", (Vector2){slider.x, slider.y}, 16, 0, BLACK);
        slider.y += 20;
        if (GuiSliderBar(slider, "1", "360", &epic_data->number_of_rotations, 1.0f, 360.0f)) {
            epic_data->do_epic_rotate_task = true;
        }


        slider.y += slider.height + 30;

        const char *names[] = {
            "Ось X",
            "Ось Y",
            "Ось Z",
        };
        for (int i = 0; i < ARRAY_LEN(names); i++) {
            if (Button(slider, names[i])) {
                epic_data->axis = i;
                epic_data->do_epic_rotate_task = true;
            }
            slider.y += button_height + button_y_pad;
        }
    }

    if (Button((Rectangle) { .x = area.x + margin_left, .y = area.y + area.height - 100, .width = all_buttons_width, .height = 90 }, "Скрыть")) {
        epic_data->hide = true;
    }
}

void change_polyhedron(int new_selection) {
    objs[prev_obj]->visible = false;
    objs[new_selection]->visible = true;

    prev_obj = new_selection;
    selected = new_selection;

    // user_translation = (Vector3) {0, 0, 0};
    // user_rotation = (Vector3) {0, 0, 0};
    // user_scale = (Vector3) {1, 1, 1};
}
