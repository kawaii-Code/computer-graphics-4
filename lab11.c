#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <math.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>


#define ARRAY_LEN(a) (sizeof(a) / sizeof(*(a)))


enum {
    MODE_CONSTANT_FLAT_COLOR = 0,
    MODE_UNIFORM_FLAT_COLOR,
    MODE_GRADIENT,
};

enum {
    TRIANGLE = 0,
    RECTANGLE,
    FAN,
    PENTAGON,
    VERTEX_BUFFERS_COUNT,
};

enum {
    TRIANGLE_VAO = 0,
    RECTANGLE_VAO,
    FAN_VAO,
    PENTAGON_VAO,
    VAOS_COUNT,
};


typedef struct {
    float x;
    float y;
    float z;
    float r;
    float g;
    float b;
} Vertex;

typedef struct {
    float x;
    float y;
} Vector2;

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
    int time;
} GradientShader;

typedef struct {
    FlatColorShader flat_color;
    UniformColorShader uniform_flat_color;
    GradientShader  gradient;
} Shaders;

typedef struct {
    GLFWwindow *window;
    Window_Info window_info;
    Shaders shaders;

    Vector2 mouse_position;
    Keyboard_Key keys[GLFW_KEY_LAST];
} Program;


Vertex triangle_vertices[] = {
    { -1.0f, -0.7f,  0.0f,  1.0f, 0.0f, 0.0f },
    {  0.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f },
    {  1.0f, -0.7f,  0.0f,  0.0f, 0.0f, 1.0f },
};

GradientVertex triangle2d_vertices[] = {
    { { -1.0f, -0.7f }, { 1.0f, 0.0f, 0.0f } },
    { {  0.0f,  0.8f }, { 0.0f, 1.0f, 0.0f } },
    { {  1.0f, -0.7f }, { -.0f, 0.0f, 1.0f } },
};

GradientVertex rectangle_vertices[] = {
    { { -0.9f, -0.7f }, { 0.0f, 0.0f, 0.0f } },
    { { -0.9f,  0.7f }, { 0.0f, 1.0f, 0.0f } },
    { {  0.9f, -0.7f }, { 0.0f, 0.0f, 1.0f } },
    { { -0.9f,  0.7f }, { 0.0f, 1.0f, 0.0f } },
    { {  0.9f,  0.7f }, { 1.0f, 1.0f, 1.0f } },
    { {  0.9f, -0.7f }, { 0.0f, 0.0f, 1.0f } },
};

GradientVertex fan_vertices[] = {
    { {  0.0f, -0.5f }, { 1.0f, 0.0f, 0.0f }},
    { { -0.5f,  0.3f }, { 0.0f, 1.0f, 0.0f }},
    { { -0.2f,  0.7f }, { 0.0f, 0.0f, 1.0f }},
    { {  0.0f,  0.5f }, { 0.0f, 0.0f, 1.0f }},
    { {  0.2f,  0.7f }, { 0.0f, 1.0f, 0.0f }},
    { {  0.5f,  0.3f }, { 1.0f, 0.0f, 0.0f }},
};

int vertex_buffers[VERTEX_BUFFERS_COUNT];
int vaos[VAOS_COUNT];


void glfw_error_callback(int error_code, const char* description) {
    fprintf(stderr, "GLFW ERROR %d: %s\n", error_code, description);
}

void glfw_window_resize_callback(GLFWwindow *window, int width, int height) {
    Program *program = (Program *)glfwGetWindowUserPointer(window);

    program->window_info.width = width;
    program->window_info.height = height;
}

void glfw_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    Program *program = (Program *)glfwGetWindowUserPointer(window);
    if (action == GLFW_PRESS) {
        program->keys[key].pressed_this_frame = true;
        program->keys[key].pressed = true;
    }
    if (action == GLFW_RELEASE) {
        program->keys[key].pressed = false;
    }
}

void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    //fprintf(stderr, "%d\n", button);
}

void glfw_cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    //fprintf(stderr, "%f %f\n", xpos, ypos);
}

char *read_entire_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *result = malloc(file_length);
    size_t bytes_read = fread(result, 1, file_length, file);

    if (bytes_read != file_length) {
        return NULL;
    }
    result[bytes_read] = '\0';

    fclose(file);
    return result;
}

void check_for_opengl_errors() {
    int error_code = glGetError();
    if (error_code == GL_NO_ERROR) {
        return;
    }

    fprintf(stderr, "GL Error: %d\n", error_code);
}

int compile_shader(const char *shader_path) {
    char log[256];
    int log_length;

    char vertex_shader_path[128];
    char fragment_shader_path[128];
    snprintf(vertex_shader_path, ARRAY_LEN(vertex_shader_path), "%s.vs", shader_path);
    snprintf(fragment_shader_path, ARRAY_LEN(fragment_shader_path), "%s.fs", shader_path);

    char *vertex_shader_source = read_entire_file(vertex_shader_path);
    char *fragment_shader_source = read_entire_file(fragment_shader_path);
    assert(vertex_shader_source != NULL);
    assert(fragment_shader_source != NULL);

    int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, (const char * const *)&vertex_shader_source, NULL);
    glCompileShader(vertex_shader);

    glGetShaderInfoLog(vertex_shader, ARRAY_LEN(log), &log_length, log);
    printf("Vertex shader compile log: '%s'\n", log);

    int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, (const char * const *)&fragment_shader_source, NULL);
    glCompileShader(fragment_shader);

    glGetShaderInfoLog(fragment_shader, ARRAY_LEN(log), &log_length, log);
    printf("Fragment shader compile log: '%s'\n", log);

    int shader = glCreateProgram();
    glAttachShader(shader, vertex_shader);
    glAttachShader(shader, fragment_shader);
    glLinkProgram(shader);

    glGetProgramInfoLog(shader, ARRAY_LEN(log), &log_length, log);
    log[log_length] = '\0';
    printf("Shader link log: '%s'\n", log);

    free(vertex_shader_source);
    free(fragment_shader_source);

    return shader;
}

//void GLAPIENTRY opengl_debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *user_param) {
//    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n", (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message );
//}

int main() {
    Program _program = {0};
    Program *program = &_program;
    Shaders *shaders = &program->shaders;

    glfwSetErrorCallback(glfw_error_callback);

    int init_ok = glfwInit();
    assert(init_ok);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(800, 600, "Лаба 10 🦭", NULL, NULL);
    assert(window != NULL);

    program->window = window;

    glfwMakeContextCurrent(window);
    gladLoadGL();
    glfwSwapInterval(1);

    glfwSetWindowUserPointer(window, program);

    glfwSetKeyCallback(window, glfw_key_callback);
    glfwSetMouseButtonCallback(window, glfw_mouse_button_callback);
    glfwSetCursorPosCallback(window, glfw_cursor_pos_callback);
    glfwSetWindowSizeCallback(window, glfw_window_resize_callback);

    {
        int flat_color = compile_shader("shaders/flat_color");
        int uniform_flat_color = compile_shader("shaders/uniform_flat_color");
        int gradient = compile_shader("shaders/gradient");

        shaders->flat_color.id = flat_color;
        shaders->flat_color.vertex_position = glGetAttribLocation(flat_color, "vertex_position");

        shaders->uniform_flat_color.id = uniform_flat_color;
        shaders->uniform_flat_color.vertex_position = glGetAttribLocation(uniform_flat_color, "vertex_position");
        shaders->uniform_flat_color.color = glGetUniformLocation(uniform_flat_color, "color");

        shaders->gradient.id = gradient;
        shaders->gradient.vertex_position = glGetAttribLocation(gradient, "vertex_position");
        shaders->gradient.vertex_color = glGetAttribLocation(gradient, "vertex_color");
        shaders->gradient.time = glGetUniformLocation(gradient, "time");
    }

    GradientVertex pentagon_vertices[5] = {0};
    float angle_step = (2 * M_PI) / 5;
    for (int i = 0; i < 5; i++) {
        pentagon_vertices[i].position = (Vector2) {
            .x = 0.9f * cosf(i * angle_step + M_PI / 2),
            .y = 0.9f * sinf(i * angle_step + M_PI / 2),
        };
        pentagon_vertices[i].color.r = cosf(i * angle_step + M_PI / 2);
        pentagon_vertices[i].color.g = sinf(i * angle_step + M_PI / 2);
        pentagon_vertices[i].color.b = sinf(i * angle_step + M_PI / 2);
    }

    glGenBuffers(VERTEX_BUFFERS_COUNT, vertex_buffers);
    {
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[TRIANGLE]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(triangle2d_vertices), triangle2d_vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[RECTANGLE]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(rectangle_vertices), rectangle_vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[FAN]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(fan_vertices), fan_vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[PENTAGON]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(pentagon_vertices), pentagon_vertices, GL_STATIC_DRAW);
    }

    glGenVertexArrays(VAOS_COUNT, vaos);
    for (int i = 0; i < VAOS_COUNT; i++) {
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[i]);
        glBindVertexArray(vaos[i]);
        glEnableVertexAttribArray(shaders->gradient.vertex_position);
        glVertexAttribPointer(shaders->gradient.vertex_position, 2, GL_FLOAT, GL_FALSE, sizeof(GradientVertex), (void *)0);
        glEnableVertexAttribArray(shaders->gradient.vertex_color);
        glVertexAttribPointer(shaders->gradient.vertex_color, 3, GL_FLOAT, GL_FALSE, sizeof(GradientVertex), (void *)(2 * sizeof(float)));
    }

    int mode = MODE_CONSTANT_FLAT_COLOR;
    int figure = FAN;

    ColorRGB flat_color = { 1.0f, 0.0f, 0.0f };

    while (!glfwWindowShouldClose(window)) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float ratio = width / (float)height;

        if (program->keys[GLFW_KEY_1].pressed_this_frame) {
            mode = MODE_CONSTANT_FLAT_COLOR;
        }
        if (program->keys[GLFW_KEY_2].pressed_this_frame) {
            mode = MODE_UNIFORM_FLAT_COLOR;
        }
        if (program->keys[GLFW_KEY_3].pressed_this_frame) {
            mode = MODE_GRADIENT;
        }

        if (program->keys[GLFW_KEY_Z].pressed_this_frame) {
            flat_color = (ColorRGB) { 1.0f, 0.0f, 0.0f };
        }
        if (program->keys[GLFW_KEY_X].pressed_this_frame) {
            flat_color = (ColorRGB) { 0.0f, 1.0f, 0.0f };
        }
        if (program->keys[GLFW_KEY_C].pressed_this_frame) {
            flat_color = (ColorRGB) { 1.0f, 1.0f, 1.0f };
        }

        if (program->keys[GLFW_KEY_T].pressed_this_frame) {
            figure = TRIANGLE;
        }
        if (program->keys[GLFW_KEY_R].pressed_this_frame) {
            figure = RECTANGLE;
        }
        if (program->keys[GLFW_KEY_F].pressed_this_frame) {
            figure = FAN;
        }
        if (program->keys[GLFW_KEY_P].pressed_this_frame) {
            figure = PENTAGON;
        }

        float time = glfwGetTime();

        glViewport(0, 0, width, height);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        switch (mode) {

        case MODE_CONSTANT_FLAT_COLOR: {
            glUseProgram(shaders->flat_color.id);
        } break;
        case MODE_UNIFORM_FLAT_COLOR: {
            glUseProgram(shaders->uniform_flat_color.id);
            glUniform3f(shaders->uniform_flat_color.color, flat_color.r, flat_color.g, flat_color.b);
        } break;
        case MODE_GRADIENT: {
            glUseProgram(shaders->gradient.id);
            glUniform1f(shaders->gradient.time, time);
        } break;

        }

        switch (figure) {

        case TRIANGLE: {
            glBindVertexArray(vaos[TRIANGLE_VAO]);
            glDrawArrays(GL_TRIANGLES, 0, ARRAY_LEN(triangle_vertices));
        } break;

        case RECTANGLE: {
            glBindVertexArray(vaos[RECTANGLE_VAO]);
            glDrawArrays(GL_TRIANGLES, 0, ARRAY_LEN(rectangle_vertices));
        } break;

        case FAN: {
            glBindVertexArray(vaos[FAN_VAO]);
            glDrawArrays(GL_TRIANGLE_FAN, 0, ARRAY_LEN(fan_vertices));
        } break;

        case PENTAGON: {
            glBindVertexArray(vaos[PENTAGON_VAO]);
            glDrawArrays(GL_TRIANGLE_FAN, 0, ARRAY_LEN(pentagon_vertices));
        } break;

        }

        glfwSwapBuffers(window);
        for (int i = 0; i < GLFW_KEY_LAST; i++) {
            program->keys[i].pressed_this_frame = false;
        }
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}
