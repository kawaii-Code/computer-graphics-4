#include "common.h"

float bezier_point_radius = 25.0f;
float bezier_point_border_radius = 3.0f;

int bezier_approximation_points = 100;

static Vector2 bezier_points[128];
static int     bezier_points_len;

Vector2 dp[128];

Vector2 evaluate_bezier_curve(float t) {
    int n = bezier_points_len;

    if (n == 0) {
        return (Vector2){-100, -100};
    }
    if (n == 1) {
        return bezier_points[0];
    }

    for (int i = 0; i < n; i++) {
        dp[i] = bezier_points[i];
    }
    for (int j = 1; j < n; j++) {
        for (int i = 0; i < n - j; i++) {
            float x = dp[i].x * (1 - t) + dp[i + 1].x * t;
            float y = dp[i].y * (1 - t) + dp[i + 1].y * t;
            dp[i] = (Vector2){x, y};
        }
    }
    return dp[0];
}

void task3(int argc, char** argv) {
    SetWindowTitle("Задание 2");
    SetWindowSize(task_window_width, task_window_height);

    Font font = fonts[FONT_MAIN];

    Color bezier_point_hovered_color = RED;
    Color bezier_point_default_color = YELLOW;

    int dragged_point_index = -1;
    while (!WindowShouldClose()) {
        Vector2 mouse_position = GetMousePosition();

        int hovered_point_index = -1;
        for (int i = 0; i < bezier_points_len; i++) {
            if (CheckCollisionPointCircle(mouse_position, bezier_points[i], bezier_point_radius)) {
                hovered_point_index = i;
            }
        }

        if (dragged_point_index != -1) {
            bezier_points[dragged_point_index] = mouse_position;
        }

        if (IsKeyPressed(KEY_C)) {
            bezier_points_len = 0;
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (IsKeyDown(KEY_LEFT_SHIFT) && hovered_point_index == -1) {
                bezier_points[bezier_points_len] = (Vector2){ .x = mouse_position.x, .y = mouse_position.y };
                bezier_points_len += 1;
            } else if (hovered_point_index != -1) {
                dragged_point_index = hovered_point_index;
            }
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            dragged_point_index = -1;
        }

        BeginDrawing();

        DrawTextEx(font, "ЛКМ + SHIFT: Поставить точку", (Vector2){2, 2}, font.baseSize, 0.0, ui_text_color);
        DrawTextEx(font, "C: Стереть всё", (Vector2){2, font.baseSize + 4}, font.baseSize, 0.0, ui_text_color);

        ClearBackground(ui_background_color);
        for (int i = 0; i < bezier_points_len; i++) {
            Color color = (i == hovered_point_index) ? bezier_point_hovered_color : bezier_point_default_color;
            if (i == dragged_point_index) {
                color = bezier_point_hovered_color;
                color.r *= 0.8f;
                color.g *= 0.8f;
                color.b *= 0.8f;
            }
            if (i < bezier_points_len - 1) {
                DrawLineEx(bezier_points[i], bezier_points[i + 1], 2, GRAY);
            }
            DrawCircleV(bezier_points[i], bezier_point_radius, BLACK);
            DrawCircleV(bezier_points[i], bezier_point_radius - bezier_point_border_radius, color);
        }
        for (int i = 0; i <= bezier_approximation_points; i++) {
            float t = (float)i / bezier_approximation_points;
            Vector2 point = evaluate_bezier_curve(t);
            DrawCircleV(point, 3.0f, BLACK);
        }
        EndDrawing();
    }

    SetWindowSize(menu_window_width, menu_window_height);
}
