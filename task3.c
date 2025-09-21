#include "lab2.h"

static Texture2D texture;

void task3(int argc, char **argv) {
    SetWindowTitle("Задание 3");
    SetWindowSize(task_window_width, task_window_height);

    texture = LoadTexture("images/bird.jpg");

    Font font = fonts[FONT_MAIN];

    while (!WindowShouldClose()) {
        Window_Info window = get_window_info();

        BeginDrawing();
        ClearBackground(ui_background_color);

        draw_texture_centered(texture, window.center.x, window.center.y - 160);
        draw_text_centered(font, "Привет!\nТут должно быть третье задание.", window.center.x, window.center.y, ui_text_color);

        EndDrawing();
    }

    SetWindowSize(menu_window_width, menu_window_height);
}
