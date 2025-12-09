#include "cornell_box.h"
#include "third_party/include/raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define RAYGUI_IMPLEMENTATION
#include "third_party/include/raygui.h"

#define RENDER_WIDTH 400
#define RENDER_HEIGHT 300
#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800
#define UI_PANEL_WIDTH 320

static Font cyrillic_font = {0};

void DrawTextRu(const char* text, float x, float y, int fontSize, Color color) {
    if (cyrillic_font.texture.id != 0) {
        DrawTextEx(cyrillic_font, text, (Vector2){x, y}, (float)fontSize, 1, color);
    } else {
        DrawText(text, (int)x, (int)y, fontSize, color);
    }
}

const char* wall_names[] = {
    "Левая (красная)",
    "Правая (зелёная)",
    "Пол",
    "Потолок",
    "Задняя",
    "Передняя"
};

const char* object_names[] = {
    "Левая стена",
    "Правая стена",
    "Пол",
    "Потолок",
    "Задняя стена",
    "Передняя стена",
    "Куб 1 (большой)",
    "Куб 2 (маленький)",
    "Сфера 1 (красная)",
    "Сфера 2 (зеркальная)"
};

typedef struct {
    RT_Scene scene;
    Image render_image;
    Texture2D render_texture;
    bool needs_render;

    int selected_object;
    int selected_wall_for_mirror;
    bool show_help;

    int selected_light;
    float light_move_speed;

    bool progressive_render;
    int render_scale;
} AppState;

void init_app(AppState* app) {
    rt_scene_create_cornell_box(&app->scene);

    app->render_image = GenImageColor(RENDER_WIDTH, RENDER_HEIGHT, BLACK);
    app->render_texture = LoadTextureFromImage(app->render_image);
    app->needs_render = true;

    app->selected_object = 6;
    app->selected_wall_for_mirror = -1;
    app->show_help = false;
    app->selected_light = 1;
    app->light_move_speed = 1.0f;
    app->render_scale = 1;
    app->progressive_render = false;
}

void cleanup_app(AppState* app) {
    UnloadImage(app->render_image);
    UnloadTexture(app->render_texture);
}

void render_scene(AppState* app) {
    rt_render(&app->scene, &app->render_image);
    UpdateTexture(app->render_texture, app->render_image.data);
    app->needs_render = false;
}

void handle_input(AppState* app) {
    if (app->selected_light >= 0 && app->selected_light < app->scene.light_count) {
        RT_Light* light = &app->scene.lights[app->selected_light];

        // Используем IsKeyPressed для пошагового перемещения
        if (IsKeyPressed(KEY_W)) { light->position.z -= app->light_move_speed; app->needs_render = true; }
        if (IsKeyPressed(KEY_S)) { light->position.z += app->light_move_speed; app->needs_render = true; }
        if (IsKeyPressed(KEY_A)) { light->position.x -= app->light_move_speed; app->needs_render = true; }
        if (IsKeyPressed(KEY_D)) { light->position.x += app->light_move_speed; app->needs_render = true; }
        if (IsKeyPressed(KEY_Q)) { light->position.y += app->light_move_speed; app->needs_render = true; }
        if (IsKeyPressed(KEY_E)) { light->position.y -= app->light_move_speed; app->needs_render = true; }
    }

    if (IsKeyPressed(KEY_TAB)) {
        app->selected_light = (app->selected_light + 1) % app->scene.light_count;
    }

    if (IsKeyPressed(KEY_LEFT)) {
        if (app->selected_object > 6) app->selected_object--;
        else app->selected_object = app->scene.primitive_count - 1;
    }
    if (IsKeyPressed(KEY_RIGHT)) {
        if (app->selected_object < app->scene.primitive_count - 1) app->selected_object++;
        else app->selected_object = 6;
    }

    if (IsKeyPressed(KEY_M) && app->selected_object >= 6) {
        RT_Primitive* prim = &app->scene.primitives[app->selected_object];
        prim->material.is_mirror = !prim->material.is_mirror;
        if (prim->material.is_mirror) {
            prim->material.reflectivity = 0.8f;
        }
        app->needs_render = true;
    }

    if (IsKeyPressed(KEY_T) && app->selected_object >= 6) {
        RT_Primitive* prim = &app->scene.primitives[app->selected_object];
        prim->material.is_transparent = !prim->material.is_transparent;
        if (prim->material.is_transparent) {
            prim->material.transparency = 0.7f;
            prim->material.refraction_index = 1.5f;
        }
        app->needs_render = true;
    }
}

void draw_ui(AppState* app) {
    int ui_x = WINDOW_WIDTH - UI_PANEL_WIDTH + 10;
    int y = 10;

    DrawRectangle(WINDOW_WIDTH - UI_PANEL_WIDTH, 0, UI_PANEL_WIDTH, WINDOW_HEIGHT, (Color){30, 30, 40, 255});
    DrawLine(WINDOW_WIDTH - UI_PANEL_WIDTH, 0, WINDOW_WIDTH - UI_PANEL_WIDTH, WINDOW_HEIGHT, GRAY);

    DrawTextRu("ОБЪЕКТЫ", ui_x, y, 14, YELLOW);
    y += 18;

    DrawTextRu("Выбранный объект:", ui_x, y, 12, WHITE);
    y += 16;

    for (int i = 6; i < app->scene.primitive_count; i++) {
        RT_Primitive* prim = &app->scene.primitives[i];
        Color btn_color = (i == app->selected_object) ? (Color){80, 80, 120, 255} : (Color){50, 50, 70, 255};

        Rectangle btn = {ui_x, y, UI_PANEL_WIDTH - 30, 18};
        DrawRectangleRec(btn, btn_color);
        DrawRectangleLinesEx(btn, 1, (i == app->selected_object) ? YELLOW : GRAY);

        char label[64];
        const char* type_str = (prim->type == PRIMITIVE_SPHERE) ? "[Сфера]" : "[Куб]";
        snprintf(label, sizeof(label), "%s %s", object_names[i], type_str);
        DrawTextRu(label, ui_x + 5, y + 2, 11, WHITE);

        if (CheckCollisionPointRec(GetMousePosition(), btn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            app->selected_object = i;
        }
        y += 20;
    }
    y += 5;

    if (app->selected_object >= 6 && app->selected_object < app->scene.primitive_count) {
        RT_Primitive* prim = &app->scene.primitives[app->selected_object];

        DrawTextRu("Свойства:", ui_x, y, 11, WHITE);
        y += 16;

        bool mirror = prim->material.is_mirror;
        if (GuiCheckBox((Rectangle){ui_x, y, 14, 14}, "Зеркальный", &mirror)) {
            prim->material.is_mirror = mirror;
            if (mirror) prim->material.reflectivity = 0.8f;
            app->needs_render = true;
        }
        y += 18;

        if (prim->material.is_mirror) {
            DrawTextRu("Отражение:", ui_x + 15, y, 10, LIGHTGRAY);
            float refl = prim->material.reflectivity;
            if (GuiSlider((Rectangle){ui_x + 85, y, 100, 14}, NULL, NULL, &refl, 0, 1)) {
                prim->material.reflectivity = refl;
                app->needs_render = true;
            }
            char refl_text[32];
            snprintf(refl_text, sizeof(refl_text), "%.2f", refl);
            DrawTextRu(refl_text, ui_x + 190, y, 10, WHITE);
            y += 18;
        }

        bool transp = prim->material.is_transparent;
        if (GuiCheckBox((Rectangle){ui_x, y, 14, 14}, "Прозрачный", &transp)) {
            prim->material.is_transparent = transp;
            if (transp) {
                prim->material.transparency = 0.7f;
                prim->material.refraction_index = 1.5f;
            }
            app->needs_render = true;
        }
        y += 18;

        if (prim->material.is_transparent) {
            DrawTextRu("Прозрачность:", ui_x + 15, y, 10, LIGHTGRAY);
            float tr = prim->material.transparency;
            if (GuiSlider((Rectangle){ui_x + 95, y, 90, 14}, NULL, NULL, &tr, 0, 1)) {
                prim->material.transparency = tr;
                app->needs_render = true;
            }
            char tr_text[32];
            snprintf(tr_text, sizeof(tr_text), "%.2f", tr);
            DrawTextRu(tr_text, ui_x + 190, y, 10, WHITE);
            y += 18;

            DrawTextRu("Преломление:", ui_x + 15, y, 10, LIGHTGRAY);
            float ior = prim->material.refraction_index;
            if (GuiSlider((Rectangle){ui_x + 95, y, 90, 14}, NULL, NULL, &ior, 1.0f, 2.5f)) {
                prim->material.refraction_index = ior;
                app->needs_render = true;
            }
            char ior_text[32];
            snprintf(ior_text, sizeof(ior_text), "%.2f", ior);
            DrawTextRu(ior_text, ui_x + 190, y, 10, WHITE);
            y += 18;
        }
    }
    y += 5;

    DrawLine(ui_x - 5, y, WINDOW_WIDTH - 10, y, GRAY);
    y += 8;

    DrawTextRu("ЗЕРКАЛЬНЫЕ СТЕНЫ", ui_x, y, 14, YELLOW);
    y += 18;

    for (int i = 0; i < 6; i++) {
        bool mirror = app->scene.wall_mirror[i];
        char label[64];
        snprintf(label, sizeof(label), "%s", wall_names[i]);
        if (GuiCheckBox((Rectangle){ui_x, y, 14, 14}, label, &mirror)) {
            app->scene.wall_mirror[i] = mirror;
            app->needs_render = true;
        }
        y += 18;
    }
    y += 5;

    DrawLine(ui_x - 5, y, WINDOW_WIDTH - 10, y, GRAY);
    y += 8;

    DrawTextRu("ОСВЕЩЕНИЕ", ui_x, y, 14, YELLOW);
    y += 18;

    for (int i = 0; i < app->scene.light_count; i++) {
        RT_Light* light = &app->scene.lights[i];

        Color btn_color = (i == app->selected_light) ? (Color){80, 80, 120, 255} : (Color){50, 50, 70, 255};
        Rectangle btn = {ui_x, y, UI_PANEL_WIDTH - 30, 36};
        DrawRectangleRec(btn, btn_color);
        DrawRectangleLinesEx(btn, 1, (i == app->selected_light) ? YELLOW : GRAY);

        char label[64];
        snprintf(label, sizeof(label), "Источник %d %s", i + 1, i == 0 ? "(основной)" : "(доп.)");
        DrawTextRu(label, ui_x + 5, y + 3, 11, WHITE);

        char pos_text[64];
        snprintf(pos_text, sizeof(pos_text), "Позиция: (%.1f, %.1f, %.1f)",
                 light->position.x, light->position.y, light->position.z);
        DrawTextRu(pos_text, ui_x + 5, y + 16, 9, LIGHTGRAY);

        bool enabled = light->enabled;
        if (GuiCheckBox((Rectangle){ui_x + UI_PANEL_WIDTH - 55, y + 10, 14, 14}, "", &enabled)) {
            light->enabled = enabled;
            app->needs_render = true;
        }

        if (CheckCollisionPointRec(GetMousePosition(), btn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            app->selected_light = i;
        }
        y += 40;
    }

    DrawTextRu("WASD/QE - двигать свет", ui_x, y, 10, GRAY);
    y += 12;
    DrawTextRu("TAB - переключить источник", ui_x, y, 10, GRAY);
    y += 15;

    DrawLine(ui_x - 5, y, WINDOW_WIDTH - 10, y, GRAY);
    y += 8;

    DrawTextRu("ГОРЯЧИЕ КЛАВИШИ", ui_x, y, 14, YELLOW);
    y += 18;

    DrawTextRu("M - зеркальность", ui_x, y, 10, LIGHTGRAY);
    DrawTextRu("T - прозрачность", ui_x + 110, y, 10, LIGHTGRAY);
    y += 14;
    DrawTextRu("<-/-> - выбор объекта", ui_x, y, 10, LIGHTGRAY);
}

void draw_render_view(AppState* app) {
    int render_area_width = WINDOW_WIDTH - UI_PANEL_WIDTH;
    int render_area_height = WINDOW_HEIGHT;

    DrawRectangle(0, 0, render_area_width, render_area_height, (Color){20, 20, 25, 255});

    float scale_x = (float)(render_area_width - 40) / RENDER_WIDTH;
    float scale_y = (float)(render_area_height - 80) / RENDER_HEIGHT;
    float scale = fminf(scale_x, scale_y);

    int display_width = (int)(RENDER_WIDTH * scale);
    int display_height = (int)(RENDER_HEIGHT * scale);
    int display_x = (render_area_width - display_width) / 2;
    int display_y = (render_area_height - display_height) / 2;

    DrawRectangleLines(display_x - 2, display_y - 2, display_width + 4, display_height + 4, GRAY);

    DrawTexturePro(
        app->render_texture,
        (Rectangle){0, 0, RENDER_WIDTH, RENDER_HEIGHT},
        (Rectangle){display_x, display_y, display_width, display_height},
        (Vector2){0, 0},
        0,
        WHITE
    );
}

int main(void) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Cornell Box - Ray Tracing");
    SetTargetFPS(60);

    int codepoints[512];
    int count = 0;
    for (int i = 32; i < 127; i++) codepoints[count++] = i;
    for (int i = 0x0400; i <= 0x04FF; i++) codepoints[count++] = i;

    cyrillic_font = LoadFontEx("fonts/jetbrains-mono.ttf", 32, codepoints, count);
    if (cyrillic_font.texture.id == 0) {
        cyrillic_font = LoadFontEx("/System/Library/Fonts/Supplemental/Arial Unicode.ttf", 32, codepoints, count);
    }
    SetTextureFilter(cyrillic_font.texture, TEXTURE_FILTER_BILINEAR);

    GuiSetFont(cyrillic_font);

    AppState app = {0};
    init_app(&app);

    render_scene(&app);

    while (!WindowShouldClose()) {
        handle_input(&app);

        if (app.needs_render) {
            render_scene(&app);
        }

        BeginDrawing();
        ClearBackground(BLACK);

        draw_render_view(&app);
        draw_ui(&app);

        EndDrawing();
    }

    cleanup_app(&app);
    UnloadFont(cyrillic_font);
    CloseWindow();

    return 0;
}

