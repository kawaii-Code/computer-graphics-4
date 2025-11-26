#include "common.h"

float bezier_point_radius = 25.0f;
float bezier_point_border_radius = 3.0f;

int bezier_approximation_points = 250;

static Vector2 bezier_points[128];
static int     bezier_points_len;

static Vector2 bezier_points2[128];
static int     bezier_points2_len;

Vector2 dp[128];

Vector2 evaluate_bezier_curve(float t) {
    int n = bezier_points_len;

    if (n == 0) {
        return (Vector2) { -100, -100 };
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
            dp[i] = (Vector2){ x, y };
        }
    }
    return dp[0];
}

float lerpf2(float a, float b, float t) {
    return a + (b - a) * t;
}

Vector2 v2lerpf(Vector2 a, Vector2 b, float t) {
    Vector2 result;
    result.x = lerpf2(a.x, b.x, t);
    result.y = lerpf2(a.y, b.y, t);
    return result;
}

Vector2 evaluate_bezier_curve2(Vector2 v[4], float t) {
    Vector2 f1 = v2lerpf(v[0], v[1], t);
    Vector2 f2 = v2lerpf(v[1], v[2], t);
    Vector2 f3 = v2lerpf(v[2], v[3], t);
    Vector2 g1 = v2lerpf(f1, f2, t);
    Vector2 g2 = v2lerpf(f2, f3, t);
    return v2lerpf(g1, g2, t);

    int n = 4;

    if (n == 0) {
        return (Vector2) { -100, -100 };
    }
    if (n == 1) {
        return v[0];
    }

    for (int i = 0; i < n; i++) {
        dp[i] = v[i];
    }
    for (int j = 1; j < n; j++) {
        for (int i = 0; i < n - j; i++) {
            float x = dp[i].x * (1 - t) + dp[i + 1].x * t;
            float y = dp[i].y * (1 - t) + dp[i + 1].y * t;
            dp[i] = (Vector2){ x, y };
        }
    }
    return dp[0];
}

void task3(int argc, char** argv) {
    SetWindowTitle("Задание 3");
    SetWindowSize(task_window_width, task_window_height);

    Font font = fonts[FONT_MAIN];

    Color bezier_point_hovered_color = RED;
    Color bezier_point_default_color = YELLOW;

    int dragged_point_index = -1;
    while (!WindowShouldClose()) {
        Vector2 mouse_position = GetMousePosition();

        int hovered_point_index = -1;
        for (int i = 0; i < bezier_points2_len; i++) {
            if (CheckCollisionPointCircle(mouse_position, bezier_points2[i], bezier_point_radius)) {
                hovered_point_index = i;
            }
        }

        if (dragged_point_index != -1) {
            if (dragged_point_index % 3 == 1) {
                Vector2 a = bezier_points2[dragged_point_index - 1];
                Vector2* b = &bezier_points2[dragged_point_index];
                Vector2* c = &bezier_points2[dragged_point_index + 1];
                *b = mouse_position;
                c->x = a.x - (b->x - a.x);
                c->y = a.y - (b->y - a.y);
            }
            else if (dragged_point_index % 3 == 2) {
                Vector2 a = bezier_points2[dragged_point_index - 2];
                Vector2* b = &bezier_points2[dragged_point_index - 1];
                Vector2* c = &bezier_points2[dragged_point_index];
                *c = mouse_position;
                b->x = a.x - (c->x - a.x);
                b->y = a.y - (c->y - a.y);
            }
            else {
                Vector2* a = &bezier_points2[dragged_point_index];
                Vector2* b = &bezier_points2[dragged_point_index + 1];
                Vector2* c = &bezier_points2[dragged_point_index + 2];
                *b = (Vector2){ .x = b->x - (a->x - mouse_position.x), .y = b->y - (a->y - mouse_position.y) };
                *c = (Vector2){ .x = c->x - (a->x - mouse_position.x), .y = c->y - (a->y - mouse_position.y) };
                *a = mouse_position;
            }
        }

        if (IsKeyPressed(KEY_C)) {
            bezier_points_len = 0;
            bezier_points2_len = 0;
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (IsKeyDown(KEY_LEFT_SHIFT) && hovered_point_index == -1) {
                bezier_points[bezier_points_len] = mouse_position;
                bezier_points_len += 1;
            }
            else if (IsKeyDown(KEY_LEFT_CONTROL)) {
                Vector2 p = mouse_position;
                bezier_points2[bezier_points2_len] = p;
                p.x += 30.0f;
                p.y += 30.0f;
                bezier_points2[bezier_points2_len + 2] = p;
                p.x -= 60.0f;
                p.y -= 60.0f;
                bezier_points2[bezier_points2_len + 1] = p;
                bezier_points2_len += 3;
            }
            else if (hovered_point_index != -1) {
                dragged_point_index = hovered_point_index;
            }
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            dragged_point_index = -1;
        }

        BeginDrawing();

        DrawTextEx(font, "ЛКМ + SHIFT: Поставить точку", (Vector2) { 2, 2 }, font.baseSize, 0.0, ui_text_color);
        DrawTextEx(font, "C: Стереть всё", (Vector2) { 2, font.baseSize + 4 }, font.baseSize, 0.0, ui_text_color);

        ClearBackground(ui_background_color);

        for (int i = 0; i < bezier_points2_len - 2; i += 3) {
            DrawLineEx(bezier_points2[i], bezier_points2[i + 1], 3.0f, GRAY);
            DrawLineEx(bezier_points2[i], bezier_points2[i + 2], 3.0f, GRAY);
            Color color = (i == hovered_point_index) ? bezier_point_hovered_color : bezier_point_default_color;
            DrawCircleV(bezier_points2[i], bezier_point_radius, BLACK);
            DrawCircleV(bezier_points2[i], bezier_point_radius - bezier_point_border_radius, color);
            color = (i + 1 == hovered_point_index) ? bezier_point_hovered_color : bezier_point_default_color;
            DrawCircleV(bezier_points2[i + 1], 0.5f * bezier_point_radius, color);
            DrawCircleV(bezier_points2[i + 1], 0.5f * (bezier_point_radius - bezier_point_border_radius), RED);
            color = (i + 2 == hovered_point_index) ? bezier_point_hovered_color : bezier_point_default_color;
            DrawCircleV(bezier_points2[i + 2], 0.5f * bezier_point_radius, color);
            DrawCircleV(bezier_points2[i + 2], 0.5f * (bezier_point_radius - bezier_point_border_radius), RED);
        }

        for (int i = 0; i < bezier_points2_len - 4; i += 3) {
            for (int j = 0; j <= bezier_approximation_points; j++) {
                float t = (float)j / bezier_approximation_points;
                Vector2 points[4] = {
                    bezier_points2[i],
                    bezier_points2[i + 2],
                    bezier_points2[i + 4],
                    bezier_points2[i + 3],
                };
                Vector2 point = evaluate_bezier_curve2(points, t);
                DrawCircleV(point, 3.0f, BLACK);
            }
        }

        //for (int i = 0; i < bezier_points_len; i++) {
        //    Color color = (i == hovered_point_index) ? bezier_point_hovered_color : bezier_point_default_color;
        //    if (i == dragged_point_index) {
        //        color = bezier_point_hovered_color;
        //        color.r *= 0.8f;
        //        color.g *= 0.8f;
        //        color.b *= 0.8f;
        //    }
        //    if (i < bezier_points_len - 1) {
        //        DrawLineEx(bezier_points[i], bezier_points[i + 1], 2, GRAY);
        //    }
        //    DrawCircleV(bezier_points[i], bezier_point_radius, BLACK);
        //    DrawCircleV(bezier_points[i], bezier_point_radius - bezier_point_border_radius, color);
        //}
        //for (int i = 0; i <= bezier_approximation_points; i++) {
        //    float t = (float)i / bezier_approximation_points;
        //    Vector2 point = evaluate_bezier_curve(t);
        //    DrawCircleV(point, 3.0f, BLACK);
        //}
        EndDrawing();
    }

    SetWindowSize(menu_window_width, menu_window_height);
}
