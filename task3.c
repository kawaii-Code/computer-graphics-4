// Задание 3. Выполнить градиентное окрашивание произвольного 
// треугольника, у которого все три вершины разного цвета, используя алгоритм растеризации треугольника.

#include "lab2.h"
#include <math.h>

typedef struct {
    Vector2 position;
    Color color;
} Vertex;

typedef struct {
    Vertex v1, v2, v3;
} Triangle;

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

Color interpolate_color(Color c1, Color c2, Color c3, Vector3 bary) {
    return (Color) {
        (uint8_t)(c1.r * bary.x + c2.r * bary.y + c3.r * bary.z),
            (uint8_t)(c1.g * bary.x + c2.g * bary.y + c3.g * bary.z),
            (uint8_t)(c1.b * bary.x + c2.b * bary.y + c3.b * bary.z),
            (uint8_t)(c1.a * bary.x + c2.a * bary.y + c3.a * bary.z)
    };
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

void draw_gradient_triangle(Triangle triangle) {
    Vector2 v1 = triangle.v1.position;
    Vector2 v2 = triangle.v2.position;
    Vector2 v3 = triangle.v3.position;

    Color c1 = triangle.v1.color;
    Color c2 = triangle.v2.color;
    Color c3 = triangle.v3.color;

    int min_x, max_x, min_y, max_y;
    get_triangle_bounding_box(v1, v2, v3, &min_x, &max_x, &min_y, &max_y);

    for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
            Vector2 p = { (float)x, (float)y };

            Vector3 bary = barycentric_coordinates(p, v1, v2, v3);

            if (point_in_triangle(bary)) {
                Color pixel_color = interpolate_color(c1, c2, c3, bary);
                DrawPixel(x, y, pixel_color);
            }
        }
    }
}

void task3(int argc, char** argv) {
    SetWindowTitle("Задание 3");
    int task_window_width = GetMonitorWidth(0);
    int task_window_height = GetMonitorHeight(0);
    ToggleBorderlessWindowed();

    Triangle triangle = {
        .v1 = {.position = {400, 100}, .color = RED },
        .v2 = {.position = {200, 500}, .color = GREEN },
        .v3 = {.position = {600, 500}, .color = BLUE }
    };

    bool is_dragging = false;
    int dragged_vertex = -1;
    Vector2 mouse_offset = { 0, 0 };

    while (!WindowShouldClose()) {
        Window_Info window = get_window_info();
        Vector2 mouse_pos = GetMousePosition();

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (Vector2Distance(mouse_pos, triangle.v1.position) < 20) {
                is_dragging = true;
                dragged_vertex = 0;
                mouse_offset.x = mouse_pos.x - triangle.v1.position.x;
                mouse_offset.y = mouse_pos.y - triangle.v1.position.y;
            }
            else if (Vector2Distance(mouse_pos, triangle.v2.position) < 20) {
                is_dragging = true;
                dragged_vertex = 1;
                mouse_offset.x = mouse_pos.x - triangle.v2.position.x;
                mouse_offset.y = mouse_pos.y - triangle.v2.position.y;
            }
            else if (Vector2Distance(mouse_pos, triangle.v3.position) < 20) {
                is_dragging = true;
                dragged_vertex = 2;
                mouse_offset.x = mouse_pos.x - triangle.v3.position.x;
                mouse_offset.y = mouse_pos.y - triangle.v3.position.y;
            }
        }

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && is_dragging) {
            switch (dragged_vertex) {
            case 0:
                triangle.v1.position.x = mouse_pos.x - mouse_offset.x;
                triangle.v1.position.y = mouse_pos.y - mouse_offset.y;
                break;
            case 1:
                triangle.v2.position.x = mouse_pos.x - mouse_offset.x;
                triangle.v2.position.y = mouse_pos.y - mouse_offset.y;
                break;
            case 2:
                triangle.v3.position.x = mouse_pos.x - mouse_offset.x;
                triangle.v3.position.y = mouse_pos.y - mouse_offset.y;
                break;
            }
        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            is_dragging = false;
            dragged_vertex = -1;
        }

        BeginDrawing();
        ClearBackground(ui_background_color);

        draw_gradient_triangle(triangle);

        DrawCircleV(triangle.v1.position, 8, WHITE);
        DrawCircleV(triangle.v2.position, 8, WHITE);
        DrawCircleV(triangle.v3.position, 8, WHITE);

        EndDrawing();
    }
    ToggleBorderlessWindowed();

    SetWindowSize(menu_window_width, menu_window_height);
}