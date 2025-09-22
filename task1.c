// Задание 1
// Преобразовать изображение из RGB в оттенки серого. Реализовать два варианта формулы с учетом разных 
// вкладов R, G и B в интенсивность (см презентацию). Затем найти разность полученных полутоновых изображений. 
// Построить гистограммы интенсивности после одного и второго преобразования.

#include "lab2.h"

// В цветовых пространствах PAL и NTSC используют
Color ConvertToGrayscale1(Color color)
{
    unsigned char gray = (unsigned char)(0.299f * color.r + 0.587f * color.g + 0.114f * color.b);
    return (Color) { gray, gray, gray, color.a };
}

// Для учёта особенностей восприятия изображения человеческим глазом (чувствительность к
// зелёному и синему цвету) в модели HDTV используют
Color ConvertToGrayscale2(Color color)
{
    unsigned char gray = (unsigned char)(0.2126f * color.r + 0.7152f * color.g + 0.0722f * color.b);
    return (Color) { gray, gray, gray, color.a };
}

int* CreateHistogram(Image image, int* maxValue)
{
    int* histogram = (int*)calloc(256, sizeof(int));
    *maxValue = 0;

    for (int y = 0; y < image.height; y++)
    {
        for (int x = 0; x < image.width; x++)
        {
            Color color = GetImageColor(image, x, y);
            histogram[color.r]++;
            if (histogram[color.r] > *maxValue) *maxValue = histogram[color.r];
        }
    }
    return histogram;
}

void DrawHistogram(int* histogram, int maxValue, int x, int y, int width, int height, Color color)
{
    DrawRectangle(x, y, width, height, LIGHTGRAY);
    DrawRectangleLines(x, y, width, height, GRAY);

    float barWidth = (float)width / 256.0f;
    for (int i = 0; i < 256; i++)
    {
        int barHeight = (int)((float)histogram[i] / (float)maxValue * (height - 20));
        DrawRectangle(x + (int)(i * barWidth), y + height - barHeight - 10,
            (int)barWidth, barHeight, color);
    }

    DrawText("0", x, y + height - 5, 10, BLACK);
    DrawText("255", x + width - 25, y + height - 5, 10, BLACK);
}

typedef struct {
    Vector2 position;
    Vector2 size;
    float scale;
} UiElement;

UiElement CalculateElementPosition(int row, int col, int totalRows, int totalCols,
    int screenWidth, int screenHeight, Texture2D original_texture) {
    float padding = screenWidth * 0.02f;
    float availableWidth = screenWidth - padding * (totalCols + 1);
    float availableHeight = screenHeight - padding * (totalRows + 1);

    UiElement element;
    element.size.x = availableWidth / totalCols;
    element.size.y = availableHeight / totalRows;
    element.position.x = padding + (element.size.x + padding) * col;
    element.position.y = padding + (element.size.y + padding) * row;
    element.scale = element.size.x / original_texture.width;

    return element;
}

void task1(int argc, char** argv) {
    SetWindowTitle("Задание 1");
    int task_window_width = GetMonitorWidth(0);
    int task_window_height = GetMonitorHeight(0);
    ToggleBorderlessWindowed();
    //SetWindowSize(screenWidth, screenHeight);
    SetWindowState(FLAG_WINDOW_RESIZABLE);

    

    Image original_image = LoadImage("images/fruits2.jpg");

    Image image1 = ImageCopy(original_image);
    Image image2 = ImageCopy(original_image);

    for (int y = 0; y < original_image.height; y++)
    {
        for (int x = 0; x < original_image.width; x++)
        {
            Color color = GetImageColor(original_image, x, y);

            Color gray1 = ConvertToGrayscale1(color);
            ImageDrawPixel(&image1, x, y, gray1);

            Color gray2 = ConvertToGrayscale2(color);
            ImageDrawPixel(&image2, x, y, gray2);
        }
    }

    Image diff_image = ImageCopy(original_image);
    for (int y = 0; y < original_image.height; y++)
    {
        for (int x = 0; x < original_image.width; x++)
        {
            Color color1 = GetImageColor(image1, x, y);
            Color color2 = GetImageColor(image2, x, y);

            int diff = abs(color1.r - color2.r);
            Color diff_color = (Color){ diff, diff, diff, color1.a };
            ImageDrawPixel(&diff_image, x, y, diff_color);
        }
    }

    Texture2D original_texture = LoadTextureFromImage(original_image);
    Texture2D texture1 = LoadTextureFromImage(image1);
    Texture2D texture2 = LoadTextureFromImage(image2);
    Texture2D diff_texture = LoadTextureFromImage(diff_image);


    int maxValue1, maxValue2;
    int* histogram1 = CreateHistogram(image1, &maxValue1);
    int* histogram2 = CreateHistogram(image2, &maxValue2);

    while (!WindowShouldClose()) {
        Window_Info window = get_window_info();

        BeginDrawing();
        ClearBackground(ui_background_color);

        Texture2D images[] = { original_texture, texture1, texture2, diff_texture };
        const char* titles[] = { "Original", "Grayscale 1", "Grayscale 2", "Difference" };

        for (int i = 0; i < 4; i++) {
            UiElement elem = CalculateElementPosition(0, i, 2, 4, task_window_width, task_window_height,original_texture);

            DrawTextureEx(images[i], elem.position, 0.0f, elem.scale, WHITE);
            //float imageHeight = original_texture.height * elem.scale;
            DrawText(titles[i], elem.position.x, elem.position.y - 30,
                task_window_width * 0.015f, BLACK);
        }

        UiElement hist1 = CalculateElementPosition(1, 0, 2, 2, task_window_width, task_window_height,original_texture);
        UiElement hist2 = CalculateElementPosition(1, 1, 2, 2, task_window_width, task_window_height, original_texture);

        DrawHistogram(histogram1, maxValue1, hist1.position.x, hist1.position.y,
            hist1.size.x, hist1.size.y, RED);
        DrawText("Grayscale 1", hist1.position.x, hist1.position.y + hist1.size.y + 1,
            task_window_width * 0.015f, BLACK);
        DrawHistogram(histogram2, maxValue2, hist2.position.x, hist2.position.y,
            hist2.size.x, hist2.size.y, BLUE);
        DrawText("Grayscale 2", hist2.position.x, hist2.position.y + hist2.size.y + 1,
            task_window_width * 0.015f, BLACK);

        EndDrawing();
    }
    ToggleBorderlessWindowed();
    SetWindowSize(menu_window_width, menu_window_height);

    free(histogram1);
    free(histogram2);

    UnloadImage(original_image);
    UnloadImage(image1);
    UnloadImage(image2);
    UnloadImage(diff_image);
    UnloadTexture(original_texture);
    UnloadTexture(texture1);
    UnloadTexture(texture2);
    UnloadTexture(diff_texture);
}
