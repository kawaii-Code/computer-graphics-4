#include "lab2.h"

Window_Info get_window_info(void) {
    Window_Info result = {0};

    result.width = GetScreenWidth();
    result.height = GetScreenHeight();
    result.center.x = result.width / 2.0f;
    result.center.y = result.height / 2.0f;

    return result;
}

void draw_texture_centered(Texture2D texture, float x, float y) {
    x -= texture.width / 2.0f;
    y -= texture.height / 2.0f;
    DrawTexture(texture, x, y, WHITE);
}

void draw_text_centered(Font font, const char *text, float x, float y, Color color) {
    int font_size = font.baseSize;

    int line_count;
    char **lines = TextSplit(text, '\n', &line_count);

    float total_text_height = 0.0f;
    for (int i = 0; i < line_count; i++) {
        char *line = lines[i];
        Vector2 text_size = MeasureTextEx(font, line, font_size, 0.0f);
        total_text_height += text_size.y;
    }

    y -= total_text_height / 2.0f;

    for (int i = 0; i < line_count; i++) {
        char *line = lines[i];

        Vector2 text_size = MeasureTextEx(font, line, font_size, 0.0f);

        Vector2 text_position = {
            .x = x - 0.5f * text_size.x,
            .y = y,
        };
        DrawTextEx(font, line, text_position, font_size, 0.0, color);

        y += text_size.y;
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
