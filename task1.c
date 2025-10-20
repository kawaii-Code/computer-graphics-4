#include "common.h"
#include "L-system.h"

#define MAX_FRACTALS 10

typedef struct {
    char name[50];
    char filename[50];
} FractalOption;

void task1(int argc, char** argv) {
    SetWindowTitle("L-systems");
    int task_window_width = GetMonitorWidth(0);
    int task_window_height = GetMonitorHeight(0);
    ToggleBorderlessWindowed();
    SetWindowSize(task_window_width, task_window_height);

    LSystem ls;
    lsystem_init(&ls);

    FractalOption fractals[MAX_FRACTALS] = {
        {"Снежинка Коха", "./fractals/koch.txt"},
        {"Дракон", "./fractals/dragon.txt"},
        {"Дерево", "./fractals/tree.txt"},
        {"Ковер Серпинского", "./fractals/sierpinski_carpet.txt"}
    };
    int fractalCount = 4;
    int currentFractal = 0;

    int iterations = 1;
    bool showDropdown = false;
    bool useRandomness = false;

    Rectangle dropdownBounds = { 20, 20, 250, 40 };
    Rectangle iterMinusButton = { 280, 20, 40, 40 };
    Rectangle iterPlusButton = { 330, 20, 40, 40 };

    char lstring[MAX_STRING_LENGTH] = "";

    if (lsystem_load_from_file(&ls, fractals[currentFractal].filename)) {
        lsystem_generate_string(&ls, lstring, iterations);
    }

    while (!WindowShouldClose()) {
        const char* fractalNames[4];
        for (int i = 0; i < fractalCount; i++) {
            fractalNames[i] = fractals[i].name;
        }

        if (Button((Rectangle) { dropdownBounds.x, dropdownBounds.y, dropdownBounds.width, dropdownBounds.height }, fractalNames[currentFractal]))
        {
            showDropdown = !showDropdown;
        }

        if (showDropdown && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePos = GetMousePosition();
            if (!CheckCollisionPointRec(mousePos, dropdownBounds) &&
                !CheckCollisionPointRec(mousePos, (Rectangle) { dropdownBounds.x, dropdownBounds.y + 50, dropdownBounds.width, dropdownBounds.height* fractalCount })) {
                showDropdown = false;
            }
        }

        if (showDropdown)
        {
            int button_cnt = 1;
            for (int i = 0; i < fractalCount; i++)
            {
                if (i == currentFractal) continue;
                Rectangle optionRect = { dropdownBounds.x, dropdownBounds.y + button_cnt * 50, dropdownBounds.width, dropdownBounds.height };
                if (Button(optionRect, fractalNames[i]))
                {
                    currentFractal = i;
                    showDropdown = false;
                }
                button_cnt++;
            }
        }

        static int lastFractal = -1;
        if (currentFractal != lastFractal) {
            iterations = 1;

            lsystem_free(&ls);

            if (lsystem_load_from_file(&ls, fractals[currentFractal].filename)) {
                lsystem_generate_string(&ls, lstring, iterations);
            }
            lastFractal = currentFractal;
        }

        if (Button(iterMinusButton, "-") && iterations > 1) {
            iterations--;
            lsystem_generate_string(&ls, lstring, iterations);
        }

        if (Button(iterPlusButton, "+") && iterations < 7) {
            iterations++;
            lsystem_generate_string(&ls, lstring, iterations);
        }

        BeginDrawing();
        ClearBackground(ui_background_color);

        if (strlen(lstring) > 0) {
            lsystem_draw(&ls, lstring, task_window_width, task_window_height);
        }

        EndDrawing();
    }

    SetWindowSize(menu_window_width, menu_window_height);
}