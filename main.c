#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <math.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "module3.h"

#define CIRCLE_SEGMENTS 64

const float camera_move_speed = 10.0f;
const float sensitivity = 0.3f;

float global_rotation_x = 0.0f;
float global_rotation_y = 0.0f;
float global_rotation_z = 0.0f;

float pos_x = 0.0f;
float pos_y = 0.0f;
float zoom = 0.0f;

float scale_x = 1.0f;
float scale_y = 1.0f;
float scale_z = 1.0f;

const float rotation_speed = 0.05f;
const float move_speed = 0.05f;
const float scale_speed = 0.05f;

ColorRGB color_boost = { 0.0f, 0.0f, 0.0f };
float boost_step = 0.1f;

float texture_mix = 0.0f; 
float mix_step = 1;

GradientVertex3D cube_vertices[] = {
    { { -0.5, -0.5, +0.5 }, { 0.0f, 0.0f, 0.5f } }, 
    { { -0.5, +0.5, +0.5 }, { 0.0f, 0.5f, 0.0f } }, 
    { { +0.5, +0.5, +0.5 }, { 0.5f, 0.5f, 0.5f } }, 
    { { +0.5, +0.5, +0.5 }, { 0.5f, 0.5f, 0.5f } },
    { { +0.5, -0.5, +0.5 }, { 0.5f, 0.0f, 0.0f } }, 
    { { -0.5, -0.5, +0.5 }, { 0.0f, 0.0f, 0.5f } },

    { { -0.5, -0.5, -0.5 }, { 0.3f, 0.0f, 0.3f } }, 
    { { +0.5, +0.5, -0.5 }, { 0.5f, 0.5f, 0.0f } }, 
    { { -0.5, +0.5, -0.5 }, { 0.3f, 0.3f, 0.0f } }, 
    { { +0.5, +0.5, -0.5 }, { 0.5f, 0.5f, 0.0f } },
    { { -0.5, -0.5, -0.5 }, { 0.3f, 0.0f, 0.3f } },
    { { +0.5, -0.5, -0.5 }, { 0.5f, 0.0f, 0.5f } }, 

    { { -0.5, +0.5, -0.5 }, { 0.0f, 0.5f, 0.5f } },
    { { -0.5, +0.5, +0.5 }, { 0.0f, 1.0f, 1.0f } }, 
    { { +0.5, +0.5, +0.5 }, { 0.8f, 0.8f, 0.8f } }, 
    { { +0.5, +0.5, +0.5 }, { 0.8f, 0.8f, 0.8f } },
    { { +0.5, +0.5, -0.5 }, { 0.5f, 0.5f, 1.0f } }, 
    { { -0.5, +0.5, -0.5 }, { 0.0f, 0.5f, 0.5f } },

    { { -0.5, -0.5, -0.5 }, { 0.3f, 0.2f, 0.1f } }, 
    { { +0.5, -0.5, +0.5 }, { 0.6f, 0.4f, 0.2f } }, 
    { { -0.5, -0.5, +0.5 }, { 0.4f, 0.3f, 0.1f } }, 
    { { +0.5, -0.5, +0.5 }, { 0.6f, 0.4f, 0.2f } },
    { { -0.5, -0.5, -0.5 }, { 0.3f, 0.2f, 0.1f } },
    { { +0.5, -0.5, -0.5 }, { 0.5f, 0.3f, 0.1f } }, 

    { { +0.5, -0.5, -0.5 }, { 0.5f, 0.0f, 0.3f } }, 
    { { +0.5, -0.5, +0.5 }, { 1.0f, 0.0f, 0.5f } },
    { { +0.5, +0.5, +0.5 }, { 1.0f, 0.5f, 0.8f } }, 
    { { +0.5, +0.5, +0.5 }, { 1.0f, 0.5f, 0.8f } },
    { { +0.5, +0.5, -0.5 }, { 0.8f, 0.3f, 0.6f } }, 
    { { +0.5, -0.5, -0.5 }, { 0.5f, 0.0f, 0.3f } },

    { { -0.5, -0.5, -0.5 }, { 0.4f, 0.2f, 0.0f } }, 
    { { -0.5, +0.5, +0.5 }, { 1.0f, 0.6f, 0.0f } }, 
    { { -0.5, -0.5, +0.5 }, { 0.8f, 0.4f, 0.0f } },
    { { -0.5, +0.5, +0.5 }, { 1.0f, 0.6f, 0.0f } },
    { { -0.5, -0.5, -0.5 }, { 0.4f, 0.2f, 0.0f } },
    { { -0.5, +0.5, -0.5 }, { 0.9f, 0.5f, 0.1f } }, 
};

TexturedVertex3D textured_cube_vertices[] = {
    { { -0.5, -0.5, +0.5 }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { { -0.5, +0.5, +0.5 }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } },
    { { +0.5, +0.5, +0.5 }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
    { { +0.5, +0.5, +0.5 }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
    { { +0.5, -0.5, +0.5 }, { 1.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } },
    { { -0.5, -0.5, +0.5 }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },

    { { -0.5, -0.5, -0.5 }, { 0.5f, 0.0f, 0.5f }, { 1.0f, 0.0f } },
    { { -0.5, +0.5, -0.5 }, { 0.0f, 0.5f, 0.5f }, { 1.0f, 1.0f } },
    { { +0.5, +0.5, -0.5 }, { 0.5f, 0.5f, 0.0f }, { 0.0f, 1.0f } },
    { { +0.5, +0.5, -0.5 }, { 0.5f, 0.5f, 0.0f }, { 0.0f, 1.0f } },
    { { +0.5, -0.5, -0.5 }, { 0.0f, 0.5f, 0.0f }, { 0.0f, 0.0f } },
    { { -0.5, -0.5, -0.5 }, { 0.5f, 0.0f, 0.5f }, { 1.0f, 0.0f } },

    { { -0.5, +0.5, -0.5 }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } },
    { { -0.5, +0.5, +0.5 }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
    { { +0.5, +0.5, +0.5 }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } },
    { { +0.5, +0.5, +0.5 }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } },
    { { +0.5, +0.5, -0.5 }, { 1.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } },
    { { -0.5, +0.5, -0.5 }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } },

    { { -0.5, -0.5, -0.5 }, { 0.5f, 0.0f, 0.5f }, { 1.0f, 1.0f } },
    { { -0.5, -0.5, +0.5 }, { 0.0f, 0.5f, 0.5f }, { 1.0f, 0.0f } },
    { { +0.5, -0.5, +0.5 }, { 0.5f, 0.5f, 0.0f }, { 0.0f, 0.0f } },
    { { +0.5, -0.5, +0.5 }, { 0.5f, 0.5f, 0.0f }, { 0.0f, 0.0f } },
    { { +0.5, -0.5, -0.5 }, { 0.0f, 0.5f, 0.0f }, { 0.0f, 1.0f } },
    { { -0.5, -0.5, -0.5 }, { 0.5f, 0.0f, 0.5f }, { 1.0f, 1.0f } },

    { { +0.5, -0.5, -0.5 }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
    { { +0.5, +0.5, -0.5 }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } },
    { { +0.5, +0.5, +0.5 }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
    { { +0.5, +0.5, +0.5 }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
    { { +0.5, -0.5, +0.5 }, { 1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
    { { +0.5, -0.5, -0.5 }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },

    { { -0.5, -0.5, -0.5 }, { 0.5f, 0.0f, 0.5f }, { 0.0f, 0.0f } },
    { { -0.5, +0.5, -0.5 }, { 0.0f, 0.5f, 0.5f }, { 0.0f, 1.0f } },
    { { -0.5, +0.5, +0.5 }, { 0.5f, 0.5f, 0.0f }, { 1.0f, 1.0f } },
    { { -0.5, +0.5, +0.5 }, { 0.5f, 0.5f, 0.0f }, { 1.0f, 1.0f } },
    { { -0.5, -0.5, +0.5 }, { 0.0f, 0.5f, 0.0f }, { 1.0f, 0.0f } },
    { { -0.5, -0.5, -0.5 }, { 0.5f, 0.0f, 0.5f }, { 0.0f, 0.0f } },
};

GradientVertex3D tetrahedron_vertices[] = {
    { {  0.0f, -0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f } }, 
    { { -0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
    { {  0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f } },

    { {  0.0f, -0.5f,  0.5f }, { 1.0f, 0.5f, 0.0f } }, 
    { { -0.5f, -0.5f, -0.5f }, { 1.0f, 1.0f, 0.0f } },
    { {  0.0f,  0.5f,  0.0f }, { 1.0f, 0.0f, 0.0f } }, 

    { {  0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f, 1.0f } }, 
    { {  0.0f, -0.5f,  0.5f }, { 0.5f, 0.0f, 1.0f } }, 
    { {  0.0f,  0.5f,  0.0f }, { 0.0f, 0.0f, 1.0f } }, 

    { { -0.5f, -0.5f, -0.5f }, { 1.0f, 0.7f, 0.7f } }, 
    { {  0.5f, -0.5f, -0.5f }, { 0.7f, 1.0f, 0.7f } }, 
    { {  0.0f,  0.5f,  0.0f }, { 0.7f, 0.7f, 1.0f } }, 
};

#define CHECKER_SIZE 8  
#define TEX_SIZE 64 

GradientVertex3D circle_vertices[CIRCLE_SEGMENTS + 2];
GLuint vertex_buffers[VERTEX_BUFFERS_COUNT];
GLuint vaos[VAOS_COUNT];


void init_circle_vertices();
void hue_to_rgb(float hue, float* r, float* g, float* b);
void opengl_debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *user_param);
int compile_shader(const char *shader_path);
float clamp(float x, float low, float high);

Vector3 direction_from_pitch_yaw(float pitch, float yaw);
void camera_update(Camera *camera);

int main() {
    Program _program = {0};
    Program *program = &_program;
    Shaders *shaders = &program->shaders;

    Camera camera = {
        .position = { 0, 0, -10 },
        .up = { 0, 1, 0 },
        .field_of_view = DEG2RAD * 30.0f,
        .near = 0.001f,
        .far = 10000.0f,
    };

    //
    // Обращаю внимание, что любые функции OpenGL
    // нужно вызывать после open_window().
    //
    open_window(program, 800, 600, "Модуль 3: OpenGL 🦭");

    init_circle_vertices();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(opengl_debug_message_callback, NULL);

    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_FRONT);

    {
        int gradient = compile_shader("shaders/gradient3D");

        shaders->gradient.id = gradient;
        shaders->gradient.vertex_position = glGetAttribLocation(gradient, "vertex_position");
        shaders->gradient.vertex_color = glGetAttribLocation(gradient, "vertex_color");
        shaders->gradient.time = glGetUniformLocation(gradient, "time");
        shaders->gradient.view = glGetUniformLocation(gradient, "view");
        shaders->gradient.proj = glGetUniformLocation(gradient, "proj");
        shaders->gradient.world = glGetUniformLocation(gradient, "world");

        int textured = compile_shader("shaders/textured_cube");

        shaders->textured.id = textured;
        shaders->textured.vertex_position = glGetAttribLocation(textured, "aPos");
        shaders->textured.vertex_color = glGetAttribLocation(textured, "aColor");
        shaders->textured.vertex_tex = glGetAttribLocation(textured, "aTexCoord");
        shaders->textured.rotation_x = glGetUniformLocation(textured, "rotation_x");
        shaders->textured.rotation_y = glGetUniformLocation(textured, "rotation_y");
        shaders->textured.rotation_z = glGetUniformLocation(textured, "rotation_z");
        shaders->textured.position = glGetUniformLocation(textured, "position");
        shaders->textured.scale = glGetUniformLocation(textured, "scale");
        shaders->textured.texture = glGetUniformLocation(textured, "ourTexture");
        shaders->textured.color_boost = glGetUniformLocation(textured, "colorBoost");

        int mix_textured = compile_shader("shaders/mix_textured_cube");

        shaders->mix_textured.id = mix_textured;
        shaders->mix_textured.vertex_position = glGetAttribLocation(mix_textured, "aPos");
        shaders->mix_textured.vertex_color = glGetAttribLocation(mix_textured, "aColor");
        shaders->mix_textured.vertex_tex = glGetAttribLocation(mix_textured, "aTexCoord");
        shaders->mix_textured.rotation_x = glGetUniformLocation(mix_textured, "rotation_x");
        shaders->mix_textured.rotation_y = glGetUniformLocation(mix_textured, "rotation_y");
        shaders->mix_textured.rotation_z = glGetUniformLocation(mix_textured, "rotation_z");
        shaders->mix_textured.position = glGetUniformLocation(mix_textured, "position");
        shaders->mix_textured.scale = glGetUniformLocation(mix_textured, "scale");
        shaders->mix_textured.texture1 = glGetUniformLocation(mix_textured, "texture1");
        shaders->mix_textured.texture2 = glGetUniformLocation(mix_textured, "texture2");
        shaders->mix_textured.mix_ratio = glGetUniformLocation(mix_textured, "mixRatio");
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    unsigned char* checker_pixels = malloc(TEX_SIZE * TEX_SIZE * 3);

    for (int y = 0; y < TEX_SIZE; y++) {
        for (int x = 0; x < TEX_SIZE; x++) {
            int index = (y * TEX_SIZE + x) * 3;

            int cellX = x / CHECKER_SIZE;
            int cellY = y / CHECKER_SIZE;

            if ((cellX + cellY) % 2 == 0) {
                checker_pixels[index] = 255;
                checker_pixels[index + 1] = 255;
                checker_pixels[index + 2] = 255;
            }
            else {
                checker_pixels[index] = 0;
                checker_pixels[index + 1] = 0;
                checker_pixels[index + 2] = 0;
            }
        }
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEX_SIZE, TEX_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, checker_pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    free(checker_pixels);

    GLuint texture1;
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    unsigned char chessboard[] = {
        255,255,255,  0,0,0,  255,255,255,  0,0,0,
        0,0,0,  255,255,255,  0,0,0,  255,255,255,
        255,255,255,  0,0,0,  255,255,255,  0,0,0,
        0,0,0,  255,255,255,  0,0,0,  255,255,255,
    };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 4, 4, 0, GL_RGB, GL_UNSIGNED_BYTE, chessboard);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    GLuint texture2;
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    unsigned char stripes[] = {
        255,0,0,  255,0,0,  0,255,0,  0,255,0,
        255,0,0,  255,0,0,  0,255,0,  0,255,0,
        255,0,0,  255,0,0,  0,255,0,  0,255,0,
        255,0,0,  255,0,0,  0,255,0,  0,255,0,
    };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 4, 4, 0, GL_RGB, GL_UNSIGNED_BYTE, stripes);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenBuffers(VERTEX_BUFFERS_COUNT, vertex_buffers);
    {
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[CUBE]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[TETRAHEDRON]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(tetrahedron_vertices), tetrahedron_vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[CIRCLE]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(circle_vertices), circle_vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[TEXTURED_CUBE]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(textured_cube_vertices), textured_cube_vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    glGenVertexArrays(VAOS_COUNT, vaos);
    for (int i = 0; i < TEXTURED_CUBE_VAO; i++) {
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[i]);
        glBindVertexArray(vaos[i]);
        glEnableVertexAttribArray(shaders->gradient.vertex_position);
        glVertexAttribPointer(shaders->gradient.vertex_position, 3, GL_FLOAT, GL_FALSE, sizeof(GradientVertex3D), (void *)0);
        glEnableVertexAttribArray(shaders->gradient.vertex_color);
        glVertexAttribPointer(shaders->gradient.vertex_color, 3, GL_FLOAT, GL_FALSE, sizeof(GradientVertex3D), (void *)(3 * sizeof(float)));
    }

    {
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[TEXTURED_CUBE]);
        glBindVertexArray(vaos[TEXTURED_CUBE_VAO]);
        glEnableVertexAttribArray(shaders->textured.vertex_position);
        glVertexAttribPointer(shaders->textured.vertex_position, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex3D), (void*)0);
        glEnableVertexAttribArray(shaders->textured.vertex_color);
        glVertexAttribPointer(shaders->textured.vertex_color, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex3D), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(shaders->textured.vertex_tex);
        glVertexAttribPointer(shaders->textured.vertex_tex, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex3D), (void*)(6 * sizeof(float)));
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    int mode = MODE_GRADIENT;
    int figure = CUBE;

    float time = glfwGetTime();
    float last_frame_time = glfwGetTime();
    float delta_time = 1 / 60.0f;
    while (!glfwWindowShouldClose(program->window)) {
        camera.aspect = (float)program->window_info.width / (float)program->window_info.height;
        camera_update(&camera);

        if (program->keys[GLFW_KEY_1].pressed_this_frame) mode = MODE_GRADIENT;
        if (program->keys[GLFW_KEY_2].pressed_this_frame) mode = MODE_TEXTURED;
        if (program->keys[GLFW_KEY_3].pressed_this_frame) mode = MODE_MIX_TEXTURED;

        if (program->keys[GLFW_KEY_Z].pressed_this_frame) figure = TETRAHEDRON;
        if (program->keys[GLFW_KEY_X].pressed_this_frame) figure = CUBE;
        if (program->keys[GLFW_KEY_C].pressed_this_frame) figure = CIRCLE;
        if (program->keys[GLFW_KEY_V].pressed_this_frame) figure = TEXTURED_CUBE;

        if (program->keys[GLFW_KEY_B].pressed) global_rotation_x += rotation_speed;
        if (program->keys[GLFW_KEY_N].pressed) global_rotation_y += rotation_speed;
        if (program->keys[GLFW_KEY_M].pressed) global_rotation_z += rotation_speed;

        if (program->keys[GLFW_KEY_A].pressed) pos_x -= move_speed;
        if (program->keys[GLFW_KEY_D].pressed) pos_x += move_speed;
        if (program->keys[GLFW_KEY_W].pressed) pos_y += move_speed;
        if (program->keys[GLFW_KEY_S].pressed) pos_y -= move_speed;

        if (program->keys[GLFW_KEY_Q].pressed) zoom += 0.02;
        if (program->keys[GLFW_KEY_E].pressed) zoom -= 0.02;

        if (program->keys[GLFW_KEY_U].pressed) scale_x += scale_speed;
        if (program->keys[GLFW_KEY_J].pressed) scale_x -= scale_speed;
        if (program->keys[GLFW_KEY_I].pressed) scale_y += scale_speed;
        if (program->keys[GLFW_KEY_K].pressed) scale_y -= scale_speed;
        if (program->keys[GLFW_KEY_O].pressed) scale_z += scale_speed;
        if (program->keys[GLFW_KEY_L].pressed) scale_z -= scale_speed;

        if (program->keys[GLFW_KEY_4].pressed) color_boost.r += boost_step; // + Красный
        if (program->keys[GLFW_KEY_6].pressed) color_boost.g += boost_step; // + Зелёный
        if (program->keys[GLFW_KEY_8].pressed) color_boost.b += boost_step; // + Синий

        if (program->keys[GLFW_KEY_5].pressed) color_boost.r -= boost_step; // - Красный
        if (program->keys[GLFW_KEY_7].pressed) color_boost.g -= boost_step; // - Зелёный
        if (program->keys[GLFW_KEY_9].pressed) color_boost.b -= boost_step; // -Синий

        // Camera movement
        {
            camera.yaw -= program->mouse.move.x * sensitivity * delta_time;
            camera.pitch += program->mouse.move.y * sensitivity * delta_time;
            camera.pitch = clamp(camera.pitch, -PI / 2, PI / 2);

            Vector3 move_direction = {0};

            if (program->keys[GLFW_KEY_A].pressed) move_direction.x += 1;
            if (program->keys[GLFW_KEY_D].pressed) move_direction.x -= 1;
            if (program->keys[GLFW_KEY_W].pressed) move_direction.z += 1;
            if (program->keys[GLFW_KEY_S].pressed) move_direction.z -= 1;
            if (program->keys[GLFW_KEY_Q].pressed) move_direction.y -= 1;
            if (program->keys[GLFW_KEY_E].pressed) move_direction.y += 1;

            move_direction = vec3_normalize(move_direction);

            Vector3 r = vec3_multiply(camera.right, move_direction.x);
            Vector3 u = vec3_multiply(camera.up, move_direction.y);
            Vector3 f = vec3_multiply(camera.forward, move_direction.z);

            Vector3 direction = vec3_normalize(vec3_add(r, vec3_add(u, f)));

            camera.position = vec3_add(camera.position, vec3_multiply(direction, camera_move_speed * delta_time));
        }

        if (color_boost.r < 0.0f) color_boost.r = 0.0f;
        if (color_boost.g < 0.0f) color_boost.g = 0.0f;
        if (color_boost.b < 0.0f) color_boost.b = 0.0f;

        if (color_boost.r > 2.0f) color_boost.r = 2.0f;
        if (color_boost.g > 2.0f) color_boost.g = 2.0f;
        if (color_boost.b > 2.0f) color_boost.b = 2.0f;

        if (program->keys[GLFW_KEY_RIGHT].pressed) {
            texture_mix += mix_step * delta_time;
            if (texture_mix > 1.0f) texture_mix = 1.0f;
        }
        if (program->keys[GLFW_KEY_LEFT].pressed) {
            texture_mix -= mix_step * delta_time;
            if (texture_mix < 0.0f) texture_mix = 0.0f;
        }

        if (program->keys[GLFW_KEY_R].pressed_this_frame) {
            global_rotation_x = global_rotation_y = global_rotation_z = 0.0f;
            pos_x = pos_y = zoom = 0.0f;
            scale_x = scale_y = scale_z = 1.0f;
            color_boost.r = color_boost.g = color_boost.b = 0.0f;
        }

        Vector3 target = camera.position;
        target.x += camera.forward.x;
        target.y += camera.forward.y;
        target.z += camera.forward.z;
        Matrix4x4 view = mat4_look_at(camera.position, target, (Vector3) { 0, 1, 0 });
        Matrix4x4 proj = mat4_perspective(camera.field_of_view, camera.aspect, camera.near, camera.far);

        Vector3 translation = (Vector3) { .x = 0, .y = 0, .z = 0 };
        Vector3 rotation = (Vector3) { .x = global_rotation_x, .y = global_rotation_y, .z = global_rotation_z };
        Vector3 scale = (Vector3) { .x = scale_x, .y = scale_y, .z = scale_z };
        Matrix4x4 world = mat4_world(translation, rotation, scale);

        Matrix4x4 test = mat4_multiply(mat4_multiply(proj, view), world);
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                printf("%.6f\t", test.m[4 * i + j]);
            }
            printf("\n");
        }
        printf("\n");

        glViewport(0, 0, program->window_info.width, program->window_info.height);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        switch (mode) {
            case MODE_TEXTURED: {
                glUseProgram(shaders->textured.id);
                glUniform1f(shaders->textured.rotation_x, global_rotation_x);
                glUniform1f(shaders->textured.rotation_y, global_rotation_y);
                glUniform1f(shaders->textured.rotation_z, global_rotation_z);
                glUniform2f(shaders->textured.position, pos_x, pos_y);
                glUniform3f(shaders->textured.scale, scale_x, scale_y, scale_z);
                glUniform3f(shaders->textured.color_boost, color_boost.r, color_boost.g, color_boost.b);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, texture);
                glUniform1i(shaders->textured.texture, 0);
            } break;

            case MODE_GRADIENT: {
                glUseProgram(shaders->gradient.id);
                glUniform1f(shaders->gradient.time, time);
                glUniformMatrix4fv(shaders->gradient.world, 1, true, world.m);
                glUniformMatrix4fv(shaders->gradient.view, 1, false, view.m);
                glUniformMatrix4fv(shaders->gradient.proj, 1, false, proj.m);
            } break;

            case MODE_MIX_TEXTURED: {
                glUseProgram(shaders->mix_textured.id);
                glUniform1f(shaders->mix_textured.rotation_x, global_rotation_x);
                glUniform1f(shaders->mix_textured.rotation_y, global_rotation_y);
                glUniform1f(shaders->mix_textured.rotation_z, global_rotation_z);
                glUniform2f(shaders->mix_textured.position, pos_x, pos_y);
                glUniform3f(shaders->mix_textured.scale, scale_x, scale_y, scale_z);
                glUniform1f(shaders->mix_textured.mix_ratio, texture_mix);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, texture1);
                glUniform1i(shaders->mix_textured.texture1, 0);

                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, texture2);
                glUniform1i(shaders->mix_textured.texture2, 1);
            } break;
        }

        switch (figure) {
            case CUBE: {
                glBindVertexArray(vaos[CUBE_VAO]);
                glDrawArrays(GL_TRIANGLES, 0, ARRAY_LEN(cube_vertices));
            } break;

            case TETRAHEDRON: {
                glBindVertexArray(vaos[TETRAHEDRON_VAO]);
                glDrawArrays(GL_TRIANGLES, 0, ARRAY_LEN(tetrahedron_vertices));
            } break;

            case CIRCLE: {
                glBindVertexArray(vaos[CIRCLE_VAO]);
                glDrawArrays(GL_TRIANGLE_FAN, 0, ARRAY_LEN(circle_vertices));
            } break;

            case TEXTURED_CUBE: {
                glBindVertexArray(vaos[TEXTURED_CUBE_VAO]);
                glDrawArrays(GL_TRIANGLES, 0, ARRAY_LEN(textured_cube_vertices));
            } break;
        }

        glUseProgram(0);
        glBindVertexArray(0);

        glfwSwapBuffers(program->window);
        for (int i = 0; i < GLFW_KEY_LAST; i++) {
            program->keys[i].pressed_this_frame = false;
        }
        program->mouse.left.pressed_this_frame = false;
        program->mouse.right.pressed_this_frame = false;
        program->mouse.move = (Vector2) {0};
        glfwPollEvents();

        time = glfwGetTime();
        delta_time = time - last_frame_time;
        last_frame_time = time;
    }

    // Впервые в жизни я уберу за собой!
    glDeleteProgram(shaders->gradient.id);
    glDeleteProgram(shaders->uniform_flat_color.id);
    glDeleteProgram(shaders->flat_color.id);
    glDeleteBuffers(VERTEX_BUFFERS_COUNT, vertex_buffers);
    glDeleteVertexArrays(VAOS_COUNT, vaos);

    close_window(program);
}

// Передавайте сюда имя файла БЕЗ расширения.
// Сами шейдеры должны быть с расширениями .vs для вершинного, .fs для фрагментного.
// Ну, не дети уже, сами разберетесь 🏅
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

    int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, (const char * const *)&fragment_shader_source, NULL);
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

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    free(vertex_shader_source);
    free(fragment_shader_source);

    return shader;
}

void opengl_debug_message_callback(
    GLenum source, GLenum type, GLuint id,
    GLenum severity, GLsizei length, const GLchar *message,
    const void *user_param
) {
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
        return;
    }

    fprintf(stderr, "OpenGL ");
    if (type == GL_DEBUG_TYPE_ERROR) {
        fprintf(stderr, "error: ");
    } else {
        fprintf(stderr, "warning: ");
    }

    fprintf(stderr, "%s\n", message);

    if (type == GL_DEBUG_TYPE_ERROR) {
        assert(false); // Это база.
    }
};

void init_circle_vertices() {
    circle_vertices[0].position.x = 0.0f;
    circle_vertices[0].position.y = 0.0f;
    circle_vertices[0].position.z = 0.0f;
    circle_vertices[0].color.r = 1.0f;  
    circle_vertices[0].color.g = 1.0f;  
    circle_vertices[0].color.b = 1.0f; 

    for (int i = 0; i <= CIRCLE_SEGMENTS; i++) {
        float angle = 2.0f * PI * i / CIRCLE_SEGMENTS;

        circle_vertices[i + 1].position.x = cosf(angle);
        circle_vertices[i + 1].position.y = sinf(angle);
        circle_vertices[i + 1].position.z = 0.0f;  

        float hue = (float)i / CIRCLE_SEGMENTS;  

        float r, g, b;
        hue_to_rgb(hue, &r, &g, &b);

        circle_vertices[i + 1].color.r = r;
        circle_vertices[i + 1].color.g = g;
        circle_vertices[i + 1].color.b = b;
    }
}

void hue_to_rgb(float hue, float* r, float* g, float* b) {
    float h = hue * 6.0f;  
    int sector = (int)h;
    float fraction = h - sector;

    float q = 1.0f - fraction;
    float t = fraction;

    switch (sector % 6) {
    case 0: *r = 1.0f; *g = t; *b = 0.0f; break;
    case 1: *r = q; *g = 1.0f; *b = 0.0f; break;
    case 2: *r = 0.0f; *g = 1.0f; *b = t; break;
    case 3: *r = 0.0f; *g = q; *b = 1.0f; break;
    case 4: *r = t; *g = 0.0f; *b = 1.0f; break;
    case 5: *r = 1.0f; *g = 0.0f; *b = q; break;
    }
}

float clamp(float x, float low, float high) {
    if (x < low) {
        return low;
    }
    if (x > high) {
        return high;
    }
    return x;
}

Vector3 direction_from_pitch_yaw(float pitch, float yaw) {
    Vector3 direction = {0};

    direction.x = cosf(pitch) * sinf(yaw);
    direction.y = sinf(pitch);
    direction.z = cosf(pitch) * cosf(yaw);

    return vec3_normalize(direction);
}

void camera_update(Camera *camera) {
    camera->forward = direction_from_pitch_yaw(camera->pitch, camera->yaw);
    camera->right = vec3_cross((Vector3){ 0, 1, 0 }, camera->forward);
    camera->up = vec3_cross(camera->forward, camera->right);
}
