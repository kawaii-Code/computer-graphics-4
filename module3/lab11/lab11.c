#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <math.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "module3.h"

#define PI 3.141592653589793

GradientVertex triangle_vertices[] = {
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


void opengl_debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* user_param);
int compile_shader(const char* shader_path);


int main() {
    Program _program = { 0 };
    Program* program = &_program;
    Shaders* shaders = &program->shaders;

    open_window(program, 800, 600, "Модуль 3: OpenGL 🦭");

    //
    // Обращаю внимание, что любые функции OpenGL
    // нужно вызывать после open_window().
    //
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(opengl_debug_message_callback, NULL);

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

    GradientVertex pentagon_vertices[5] = { 0 };
    float angle_step = (2 * PI) / 5;
    for (int i = 0; i < 5; i++) {
        pentagon_vertices[i].position = (Vector2){
            .x = 0.9f * cosf(i * angle_step + PI / 2),
            .y = 0.9f * sinf(i * angle_step + PI / 2),
        };
        pentagon_vertices[i].color.r = cosf(i * angle_step + PI / 2);
        pentagon_vertices[i].color.g = sinf(i * angle_step + PI / 2);
        pentagon_vertices[i].color.b = sinf(i * angle_step + PI / 2);
    }

    glGenBuffers(VERTEX_BUFFERS_COUNT, vertex_buffers);
    {
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[TRIANGLE]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_vertices), triangle_vertices, GL_STATIC_DRAW);

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
        glVertexAttribPointer(shaders->gradient.vertex_position, 2, GL_FLOAT, GL_FALSE, sizeof(GradientVertex), (void*)0);
        glEnableVertexAttribArray(shaders->gradient.vertex_color);
        glVertexAttribPointer(shaders->gradient.vertex_color, 3, GL_FLOAT, GL_FALSE, sizeof(GradientVertex), (void*)(2 * sizeof(float)));
    }

    int mode = MODE_CONSTANT_FLAT_COLOR;
    int figure = FAN;

    ColorRGB flat_color = { 1.0f, 0.0f, 0.0f };

    while (!glfwWindowShouldClose(program->window)) {
        float time = glfwGetTime();

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
            flat_color = (ColorRGB){ 1.0f, 0.0f, 0.0f };
        }
        if (program->keys[GLFW_KEY_X].pressed_this_frame) {
            flat_color = (ColorRGB){ 0.0f, 1.0f, 0.0f };
        }
        if (program->keys[GLFW_KEY_C].pressed_this_frame) {
            flat_color = (ColorRGB){ 1.0f, 1.0f, 1.0f };
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

        glViewport(0, 0, program->window_info.width, program->window_info.height);

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

        glfwSwapBuffers(program->window);
        for (int i = 0; i < GLFW_KEY_LAST; i++) {
            program->keys[i].pressed_this_frame = false;
        }
        glfwPollEvents();
    }

    close_window(program);
}

// Передавайте сюда имя файла БЕЗ расширения.
// Сами шейдеры должны быть с расширениями .vs для вершинного, .fs для фрагментного.
// Ну, не дети уже, сами разберетесь 🏅
int compile_shader(const char* shader_path) {
    char log[256];
    int log_length;

    char vertex_shader_path[128];
    char fragment_shader_path[128];
    snprintf(vertex_shader_path, ARRAY_LEN(vertex_shader_path), "%s.vs", shader_path);
    snprintf(fragment_shader_path, ARRAY_LEN(fragment_shader_path), "%s.fs", shader_path);

    char* vertex_shader_source = read_entire_file(vertex_shader_path);
    char* fragment_shader_source = read_entire_file(fragment_shader_path);
    assert(vertex_shader_source != NULL);
    assert(fragment_shader_source != NULL);

    int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, (const char* const*)&vertex_shader_source, NULL);
    glCompileShader(vertex_shader);

    int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, (const char* const*)&fragment_shader_source, NULL);
    glCompileShader(fragment_shader);

    int shader = glCreateProgram();
    glAttachShader(shader, vertex_shader);
    glAttachShader(shader, fragment_shader);
    glLinkProgram(shader);

    int link_ok;
    glGetProgramiv(shader, GL_LINK_STATUS, &link_ok);
    if (!link_ok) {
        char log[256];
        int length;
        glGetProgramInfoLog(shader, sizeof(log), &length, log);
        log[length] = '\0';
        fprintf(stderr, "GLSL link %s", log);
        assert(false);
    }

    free(vertex_shader_source);
    free(fragment_shader_source);

    return shader;
}

void opengl_debug_message_callback(
    GLenum source, GLenum type, GLuint id,
    GLenum severity, GLsizei length, const GLchar* message,
    const void* user_param
) {
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
        return;
    }

    fprintf(stderr, "OpenGL ");
    if (type == GL_DEBUG_TYPE_ERROR) {
        fprintf(stderr, "error: ");
    }
    else {
        fprintf(stderr, "warning: ");
    }

    fprintf(stderr, "%s\n", message);

    if (type == GL_DEBUG_TYPE_ERROR) {
        assert(false); // Это база.
    }
};
