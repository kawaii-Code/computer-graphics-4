#include "lab2.h"

Image images[3];
Texture textures[3];

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
} HSV;

HSV rgbToHsv(Color c) {
    float r = c.r / 255.0f;
    float g = c.g / 255.0f;
    float b = c.b / 255.0f;

    float max = r > g ? (r > b ? r : b) : (g > b ? g : b);
    float min = r < g ? (r < b ? r : b) : (g < b ? g : b);
    float delta = max - min;

    HSV hsv;
    // Hue
    if (delta == 0)
        hsv.h = 0;
    else if (max == r)
        hsv.h = 60 * (((g - b) / delta));
    else if (max == g)
        hsv.h = 60 * (((b - r) / delta) + 2);
    else // max == b
        hsv.h = 60 * (((r - g) / delta) + 4);

    if (hsv.h < 0) hsv.h += 360;

    // Saturation
    hsv.s = (max == 0) ? 0 : delta / max;

    // Value
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
        255
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

    const char *images_names[3];
    images_names[0] = "images/bird.jpg";
    images_names[1] = "images/ryan.jpg";
    images_names[2] = "images/pug.png";

    for (int i = 0; i < 3; i++) {
        images[i] = LoadImage(images_names[i]);
        ImageFormat(&images[i], PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
        textures[i] = LoadTextureFromImage(images[i]);
    }

    int selectedOption = 0;
    const char *options[3] = { "Птичка", "Райан Гослинг", "Пуг" };

    bool showDropdown = false;

    hue = 0;
    saturation = 1;
    brightness = 1;

    drawSelectedImage(0);

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

        DropdownMenu((Rectangle){350, 100, 100, 40}, &selectedOption, &showDropdown, options, &drawSelectedImage);

        GuiSlider((Rectangle){50, 450, 200, 20}, "0", "360", &hue, 0, 360);
        GuiSlider((Rectangle){275, 450, 200, 20}, "0", "1", &saturation, 0, 1);
        GuiSlider((Rectangle){500, 450, 200, 20}, "0", "1", &brightness, 0, 1);

        for (int i = 0; i < current_texture.width * current_texture.height; i++) {
            HSV hsv = rgbToHsv(pixels[i]);

            hsv.h = fmodf(hsv.h + hue, 360.0f);
            hsv.s = Clamp(hsv.s * (saturation), 0.0f, 1.0f);
            hsv.v = Clamp(hsv.v * (brightness), 0.0f, 1.0f);

            modified_pixels[i] = hsvToRgb(hsv);
        }

        UpdateTexture(modified_texture, modified_pixels);

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

    for (int i = 0; i < 3; i++) {
        UnloadTexture(textures[i]);
        UnloadImage(images[i]);
    }

    free(modified_pixels);
    UnloadImage(modified_image);
    UnloadTexture(modified_texture);
    SetWindowSize(menu_window_width, menu_window_height);
}
