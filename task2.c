// Задание 2.
// Выделить из полноцветного изображения каждый из каналов R, G, B и вывести результат.
// Построить гистограмму по цветам (3 штуки).


#include "common.c"
#include "third_party/include/raylib.h"


#define ARRAY_LEN(array) (sizeof(array) / sizeof(*array))


const char *window_title = "Лаба 2 Задание 2: RGB";

typedef enum {
    FONT_FOR_DEBUG_WINDOW = 0,
    FONT_MAIN,
} Font_Id;

typedef enum {
    CHANNEL_RED = 0,
    CHANNEL_GREEN,
    CHANNEL_BLUE,
    CHANNEL_COUNT,
} Channel;

typedef struct {
    Rectangle bounds;
    int       bar_count;   // bar_count <= 256. А что делать без векторов? Так и живём.
    int       values[256];
    Color     color;
} Histogram;

Image     image;
Font      fonts[3];

Texture2D textures[4];
Clay_String texture_names[ARRAY_LEN(textures)] = {};

Color primary_colors[CHANNEL_COUNT] = {};

Histogram histogram_for_all_colors = { .bar_count = 3 };
Histogram histograms[CHANNEL_COUNT] = {
};


// void init();
void read_input();
void draw_ui();
void draw_histogram_for_all_colors(Histogram histogram);
void draw_histogram(Histogram histogram);
Histogram make_smaller_histogram(Histogram histogram, int ratio);
Rectangle pad_bounding_box(Clay_BoundingBox box, float amount);


int task2(int argc, char **argv) {
    texture_names[0] = CLAY_STRING("Оригинал");
    texture_names[1] = CLAY_STRING("Красный канал");
    texture_names[2] = CLAY_STRING("Зеленый канал");
    texture_names[3] = CLAY_STRING("Синий канал");

    primary_colors[CHANNEL_RED] = RED;
    primary_colors[CHANNEL_GREEN] = GREEN;
    primary_colors[CHANNEL_BLUE] = BLUE;

    histograms[0] = (Histogram){ .color = RED,   .bar_count = 256 };
    histograms[1] = (Histogram){ .color = GREEN, .bar_count = 256 };
    histograms[2] = (Histogram){ .color = BLUE,  .bar_count = 256 };

    SetWindowSize(task_window_width, task_window_height);

    char *image_path = argc < 2 ? "images/bird.jpg" : argv[1];
    image = LoadImage(image_path);
    ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

    Color *image_colors = LoadImageColors(image);
    for (int i = 0; i < image.width * image.height; i++) {
        Color color = image_colors[i];
        if (color.a == 0) {
            continue;
        }

        if (color.r > color.b && color.r > color.g) {
            histogram_for_all_colors.values[CHANNEL_RED] += 1;
        } else if (color.g > color.r && color.g > color.b) {
            histogram_for_all_colors.values[CHANNEL_GREEN] += 1;
        } else if (color.b > color.r && color.b > color.g) {
            histogram_for_all_colors.values[CHANNEL_BLUE] += 1;
        }

        histograms[CHANNEL_RED].values[color.r] += 1;
        histograms[CHANNEL_GREEN].values[color.g] += 1;
        histograms[CHANNEL_BLUE].values[color.b] += 1;
    }

    Image red_channel = ImageCopy(image);
    Image green_channel = ImageCopy(image);
    Image blue_channel = ImageCopy(image);
    for (int i = 0; i < image.width * image.height; i++) {
        Color *color;

        color = (Color *)red_channel.data + i;
        color->g = 0;
        color->b = 0;

        color = (Color *)green_channel.data + i;
        color->r = 0;
        color->b = 0;

        color = (Color *)blue_channel.data + i;
        color->r = 0;
        color->g = 0;
    }

    //ExportImage(red_channel, "red.png");
    //ExportImage(green_channel, "green.png");
    //ExportImage(blue_channel, "blue.png");

    textures[0] = LoadTextureFromImage(image);
    textures[1] = LoadTextureFromImage(red_channel);
    textures[2] = LoadTextureFromImage(green_channel);
    textures[3] = LoadTextureFromImage(blue_channel);

    while (!WindowShouldClose()) {
        read_input();

        BeginDrawing();
        ClearBackground(BLACK);

        draw_ui();

        draw_histogram_for_all_colors(histogram_for_all_colors);
        for (int i = CHANNEL_RED; i <= CHANNEL_BLUE; i++) {
            Histogram compact_histogram = make_smaller_histogram(histograms[i], 16);
            draw_histogram(compact_histogram);
        }

        EndDrawing();
    }

    SetWindowSize(menu_window_width, menu_window_height);
}

// void init() {
//     SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
//
//     uint64_t total_memory_size = Clay_MinMemorySize();
//     Clay_Arena arena = Clay_CreateArenaWithCapacityAndMemory(total_memory_size, malloc(total_memory_size));
//     Clay_Initialize(arena, (Clay_Dimensions) { task_window_width, task_window_height }, (Clay_ErrorHandler){ NULL });
//
//     fonts[FONT_FOR_DEBUG_WINDOW] = LoadFontEx("fonts/jetbrains-mono.ttf", 16, 0, 0);
//
//     int codepoints[512] = {0};
//     int count = 0;
//     // ASCII
//     for (int i = 0x00; i <= 0x7F; i++) {
//         codepoints[count++] = i;
//     }
//     // UTF8 Кириллица
//     for (int i = 0x400; i <= 0x4FF; i++) {
//         codepoints[count++] = i;
//     }
//     fonts[FONT_MAIN] = LoadFontEx("fonts/jetbrains-mono.ttf", 32, codepoints, count);
//
//     Clay_SetMeasureTextFunction(Raylib_MeasureText, fonts);
// }

void read_input() {
    float delta_time = GetFrameTime();

    if (IsKeyPressed(KEY_F11)) {
        Clay_SetDebugModeEnabled(!Clay_IsDebugModeEnabled());
    }

    Clay_SetLayoutDimensions((Clay_Dimensions) { GetScreenWidth(), GetScreenHeight() });

    Vector2 mouse_position = GetMousePosition();
    bool is_lmb_down = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    Clay_SetPointerState((Clay_Vector2) { mouse_position.x, mouse_position.y }, is_lmb_down);

    Vector2 mouse_wheel = GetMouseWheelMoveV();
    Clay_UpdateScrollContainers(true, (Clay_Vector2) { mouse_wheel.x, mouse_wheel.y }, delta_time);
}

void draw_ui() {
    Clay_BeginLayout();

    Clay_TextElementConfig text_config = {
        .fontId = FONT_MAIN,
        .fontSize = 32,
        .textColor = ui_text_color,
    };

    CLAY({
        .id = CLAY_ID("Content"),
        .clip = {
            .vertical = true,
            .childOffset = Clay_GetScrollOffset()
        },
        .layout = {
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
            .padding = CLAY_PADDING_ALL(16),
            .childGap = 16,
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER },
        },
        .backgroundColor = ui_background_color,
    }) {
        for (int i = 0; i < ARRAY_LEN(textures); i++) {
            CLAY({
                .id = CLAY_IDI("ContentRow", i),
                .layout = {
                    .layoutDirection = CLAY_LEFT_TO_RIGHT,
                    .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
                    .padding = CLAY_PADDING_ALL(16),
                    .childGap = 16,
                },
            }) {
                CLAY({
                     .layout = {
                         .layoutDirection = CLAY_TOP_TO_BOTTOM,
                         .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
                         .childAlignment = { .x = CLAY_ALIGN_X_CENTER },
                         .padding = CLAY_PADDING_ALL(2),
                     },
                     .border = {
                        .color = ui_border_color,
                        .width = CLAY_BORDER_ALL(4),
                     },
                     .cornerRadius = CLAY_CORNER_RADIUS(16),
                }) {
                    CLAY_TEXT(texture_names[i], CLAY_TEXT_CONFIG(text_config));
                    CLAY({
                         .layout = {
                             .layoutDirection = CLAY_LEFT_TO_RIGHT,
                             .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
                             .childAlignment = { .x = CLAY_ALIGN_X_CENTER },
                         },
                    }) {
                        CLAY({
                             .layout = {
                                .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) },
                             },
                             .aspectRatio = { 1.0f },
                             .image = { .imageData = &textures[i] },
                        });

                        CLAY({
                            .id = CLAY_IDI("Histogram", i),
                            .layout = {
                                .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) },
                                .padding = CLAY_PADDING_ALL(16),
                            },
                            .border = {
                                .color = ui_border_color,
                                .width = CLAY_BORDER_ALL(4),
                            },
                        }) {
                            Rectangle bounds = pad_bounding_box(Clay_GetElementData(CLAY_IDI("Histogram", i)).boundingBox, 4);
                            if (i == 0) {
                                histogram_for_all_colors.bounds = bounds;
                            } else {
                                histograms[i - 1].bounds = bounds;
                            }
                        };
                    }
                };
            }
        }
    }

    Clay_RenderCommandArray render_commands = Clay_EndLayout();

    Clay_Raylib_Render(render_commands, fonts);
}

void draw_histogram_for_all_colors(Histogram histogram) {
    int n = histogram.bar_count;
    Rectangle bounds = histogram.bounds;

    float max_y_value = 0;
    for (int i = 0; i < n; i++) {
        max_y_value = fmax(max_y_value, (float)histogram.values[i]);
    }

    float x = histogram.bounds.x;
    float bar_width = (float)bounds.width / n;
    for (int i = 0; i < n; i++) {
        float bar_height = ((float)histogram.values[i] / max_y_value) * (float)bounds.height;

        Rectangle bar = {
            .x = x,
            .y = bounds.y + bounds.height - bar_height,
            .width = bar_width,
            .height = bar_height,
        };
        Color color = primary_colors[i];
        DrawRectangleRec(bar, color);

        x += bar_width;
    }
}

Histogram make_smaller_histogram(Histogram histogram, int ratio) {
    Histogram compact_histogram = histogram;

    compact_histogram.bar_count = histogram.bar_count / ratio;

    for (int i = 0; i < histogram.bar_count; i++) {
        compact_histogram.values[i / ratio] += histogram.values[i];
    }

    return compact_histogram;
}

void draw_histogram(Histogram histogram) {
    int n = histogram.bar_count;
    Rectangle bounds = histogram.bounds;

    float max_y_value = 0;
    for (int i = 0; i < n; i++) {
        max_y_value = fmax(max_y_value, (float)histogram.values[i]);
    }

    float x = bounds.x;
    for (int i = 0; i < n; i++) {
        float bar_width = (float)bounds.width / n;
        float bar_height = ((float)histogram.values[i - 1] / max_y_value) * (float)bounds.height;

        Rectangle bar = {
            .x = x,
            .y = bounds.y + bounds.height - bar_height,
            .width = bar_width,
            .height = bar_height,
        };

        float brightness = (float)i / n;
        Color adjusted_color = histogram.color;
        adjusted_color.r *= brightness;
        adjusted_color.g *= brightness;
        adjusted_color.b *= brightness;

        DrawRectangleRec(bar, adjusted_color);

        x += bounds.width / histogram.bar_count;
    }
}

Rectangle pad_bounding_box(Clay_BoundingBox box, float amount) {
    Rectangle result = {0};

    result.x = box.x + amount;
    result.y = box.y + amount;
    result.width = box.width - 2 * amount;
    result.height = box.height - 2 * amount;

    return result;
}
