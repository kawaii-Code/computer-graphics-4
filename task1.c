#include "lab2.h"

#define CANVAS_WIDTH  800
#define CANVAS_HEIGHT 600

typedef enum {
    PENCIL = 0,
    ERASER,
    BUCKET,
    IMAGE_BUCKET,
    BORDERS,
    CLEAR,
    TOOL_COUNT,
} Drawing_Tool;

typedef struct {
    int x;
    int y;
} Point;


static Image     canvas;
static Texture2D tool_textures[TOOL_COUNT];
static int       tool_button_size = 64;
static int       selected_tool = PENCIL;
static int       brush_size = 10;
static bool      visited[CANVAS_WIDTH * CANVAS_HEIGHT];
static Point     queue[CANVAS_WIDTH * CANVAS_HEIGHT];


bool tool_button(Drawing_Tool tool, int x, int y);
void int_slider(Rectangle rect, int min, int max, int *value);
Rectangle shrink_rect(Rectangle rect, float amount);
bool color_equal(Color a, Color b);
void fill(Color *pixels, int x, int y, Color replaced_color, Color new_color);
void fill_with_image(Color *pixels, int start_x, int start_y, int x, int y, Color replaced_color, Image image);
void find_borders(Image *destination, Color *pixels, int x, int y);


void task1(int argc, char** argv) {
    SetWindowTitle("Задание 1");
    ToggleBorderlessWindowed();

    canvas = GenImageColor(CANVAS_WIDTH, CANVAS_HEIGHT, RAYWHITE);
    ImageFormat(&canvas, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    Texture canvas_texture = LoadTextureFromImage(canvas);

    Image borders = GenImageColor(CANVAS_WIDTH, CANVAS_HEIGHT, BLANK);
    ImageFormat(&borders, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    Texture borders_texture = LoadTextureFromImage(borders);

    Image bird = LoadImage("images/bird.jpg");

    tool_textures[PENCIL] = LoadTexture("images/ui/pencil.png");
    tool_textures[ERASER] = LoadTexture("images/ui/eraser.png");
    tool_textures[BUCKET] = LoadTexture("images/ui/bucket.png");
    tool_textures[IMAGE_BUCKET] = LoadTexture("images/ui/bird.jpg");
    tool_textures[BORDERS] = LoadTexture("images/ui/borders.png");
    tool_textures[CLEAR] = LoadTexture("images/ui/skull.png");

    while (!WindowShouldClose()) {
        Window_Info window = get_window_info();

        int canvas_x = window.center.x - canvas.width / 2;
        int canvas_y = window.center.y - canvas.height / 2;
        Rectangle canvas_rect = {
            .x = canvas_x,
            .y = canvas_y,
            .width = canvas.width,
            .height = canvas.height,
        };

        if (CheckCollisionPointRec(GetMousePosition(), canvas_rect)) {
            Vector2 draw_position = GetMousePosition();
            draw_position.x -= canvas_x;
            draw_position.y -= canvas_y;

            switch (selected_tool) {
            case PENCIL:
                if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                    ImageDrawCircleV(&canvas, draw_position, brush_size, BLACK);
                }
                break;
            case ERASER:
                if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                    ImageDrawCircleV(&canvas, draw_position, brush_size, RAYWHITE);
                }
                break;
            case BUCKET:
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    int x = (int)draw_position.x;
                    int y = (int)draw_position.y;
                    Color replaced_color = GetImageColor(canvas, x, y);
                    Color new_color = RED;
                    if (!color_equal(replaced_color, new_color)) {
                        fill((Color *)canvas.data, x, y, replaced_color, RED);
                    }
                }
                break;
            case IMAGE_BUCKET:
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    int x = (int)draw_position.x;
                    int y = (int)draw_position.y;
                    Color replaced_color = GetImageColor(canvas, x, y);
                    memset(visited, false, sizeof(visited));
                    fill_with_image((Color *)canvas.data, bird.width/2, bird.height/2, x, y, replaced_color, bird);
                }
                break;
            case BORDERS:
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    find_borders(&borders, (Color *)canvas.data, (int)draw_position.x, (int)draw_position.y);
                    UpdateTexture(borders_texture, borders.data);
                }
                break;
            default:
                break;
            }
        }

        BeginDrawing();
        ClearBackground(ui_background_color);

        UpdateTexture(canvas_texture, canvas.data);
        DrawRectangle(
            canvas_x - 5,
            canvas_y - 5,
            canvas.width + 10,
            canvas.height + 10,
            ORANGE
        );
        DrawTexture(canvas_texture, canvas_x, canvas_y, WHITE);

        DrawTexture(borders_texture, canvas_x, canvas_y, WHITE);

        int tool_button_y = canvas_y - tool_button_size - 10;
        int tool_button_x = canvas_x - 5;
        if (tool_button(PENCIL, tool_button_x, tool_button_y)) {
            selected_tool = PENCIL;
        }
        tool_button_x += tool_button_size + 10;
        if (tool_button(ERASER, tool_button_x, tool_button_y)) {
            selected_tool = ERASER;
        }
        tool_button_x += tool_button_size + 10;
        if (tool_button(BUCKET, tool_button_x, tool_button_y)) {
            selected_tool = BUCKET;
        }
        tool_button_x += tool_button_size + 10;
        if (tool_button(IMAGE_BUCKET, tool_button_x, tool_button_y)) {
            selected_tool = IMAGE_BUCKET;
        }
        tool_button_x += tool_button_size + 10;
        if (tool_button(BORDERS, tool_button_x, tool_button_y)) {
            selected_tool = BORDERS;
        }
        tool_button_x += tool_button_size + 10;
        if (tool_button(CLEAR, tool_button_x, tool_button_y)) {
            ImageClearBackground(&canvas, RAYWHITE);
            ImageClearBackground(&borders, BLANK);
            UpdateTexture(borders_texture, borders.data);
        }
        tool_button_x += tool_button_size + 10;

        Rectangle slider_rect = {
            .x = canvas_x,
            .y = canvas_y + canvas.height + tool_button_size / 2,
            .width = 8 * tool_button_size,
            .height = tool_button_size / 2,
        };
        int_slider(slider_rect, 1, 100, &brush_size);

        if (CheckCollisionPointRec(GetMousePosition(), canvas_rect)) {
            switch (selected_tool) {
            case PENCIL:
                DrawCircleV(GetMousePosition(), brush_size, BLACK);
                break;
            case ERASER:
                DrawCircleLinesV(GetMousePosition(), brush_size, BLACK);
                break;
            default:
                break;
            }
        }

        EndDrawing();
    }

    ToggleBorderlessWindowed();
}

Rectangle shrink_rect(Rectangle rect, float amount) {
    float half_amount = amount / 2.0f;
    rect.x += half_amount;
    rect.y += half_amount;
    rect.width -= amount;
    rect.height -= amount;
    return rect;
}

void int_slider(Rectangle rect, int min, int max, int *value) {
    DrawRectangleRec(rect, BLACK);
    DrawRectangleRec(shrink_rect(rect, 4), GRAY);

    bool hovered = CheckCollisionPointRec(GetMousePosition(), rect);

    float progress = (float)(*value - min) / (float)(max - min);
    float handle_width = 10;
    Rectangle handle_rect = {
        .x = rect.x + progress * rect.width - handle_width / 2.0f,
        .y = rect.y,
        .width = handle_width,
        .height = rect.height,
    };

    DrawRectangleRec(handle_rect, hovered ? ORANGE : BLACK);
    draw_text_centered(fonts[FONT_MAIN], TextFormat("%d", min), rect.x - 20, rect.y + rect.height / 2.0f, WHITE);
    draw_text_centered(fonts[FONT_MAIN], TextFormat("%d", max), rect.x + rect.width + 20, rect.y + rect.height / 2.0f, WHITE);

    if (hovered && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        Vector2 mouse_position = GetMousePosition();
        float progress = (mouse_position.x - rect.x) / rect.width;

        *value = (int)(min + progress * (max - min));
    }
}

bool tool_button(Drawing_Tool tool, int x, int y) {
    Texture2D texture = tool_textures[tool];
    Rectangle texture_rect = {
        .x = 0,
        .y = 0,
        .width = texture.width,
        .height = texture.height,
    };
    Rectangle button_rect = {
        .x = x,
        .y = y,
        .width = tool_button_size,
        .height = tool_button_size,
    };

    Rectangle inside_rect = shrink_rect(button_rect, 6);

    bool hovered = CheckCollisionPointRec(GetMousePosition(), button_rect);
    Color background_color = (selected_tool == tool) ? RED : hovered ? YELLOW : BLACK;

    DrawRectangleLinesEx(button_rect, 3.0f, background_color);
    DrawTexturePro(texture, texture_rect, inside_rect, (Vector2){0, 0}, 0.0f, WHITE);

    if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        return true;
    }
    return false;
}

bool color_equal(Color a, Color b) {
    return *(uint32_t *)&a == *(uint32_t *)&b;
}

void find_borders(Image *destination, Color *pixels, int x, int y) {
    ImageClearBackground(destination, BLANK);
    Color *result_pixels = (Color *)destination->data;

    Color border_color = pixels[y * CANVAS_WIDTH + x];

    memset(visited, false, sizeof(visited));
    int read = 0;
    int write = 0;
    int queue_size = 1;
    queue[0] = (Point) { .x = x, .y = y };
    while (queue_size > 0) {
        Point pixel = queue[read];
        queue_size -= 1;
        read = (read + 1) % ARRAY_LEN(queue);

        int i = pixel.y * canvas.width + pixel.x;
        if (!(0 <= i && i < CANVAS_WIDTH * CANVAS_HEIGHT)) {
            continue;
        }
        if (visited[i]) {
            continue;
        }

        visited[i] = true;

        if (color_equal(pixels[i], border_color)) {
            result_pixels[i] = BLUE;

            write = (write + 1) % ARRAY_LEN(queue);
            queue[write] = (Point) { .x = pixel.x - 1, .y = pixel.y };
            write = (write + 1) % ARRAY_LEN(queue);
            queue[write] = (Point) { .x = pixel.x + 1, .y = pixel.y };
            write = (write + 1) % ARRAY_LEN(queue);
            queue[write] = (Point) { .x = pixel.x, .y = pixel.y - 1 };
            write = (write + 1) % ARRAY_LEN(queue);
            queue[write] = (Point) { .x = pixel.x, .y = pixel.y + 1 };
            queue_size += 4;
        }
    }
}

void fill(Color *pixels, int x, int y, Color replaced_color, Color new_color) {
    int i = y * CANVAS_WIDTH + x;
    if (!color_equal(pixels[i], replaced_color)) {
        return;
    }

    int left, right;
    for (left = x - 1; left >= 0; left--) {
        if (!color_equal(pixels[y * CANVAS_WIDTH + left], replaced_color)) {
            left += 1;
            break;
        }
    }
    for (right = x + 1; right < CANVAS_WIDTH; right++) {
        if (!color_equal(pixels[y * CANVAS_WIDTH + right], replaced_color)) {
            right -= 1;
            break;
        }
    }

    left = fmax(left, 0);
    right = fmin(CANVAS_WIDTH - 1, right);

    for (int col = left; col <= right; col++) {
        pixels[y * CANVAS_WIDTH + col] = new_color;
    }
    for (int col = left; col <= right; col++) {
        if (y > 0) {
            fill(pixels, col, y - 1, replaced_color, new_color);
        }
        if (y < CANVAS_HEIGHT - 1) {
            fill(pixels, col, y + 1, replaced_color, new_color);
        }
    }
}

void fill_with_image(Color *pixels, int start_x, int start_y, int x, int y, Color replaced_color, Image image) {
    int i = y * CANVAS_WIDTH + x;
    if (visited[i]) {
        return;
    }
    visited[i] = true;
    if (!color_equal(pixels[i], replaced_color)) {
        return;
    }

    int left, right;
    for (left = x - 1; left >= 0; left--) {
        if (!color_equal(pixels[y * CANVAS_WIDTH + left], replaced_color)) {
            left += 1;
            break;
        }
    }
    for (right = x + 1; right < CANVAS_WIDTH; right++) {
        if (!color_equal(pixels[y * CANVAS_WIDTH + right], replaced_color)) {
            right -= 1;
            break;
        }
    }

    left = fmax(left, 0);
    right = fmin(CANVAS_WIDTH - 1, right);

    for (int col = left; col <= right; col++) {
        int image_x = fabsf(start_x - col);
        int image_y = fabsf(start_y - y);
        image_x %= image.width;
        image_y %= image.height;
        pixels[y * CANVAS_WIDTH + col] = GetImageColor(image, image_x, image_y);
    }
    for (int col = left; col <= right; col++) {
        if (y > 0) {
            fill_with_image(pixels, start_x, start_y, col, y - 1, replaced_color, image);
        }
        if (y < CANVAS_HEIGHT - 1) {
            fill_with_image(pixels, start_x, start_y, col, y + 1, replaced_color, image);
        }
    }
}
