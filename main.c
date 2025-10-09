#include "common.h"
#include "vector.h"
#include "polygon.h"
#include "matrix-transforms.h"

#define RAYGUI_IMPLEMENTATION
#include "third_party/include/raygui.h"

Font fonts[FONT_COUNT];

int selected_polygon = 0;

int main(int argc, char **argv) {
    init();
    SetWindowSize(task_window_width, task_window_height);
    Rectangle unclickableArea;

    tool_textures[TRANSFORM_MOVE] = LoadTexture("images/lab4/move.png");
    tool_textures[TRANSFORM_ROTATE] = LoadTexture("images/lab4/rotate.png");
    tool_textures[TRANSFORM_ROTATE_AROUND_POINT] = LoadTexture("images/lab4/rotate-around.png");
    tool_textures[TRANSFORM_SCALE] = LoadTexture("images/lab4/scale.png");
    tool_textures[TRANSFORM_SCALE_AROUND_POINT] = LoadTexture("images/lab4/scale-around.png");

    init_matrix_transforms();

    vector(Polygon) polygons;
    vector_init(polygons);
    vector_append(polygons, polygon_create());

    while (!WindowShouldClose()) {
        Window_Info window = get_window_info();

        int unclickableAreaHeight = task_window_height / 8;
        unclickableArea = (Rectangle){0, 0, window.width, unclickableAreaHeight};

        Rectangle btn_clear = {20, unclickableArea.height / 2 - 20, 60, 40};

        BeginDrawing();
        ClearBackground(ui_background_color);

        if (Button(btn_clear, "Clear")) {
            for (size_t i = 0; i < polygons.len; i++) {
                polygon_free(polygons.head[i]);
            }
            polygons.len = 0;
            vector_append(polygons, polygon_create());
            selected_polygon = 0;
        }

        DrawLine(0, unclickableArea.height, unclickableArea.width, unclickableArea.height, BLACK);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && IsKeyDown(KEY_LEFT_SHIFT)) {
            Vector2 mouse_position = GetMousePosition();
            for (size_t i = 0; i < polygons.len; i++) {
                if (polygon_contains(vector_get(polygons, i), mouse_position)) {
                    selected_polygon = i;
                    break;
                }
            }
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            Vector2 mouse_position = GetMousePosition();
            bool skip = false;
            if (CheckCollisionPointRec(mouse_position, unclickableArea)) {
                skip = true;
            }
            if (polygon_do_edges_intersect_new(vector_get(polygons, selected_polygon), mouse_position)) {
                skip = true;
            }
            for (size_t i = 0; i < polygons.len; i++) {
                if (polygon_contains(vector_get(polygons, i), mouse_position)) {
                    skip = true;
                    break;
                }
            }

            if (skip) {
                DrawCircle(mouse_position.x, mouse_position.y, 7, RED);  
            } else {
                polygon_add_vertice(vector_get_ptr(polygons, selected_polygon), mouse_position);
            }
        }
        if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
            Vector2 mouse_position = GetMousePosition();
            if (!CheckCollisionPointRec(mouse_position, unclickableArea)) {
                vector_append(polygons, polygon_create());
            }
            selected_polygon = polygons.len - 1;
        }

        draw_and_read_edits(vector_get(polygons, selected_polygon));

        BeginScissorMode(0, unclickableAreaHeight, window.width, window.height);
        for (size_t i = 0; i < polygons.len; i++) {
            polygon_draw(vector_get(polygons, i), i == selected_polygon);
        }
        EndScissorMode();

        EndDrawing();
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
