#include "lab2.h"

static Texture2D texture;

typedef struct {
    int x;
    int y;
} ScreenPosition;

static void DrawBrush(int cx, int cy, int width) {
    int half = width / 2;
    for (int dx = -half; dx <= half; dx++) {
        for (int dy = -half; dy <= half; dy++) {
            DrawPixel(cx + dx, cy + dy, BLACK);
        }
    }
}

static void plotLineLow(const ScreenPosition beg, const ScreenPosition end, int width) {
    int dx = end.x - beg.x;
    int dy = end.y - beg.y;
    int yi = 1;

    if (dy < 0) {
        yi = -1;
        dy *= -1;
    }

    int D = (2 * dy) - dx;
    int y = beg.y;

    for (int x = beg.x; x <= end.x; x++) {
        DrawBrush(x, y, width);
        if (D > 0) {
            y += yi;
            D += 2 * (dy - dx);
        } else {
            D += 2 * dy;
        }
    }
}

static void plotLineHigh(const ScreenPosition beg, const ScreenPosition end, int width) {
    int dx = end.x - beg.x;
    int dy = end.y - beg.y;
    int xi = 1;

    if (dx < 0) {
        xi = -1;
        dx *= -1;
    }

    int D = (2 * dx) - dy;
    int x = beg.x;

    for (int y = beg.y; y <= end.y; y++) {
        DrawBrush(x, y, width);
        if (D > 0) {
            x += xi;
            D += 2 * (dx - dy);
        } else {
            D += 2 * dx;
        }
    }
}

static void plotLine(const ScreenPosition beg, const ScreenPosition end, int width) {
    if (abs(end.y - beg.y) < abs(end.x - beg.x)) {
        if (beg.x > end.x) {
            plotLineLow(end, beg, width);
        } else {
            plotLineLow(beg, end, width);
        }
    } else {
        if (beg.y > end.y) {
            plotLineHigh(end, beg, width);
        } else {
            plotLineHigh(beg, end, width);
        }
    }
}

static float rfpart(float x) { return 1 - (x - floorf(x)); }
static float fpart(float x)  { return x - floorf(x); }
static float roundf_custom(float x) { return floorf(x + 0.5f); }

static void DrawPixelAlpha(int x, int y, float brightness, Color base) {
    Color c = base;
    c.a = (unsigned char)(brightness * 255);
    DrawPixel(x, y, c);
}

static void plotLineWu(const ScreenPosition beg, const ScreenPosition end, int width) {
    float x0 = beg.x;
    float y0 = beg.y;
    float x1 = end.x;
    float y1 = end.y;

    bool steep = fabsf(y1 - y0) > fabsf(x1 - x0);
    if (steep) {
        float tmp;
        tmp = x0; x0 = y0; y0 = tmp;
        tmp = x1; x1 = y1; y1 = tmp;
    }
    if (x0 > x1) {
        float tmp;
        tmp = x0; x0 = x1; x1 = tmp;
        tmp = y0; y0 = y1; y1 = tmp;
    }

    float dx = x1 - x0;
    float dy = y1 - y0;
    float gradient = (dx == 0.0f) ? 1.0f : dy / dx;

    float xend = roundf_custom(x0);
    float yend = y0 + gradient * (xend - x0);
    float xgap = rfpart(x0 + 0.5f);
    int xpxl1 = (int)xend;
    int ypxl1 = (int)floorf(yend);
    if (steep) {
        DrawPixelAlpha(ypxl1,   xpxl1, rfpart(yend) * xgap, BLACK);
        DrawPixelAlpha(ypxl1+1, xpxl1, fpart(yend) * xgap, BLACK);
    } else {
        DrawPixelAlpha(xpxl1, ypxl1,   rfpart(yend) * xgap, BLACK);
        DrawPixelAlpha(xpxl1, ypxl1+1, fpart(yend) * xgap, BLACK);
    }
    float intery = yend + gradient;

    xend = roundf_custom(x1);
    yend = y1 + gradient * (xend - x1);
    xgap = fpart(x1 + 0.5f);
    int xpxl2 = (int)xend;
    int ypxl2 = (int)floorf(yend);
    if (steep) {
        DrawPixelAlpha(ypxl2,   xpxl2, rfpart(yend) * xgap, BLACK);
        DrawPixelAlpha(ypxl2+1, xpxl2, fpart(yend) * xgap, BLACK);
    } else {
        DrawPixelAlpha(xpxl2, ypxl2,   rfpart(yend) * xgap, BLACK);
        DrawPixelAlpha(xpxl2, ypxl2+1, fpart(yend) * xgap, BLACK);
    }

    if (steep) {
        for (int x = xpxl1 + 1; x < xpxl2; x++) {
            DrawPixelAlpha((int)floorf(intery),   x, rfpart(intery), BLACK);
            DrawPixelAlpha((int)floorf(intery)+1, x, fpart(intery), BLACK);
            intery += gradient;
        }
    } else {
        for (int x = xpxl1 + 1; x < xpxl2; x++) {
            DrawPixelAlpha(x, (int)floorf(intery),   rfpart(intery), BLACK);
            DrawPixelAlpha(x, (int)floorf(intery)+1, fpart(intery), BLACK);
            intery += gradient;
        }
    }
}

static void emptyDraw(const ScreenPosition beg, const ScreenPosition end, int width) {

}

const int maxY = 100;

void task2(int argc, char** argv) {
    SetWindowTitle("Задание 2");
    SetWindowSize(task_window_width, task_window_height);

    Font font = fonts[FONT_MAIN];

    bool isDrawn = false;
    bool isPressed = false;
    ScreenPosition curBeg = {};
    ScreenPosition curEnd = {};
    float width = 1;

    void (*drawLine)(ScreenPosition, ScreenPosition, int) = emptyDraw;

    while (!WindowShouldClose()) {
        Window_Info window = get_window_info();

        BeginDrawing();
        ClearBackground(ui_background_color);

        if (Button((Rectangle) {50, 50, 120, 40}, "Брезенхем")) {
            drawLine = plotLine;
            isDrawn = false;
            isPressed = false;
        }
        if (Button((Rectangle) {200, 50, 120, 40}, "Ву")) {
            drawLine = plotLineWu;
            isDrawn = false;
            isPressed = false;
        }
        GuiSlider((Rectangle){350, 50, 200, 40}, "1", "20", &width, 1, 20);

        DrawLine(0, maxY, window.width, maxY, BLACK);

        Vector2 curMousePos = GetMousePosition();
        if (curMousePos.y > maxY) {
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !isPressed) {
                isPressed = true;
                isDrawn = true;

                curBeg = (ScreenPosition) {curMousePos.x, curMousePos.y};
            }
        }

        curMousePos.y = curMousePos.y > maxY ? curMousePos.y : (maxY + width / 2);
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && curMousePos.y > (maxY + width / 2)) {
            curEnd = (ScreenPosition) {curMousePos.x, curMousePos.y};
            isPressed = false;
        }

        if (isDrawn) {
            if (isPressed) {
                curEnd = (ScreenPosition) {curMousePos.x, curMousePos.y};
            }
            drawLine(curBeg, curEnd, width);
        }


        EndDrawing();
    }

    SetWindowSize(menu_window_width, menu_window_height);
}