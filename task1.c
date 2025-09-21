#include "lab2.h"

static Texture2D texture;

void task1(int argc, char **argv) {
    SetWindowTitle("Задание 1");
    SetWindowSize(task_window_width, task_window_height);

    texture = LoadTexture("images/bird.jpg");

    Font font = fonts[FONT_MAIN];

    float delta_x = 0.0f;

    while (!WindowShouldClose()) {
        Window_Info window = get_window_info();

        BeginDrawing();
        ClearBackground(ui_background_color);

        draw_texture_centered(texture, window.center.x, window.center.y - 160);

        Rectangle slider_bounds = {
            .x = window.center.x - 100,
            .y = window.center.y + 100,
            .width = 200,
            .height = 20,
        };

        GuiSlider(slider_bounds, "-200.0", "+200.0", &delta_x, -200.0f, 200.0f);
        draw_text_centered(font, "Привет!\nТут должно быть первое задание.", window.center.x + delta_x, window.center.y, ui_text_color);

        EndDrawing();
    }

    SetWindowSize(menu_window_width, menu_window_height);
}
