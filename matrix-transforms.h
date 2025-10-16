#include "common.h"
#include "polygon.h"
#include "vector.h"

typedef struct {
    Rectangle bounding_box;
    int pad_x;
    int pad_y;

    int x;
} Row_Layout;

typedef union {
    struct {
        float m00, m01, m02;
        float m10, m11, m12;
        float m20, m21, m22;
    };
    float m[9];
} Matrix3x3;

typedef struct {
    const char *name;
    float min;
    float max;
} Number_Input_Field_Description;

typedef struct {
    char buf[16];
    int  len;
} Number_Input_Field_State;

typedef enum {
    TRANSFORM_MOVE = 0,
    TRANSFORM_ROTATE_AROUND_POINT,
    TRANSFORM_ROTATE,
    TRANSFORM_SCALE_AROUND_POINT,
    TRANSFORM_SCALE,
    TRANSFORM_TRIANGULATE,
    TRANSFORM_COUNT,
} Transform_Type;


extern Texture tool_textures[TRANSFORM_COUNT];
extern Number_Input_Field_Description number_input_fields[TRANSFORM_COUNT];
extern Transform_Type current_tool;


void init_matrix_transforms();
void number_input_field(int index, Number_Input_Field_State *input, Number_Input_Field_Description description, Row_Layout *layout);
void      row_skip(Row_Layout *layout, int amount);
int       row_height(Row_Layout *layout);
Rectangle row_make_square(Row_Layout *layout);
Rectangle row_make_rect(Row_Layout *layout, int width);
bool tool_button(Transform_Type tool, Row_Layout *layout);
void draw_and_read_edits(VECTOR_TYPE(Polygon)* polygons, int i, VECTOR_TYPE(Diagonal)* diagonals);
