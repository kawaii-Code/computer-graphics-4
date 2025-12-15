#ifndef MODULE_3_H_
#define MODULE_3_H_

#include <stdbool.h>

#include <GLFW/glfw3.h>

#include "linalg.h"

#define ARRAY_LEN(a) (sizeof(a) / sizeof(*(a)))

enum {
    MODE_CONSTANT_FLAT_COLOR = 0,
    MODE_UNIFORM_FLAT_COLOR,
    MODE_GRADIENT,
    MODE_TEXTURED,
    MODE_MIX_TEXTURED,
    MODE_SOLAR_SYSTEM,
    MODE_MULTIPLE_MODELS,
    MODE_LIT_SCENE,
};

enum {
    CUBE = 0,
    TETRAHEDRON,
    CIRCLE,
    TEXTURED_CUBE,
    VERTEX_BUFFERS_COUNT,
};

enum {
    CUBE_VAO = 0,
    TETRAHEDRON_VAO,
    CIRCLE_VAO,
    TEXTURED_CUBE_VAO,
    VAOS_COUNT,
};

typedef struct {
    float u;
    float v;
} Texture;

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
    int world;
    int view_proj;
    int view;
    int proj;
    int time;
} GradientShader;

typedef struct {
    int id;
    int vertex_position;
    int vertex_color;
    int vertex_tex;
    int view;
    int proj;
    int world;
    int texture;
    int color_boost;
} TexturedShader;

typedef struct {
    int id;
    int vertex_position;
    int vertex_color;
    int vertex_tex;
    int texture1;
    int texture2;
    int mix_ratio;
    int view;
    int proj;
    int world;
} MixTexturedShader;

typedef struct {
    int id;
    int vertex_position;
    int vertex_tex;
    int texture;
    int view;
    int proj;
    int world;
} OBJTexturedShader;

typedef struct {
    int id;
    int vertex_position;
    int vertex_tex;
    int texture;
    int view;
    int proj;
} OBJInstancedShader;

// Шейдер для отображения источников света
typedef struct {
    int id;
    int vertex_position;
    int world;
    int view;
    int proj;
    int lightColor;
} LightSourceShader;

// Шейдер для освещённых объектов (поддерживает instancing)
typedef struct {
    int id;
    int view;
    int proj;
    int viewPos;
    int texture;
    int ambientColor;
    int lightingModel;

    // Point Light
    int pointLightEnabled;
    int pointLightPos;
    int pointLightColor;
    int pointLightIntensity;
    int pointLightConstant;
    int pointLightLinear;
    int pointLightQuadratic;

    // Directional Light
    int dirLightEnabled;
    int dirLightDirection;
    int dirLightColor;
    int dirLightIntensity;

    // Spot Light
    int spotLightEnabled;
    int spotLightPos;
    int spotLightDirection;
    int spotLightColor;
    int spotLightIntensity;
    int spotLightCutOff;
    int spotLightOuterCutOff;
    int spotLightConstant;
    int spotLightLinear;
    int spotLightQuadratic;
} LitInstancedShader;

typedef struct {
    FlatColorShader flat_color;
    UniformColorShader uniform_flat_color;
    GradientShader  gradient;
    TexturedShader textured;
    MixTexturedShader mix_textured;
    OBJTexturedShader obj_textured;
    OBJInstancedShader obj_instanced;
    LightSourceShader light_source;
    LitInstancedShader lit_instanced;
} Shaders;

typedef struct {
    Vector3 position;

    float pitch;
    float yaw;

    float field_of_view;
    float near;
    float far;
    float aspect;

    // Вручную не менять! Заполняется в camera_update().
    Vector3 forward;
    Vector3 right;
    Vector3 up;
} Camera;

typedef struct {
    Vector2 position;
    Vector2 move;
    Keyboard_Key right;
    Keyboard_Key left;
} Mouse;

typedef struct {
    GLFWwindow *window;
    Window_Info window_info;
    Shaders shaders;

    Mouse mouse;
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

// Функции для работы с камерой
void camera_update(Camera *camera);
Vector3 direction_from_pitch_yaw(float pitch, float yaw);
float clamp(float x, float low, float high);

// Callback для OpenGL debug
void opengl_debug_message_callback(GLenum source, GLenum type, GLuint id,
                                   GLenum severity, GLsizei length,
                                   const GLchar *message, const void *user_param);

#endif // MODULE_3_H_
