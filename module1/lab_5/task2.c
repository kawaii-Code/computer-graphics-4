#include "common.h"

DEFINE_NODE(Vector2);

void task2(int argc, char** argv) {
    SetWindowTitle("Задание 2");
    SetWindowSize(task_window_width, task_window_height);

    Font font = fonts[FONT_MAIN];

    float p1y = task_window_height - 100;
    float p2y = task_window_height - 100;

    Vector2 p1 = (Vector2){ 50, p1y };
    Vector2 p2 = (Vector2){ task_window_width - 50, p2y };

    llist(Vector2) lst;
    llist_init(lst);

    llist_add_after(lst, Vector2, p1, 0);
    llist_add_after(lst, Vector2, p2, 0);

    _NODE_TYPE(Vector2)* first = lst.head;
    _NODE_TYPE(Vector2)* last = lst.head->next;

    float R = 0.3;

    while (!WindowShouldClose()) {
        Window_Info window = get_window_info();

        BeginDrawing();
        ClearBackground(ui_background_color);

        if (GuiSlider((Rectangle) { 50, 50, 150, 20 }, "100", "500", & p1y, 100, task_window_height - 100)) {
            first->data.y = p1y;
        }
        if (GuiSlider((Rectangle) { 250, 50, 150, 20 }, "100", "500", & p2y, 100, task_window_height - 100)) {
            last->data.y = p2y;
        }
        GuiSlider((Rectangle) { 450, 50, 150, 20 }, "0", "1", & R, 0, 1);


        if (Button((Rectangle) { 650, 40, 100, 40 }, ">")) {
            _NODE_TYPE(Vector2)* cur = lst.head;
            int i = 0;
            while (cur && cur->next) {
                p1 = cur->data;
                p2 = cur->next->data;
                float len = (p2.x - p1.x);
                Vector2 p = (Vector2){ (p1.x + p2.x) / 2, (p1.y + p2.y) / 2 + random_range(-R * len, R * len) };
                llist_add_after(lst, Vector2, p, i);

                i += 2;
                cur = cur->next->next;
            }
        }

        if (Button((Rectangle) { 800, 40, 100, 40 }, "x")) {
            _NODE_TYPE(Vector2)* cur = lst.head;
            cur = cur->next;
            while (cur != last) {
                _NODE_TYPE(Vector2)* next = cur->next;
                free(cur);
                cur = next;
            }
            lst.head->next = last;
            last->prev = lst.head;
        }

        _NODE_TYPE(Vector2)* cur = lst.head;
        while (cur->next) {
            p1 = cur->data;
            p2 = cur->next->data;
            DrawLine(p1.x, p1.y, p2.x, p2.y, BLACK);

            cur = cur->next;
        }

        EndDrawing();
    }

    _NODE_TYPE(Vector2)* cur = lst.head;
    while (cur) {
        _NODE_TYPE(Vector2)* next = cur->next;
        free(cur);
        cur = next;
    }
    SetWindowSize(menu_window_width, menu_window_height);
}