#include "common.h"
#include <time.h>
#include "Polyhedron.h"

#define RAYGUI_IMPLEMENTATION
#include "third_party/include/raygui.h"

Font fonts[FONT_COUNT];

void change_polyhedron(int new_selection);

int selected_polygon = 0;
bool show_dropdown = false;

Vector3 translation = { 0 };
Vector3 rotation_axis = { 0, 1, 0 };

float rotation_angle = 0;
Vector3 scale = { 1, 1, 1 };

Polyhedron current_poly;
bool poly_initialized = false;

const char* polyhedron_names[] = {
    "Тетраэдр",
    "Гексаэдр (Куб)",
    "Октаэдр",
    "Икосаэдр"
    //"Додекаэдр"
};
const int polyhedron_count = 4;

int main(int argc, char **argv) {
    init();
    SetWindowSize(menu_window_width, menu_window_height);
    srand(time(NULL)); 

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 10.0f, 10.0f, 10.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;//вот вопрос, нам нужно самим это сделать?

    Polyhedron tetra, hexa, octa, ico, dodeca;
    Polyhedron_createTetrahedron(&tetra);
    Polyhedron_createHexahedron(&hexa);
    Polyhedron_createOctahedron(&octa);
    Polyhedron_createIcosahedron(&ico);
    //Polyhedron_createDodecahedron(&dodeca);

    current_poly = tetra;
    poly_initialized = true;

    Matrix transform = MatrixIdentity();


    while (!WindowShouldClose()) {
        Window_Info window = get_window_info();
        SetWindowTitle("Лаба 5");

        Vector2 current_mouse_position = GetMousePosition();

        typedef enum {
            MODE_NONE,
            MODE_TRANSLATE,
            MODE_ROTATE,
            MODE_SCALE
        } TransformMode;

        static TransformMode current_mode = MODE_NONE;

        static int translate_axis = 0; // 0=X, 1=Y, 2=Z
        static int rotate_axis = 1;    // 0=X, 1=Y, 2=Z

        static float temp_translation[3] = { 0 };
        static float temp_rotation_angle = 0;
        static float temp_scale[3] = { 1, 1, 1 };
        static float rotation_angles[3] = { 0, 0, 0 };

        temp_translation[0] = translation.x;
        temp_translation[1] = translation.y;
        temp_translation[2] = translation.z;
        temp_scale[0] = scale.x;
        temp_scale[1] = scale.y;
        temp_scale[2] = scale.z;

        bool rotate_around_center = false;

        //UpdateCamera(&camera, CAMERA_ORBITAL);
        
        if (IsKeyDown(KEY_W)) camera.position.z -= 0.1f;
        if (IsKeyDown(KEY_S)) camera.position.z += 0.1f;
        if (IsKeyDown(KEY_A)) camera.position.x -= 0.1f;
        if (IsKeyDown(KEY_D)) camera.position.x += 0.1f;
        if (IsKeyDown(KEY_Q)) camera.position.y -= 0.1f;
        if (IsKeyDown(KEY_E)) camera.position.y += 0.1f;

        //Matrix user_transform = CreateTransformMatrix(translation, rotation_angles, rotation_axis, scale);
        Matrix user_transform = CreateTransformMatrix(&current_poly, translation, rotation_angles, rotation_axis, scale);

        BeginDrawing();
        ClearBackground(ui_background_color);

        Rectangle dropdown_rect = { 20, 20, 200, 40 };
        DropdownMenu(dropdown_rect, &selected_polygon, &show_dropdown,
            polyhedron_count, polyhedron_names, change_polyhedron);

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
        if (Button(scale_btn, "Масштаб")) {
            current_mode = MODE_SCALE;
        }

        Rectangle reset_btn = { button_x, button_y + 3 * (button_height + button_spacing), button_width, button_height };
        if (Button(reset_btn, "Сбросить всё")) {
            translation = (Vector3){ 0 };
            rotation_angle = 0;
            rotation_axis = (Vector3){ 0, 1, 0 };
            scale = (Vector3){ 1, 1, 1 };
            current_mode = MODE_NONE;
            temp_translation[0] = temp_translation[1] = temp_translation[2] = 0;
            temp_rotation_angle = 0;
            temp_scale[0] = temp_scale[1] = temp_scale[2] = 1;
            rotation_angles[0] = rotation_angles[1] = rotation_angles[2] = 0;
        }

        int panel_x = 240;
        int panel_y = button_y + 4 * (button_height + button_spacing) + 10;
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

            if (Button(axis_x_btn, "X")) translate_axis = 0;
            if (Button(axis_y_btn, "Y")) translate_axis = 1;
            if (Button(axis_z_btn, "Z")) translate_axis = 2;

            param_y += 40;

            const char* axis_names[] = { "X", "Y", "Z" };
            DrawTextEx(fonts[FONT_MAIN], "Смещение по оси:",(Vector2) {.x = panel_x + 10, .y = param_y},16, 0, BLACK);

            GuiSliderBar((Rectangle) { panel_x + 40, param_y + 25, 120, 20 }, "-20", "20", & temp_translation[translate_axis], -20.0f, 20.0f);

            translation.x = temp_translation[0];
            translation.y = temp_translation[1];
            translation.z = temp_translation[2];

            param_y += 50;

        } break;

        case MODE_ROTATE: {
            DrawTextEx(fonts[FONT_MAIN], "Ось вращения:",(Vector2) {.x = panel_x + 10, .y = param_y},16, 0, BLACK);
            param_y += 25;

            Rectangle axis_x_btn = { panel_x + 20, param_y, 60, 25 };
            Rectangle axis_y_btn = { panel_x + 90, param_y, 60, 25 };
            Rectangle axis_z_btn = { panel_x + 160, param_y, 60, 25 };

            static int prev_rotate_axis = -1;

            if (Button(axis_x_btn, "X")) {
                rotate_axis = 0;
                rotation_axis = (Vector3){ 1, 0, 0 };
            }
            if (Button(axis_y_btn, "Y")) {
                rotate_axis = 1;
                rotation_axis = (Vector3){ 0, 1, 0 };
            }
            if (Button(axis_z_btn, "Z")) {
                rotate_axis = 2;
                rotation_axis = (Vector3){ 0, 0, 1 };
            }


            param_y += 40;

            DrawTextEx(fonts[FONT_MAIN], "Угол поворота:",(Vector2) {.x = panel_x + 10, .y = param_y},16, 0, BLACK);
            param_y += 25;

            GuiSliderBar((Rectangle) { panel_x + 20, param_y, 150, 20 }, "0", "360", & rotation_angles[rotate_axis], 0.0f, 360.0f);
            temp_rotation_angle = rotation_angles[rotate_axis];
            rotation_angle = temp_rotation_angle * DEG2RAD;

        } break;

        case MODE_SCALE: {
            DrawTextEx(fonts[FONT_MAIN], "Масштабирование:",(Vector2) {.x = panel_x + 10, .y = param_y},16, 0, BLACK);
            param_y += 25;

            DrawTextEx(fonts[FONT_MAIN], "Масштаб X:",(Vector2) {.x = panel_x + 10, .y = param_y},14, 0, BLACK);
            GuiSliderBar((Rectangle) { panel_x + 100, param_y, 120, 20 }, "0.1", "10", & temp_scale[0], 0.1f, 10.0f);
            scale.x = temp_scale[0];
            param_y += 25;

            DrawTextEx(fonts[FONT_MAIN], "Масштаб Y:",(Vector2) {.x = panel_x + 10, .y = param_y },14, 0, BLACK);
            GuiSliderBar((Rectangle) { panel_x + 100, param_y, 120, 20 }, "0.1", "10", & temp_scale[1], 0.1f, 10.0f);
            scale.y = temp_scale[1];
            param_y += 25;

            DrawTextEx(fonts[FONT_MAIN], "Масштаб Z:",(Vector2) {.x = panel_x + 10, .y = param_y},14, 0, BLACK);
            GuiSliderBar((Rectangle) { panel_x + 100, param_y, 120, 20 }, "0.1", "10", & temp_scale[2], 0.1f, 10.0f);
            scale.z = temp_scale[2];
            param_y += 25;

            DrawTextEx(fonts[FONT_MAIN], "Общий масштаб:", (Vector2) { panel_x + 10, param_y }, 14, 0, BLACK);
            static float uniform_scale = 1.0f;  
            static float previous_scale = 1.0f;
            if (GuiSliderBar((Rectangle) { panel_x + 120, param_y, 120, 20 }, "0.1", "10", & uniform_scale, 0.1f, 10.0f)) {
                float scale_factor = uniform_scale / previous_scale;

                float new_scale[3];
                for (int i = 0; i < 3; i++) {
                    new_scale[i] = temp_scale[i] * scale_factor;
                }

                float min_scale = fminf(fminf(new_scale[0], new_scale[1]), new_scale[2]);
                float max_scale = fmaxf(fmaxf(new_scale[0], new_scale[1]), new_scale[2]);

                if (min_scale >= 0.1f && max_scale <= 10.0f) {
                    for (int i = 0; i < 3; i++) {
                        temp_scale[i] = new_scale[i];
                    }
                }
                else {
                    if (min_scale < 0.1f) {
                        uniform_scale = 0.1f / (min_scale / uniform_scale);
                    }
                    else {
                        uniform_scale = 10.0f / (max_scale / uniform_scale);
                    }
                    scale_factor = uniform_scale / previous_scale;
                    for (int i = 0; i < 3; i++) {
                        temp_scale[i] *= scale_factor;
                        if (temp_scale[i] < 0.1f) temp_scale[i] = 0.1f;
                        if (temp_scale[i] > 10.0f) temp_scale[i] = 10.0f;
                    }
                }

                scale.x = temp_scale[0];
                scale.y = temp_scale[1];
                scale.z = temp_scale[2];
                previous_scale = uniform_scale;
            }
            else {
                uniform_scale = (temp_scale[0] + temp_scale[1] + temp_scale[2]) / 3.0f;
                previous_scale = uniform_scale;
            }
            param_y += 35;

        } break;

        case MODE_NONE:
        default: break;
        }


        BeginMode3D(camera);

        if (poly_initialized) {
            Polyhedron_draw(&current_poly, user_transform);
        }

        DrawLine3D((Vector3) { 0, 0, 0 }, (Vector3) { 3, 0, 0 }, RED);
        DrawLine3D((Vector3) { 0, 0, 0 }, (Vector3) { 0, 3, 0 }, GREEN);
        DrawLine3D((Vector3) { 0, 0, 0 }, (Vector3) { 0, 0, 3 }, BLUE);

        EndMode3D();

        EndDrawing();
    }
    Polyhedron_free(&tetra);
    Polyhedron_free(&hexa);
    Polyhedron_free(&octa);
    Polyhedron_free(&ico);
    Polyhedron_free(&dodeca);
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
    selected_polygon = new_selection;

    if (poly_initialized) {
        Polyhedron_free(&current_poly);
    }

    switch (new_selection) {
    case 0: Polyhedron_createTetrahedron(&current_poly); break;
    case 1: Polyhedron_createHexahedron(&current_poly); break;
    case 2: Polyhedron_createOctahedron(&current_poly); break;
    case 3: Polyhedron_createIcosahedron(&current_poly); break;
    //case 4: Polyhedron_createDodecahedron(&current_poly); break;
    }

    poly_initialized = true;

    translation = (Vector3){ 0 };
    rotation_angle = 0;
    rotation_axis = (Vector3){ 0, 1, 0 };
    scale = (Vector3){ 1, 1, 1 };
}
