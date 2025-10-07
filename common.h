#pragma once

// По поводу рисования UI
//
// Есть три варианта:
// 1. Использовать CLAY, как сделал я. Можете посмотреть
//    в task2.c. Выглядит страшно, но я довольно быстро
//    разобрался что к чему.
//
// 2. Использовать raygui. К нему особо нет нормальной документации,
//    надо просто почитать объявления функций в third_party/include/raygui.h,
//    но он должен быть довольно простым.
//
// 3. Накодить свои элементы, например как `Button` Сани. Это не сложно.
//
// Для raylib-а есть хорошая справка
// https://www.raylib.com/cheatsheet/cheatsheet.html
//
// И на официальном сайте много примеров
// слишком многа букав, не осилил

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "third_party/include/clay.h"
#include "third_party/include/raylib.h"
#include "third_party/include/raygui.h"
#include "third_party/include/raymath.h"


#define TO_CLAY_COLOR(color) ((Clay_Color){ .r = color.r, .b = color.b, .g = color.g, .a = color.a })
#define ARRAY_LEN(array) (sizeof(array) / sizeof(*array))


typedef enum {
    FONT_FOR_DEBUG_WINDOW = 0,
    FONT_MAIN,
    FONT_COUNT,
} Font_Id;

// Я делал это под себя, не знаю, может кому пригодится.
// Это очень негибкая штука.
typedef struct {
    Rectangle bounds;
    int       bar_count;   // bar_count <= 256. А что делать без векторов? Так и живём.
    int       values[256];
    Color     color;
} Histogram;

typedef struct {
    Vector2 center;
    int width;
    int height;
} Window_Info;

// Глобальный массив разных шрифтов.
// Вообще, на время написания, тут
// только один полезный, FONT_MAIN.
//
// Берите его в любом месте через
//
// fonts[FONT_MAIN]
//
extern Font fonts[FONT_COUNT];


// Возможно полезные функции
bool        Button(Rectangle bounds, const char *text);
void        DropdownMenu(Rectangle bounds, int* selectedOption, bool* showDropdown, int optionsCount, const char** options, void (*func)(int));
Window_Info get_window_info();
void        draw_text_centered(Font font, const char *text, float x, float y, Color color);
void        draw_texture_centered(Texture2D texture, float x, float y);
void        draw_histogram(Histogram histogram);
Histogram   make_smaller_histogram(Histogram histogram, int ratio);
Rectangle   shrink_rect(Rectangle rect, float amount);

void task1(int argc, char **argv);
void task2(int argc, char **argv);
void task3(int argc, char **argv);

void            init();
void            Clay_Raylib_Render(Clay_RenderCommandArray renderCommands, Font* fonts);
Clay_Dimensions Raylib_MeasureText(Clay_StringSlice text, Clay_TextElementConfig *config, void *userData);


const static int menu_window_width = 400;
const static int menu_window_height = 300;

const static int task_window_width = 800;
const static int task_window_height = 600;

const static Color ui_background_color = {224, 215, 210, 255};
const static Color ui_text_color       = {168, 66, 28, 255};
const static Color ui_border_color     = {225, 138, 50, 255};
