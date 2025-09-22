#include "lab2.h"

const int IMG_COUNT = 4;

Image images[IMG_COUNT];
Texture textures[IMG_COUNT];

Image modified_image;
Texture2D modified_texture;
Color* modified_pixels;

Texture2D current_texture;
Color* pixels;

Window_Info window;

float hue, saturation, brightness;

typedef struct {
    float h;
    float s;
    float v;
    int a;
} HSV;

HSV rgbToHsv(Color c) {
    float r = c.r / 255.0;
    float g = c.g / 255.0;
    float b = c.b / 255.0;

    float max = r > g ? (r > b ? r : b) : (g > b ? g : b);
    float min = r < g ? (r < b ? r : b) : (g < b ? g : b);
    float delta = max - min;

    HSV hsv;
    hsv.a = c.a;

    if (delta == 0) {
        hsv.h = 0;
    } else if (max == r && g >= b) {
        hsv.h = ((g - b) / delta);
    } else if (max == r && g < b) {
        hsv.h = ((b - g) / delta) + 6;
    } else if (max == g) {
        hsv.h = ((b - r) / delta) + 2;
    } else {
        hsv.h = ((r - g) / delta) + 4;
    }

    hsv.h *= 60;

    if (hsv.h < 0) hsv.h += 360;

    hsv.s = (max == 0) ? 0 : delta / max;

    hsv.v = max;

    return hsv;
}

Color hsvToRgb(HSV hsv) {
    float c = hsv.v * hsv.s;
    float x = c * (1 - fabsf(fmodf(hsv.h / 60.0f, 2) - 1));
    float m = hsv.v - c;

    float r, g, b;
    if (hsv.h < 60)      { r = c; g = x; b = 0; }
    else if (hsv.h < 120){ r = x; g = c; b = 0; }
    else if (hsv.h < 180){ r = 0; g = c; b = x; }
    else if (hsv.h < 240){ r = 0; g = x; b = c; }
    else if (hsv.h < 300){ r = x; g = 0; b = c; }
    else                 { r = c; g = 0; b = x; }

    Color out = {
        (unsigned char)((r + m) * 255),
        (unsigned char)((g + m) * 255),
        (unsigned char)((b + m) * 255),
        hsv.a
    };
    return out;
}


void drawSelectedImage(int selectedOption) {
    if (modified_pixels) free(modified_pixels);
    UnloadImage(modified_image);
    UnloadTexture(modified_texture);

    current_texture = textures[selectedOption];

    Image cur_img = images[selectedOption];
    modified_image = ImageCopy(cur_img);

    pixels = cur_img.data;

    int pixelCount = modified_image.width * modified_image.height;
    modified_pixels = malloc(pixelCount * sizeof(Color));
    memcpy(modified_pixels, modified_image.data, pixelCount * sizeof(Color));

    modified_texture = LoadTextureFromImage(modified_image);
}

void task3(int argc, char **argv) {
    SetWindowTitle("Задание 3");
    SetWindowSize(task_window_width, task_window_height);

    const char *images_names[IMG_COUNT];
    images_names[0] = "images/bird.jpg";
    images_names[1] = "images/ryan.jpg";
    images_names[2] = "images/pug.png";
    images_names[3] = "images/cabans.jpg";

    for (int i = 0; i < IMG_COUNT; i++) {
        images[i] = LoadImage(images_names[i]);
        ImageFormat(&images[i], PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
        textures[i] = LoadTextureFromImage(images[i]);
    }

    int selectedOption = 0;
    const char *options[IMG_COUNT] = { "Птичка", "Райан Гослинг", "Пуг", "Кабаны" };

    bool showDropdown = false;

    hue = 0;
    saturation = 1;
    brightness = 1;

    drawSelectedImage(0);

    int fontSize = 20;
    char* hueText = "Hue";
    char* saturationText = "Saturation";
    char* brightnessText = "Brightness";
    Vector2 hueSize = MeasureTextEx(fonts[FONT_MAIN], hueText, fontSize, 1);
    Vector2 saturationSize = MeasureTextEx(fonts[FONT_MAIN], saturationText, fontSize, 1);
    Vector2 brightnessSize = MeasureTextEx(fonts[FONT_MAIN], brightnessText, fontSize, 1);

    while (!WindowShouldClose()) {
        Window_Info window = get_window_info();
        BeginDrawing();
        ClearBackground(ui_background_color);

        if (Button((Rectangle){350, 50, 100, 40}, "Сохранить")) {
            Image imgToSave = LoadImageFromTexture(modified_texture);

            // Будет ли работать на винде??
            ExportImage(imgToSave, "images/saved_image.png");
            UnloadImage(imgToSave);

            printf("Сохранено saved_image.png\n");
        }

        DropdownMenu((Rectangle){350, 100, 100, 40}, &selectedOption, &showDropdown, IMG_COUNT, options, &drawSelectedImage);

        DrawTextEx(fonts[FONT_MAIN], hueText, (Vector2){ 50, 420 }, fontSize, 0, BLACK);
        int sl1 = GuiSlider((Rectangle){50, 450, 200, 20}, "0", "360", &hue, 0, 360);

        DrawTextEx(fonts[FONT_MAIN], saturationText, (Vector2){ 300, 420 }, fontSize, 0, BLACK);
        int sl2 = GuiSlider((Rectangle){300, 450, 200, 20}, "0", "1", &saturation, 0, 1);

        DrawTextEx(fonts[FONT_MAIN], brightnessText, (Vector2){ 550, 420 }, fontSize, 0, BLACK);
        int sl3 = GuiSlider((Rectangle){550, 450, 200, 20}, "0", "1", &brightness, 0, 1);

        if (sl1 || sl2 || sl3) {
            for (int i = 0; i < current_texture.width * current_texture.height; i++) {
                HSV hsv = rgbToHsv(pixels[i]);

                hsv.h = fmodf(hsv.h + hue, 360.0f);
                hsv.s = Clamp(hsv.s * (saturation), 0.0f, 1.0f);
                hsv.v = Clamp(hsv.v * (brightness), 0.0f, 1.0f);

                modified_pixels[i] = hsvToRgb(hsv);
            }

            UpdateTexture(modified_texture, modified_pixels);
        }

        DrawTexturePro(modified_texture,
            (Rectangle){0, 0, modified_texture.width, modified_texture.height},
            (Rectangle){window.width - 300, 0, 300, 300},
            (Vector2){0, 0},
            0,
            RAYWHITE
        );

        DrawTexturePro(current_texture,
            (Rectangle){0, 0, current_texture.width, current_texture.height},
            (Rectangle){0, 0, 300, 300},
            (Vector2){0, 0},
            0,
            RAYWHITE
        );

        EndDrawing();
    }

    for (int i = 0; i < IMG_COUNT; i++) {
        UnloadTexture(textures[i]);
        UnloadImage(images[i]);
    }

    free(modified_pixels);
    modified_pixels = 0;
    UnloadTexture(modified_texture);
    SetWindowSize(menu_window_width, menu_window_height);
}
