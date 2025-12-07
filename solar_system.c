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
#include "obj_loader.h"

#define PLANET_COUNT 6

typedef struct {
    float orbit_radius;      // Радиус орбиты
    float orbit_speed;       // Скорость вращения по орбите
    float rotation_speed;    // Скорость вращения вокруг своей оси
    float scale;            // Размер планеты
    Vector3 color;          // Цвет/оттенок планеты
    float orbit_angle;      // Текущий угол на орбите
    float rotation_angle;   // Текущий угол вращения
} Planet;

// Глобальные переменные для камеры
const float camera_move_speed = 5.0f;
const float camera_rotation_speed = 0.1f;

// Функции
void init_planets(Planet *planets);
Matrix4x4 get_planet_model_matrix(Planet *planet);
int compile_shader(const char *shader_path);
char *read_entire_file(const char *filename);
unsigned int load_texture(const char *path);

int main() {
    Program _program = {0};
    Program *program = &_program;

    // Инициализация окна
    open_window(program, 1280, 720, "Солнечная система 🪐");

    // Камера
    Camera camera = {
        .position = { 0, 5, 15 },
        .up = { 0, 1, 0 },
        .field_of_view = DEG2RAD * 45.0f,
        .near = 0.1f,
        .far = 1000.0f,
        .pitch = -0.3f,
        .yaw = 0.0f,
        .aspect = 1280.0f / 720.0f
    };
    camera_update(&camera);

    // Включаем тест глубины
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    if (glDebugMessageCallback != NULL) {
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(opengl_debug_message_callback, NULL);
    }

    // Загружаем модель
    OBJModel model;
    if (!load_obj_model("models/crystal.obj", &model)) {
        fprintf(stderr, "Не удалось загрузить модель!\n");
        return 1;
    }
    setup_obj_model_buffers(&model);

    printf("Модель загружена: %d вершин (%d полигонов)\n",
           model.vertex_count, model.vertex_count / 3);

    // Компилируем шейдеры
    int shader = compile_shader("shaders/solar_system");
    int loc_model = glGetUniformLocation(shader, "model");
    int loc_view = glGetUniformLocation(shader, "view");
    int loc_proj = glGetUniformLocation(shader, "proj");
    int loc_texture = glGetUniformLocation(shader, "texture_sampler");
    int loc_color = glGetUniformLocation(shader, "planet_color");

    // Загружаем текстуру
    unsigned int texture = load_texture("images/fruits.jpg");

    // Инициализируем планеты
    Planet planets[PLANET_COUNT];
    init_planets(planets);

    float time = 0.0f;
    float last_time = (float)glfwGetTime();

    // Главный цикл
    while (!glfwWindowShouldClose(program->window)) {
        float current_time = (float)glfwGetTime();
        float delta_time = current_time - last_time;
        last_time = current_time;
        time += delta_time;

        // Обновляем аспект камеры
        camera.aspect = (float)program->window_info.width / (float)program->window_info.height;
        camera_update(&camera);

        // Управление камерой
        {
            // Вращение камеры мышью
            camera.yaw -= program->mouse.move.x * camera_rotation_speed * delta_time;
            camera.pitch += program->mouse.move.y * camera_rotation_speed * delta_time;
            camera.pitch = clamp(camera.pitch, -PI / 2.0f + 0.01f, PI / 2.0f - 0.01f);

            // Движение камеры
            Vector3 move_direction = {0};

            if (program->keys[GLFW_KEY_W].pressed) move_direction.z += 1;
            if (program->keys[GLFW_KEY_S].pressed) move_direction.z -= 1;
            if (program->keys[GLFW_KEY_A].pressed) move_direction.x += 1;
            if (program->keys[GLFW_KEY_D].pressed) move_direction.x -= 1;
            if (program->keys[GLFW_KEY_SPACE].pressed) move_direction.y += 1;
            if (program->keys[GLFW_KEY_LEFT_SHIFT].pressed) move_direction.y -= 1;

            if (vec3_length(move_direction) > 0.01f) {
                move_direction = vec3_normalize(move_direction);

                Vector3 r = vec3_multiply(camera.right, move_direction.x);
                Vector3 u = vec3_multiply(camera.up, move_direction.y);
                Vector3 f = vec3_multiply(camera.forward, move_direction.z);

                Vector3 direction = vec3_add(r, vec3_add(u, f));

                camera.position = vec3_add(camera.position,
                                          vec3_multiply(direction, camera_move_speed * delta_time));
            }
        }

        // Обновляем планеты
        for (int i = 0; i < PLANET_COUNT; i++) {
            planets[i].orbit_angle += planets[i].orbit_speed * delta_time;
            planets[i].rotation_angle += planets[i].rotation_speed * delta_time;
        }

        // Матрицы view и projection
        Vector3 target = vec3_add(camera.position, camera.forward);
        Matrix4x4 view = mat4_look_at(camera.position, target, (Vector3){0, 1, 0});
        Matrix4x4 proj = mat4_perspective(camera.field_of_view, camera.aspect, camera.near, camera.far);

        // Очищаем экран
        glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Используем шейдер и модель
        glUseProgram(shader);
        glBindVertexArray(model.vao);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(loc_texture, 0);

        glUniformMatrix4fv(loc_view, 1, GL_FALSE, view.m);
        glUniformMatrix4fv(loc_proj, 1, GL_FALSE, proj.m);

        // Рисуем центральное "Солнце"
        {
            Matrix4x4 model_mat = mat4_identity();
            model_mat = mat4_multiply(model_mat, mat4_scale((Vector3){2.5f, 2.5f, 2.5f}));
            model_mat = mat4_multiply(model_mat, mat4_rotation_y(time * 0.2f));

            glUniformMatrix4fv(loc_model, 1, GL_FALSE, model_mat.m);
            glUniform3f(loc_color, 1.5f, 1.3f, 0.8f); // Жёлтое свечение

            glDrawArrays(GL_TRIANGLES, 0, model.vertex_count);
        }

        // Рисуем планеты
        for (int i = 0; i < PLANET_COUNT; i++) {
            Matrix4x4 model_mat = get_planet_model_matrix(&planets[i]);

            glUniformMatrix4fv(loc_model, 1, GL_FALSE, model_mat.m);
            glUniform3f(loc_color, planets[i].color.x, planets[i].color.y, planets[i].color.z);

            glDrawArrays(GL_TRIANGLES, 0, model.vertex_count);
        }

        glBindVertexArray(0);
        glUseProgram(0);

        // Обновляем окно
        glfwSwapBuffers(program->window);

        // Сбрасываем состояния
        for (int i = 0; i < GLFW_KEY_LAST; i++) {
            program->keys[i].pressed_this_frame = false;
        }
        program->mouse.move = (Vector2){0, 0};

        glfwPollEvents();
    }

    // Очищаем ресурсы
    free_obj_model(&model);
    glDeleteTextures(1, &texture);
    glDeleteProgram(shader);

    close_window(program);

    return 0;
}

void init_planets(Planet *planets) {
    // Планета 1 - Меркурий (быстрая, маленькая)
    planets[0] = (Planet){
        .orbit_radius = 4.0f,
        .orbit_speed = 2.0f,
        .rotation_speed = 3.0f,
        .scale = 0.4f,
        .color = {0.8f, 0.7f, 0.6f},
        .orbit_angle = 0.0f,
        .rotation_angle = 0.0f
    };

    // Планета 2 - Венера
    planets[1] = (Planet){
        .orbit_radius = 6.0f,
        .orbit_speed = 1.5f,
        .rotation_speed = 2.0f,
        .scale = 0.7f,
        .color = {1.0f, 0.9f, 0.7f},
        .orbit_angle = PI / 3.0f,
        .rotation_angle = 0.0f
    };

    // Планета 3 - Земля
    planets[2] = (Planet){
        .orbit_radius = 8.5f,
        .orbit_speed = 1.0f,
        .rotation_speed = 4.0f,
        .scale = 0.8f,
        .color = {0.3f, 0.5f, 1.0f},
        .orbit_angle = 2.0f * PI / 3.0f,
        .rotation_angle = 0.0f
    };

    // Планета 4 - Марс
    planets[3] = (Planet){
        .orbit_radius = 11.0f,
        .orbit_speed = 0.8f,
        .rotation_speed = 3.5f,
        .scale = 0.6f,
        .color = {1.0f, 0.4f, 0.3f},
        .orbit_angle = PI,
        .rotation_angle = 0.0f
    };

    // Планета 5 - Юпитер (большая)
    planets[4] = (Planet){
        .orbit_radius = 15.0f,
        .orbit_speed = 0.5f,
        .rotation_speed = 5.0f,
        .scale = 1.5f,
        .color = {0.9f, 0.7f, 0.5f},
        .orbit_angle = 4.0f * PI / 3.0f,
        .rotation_angle = 0.0f
    };

    // Планета 6 - Сатурн
    planets[5] = (Planet){
        .orbit_radius = 19.0f,
        .orbit_speed = 0.3f,
        .rotation_speed = 4.5f,
        .scale = 1.3f,
        .color = {0.95f, 0.9f, 0.7f},
        .orbit_angle = 5.0f * PI / 3.0f,
        .rotation_angle = 0.0f
    };
}

Matrix4x4 get_planet_model_matrix(Planet *planet) {
    // Позиция на орбите
    float x = planet->orbit_radius * cosf(planet->orbit_angle);
    float z = planet->orbit_radius * sinf(planet->orbit_angle);

    Matrix4x4 model = mat4_identity();

    // Перемещение на орбиту
    model = mat4_multiply(model, mat4_translation((Vector3){x, 0, z}));

    // Вращение вокруг своей оси
    model = mat4_multiply(model, mat4_rotation_y(planet->rotation_angle));

    // Масштабирование
    model = mat4_multiply(model, mat4_scale((Vector3){
        planet->scale, planet->scale, planet->scale
    }));

    return model;
}

// Вспомогательные функции

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

int compile_shader(const char *shader_path) {
    char vertex_path[256];
    char fragment_path[256];
    snprintf(vertex_path, sizeof(vertex_path), "%s.vs", shader_path);
    snprintf(fragment_path, sizeof(fragment_path), "%s.fs", shader_path);

    char *vertex_source = read_entire_file(vertex_path);
    char *fragment_source = read_entire_file(fragment_path);

    if (!vertex_source || !fragment_source) {
        return 0;
    }

    // Компилируем вершинный шейдер
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, (const char **)&vertex_source, NULL);
    glCompileShader(vertex_shader);

    int success;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(vertex_shader, 512, NULL, log);
        fprintf(stderr, "Ошибка компиляции вершинного шейдера: %s\n", log);
    }

    // Компилируем фрагментный шейдер
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, (const char **)&fragment_source, NULL);
    glCompileShader(fragment_shader);

    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(fragment_shader, 512, NULL, log);
        fprintf(stderr, "Ошибка компиляции фрагментного шейдера: %s\n", log);
    }

    // Линкуем программу
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(program, 512, NULL, log);
        fprintf(stderr, "Ошибка линковки шейдерной программы: %s\n", log);
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    free(vertex_source);
    free(fragment_source);

    return program;
}

unsigned int load_texture(const char *path) {
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, channels;
    unsigned char *data = stbi_load(path, &width, &height, &channels, 0);

    if (data) {
        GLenum format = (channels == 3) ? GL_RGB : GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        fprintf(stderr, "Не удалось загрузить текстуру: %s\n", path);
    }

    stbi_image_free(data);
    return texture;
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
}

float clamp(float x, float low, float high) {
    if (x < low) return low;
    if (x > high) return high;
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
    camera->right = vec3_cross((Vector3){0, 1, 0}, camera->forward);
    camera->up = vec3_cross(camera->forward, camera->right);
}

