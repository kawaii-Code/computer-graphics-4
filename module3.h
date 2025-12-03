#ifndef MODULE_3_H_
#define MODULE_3_H_

#include <stdbool.h>

#include <GLFW/glfw3.h>


#define ARRAY_LEN(a) (sizeof(a) / sizeof(*(a)))

#define PI 3.141592653589793

enum {
    MODE_CONSTANT_FLAT_COLOR = 0,
    MODE_UNIFORM_FLAT_COLOR,
    MODE_GRADIENT,
    MODE_TEXTURED,
};

enum {
    CUBE = 0,
    TETRAHEDRON,
    CIRCLE,
    TEXTURED_CUBE,
    TWO_TEXTURED_CUBE,
    VERTEX_BUFFERS_COUNT,
};

enum {
    CUBE_VAO = 0,
    TETRAHEDRON_VAO,
    CIRCLE_VAO,
    TEXTURED_CUBE_VAO,
    TWO_TEXTURED_CUBE_VAO,
    VAOS_COUNT,
};

typedef struct {
    float x;
    float y;
} Vector2;

typedef struct {
    float x;
    float y;
    float z;
} Vector3;

typedef struct {
    float u;
    float v;
}Texture;

typedef struct {
    float r;
    float g;
    float b;
} ColorRGB;

typedef struct {
    Vector2  position;
    ColorRGB color;
} GradientVertex;

typedef struct {
    Vector3  position;
    ColorRGB color;
} GradientVertex3D;

typedef struct {
    Vector3 position;    
    Vector3 color;      
    Texture tex_coord;   
} TexturedVertex3D;

typedef struct {
    Vector2 center;
    int width;
    int height;
} Window_Info;

typedef struct {
    bool pressed_this_frame;
    bool pressed;
} Keyboard_Key;

typedef struct {
    int id;
    int vertex_position;
    int color;
} UniformColorShader;

typedef struct {
    int id;
    int vertex_position;
} FlatColorShader;

typedef struct {
    int id;
    int vertex_position;
    int vertex_color;
    float rotation_x;  
    float rotation_y;  
    float rotation_z;
    float position;
    float zoom;
    float scale;
    int time;
} GradientShader;

typedef struct {
    int id;
    int vertex_position;
    int vertex_color;
    int vertex_tex;
    float rotation_x;
    float rotation_y;
    float rotation_z;
    float position;
    float scale;
    int texture;
    int color_boost;
}TexturedShader;

typedef struct {
    FlatColorShader flat_color;
    UniformColorShader uniform_flat_color;
    GradientShader  gradient;
    TexturedShader textured;
} Shaders;

typedef struct {
    GLFWwindow *window;
    Window_Info window_info;
    Shaders shaders;

    Vector2 mouse_position;
    Keyboard_Key keys[GLFW_KEY_LAST];
} Program;

void open_window(Program *program, int window_width, int window_height, const char *window_title);
void close_window(Program *program);

void glfw_callback_error(int error_code, const char* description);
void glfw_callback_window_resize(GLFWwindow *window, int width, int height);
void glfw_callback_key(GLFWwindow *window, int key, int scancode, int action, int mods);
void glfw_callback_mouse_button(GLFWwindow* window, int button, int action, int mods);
void glfw_callback_cursor_pos(GLFWwindow* window, double xpos, double ypos);

char *read_entire_file(const char *filename);

#endif // MODULE_3_H_
