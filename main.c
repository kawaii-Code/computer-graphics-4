#include "common.h"
#include <time.h>
#include "Polyhedron.h"
#include "camera.h"
#include "scene.h"

#define RAYGUI_IMPLEMENTATION
#include "third_party/include/raygui.h"

Font fonts[FONT_COUNT];

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
const int polyhedron_count = 6;

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

int main(int argc, char **argv) {
    init();
    SetWindowSize(menu_window_width, menu_window_height);
    srand(time(NULL));

    CameraZ* camera = cameraz_create((Vector3) {0, 0, 10},
        (Vector3) {0, 0, 0},
        (Vector3) {0, 1, 0},
        45, 1, 1, 0.1, 1000, menu_window_width, menu_window_height, PERSPECTIVE_TYPE);
    Scene* scene = scene_create(camera);

    Polyhedron* tetra = Polyhedron_createTetrahedron();
    Polyhedron* hexa = Polyhedron_createHexahedron();
    Polyhedron* ico = Polyhedron_createIcosahedron();
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

    static char loadFilePath[256] = "Mash/utah_teapot_lowpoly.obj";
    static char saveFilePath[256] = "Mash/result.obj";
    static bool editLoadFile = false;
    static bool editSaveFile = false;

    while (!WindowShouldClose()) {
        Window_Info window = get_window_info();
        SetWindowTitle("Лаба 5");

        typedef enum {
            MODE_NONE,
            MODE_TRANSLATE,
            MODE_ROTATE,
            MODE_SCALE,
            MODE_REFLECT,
            MODE_ARBITRARY_ROT,
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

        Rectangle load_panel = { button_x + 220, button_y, button_width, button_height };
        DrawTextEx(fonts[FONT_MAIN], "Загрузка OBJ:", (Vector2) { load_panel.x + 5, load_panel.y + 5 }, 14, 0, BLACK);
        Rectangle load_file_input = { load_panel.x + 10, load_panel.y + 25, load_panel.width - 20, 25 };
        if (GuiTextBox(load_file_input, loadFilePath, 256, editLoadFile)) {
            editLoadFile = !editLoadFile;
        }
        Rectangle load_btn = { load_panel.x + 10, load_panel.y + 55, load_panel.width - 20, 20 };
        if (Button(load_btn, "Загрузить")) {
            if (Polyhedron_loadFromObj(current_poly, loadFilePath)) {
                printf("Модель успешно загружена из %s\n", loadFilePath);

                objs[selected]->visible = false;
                *objs[5] = *scene_obj_create(current_poly, 0, 1, (Vector3) {0, 0, 0}, (Vector3) {0, 0, 0}, (Vector3) {1, 1, 1});
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

        Rectangle reset_btn = { button_x, button_y + 5 * (button_height + button_spacing), button_width, button_height };
        if (Button(reset_btn, "Сбросить всё")) {
            user_translation = (Vector3) {0, 0, 0};
            user_rotation = (Vector3) {0, 0, 0};
            user_scale = (Vector3) {1, 1, 1};
            line_p1 =  (Vector3) {0, 0, 0};
            line_p2 =  (Vector3) {0, 0, 0};

            current_mode = MODE_NONE;
        }

        int panel_x = 240;
        int panel_y = button_y + 6 * (button_height + button_spacing) + 10;
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

            printf("[DEBUG] plane=%c | before=(%.2f, %.2f, %.2f) -> after=(%.2f, %.2f, %.2f)\n",
                   reflection_plane, v.x, v.y, v.z, v_after.x, v_after.y, v_after.z);
        }

        objs[selected]->position = user_translation;
        objs[selected]->rotation = (Vector3){user_rotation.x, user_rotation.y, user_rotation.z};
        objs[selected]->scale = user_scale;
        objs[selected]->reflection_plane = reflection_plane;
        objs[selected]->line_p1 = line_p1;
        objs[selected]->line_p2 = line_p2;
        objs[selected]->line_angle = line_angle * DEG2RAD;
        scene_draw(scene);

        Vector2 p1 = cameraz_world_to_screen(line_p1, camera);
        Vector2 p2 = cameraz_world_to_screen(line_p2, camera);

        DrawLineV(p1, p2, BLACK);
        EndDrawing();
    }
    Polyhedron_free(tetra);
    Polyhedron_free(hexa);
    Polyhedron_free(octa);
    Polyhedron_free(ico);
    Polyhedron_free(dodeca);
    free_positions();
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


void change_polyhedron(int new_selection) {
    objs[prev_obj]->visible = false;
    objs[new_selection]->visible = true;

    prev_obj = new_selection;
    selected = new_selection;

    // user_translation = (Vector3) {0, 0, 0};
    // user_rotation = (Vector3) {0, 0, 0};
    // user_scale = (Vector3) {1, 1, 1};
}
