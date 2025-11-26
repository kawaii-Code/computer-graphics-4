#include <stdio.h>
#include <assert.h>

#include <glad/glad.h>

#include "module3.h"

void open_window(Program *program, int window_width, int window_height, const char *window_title) {
    glfwSetErrorCallback(glfw_callback_error);

    int init_ok = glfwInit();
    assert(init_ok);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(window_width, window_height, window_title, NULL, NULL);
    assert(window != NULL);

    program->window = window;
    program->window_info.width = window_width;
    program->window_info.height = window_height;

    glfwMakeContextCurrent(window);
    gladLoadGL();
    glfwSwapInterval(1);

    glfwSetWindowUserPointer(window, program);

    glfwSetKeyCallback(window, glfw_callback_key);
    glfwSetMouseButtonCallback(window, glfw_callback_mouse_button);
    glfwSetCursorPosCallback(window, glfw_callback_cursor_pos);
    glfwSetWindowSizeCallback(window, glfw_callback_window_resize);
}

void close_window(Program *program) {
    glfwDestroyWindow(program->window);
    glfwTerminate();
}

void glfw_callback_error(int error_code, const char* description) {
    fprintf(stderr, "GLFW ERROR(%d): %s\n", error_code, description);
}

void glfw_callback_window_resize(GLFWwindow *window, int width, int height) {
    Program *program = (Program *)glfwGetWindowUserPointer(window);

    program->window_info.width = width;
    program->window_info.height = height;
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
    //fprintf(stderr, "%d\n", button);
}

void glfw_callback_cursor_pos(GLFWwindow* window, double xpos, double ypos) {
    //fprintf(stderr, "%f %f\n", xpos, ypos);
}
