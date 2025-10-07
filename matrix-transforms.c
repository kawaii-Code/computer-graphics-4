#include "vector.h"
#include "matrix-transforms.h"

Texture        tool_textures[TRANSFORM_COUNT];
Transform_Type current_tool = TRANSFORM_ROTATE;

Number_Input_Field_Description *input_field_descriptions[TRANSFORM_COUNT];
Number_Input_Field_State       *input_field_states[TRANSFORM_COUNT];
int                             input_field_count[TRANSFORM_COUNT];

int focused_input_field = 0;

static int tool_button_size = 64;

Matrix3x3 matrix_identity() {
    Matrix3x3 result = {0};
    result.m00 = 1;
    result.m11 = 1;
    result.m22 = 1;
    return result;
}

Matrix3x3 move_transform(float dx, float dy) {
    Matrix3x3 result = matrix_identity();
    result.m20 = dx;
    result.m21 = dy;
    return result;
}

Matrix3x3 rotate_transform(float phi) {
    Matrix3x3 result = matrix_identity();
    phi *= (2 * M_PI) / 360.0f;
    result.m00 = cosf(phi);
    result.m01 = sinf(phi);
    result.m10 = -sinf(phi);
    result.m11 = cosf(phi);
    return result;
}

Matrix3x3 matrix_multiply_by_matrix(Matrix3x3 a, Matrix3x3 b) {
    Matrix3x3 result = {0};
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                result.m[i*3 + j] += a.m[i*3 + k] * b.m[k*3 + j];
            }
        }
    }
    return result;
}

Matrix3x3 scale_transform(float amount) {
    Matrix3x3 result = matrix_identity();
    result.m00 = 1.0f / amount;
    result.m11 = 1.0f / amount;
    return result;
}

Matrix3x3 scale_around_transform(float x, float y, float amount) {
    Matrix3x3 result = matrix_identity();

    Matrix3x3 t0 = move_transform(-x, -y);
    Matrix3x3 scale = scale_transform(amount);
    Matrix3x3 t1 = move_transform(x, y);

    result = matrix_multiply_by_matrix(result, t0);
    result = matrix_multiply_by_matrix(result, scale);
    result = matrix_multiply_by_matrix(result, t1);

    return result;
}

Matrix3x3 rotate_around_transform(float x, float y, float phi) {
    Matrix3x3 result = matrix_identity();

    Matrix3x3 t0 = move_transform(-x, -y);
    Matrix3x3 rotate = rotate_transform(phi);
    Matrix3x3 t1 = move_transform(x, y);

    result = matrix_multiply_by_matrix(result, t0);
    result = matrix_multiply_by_matrix(result, rotate);
    result = matrix_multiply_by_matrix(result, t1);

    return result;
}


void matrix_multiply(Matrix3x3 m, float *a, float *b) {
    b[0] = a[0] * m.m[0] + a[1] * m.m[3] + a[2] * m.m[6];
    b[1] = a[0] * m.m[1] + a[1] * m.m[4] + a[2] * m.m[7];
    b[2] = a[0] * m.m[2] + a[1] * m.m[5] + a[2] * m.m[8];
}

void print_mat(Matrix3x3 m) {
    printf("[(%.2f, %.2f, %.2f), (%.2f, %.2f, %.2f), (%.2f, %.2f, %.2f)]", m.m[0], m.m[1], m.m[2], m.m[3], m.m[4], m.m[5], m.m[6], m.m[7], m.m[8]);
}

void print_vec(float v[3]) {
    printf("(%.2f, %.2f, %.2f)", v[0], v[1], v[2]);
}

void init_matrix_transforms() {
    static Number_Input_Field_Description move[] = {
        { .name = "dx", .min = -100, .max = 100 },
        { .name = "dy", .min = -100, .max = 100 },
    };
    static Number_Input_Field_Description rotate_around[] = {
        { .name = "x", .min = 0, .max = 800 },
        { .name = "y", .min = 0, .max = 800 },
        { .name = "phi", .min = -360, .max = 360 },
    };
    static Number_Input_Field_Description rotate[] = {
        { .name = "phi", .min = -360, .max = 360 },
    };
    static Number_Input_Field_Description scale_around[] = {
        { .name = "x", .min = 0, .max = 800 },
        { .name = "y", .min = 0, .max = 800 },
        { .name = "s", .min = -5, .max = 5 },
    };
    static Number_Input_Field_Description scale[] = {
        { .name = "s", .min = -5, .max = 5 },
    };

    static Number_Input_Field_State state_move[ARRAY_LEN(move)] = {0};
    static Number_Input_Field_State state_rotate_around[ARRAY_LEN(rotate_around)] = {0};
    static Number_Input_Field_State state_rotate[ARRAY_LEN(rotate)] = {0};
    static Number_Input_Field_State state_scale_around[ARRAY_LEN(scale_around)] = {0};
    static Number_Input_Field_State state_scale[ARRAY_LEN(scale)] = {0};

    input_field_descriptions[TRANSFORM_MOVE] = move;
    input_field_descriptions[TRANSFORM_ROTATE_AROUND_POINT] = rotate_around;
    input_field_descriptions[TRANSFORM_ROTATE] = rotate;
    input_field_descriptions[TRANSFORM_SCALE_AROUND_POINT] = scale_around;
    input_field_descriptions[TRANSFORM_SCALE] = scale;

    input_field_states[TRANSFORM_MOVE] = state_move;
    input_field_states[TRANSFORM_ROTATE_AROUND_POINT] = state_rotate_around;
    input_field_states[TRANSFORM_ROTATE] = state_rotate;
    input_field_states[TRANSFORM_SCALE_AROUND_POINT] = state_scale_around;
    input_field_states[TRANSFORM_SCALE] = state_scale;

    input_field_count[TRANSFORM_MOVE] = ARRAY_LEN(move);
    input_field_count[TRANSFORM_ROTATE_AROUND_POINT] = ARRAY_LEN(rotate_around);
    input_field_count[TRANSFORM_ROTATE] = ARRAY_LEN(rotate);
    input_field_count[TRANSFORM_SCALE_AROUND_POINT] = ARRAY_LEN(scale_around);
    input_field_count[TRANSFORM_SCALE] = ARRAY_LEN(scale);
}

void draw_and_read_edits(Polygon polygon) {
    Window_Info window = get_window_info();

    Row_Layout layout = {
        .bounding_box = {
            .x = 100,
            .y = 0,
            .width = window.width - 100,
            // Копипаста, если не забуду всё переделать, обязательно это уберу
            .height = task_window_height / 8,
        },
        .pad_x = 8,
        .pad_y = 8,
    };

    for (int i = 0; i < TRANSFORM_COUNT; i++) {
        if (tool_button(i, &layout)) {
            focused_input_field = 0;
            current_tool = i;
        }
    }

    int key = 0;
    while ((key = GetCharPressed()) != 0) {
        if (('0' <= key && key <= '9') || key == '-' || key == '+') {
            Number_Input_Field_State *input_field = &input_field_states[current_tool][focused_input_field];
            Number_Input_Field_Description *description = &input_field_descriptions[current_tool][focused_input_field];
            if (input_field->len < ARRAY_LEN(input_field->buf)) {
                input_field->buf[input_field->len] = key;
                input_field->len += 1;

                int result = atoi(input_field->buf);
                if (result < description->min || result > description->max) {
                    input_field->buf[input_field->len - 1] = '\0';
                    input_field->len -= 1;
                }
            }
        }
    }
    if (IsKeyPressed(KEY_BACKSPACE)) {
        Number_Input_Field_State *input_field = &input_field_states[current_tool][focused_input_field];
        if (input_field->len > 0) {
            input_field->buf[input_field->len - 1] = '\0';
            input_field->len -= 1;
        }
    }
    if (IsKeyPressed(KEY_TAB)) {
        focused_input_field = (focused_input_field + 1) % input_field_count[current_tool];
    }

    if (IsKeyPressed(KEY_ENTER)) {
        Matrix3x3 transform = matrix_identity();

        switch (current_tool) {

        case TRANSFORM_MOVE: {
            int dx = atoi(input_field_states[TRANSFORM_MOVE][0].buf);
            int dy = atoi(input_field_states[TRANSFORM_MOVE][1].buf);
            transform = move_transform(dx, dy);
        } break;

        case TRANSFORM_ROTATE_AROUND_POINT: {
            int x = atoi(input_field_states[TRANSFORM_ROTATE_AROUND_POINT][0].buf);
            int y = atoi(input_field_states[TRANSFORM_ROTATE_AROUND_POINT][1].buf);
            int phi = atoi(input_field_states[TRANSFORM_ROTATE_AROUND_POINT][2].buf);
            transform = rotate_around_transform(x, y, phi);
        } break;

        case TRANSFORM_ROTATE: {
            int phi = atoi(input_field_states[TRANSFORM_ROTATE][0].buf);

            float midpoint_x = 0.0f;
            float midpoint_y = 0.0f;
            for (int i = 0; i < polygon.vertices.len; i++) {
                Point p = vector_get(polygon.vertices, i);
                midpoint_x += p.x;
                midpoint_y += p.y;
            }
            midpoint_x /= (float)polygon.vertices.len;
            midpoint_y /= (float)polygon.vertices.len;

            transform = rotate_around_transform(midpoint_x, midpoint_y, phi);
        } break;

        case TRANSFORM_SCALE_AROUND_POINT: {
            int x = atoi(input_field_states[TRANSFORM_SCALE_AROUND_POINT][0].buf);
            int y = atoi(input_field_states[TRANSFORM_SCALE_AROUND_POINT][1].buf);
            int amount = atoi(input_field_states[TRANSFORM_SCALE_AROUND_POINT][2].buf);
            if (amount != 0) {
                transform = scale_around_transform(x, y, amount);
            }
        } break;

        case TRANSFORM_SCALE: {
            int amount = atoi(input_field_states[TRANSFORM_SCALE][0].buf);

            float midpoint_x = 0.0f;
            float midpoint_y = 0.0f;
            for (int i = 0; i < polygon.vertices.len; i++) {
                Point p = vector_get(polygon.vertices, i);
                midpoint_x += p.x;
                midpoint_y += p.y;
            }
            midpoint_x /= (float)polygon.vertices.len;
            midpoint_y /= (float)polygon.vertices.len;

            if (amount != 0) {
                transform = scale_around_transform(midpoint_x, midpoint_y, amount);
            }
        } break;

        }

        for (int i = 0; i < polygon.vertices.len; i++) {
            Point *p = vector_get_ptr(polygon.vertices, i);
            float a[3] = { p->x, p->y, 1 };
            float b[3] = {0};
            matrix_multiply(transform, a, b);
            p->x = b[0];
            p->y = b[1];
        }
    }

    for (int i = 0; i < input_field_count[current_tool]; i++) {
        Number_Input_Field_Description description = input_field_descriptions[current_tool][i];
        Number_Input_Field_State *state = &input_field_states[current_tool][i];
        row_skip(&layout, 0.7f * row_height(&layout));
        number_input_field(i, state, description, &layout);
    }
}

void row_skip(Row_Layout *layout, int amount) {
    layout->x += amount;
}

int row_height(Row_Layout *layout) {
    return layout->bounding_box.height - 2 * layout->pad_y;
}

Rectangle row_make_rect(Row_Layout *layout, int width) {
    int height = row_height(layout);

    Rectangle result = {
        .x = layout->bounding_box.x + layout->x,
        .y = layout->bounding_box.y + layout->pad_y,
        .width = width,
        .height = height,
    };

    layout->x += result.width + layout->pad_x;

    return result;
}

Rectangle row_make_square(Row_Layout *layout) {
    int height = row_height(layout);
    return row_make_rect(layout, height);
}

void number_input_field(int index, Number_Input_Field_State *state, Number_Input_Field_Description description, Row_Layout *layout) {
    Rectangle field_rect = row_make_rect(layout, 2 * row_height(layout));
    Rectangle inside_rect = shrink_rect(field_rect, 6);

    bool hovered = CheckCollisionPointRec(GetMousePosition(), field_rect);
    Color background_color = (focused_input_field == index) ? RED : hovered ? YELLOW : BLACK;

    DrawRectangleRec(field_rect, background_color);
    DrawRectangleRec(inside_rect, GRAY);

    float description_x = field_rect.x - layout->pad_x - 0.2f * (strlen(description.name) * fonts[FONT_MAIN].baseSize);

    float center_y = field_rect.y + field_rect.height / 2;
    draw_text_centered(fonts[FONT_MAIN], description.name, description_x, center_y, ORANGE);
    draw_text_centered(fonts[FONT_MAIN], state->buf, inside_rect.x + inside_rect.width / 2, center_y, WHITE);

    if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        focused_input_field = index;
    }
}

bool tool_button(Transform_Type tool, Row_Layout *layout) {
    Texture2D texture = tool_textures[tool];
    Rectangle texture_rect = {
        .x = 0,
        .y = 0,
        .width = texture.width,
        .height = texture.height,
    };
    Rectangle button_rect = row_make_square(layout);

    Rectangle inside_rect = shrink_rect(button_rect, 6);

    bool hovered = CheckCollisionPointRec(GetMousePosition(), button_rect);
    Color background_color = (current_tool == tool) ? RED : hovered ? YELLOW : BLACK;

    DrawRectangleLinesEx(button_rect, 3.0f, background_color);
    DrawTexturePro(texture, texture_rect, inside_rect, (Vector2){0, 0}, 0.0f, WHITE);

    if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        return true;
    }
    return false;
}
