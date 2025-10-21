#include "common.h"
#include "L-system.h"

#define MAX_FRACTALS 13

typedef struct {
    char name[50];
    char filename[50];
} FractalOption;

void task1(int argc, char** argv) {
    SetWindowTitle("L-systems");
    int task_window_width = GetMonitorWidth(0);
    int task_window_height = GetMonitorHeight(0);
    ToggleBorderlessWindowed();
    //SetWindowSize(task_window_width, task_window_height);

    LSystem ls;
    lsystem_init(&ls);

    FractalOption fractals[MAX_FRACTALS] = {
        {"Кривая Коха", "./fractals/koch.txt"},
        {"Снежинка Коха", "./fractals/snowflake.txt"},
        {"Квадратный остров Коха", "./fractals/island.txt"},
        {"Дракон", "./fractals/dragon.txt"},
        {"Дракон Хартера-Хейтуэя", "./fractals/dragon2.txt"},
        {"Куст 1", "./fractals/tree1.txt"},
        {"Куст 2", "./fractals/tree2.txt"},
        {"Куст 3", "./fractals/tree3.txt"},
        {"Треугольник Серпинского", "./fractals/sierpinski_carpet.txt"},
        {"Наконечник Серпинского", "./fractals/peak.txt" },
        {"Кривая Гилберта", "./fractals/gilbert_curve.txt" },
        {"Кривая Госпера", "./fractals/gosper_curve.txt" },
        {"Шестиугольная мозаика", "./fractals/mosaic.txt" }
    };
    int fractalCount = 13;
    int currentFractal = 0;

    int iterations = 1;
    bool showDropdown = false;
    bool useRandom = false;

    Rectangle dropdownBounds = { 20, 20, 250, 40 };
    Rectangle iterMinusButton = { 280, 20, 40, 40 };
    Rectangle iterPlusButton = { 330, 20, 40, 40 };
    Rectangle randomButton = { 380, 20, 200, 40 };
    Rectangle regenerateButton = { 590, 20, 250, 40 };

    char lstring[MAX_STRING_LENGTH] = "";

    if (lsystem_load_from_file(&ls, fractals[currentFractal].filename)) {
        lsystem_generate_string(&ls, lstring, iterations);
    }

    while (!WindowShouldClose()) {
        const char* fractalNames[13];
        for (int i = 0; i < fractalCount; i++) {
            fractalNames[i] = fractals[i].name;
        }

        if (Button((Rectangle) { dropdownBounds.x, dropdownBounds.y, dropdownBounds.width, dropdownBounds.height }, fractalNames[currentFractal]))
        {
            showDropdown = !showDropdown;
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

        if (Button(iterPlusButton, "+") && iterations < ls.max_iter) {
            iterations++;
            lsystem_generate_string(&ls, lstring, iterations);
        }

        if (Button(randomButton, ls.use_random ? "Вариации: ВКЛ" : "Вариации: ВЫКЛ")) {
            ls.use_random = !ls.use_random;
        }

        if (ls.use_random && Button(regenerateButton, "Новые вариации")) {
            lsystem_regenerate_variations(&ls, lstring);
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