#include <stdio.h>

#include "common.c"
#include "task2.c"
#include "third_party/include/raylib.h"

bool Button(Rectangle bounds, const char *text) {
    Vector2 mouse = GetMousePosition();
    bool hover = CheckCollisionPointRec(mouse, bounds);
    bool pressed = hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    Color color = hover ? LIGHTGRAY : GRAY;

    DrawRectangleRec(bounds, color);
    DrawRectangleLinesEx(bounds, 2, BLACK);

    int fontSize = 20;
    Vector2 textSize = MeasureTextEx(GetFontDefault(), text, fontSize, 1);
    DrawText(text,
             bounds.x + (bounds.width - textSize.x) / 2,
             bounds.y + (bounds.height - textSize.y) / 2,
             fontSize,
             BLACK);

    return pressed;
}

void init() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(menu_window_width, menu_window_height, "Lab 2");

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


int main(int argc, char **argv) {
    init();
    SetTargetFPS(60);

    Rectangle btn1 = {100, 60, 200, 50};
    Rectangle btn2 = {100, 130, 200, 50};
    Rectangle btn3 = {100, 200, 200, 50};

    while (!WindowShouldClose()) {
        // BeginDrawing();
        // ClearBackground(RAYWHITE);
        Clay_BeginLayout();

        CLAY({ .id = CLAY_ID("Menu")})

        if (Button(btn1, "Task 1")) {
            printf("Task 1 launched!\n");
            // TODO: replace with actual task
        }

        if (Button(btn2, "Task 2")) {
            printf("Task 2 launched!\n");
            task2(argc, argv);
        }

        if (Button(btn3, "Task 3")) {
            printf("Task 3 launched!\n");
            // TODO: replace with actual task
        }

        // EndDrawing();
        Clay_EndLayout();
    }

    CloseWindow();
    return 0;
}
