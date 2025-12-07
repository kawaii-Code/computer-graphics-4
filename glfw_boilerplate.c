#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <glad/glad.h>

#include "module3.h"

void open_window(Program *program, int window_width, int window_height, const char *window_title) {
    glfwSetErrorCallback(glfw_callback_error);

    int init_ok = glfwInit();
    assert(init_ok);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    GLFWwindow *window = glfwCreateWindow(window_width, window_height, window_title, NULL, NULL);
    assert(window != NULL);

    program->window = window;
    program->window_info.width = window_width;
    program->window_info.height = window_height;

    glfwMakeContextCurrent(window);
    int glad_loaded = gladLoadGL();
    if (!glad_loaded) {
        fprintf(stderr, "ОШИБКА: Не удалось загрузить функции OpenGL через GLAD\n");
        glfwTerminate();
        assert(false);
    }

    // Это включает VSync. Для максимума FPS-ов и прогрева процессора замените 1 на 0.
    glfwSwapInterval(1);

    // Устанавливаем начальный viewport
    // Используем framebuffer size для корректной работы на Retina дисплеях
    int fb_width, fb_height;
    glfwGetFramebufferSize(window, &fb_width, &fb_height);
    glViewport(0, 0, fb_width, fb_height);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetWindowUserPointer(window, program);

    // Инициализируем позицию мыши, чтобы избежать скачка при первом движении
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    program->mouse.position.x = (float)xpos;
    program->mouse.position.y = (float)ypos;
    program->mouse.move.x = 0.0f;
    program->mouse.move.y = 0.0f;

    glfwSetKeyCallback(window, glfw_callback_key);
    glfwSetMouseButtonCallback(window, glfw_callback_mouse_button);
    glfwSetCursorPosCallback(window, glfw_callback_cursor_pos);
    glfwSetWindowSizeCallback(window, glfw_callback_window_resize);
}

void close_window(Program *program) {
    glfwTerminate();
}

void glfw_callback_error(int error_code, const char* description) {
    fprintf(stderr, "GLFW ERROR(%d): %s\n", error_code, description);
}

void glfw_callback_window_resize(GLFWwindow *window, int width, int height) {
    Program *program = (Program *)glfwGetWindowUserPointer(window);

    program->window_info.width = width;
    program->window_info.height = height;

    // Обновляем viewport при изменении размера окна
    // Используем framebuffer size, который может отличаться на Retina дисплеях
    int fb_width, fb_height;
    glfwGetFramebufferSize(window, &fb_width, &fb_height);
    glViewport(0, 0, fb_width, fb_height);
}

void glfw_callback_key(GLFWwindow *window, int key, int scancode, int action, int mods) {
    Program *program = (Program *)glfwGetWindowUserPointer(window);

    if (action == GLFW_PRESS) {
        program->keys[key].pressed_this_frame = true;
        program->keys[key].pressed = true;
    } else if (action == GLFW_RELEASE) {
        program->keys[key].pressed = false;
    }
}

void glfw_callback_mouse_button(GLFWwindow* window, int button, int action, int mods) {
    Program *program = (Program *)glfwGetWindowUserPointer(window);

    Keyboard_Key *btn = (button == GLFW_MOUSE_BUTTON_LEFT) ? &program->mouse.left : &program->mouse.right;
    if (action == GLFW_PRESS) {
        btn->pressed_this_frame = true;
        btn->pressed = true;
    } else if (action == GLFW_RELEASE) {
        btn->pressed = false;
    }
}

void glfw_callback_cursor_pos(GLFWwindow* window, double xpos, double ypos) {
    Program *program = (Program *)glfwGetWindowUserPointer(window);

    Mouse *mouse = &program->mouse;

    mouse->move.x = xpos - mouse->position.x;
    mouse->move.y = -1 * (ypos - mouse->position.y);

    mouse->position.x = xpos;
    mouse->position.y = ypos;
}

char *read_entire_file(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Не удалось открыть файл: %s\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = malloc(size + 1);
    fread(buffer, 1, size, file);
    buffer[size] = '\0';

    fclose(file);
    return buffer;
}

