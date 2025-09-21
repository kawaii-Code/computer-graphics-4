// Задание 2.
// Выделить из полноцветного изображения каждый из каналов R, G, B и вывести результат.
// Построить гистограмму по цветам (3 штуки).

#include "lab2.h"

typedef enum {
    CHANNEL_RED = 0,
    CHANNEL_GREEN,
    CHANNEL_BLUE,
    CHANNEL_COUNT,
} Channel;


//
// Я всё храню в глобальных переменных, ибо лабы короткие
// и получается довольно удобно. Главное, не забывайте
// помечать их static, иначе могут быть конфликты имён.
//
static Image       image;
static Texture2D   textures[4];
static Clay_String texture_names[ARRAY_LEN(textures)] = {};
static Color       primary_colors[CHANNEL_COUNT] = {};
static Histogram   histogram_for_all_colors = { .bar_count = 3 };
static Histogram   histograms[CHANNEL_COUNT] = {};


static void read_input();
static void draw_ui();
static void draw_histogram_for_all_colors(Histogram histogram);
static Rectangle pad_bounding_box(Clay_BoundingBox box, float amount);


void task2(int argc, char **argv) {
    SetWindowTitle("Лаба 2 Задание 2: RGB");
    SetWindowSize(task_window_width, task_window_height);

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

static void read_input() {
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

static void draw_ui() {
    Clay_BeginLayout();

    Clay_TextElementConfig text_config = {
        .fontId = FONT_MAIN,
        .fontSize = 32,
        .textColor = TO_CLAY_COLOR(ui_text_color),
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
        .backgroundColor = TO_CLAY_COLOR(ui_background_color),
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
                        .color = TO_CLAY_COLOR(ui_border_color),
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
                                .color = TO_CLAY_COLOR(ui_border_color),
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

static void draw_histogram_for_all_colors(Histogram histogram) {
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

static Rectangle pad_bounding_box(Clay_BoundingBox box, float amount) {
    Rectangle result = {0};

    result.x = box.x + amount;
    result.y = box.y + amount;
    result.width = box.width - 2 * amount;
    result.height = box.height - 2 * amount;

    return result;
}
