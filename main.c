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

#define HOUSE_COUNT 5
#define BENCH_COUNT 7
#define GIFT_COUNT 4
#define KITE_COUNT 4
#define CLOUD_COUNT 5

Matrix4x4 house_instances[HOUSE_COUNT];
Matrix4x4 bench_instances[BENCH_COUNT];
Matrix4x4 skull_instances[GIFT_COUNT];
Matrix4x4 kite_instances[KITE_COUNT];
Matrix4x4 cloud_instances[CLOUD_COUNT];

#define MAX_OBJECTS (HOUSE_COUNT + BENCH_COUNT + GIFT_COUNT + KITE_COUNT + CLOUD_COUNT + 1)

Vector3 all_positions[MAX_OBJECTS];
float all_radii[MAX_OBJECTS];
int placed_count = 0;

GLuint terrain_vao = 0;
GLuint terrain_vbo = 0;
GLuint terrain_ebo = 0;
GLuint terrain_texture = 0;

int terrain_index_count = 0;

unsigned char* terrain_heightmap = NULL;
int terrain_w = 0;
int terrain_h = 0;
float terrain_scale = 1.0f;   // scale по XZ
float terrain_height_scale = 15.0f; // высота
int terrain_size = 256;

const float sensitivity = 0.1f;

Vector3 airship_pos = { 3.0f, 15.0f, 3.0f };
Vector3 airship_rot = { 0.0f, 0.0f, 0.0f }; 
float airship_speed = 8.0f;
float airship_turn_speed = 6.0f;

float camera_yaw = 0.0f;
float camera_pitch = 0.3f;
float camera_distance = 20.0f;

void opengl_debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *user_param);
int compile_shader(const char *shader_path);
GLuint load_texture_from_file(const char *path);
float clamp(float x, float low, float high);
float lerp_angle(float a, float b, float t);
float randf(float min, float max);
bool can_place(Vector3 p, float radius);
bool load_obj_model_try_paths(const char** paths, int path_count, OBJModel* model, const char* model_name);
void create_terrain(int size, float scale);
GLuint load_heightmap_texture(const char* path);
float terrain_height(float x, float z);

int main() {
    srand((unsigned int)time(NULL));
    Program _program = {0};
    Program *program = &_program;
    Shaders *shaders = &program->shaders;

    Camera camera = {
        .position = { 0, 0, -10 },
        .up = { 0, 1, 0 },
        .field_of_view = DEG2RAD * 30.0f,
        .near = 0.1f,
        .far = 10000.0f,
        .pitch = 0.0f,
        .yaw = 0.0f,
    };

    all_positions[placed_count] = (Vector3){ 0.0f,0.0f,0.0f };
    placed_count++;

    // Воздушный змей
    for (int i = 0; i < KITE_COUNT; i++) {
        Vector3 pos;

        do {
            pos = (Vector3){
                randf(-35.0f, 35.0f),
                randf(9.0f, 14.0f),
                randf(-35.0f, 35.0f)
            };
        } while (!can_place(pos, 4.0f));

        all_positions[placed_count] = pos;
        all_radii[placed_count] = 4.0f;
        placed_count++;

        Matrix4x4 m = mat4_identity();
        m = mat4_multiply(m, mat4_translation(pos));
        m = mat4_multiply(m, mat4_scale((Vector3) { 0.3f, 0.3f, 0.3f }));
        kite_instances[i] = m;
    }

    // ОБлака
    for (int i = 0; i < CLOUD_COUNT; i++) {
        Vector3 pos;

        do {
            pos = (Vector3){
                randf(-35.0f, 35.0f),
                randf(12.0f, 15.0f),
                randf(-35.0f, 35.0f)
            };
        } while (!can_place(pos, 4.0f));

        all_positions[placed_count] = pos;
        all_radii[placed_count] = 4.0f;
        placed_count++;

        Matrix4x4 m = mat4_identity();
        m = mat4_multiply(m, mat4_translation(pos));
        m = mat4_multiply(m, mat4_scale((Vector3) { 0.1f, 0.1f, 0.1f }));
        cloud_instances[i] = m;
    }

    open_window(program, 800, 600, "Модуль 3: OpenGL 🦭");

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    if (glDebugMessageCallback != NULL) {
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(opengl_debug_message_callback, NULL);
    } else {
        printf("glDebugMessageCallback не поддерживается на этой платформе\n");
    }

    int terrain_shader = compile_shader("shaders/terrain");

    create_terrain(terrain_size, terrain_scale);
    float terrain_world_size = (terrain_size - 1) * terrain_scale;

    terrain_texture = load_heightmap_texture("models/indiv_3/heightmap.png");

    if (terrain_texture == 0) {
        fprintf(stderr, "Terrain heightmap failed to load\n");
        exit(1); 
    }

    // Дома
    for (int i = 0; i < HOUSE_COUNT; i++) {
        Vector3 pos;

        do {
            pos = (Vector3){
                randf(-50.0f, 50.0f),
                0.0f,
                randf(-50.0f, 50.0f)
            };
            pos.y = terrain_height(pos.x, pos.z) + 0.01f;
        } while (!can_place(pos, 6.0f));

        all_positions[placed_count] = pos;
        all_radii[placed_count] = 6.0f;
        placed_count++;

        Matrix4x4 m = mat4_identity();
        m = mat4_multiply(m, mat4_translation(pos));
        m = mat4_multiply(m, mat4_scale((Vector3) { 1.0f, 1.0f, 1.0f }));
        house_instances[i] = m;
    }

    // Лавки
    for (int i = 0; i < BENCH_COUNT; i++) {
        Vector3 pos;

        do {
            pos = (Vector3){
                randf(-40.0f, 40.0f),
                0.0f,
                randf(-40.0f, 40.0f)
            };
            pos.y = terrain_height(pos.x, pos.z) + 0.4f;
        } while (!can_place(pos, 4.0f));

        all_positions[placed_count] = pos;
        all_radii[placed_count] = 4.0f;
        placed_count++;

        Matrix4x4 m = mat4_identity();
        m = mat4_multiply(m, mat4_translation(pos));
        m = mat4_multiply(m, mat4_scale((Vector3) { 1.5f, 1.5f, 1.5f }));
        bench_instances[i] = m;
    }

    // Подарки
    for (int i = 0; i < GIFT_COUNT; i++) {
        Vector3 pos;

        do {
            pos = (Vector3){
                randf(-35.0f, 35.0f),
                0.0f,
                randf(-35.0f, 35.0f)
            };
            pos.y = terrain_height(pos.x, pos.z) + 0.01f;
        } while (!can_place(pos, 4.0f));

        all_positions[placed_count] = pos;
        all_radii[placed_count] = 4.0f;
        placed_count++;

        Matrix4x4 m = mat4_identity();
        m = mat4_multiply(m, mat4_translation(pos));
        m = mat4_multiply(m, mat4_scale((Vector3) { 0.05f, 0.05f, 0.05f }));
        skull_instances[i] = m;
    }

    {
        int lit_instanced = compile_shader("shaders/lit_instanced");

        shaders->lit_instanced.id = lit_instanced;
        shaders->lit_instanced.view = glGetUniformLocation(lit_instanced, "view");
        shaders->lit_instanced.proj = glGetUniformLocation(lit_instanced, "proj");
        shaders->lit_instanced.viewPos = glGetUniformLocation(lit_instanced, "viewPos");
        shaders->lit_instanced.texture = glGetUniformLocation(lit_instanced, "ourTexture");
        shaders->lit_instanced.ambientColor = glGetUniformLocation(lit_instanced, "ambientColor");
        shaders->lit_instanced.lightingModel = glGetUniformLocation(lit_instanced, "lightingModel");
        shaders->lit_instanced.pointLightEnabled = glGetUniformLocation(lit_instanced, "pointLightEnabled");
        shaders->lit_instanced.pointLightPos = glGetUniformLocation(lit_instanced, "pointLightPos");
        shaders->lit_instanced.pointLightColor = glGetUniformLocation(lit_instanced, "pointLightColor");
        shaders->lit_instanced.pointLightIntensity = glGetUniformLocation(lit_instanced, "pointLightIntensity");
        shaders->lit_instanced.pointLightConstant = glGetUniformLocation(lit_instanced, "pointLightConstant");
        shaders->lit_instanced.pointLightLinear = glGetUniformLocation(lit_instanced, "pointLightLinear");
        shaders->lit_instanced.pointLightQuadratic = glGetUniformLocation(lit_instanced, "pointLightQuadratic");
        shaders->lit_instanced.dirLightEnabled = glGetUniformLocation(lit_instanced, "dirLightEnabled");
        shaders->lit_instanced.dirLightDirection = glGetUniformLocation(lit_instanced, "dirLightDirection");
        shaders->lit_instanced.dirLightColor = glGetUniformLocation(lit_instanced, "dirLightColor");
        shaders->lit_instanced.dirLightIntensity = glGetUniformLocation(lit_instanced, "dirLightIntensity");
        shaders->lit_instanced.spotLightEnabled = glGetUniformLocation(lit_instanced, "spotLightEnabled");
        shaders->lit_instanced.spotLightPos = glGetUniformLocation(lit_instanced, "spotLightPos");
        shaders->lit_instanced.spotLightDirection = glGetUniformLocation(lit_instanced, "spotLightDirection");
        shaders->lit_instanced.spotLightColor = glGetUniformLocation(lit_instanced, "spotLightColor");
        shaders->lit_instanced.spotLightIntensity = glGetUniformLocation(lit_instanced, "spotLightIntensity");
        shaders->lit_instanced.spotLightCutOff = glGetUniformLocation(lit_instanced, "spotLightCutOff");
        shaders->lit_instanced.spotLightOuterCutOff = glGetUniformLocation(lit_instanced, "spotLightOuterCutOff");
        shaders->lit_instanced.spotLightConstant = glGetUniformLocation(lit_instanced, "spotLightConstant");
        shaders->lit_instanced.spotLightLinear = glGetUniformLocation(lit_instanced, "spotLightLinear");
        shaders->lit_instanced.spotLightQuadratic = glGetUniformLocation(lit_instanced, "spotLightQuadratic");
        shaders->lit_instanced.objectAlpha = glGetUniformLocation(lit_instanced, "objectAlpha");
    }

    OBJModel model_house, model_tree, model_sledge, model_gift, model_bench, model_kite, model_cloud;
    GLuint texture_tree, texture_sledge, texture_gift, texture_house, texture_bench, texture_kite, texture_cloud;
    {
        const char* house_paths[] = {
            "models/indiv_3/Wood_house.obj",
            "../models/indiv_3/Wood_house.obj"
        };
        if (!load_obj_model_try_paths(house_paths, 2, &model_house, "Бомба"))  return 1;
        setup_obj_model_buffers(&model_house);

        const char* tree_path[] = {
            "models/indiv_3/Christmas Tree-blobj.obj",
            "../models/indiv_3/Christmas Tree-blobj.obj"
        };
        if (!load_obj_model_try_paths(tree_path, 2, &model_tree, "Елка")) return 1;
        setup_obj_model_buffers(&model_tree);

        const char* sledge_paths[] = {
            "models/indiv_3/Sledge.obj",
            "../models/indiv_3/Sledge.obj"
        };
        if (!load_obj_model_try_paths(sledge_paths, 2, &model_sledge, "Сани")) return 1;
        setup_obj_model_buffers(&model_sledge);

        const char* gift_paths[] = {
            "models/indiv_3/Gift.obj",
            "../models/indiv_3/Gift.obj"
        };
        if (!load_obj_model_try_paths(gift_paths, 2, &model_gift, "Череп")) return 1;
        setup_obj_model_buffers(&model_gift);

        const char* bench_paths[] = {
            "models/indiv_3/Classic_Garden_Bench.obj",
            "../models/indiv_3/Classic_Garden_Bench.obj"
        };
        if (!load_obj_model_try_paths(bench_paths, 2, &model_bench, "Лавка")) return 1;
        setup_obj_model_buffers(&model_bench);

        const char* kite_paths[] = {
            "models/indiv_3/Kite_OBJ.obj",
            "../models/indiv_3/Kite_OBJ.obj"
        };
        if (!load_obj_model_try_paths(kite_paths, 2, &model_kite, "Воздушный змей")) return 1;
        setup_obj_model_buffers(&model_kite);

        const char* cloud_paths[] = {
            "models/indiv_3/cloud rain.obj",
            "../models/indiv_3/cloud rain.obj"
        };
        if (!load_obj_model_try_paths(cloud_paths, 2, &model_cloud, "Воздушный змей")) return 1;
        setup_obj_model_buffers(&model_cloud);

        texture_tree = load_texture_from_file("models/indiv_3/UV Christmas Tree.png");
        if (texture_tree == 0) {
            texture_tree = load_texture_from_file("../models/indiv_3/UV Christmas Tree.png");
        }

        texture_sledge = load_texture_from_file("models/indiv_3/sleigh_DefaultMaterial_BaseColor.png");
        if (texture_sledge == 0) {
            texture_sledge = load_texture_from_file("../models/indiv_3/sleigh_DefaultMaterial_BaseColor.png");
        }

        texture_gift = load_texture_from_file("models/indiv_3/gift.jpg");
        if (texture_gift == 0) {
            texture_gift = load_texture_from_file("../models/indiv_3/gift.jpg");
        }

        texture_house = load_texture_from_file("models/indiv_3/Wood_house_BaseColor.1001.jpg");
        if (texture_house == 0) {
            texture_house = load_texture_from_file("../models/indiv_3/Wood_house_BaseColor.1001.jpg");
        }

        texture_bench = load_texture_from_file("models/indiv_3/Bench_Base_Color_4K.jpg");
        if (texture_bench == 0) {
            texture_bench = load_texture_from_file("../models/indiv_3/Bench_Base_Color_4K.jpg");
        }

        texture_kite = load_texture_from_file("models/indiv_3/Kite_TT.jpg");
        if (texture_kite == 0) {
            texture_kite = load_texture_from_file("../models/indiv_3/Kite_TT.jpg");
        }

        texture_cloud = load_texture_from_file("models/indiv_3/clouds.png");
        if (texture_cloud == 0) {
            texture_cloud = load_texture_from_file("../models/indiv_3/clouds.png");
        }
    }

    #define MAX_INSTANCES 64
    GLuint instance_vbo;

    glGenBuffers(1, &instance_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_INSTANCES * sizeof(Matrix4x4), NULL, GL_DYNAMIC_DRAW);

    OBJModel* models_to_setup[] = {&model_house, &model_gift, &model_tree, &model_sledge, &model_bench, &model_kite, &model_cloud};
    for (int m = 0; m < 7; m++) {
        glBindVertexArray(models_to_setup[m]->vao);
        glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);
        for (int i = 0; i < 4; i++) {
            glEnableVertexAttribArray(3 + i);
            glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix4x4), (void*)(sizeof(float) * 4 * i));
            glVertexAttribDivisor(3 + i, 1);
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    float current_time = glfwGetTime();
    float last_frame_time = glfwGetTime();
    float delta_time = 1 / 60.0f;

    while (!glfwWindowShouldClose(program->window)) {
        camera.aspect = (float)program->window_info.width / (float)program->window_info.height;

        {
            camera_yaw -= program->mouse.move.x * sensitivity * delta_time;
            camera_pitch -= program->mouse.move.y * sensitivity * delta_time;

            Vector3 cam_forward = {sinf(camera_yaw),0.0f, cosf(camera_yaw) };
            Vector3 cam_right = {cosf(camera_yaw), 0.0f,-sinf(camera_yaw)};

            Vector3 move = { 0 };

            if (program->keys[GLFW_KEY_W].pressed) move = vec3_add(move, vec3_multiply(cam_forward, -1.0f));
            if (program->keys[GLFW_KEY_S].pressed) move = vec3_add(move, cam_forward);
            if (program->keys[GLFW_KEY_D].pressed) move = vec3_add(move, cam_right);
            if (program->keys[GLFW_KEY_A].pressed) move = vec3_add(move, vec3_multiply(cam_right, -1.0f));
            if (program->keys[GLFW_KEY_Q].pressed) move.y -= 1.0f;
            if (program->keys[GLFW_KEY_E].pressed) move.y += 1.0f;

            camera_pitch = clamp(camera_pitch, -1.2f, 1.2f);

            if (vec3_length(move) > 0.0f) {
                move = vec3_normalize(move);
                airship_pos = vec3_add(airship_pos,vec3_multiply(move, airship_speed * delta_time));

                float target_yaw = atan2f(move.x, move.z);
                airship_rot.y = lerp_angle(airship_rot.y,target_yaw,airship_turn_speed * delta_time);
            }

            Vector3 offset;
            offset.x = camera_distance * cosf(camera_pitch) * sinf(camera_yaw);
            offset.y = camera_distance * sinf(camera_pitch);
            offset.z = camera_distance * cosf(camera_pitch) * cosf(camera_yaw);

            Vector3 camera_target = airship_pos;
            camera_target.y += 10.0f; 
            camera.position = vec3_add(camera_target, offset);
        }

        Matrix4x4 view = mat4_look_at(camera.position,airship_pos,(Vector3) {0, 1, 0});
        Matrix4x4 proj = mat4_perspective(camera.field_of_view, camera.aspect, camera.near, camera.far);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_CULL_FACE);
        glFrontFace(GL_CCW);

        {
            glUseProgram(terrain_shader);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, terrain_texture);
            glUniform1i(glGetUniformLocation(terrain_shader, "heightmap"), 0);

            glUniform1f(glGetUniformLocation(terrain_shader, "heightScale"),terrain_height_scale);
            glUniform1f(glGetUniformLocation(terrain_shader, "terrainSize"),terrain_world_size);
            glUniform3f(glGetUniformLocation(terrain_shader, "lightDir"),-0.3f, -1.0f, -0.2f);
            glUniform3f(glGetUniformLocation(terrain_shader, "lightColor"), 1.0f, 1.0f, 1.0f);
            glUniform3f(glGetUniformLocation(terrain_shader, "viewPos"),camera.position.x,camera.position.y,camera.position.z);

            glUniformMatrix4fv(glGetUniformLocation(terrain_shader, "view"),1, GL_FALSE, view.m);
            glUniformMatrix4fv(glGetUniformLocation(terrain_shader, "proj"),1, GL_FALSE, proj.m);

            glBindVertexArray(terrain_vao);
            glDrawElements(GL_TRIANGLES, terrain_index_count, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }

        {
            glUseProgram(shaders->lit_instanced.id);
            glUniformMatrix4fv(shaders->lit_instanced.view, 1, false, view.m);
            glUniformMatrix4fv(shaders->lit_instanced.proj, 1, false, proj.m);
            glUniform3f(shaders->lit_instanced.viewPos, camera.position.x, camera.position.y, camera.position.z);

            glUniform3f(shaders->lit_instanced.ambientColor, 0.7f, 0.7f, 0.7f);
            glUniform1i(shaders->lit_instanced.dirLightEnabled, 1);
            glUniform3f(shaders->lit_instanced.dirLightDirection, -1.2f, -1.0f, -0.3f);
            glUniform3f(shaders->lit_instanced.dirLightColor, 0.6f, 0.7f, 1.0f);
            glUniform1f(shaders->lit_instanced.dirLightIntensity, 5.2f);
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture_tree);
            glUniform1i(shaders->lit_instanced.texture, 0);
            glUniform1f(shaders->lit_instanced.objectAlpha, 1.0f);

            Matrix4x4 instance[1];
            instance[0] = mat4_identity();
            instance[0] = mat4_multiply(instance[0], mat4_translation((Vector3){0.0f, 6.2f, 0.0f}));
            instance[0] = mat4_multiply(instance[0], mat4_scale((Vector3){1.4f, 1.4f, 1.4f}));

            glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Matrix4x4), instance);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glBindVertexArray(model_tree.vao);
            glDrawArraysInstanced(GL_TRIANGLES, 0, model_tree.vertex_count, 1);
        }

        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture_sledge);
            glUniform1i(shaders->lit_instanced.texture, 0);
            glUniform1f(shaders->lit_instanced.objectAlpha, 1.0f);

            Matrix4x4 instance[1];
            instance[0] = mat4_identity();
            instance[0] = mat4_multiply(instance[0], mat4_translation(airship_pos));
            instance[0] = mat4_multiply(instance[0],mat4_rotation_y(airship_rot.y - PI / 2.0f));
            instance[0] = mat4_multiply(instance[0],mat4_scale((Vector3) { 0.01f, 0.01f, 0.01f }));

            glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Matrix4x4), instance);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glBindVertexArray(model_sledge.vao);
            glDrawArraysInstanced(GL_TRIANGLES,0,model_sledge.vertex_count,1);
        }

        // Подарки
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture_gift);
            glUniform1i(shaders->lit_instanced.texture, 0);
            glUniform1f(shaders->lit_instanced.objectAlpha, 1.0f);

            glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);
            glBufferSubData(GL_ARRAY_BUFFER,0, GIFT_COUNT * sizeof(Matrix4x4), skull_instances);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glBindVertexArray(model_gift.vao);
            glDrawArraysInstanced(GL_TRIANGLES,0,model_gift.vertex_count,GIFT_COUNT);
        }

        //Дома
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture_house);
            glUniform1i(shaders->lit_instanced.texture, 0);
            glUniform1f(shaders->lit_instanced.objectAlpha, 1.0f);

            glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);
            glBufferSubData(GL_ARRAY_BUFFER,0,HOUSE_COUNT * sizeof(Matrix4x4),house_instances);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glBindVertexArray(model_house.vao);
            glDrawArraysInstanced(GL_TRIANGLES,0,model_house.vertex_count,HOUSE_COUNT);
        }

        //Лавки
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture_bench);
            glUniform1i(shaders->lit_instanced.texture, 0);
            glUniform1f(shaders->lit_instanced.objectAlpha, 1.0f);

            glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, BENCH_COUNT * sizeof(Matrix4x4), bench_instances);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glBindVertexArray(model_bench.vao);
            glDrawArraysInstanced(GL_TRIANGLES, 0, model_bench.vertex_count, BENCH_COUNT);
        }

        // Воздушный змей
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture_kite);
            glUniform1i(shaders->lit_instanced.texture, 0);
            glUniform1f(shaders->lit_instanced.objectAlpha, 1.0f);

            glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, KITE_COUNT * sizeof(Matrix4x4), kite_instances);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glBindVertexArray(model_kite.vao);
            glDrawArraysInstanced(GL_TRIANGLES, 0, model_kite.vertex_count, KITE_COUNT);
        }

        // Облака
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture_cloud);
            glUniform1i(shaders->lit_instanced.texture, 0);
            glUniform1f(shaders->lit_instanced.objectAlpha, 0.9f);

            glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, CLOUD_COUNT * sizeof(Matrix4x4), cloud_instances);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glBindVertexArray(model_cloud .vao);
            glDrawArraysInstanced(GL_TRIANGLES, 0, model_cloud.vertex_count, CLOUD_COUNT);
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

        current_time = glfwGetTime();
        delta_time = current_time - last_frame_time;
        last_frame_time = current_time;
    }

    free_obj_model(&model_house);
    free_obj_model(&model_tree);
    free_obj_model(&model_sledge);
    free_obj_model(&model_gift);
    free_obj_model(&model_kite);
    free_obj_model(&model_cloud);
    free_obj_model(&model_bench);

    glDeleteTextures(1, &texture_tree);
    glDeleteTextures(1, &texture_sledge);
    glDeleteTextures(1, &texture_gift);
    glDeleteTextures(1, &texture_kite);
    glDeleteTextures(1, &texture_cloud);
    glDeleteTextures(1, &texture_house);
    glDeleteTextures(1, &texture_bench);
    glDeleteProgram(shaders->lit_instanced.id);
    glDeleteBuffers(1, &instance_vbo);

    close_window(program);
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

GLuint load_texture_from_file(const char *path) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Настройки текстуры
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Загружаем изображение (принудительно 3 канала RGB для непрозрачности)
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(path, &width, &height, &channels, STBI_rgb);

    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

    } else {
        fprintf(stderr, "[ERROR] Не удалось загрузить текстуру: %s\n", path);
        fprintf(stderr, "   Причина: %s\n", stbi_failure_reason());
    }

    stbi_image_free(data);
    return texture;
}

// Вспомогательная функция для загрузки модели с проверкой нескольких путей
bool load_obj_model_try_paths(const char** paths, int path_count, OBJModel* model, const char* model_name) {
    for (int i = 0; i < path_count; i++) {
        if (load_obj_model(paths[i], model)) {
            return true;
        }
    }
    fprintf(stderr, "не удалось загрузить %s ни из одного пути!\n", model_name);
    return false;
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


float clamp(float x, float low, float high) {
    if (x < low) {
        return low;
    }
    if (x > high) {
        return high;
    }
    return x;
}

float lerp_angle(float a, float b, float t) {
    float diff = b - a;

    while (diff > PI)  diff -= 2.0f * PI;
    while (diff < -PI) diff += 2.0f * PI;

    return a + diff * t;
}

float randf(float min, float max) {
    return min + (float)rand() / (float)RAND_MAX * (max - min);
}

bool can_place(Vector3 p, float radius) {
    for (int i = 0; i < placed_count; i++) {
        Vector3 d = vec3_subtract(p, all_positions[i]);
        float dist2 = d.x * d.x + d.z * d.z;

        float min_dist = radius + all_radii[i];
        if (dist2 < min_dist * min_dist)
            return false;
    }
    return true;
}

void create_terrain(int size, float scale) {
    const int VERTS = size * size;
    const int INDS = (size - 1) * (size - 1) * 6;

    float* vertices = malloc(sizeof(float) * VERTS * 5);
    unsigned int* indices = malloc(sizeof(unsigned int) * INDS);

    int v = 0;
    for (int z = 0; z < size; z++) {
        for (int x = 0; x < size; x++) {
            float half = (size - 1) * scale * 0.5f;

            vertices[v++] = (float)x * scale - half;
            vertices[v++] = 0.0f;
            vertices[v++] = (float)z * scale - half;
            vertices[v++] = (float)x / (size - 1);
            vertices[v++] = (float)z / (size - 1);
        }
    }

    int i = 0;
    for (int z = 0; z < size - 1; z++) {
        for (int x = 0; x < size - 1; x++) {
            int a = z * size + x;
            int b = a + size;
            int c = a + 1;
            int d = b + 1;

            indices[i++] = a;
            indices[i++] = b;
            indices[i++] = c;
            indices[i++] = c;
            indices[i++] = b;
            indices[i++] = d;
        }
    }

    terrain_index_count = INDS;

    glGenVertexArrays(1, &terrain_vao);
    glGenBuffers(1, &terrain_vbo);
    glGenBuffers(1, &terrain_ebo);

    glBindVertexArray(terrain_vao);

    glBindBuffer(GL_ARRAY_BUFFER, terrain_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * VERTS * 5, vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrain_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * INDS, indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    free(vertices);
    free(indices);
}

GLuint load_heightmap_texture(const char* path)
{
    GLuint texture = 0;

    stbi_set_flip_vertically_on_load(false);

    terrain_heightmap = stbi_load(path,&terrain_w,&terrain_h, NULL,STBI_grey);

    if (!terrain_heightmap) {
        fprintf(stderr, "FAILED TO LOAD HEIGHTMAP: %s\n", path);
        fprintf(stderr, "STB ERROR: %s\n", stbi_failure_reason());
        return 0;
    }

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D,0,GL_R8,terrain_w,terrain_h,0,GL_RED,GL_UNSIGNED_BYTE,terrain_heightmap);

    return texture;
}

float terrain_height(float x, float z)
{
    float terrain_world_size = (terrain_size - 1) * terrain_scale;
    float half = terrain_world_size * 0.5f;

    float u = (x + half) / terrain_world_size;
    float v = (z + half) / terrain_world_size;

    if (u < 0.0f || u > 1.0f || v < 0.0f || v > 1.0f)
        return 0.0f;

    float hx = u * (terrain_w - 1);
    float hz = v * (terrain_h - 1);

    int x0 = (int)hx;
    int z0 = (int)hz;

    int index = z0 * terrain_w + x0;
    float h = terrain_heightmap[index] / 255.0f;

    return h * terrain_height_scale;
}