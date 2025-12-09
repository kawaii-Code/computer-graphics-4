#include "obj_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <glad/glad.h>

typedef struct {
    Vector3 *positions;
    int position_count;
    int position_capacity;

    Vector2 *tex_coords;
    int tex_coord_count;
    int tex_coord_capacity;

    OBJVertex *vertices;
    int vertex_count;
    int vertex_capacity;
} OBJData;

static void init_obj_data(OBJData *data) {
    data->position_capacity = 1024;
    data->positions = malloc(sizeof(Vector3) * data->position_capacity);
    data->position_count = 0;

    data->tex_coord_capacity = 1024;
    data->tex_coords = malloc(sizeof(Vector2) * data->tex_coord_capacity);
    data->tex_coord_count = 0;

    data->vertex_capacity = 4096;
    data->vertices = malloc(sizeof(OBJVertex) * data->vertex_capacity);
    data->vertex_count = 0;
}

static void free_obj_data(OBJData *data) {
    free(data->positions);
    free(data->tex_coords);
    free(data->vertices);
}

static void add_position(OBJData *data, Vector3 pos) {
    if (data->position_count >= data->position_capacity) {
        data->position_capacity *= 2;
        data->positions = realloc(data->positions, sizeof(Vector3) * data->position_capacity);
    }
    data->positions[data->position_count++] = pos;
}

static void add_tex_coord(OBJData *data, Vector2 tc) {
    if (data->tex_coord_count >= data->tex_coord_capacity) {
        data->tex_coord_capacity *= 2;
        data->tex_coords = realloc(data->tex_coords, sizeof(Vector2) * data->tex_coord_capacity);
    }
    data->tex_coords[data->tex_coord_count++] = tc;
}

static void add_vertex(OBJData *data, OBJVertex vertex) {
    if (data->vertex_count >= data->vertex_capacity) {
        data->vertex_capacity *= 2;
        data->vertices = realloc(data->vertices, sizeof(OBJVertex) * data->vertex_capacity);
    }
    data->vertices[data->vertex_count++] = vertex;
}

bool load_obj_model(const char *filename, OBJModel *model) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("📂 Текущая рабочая директория: %s\n", cwd);
    }
    printf("📄 Попытка загрузить: %s\n", filename);

    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "❌ Не удалось открыть OBJ файл: %s\n", filename);
        return false;
    }

    OBJData data;
    init_obj_data(&data);

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'v' && line[1] == ' ') {
            Vector3 pos;
            sscanf(line, "v %f %f %f", &pos.x, &pos.y, &pos.z);
            add_position(&data, pos);
        }
        else if (line[0] == 'v' && line[1] == 't') {
            Vector2 tc;
            sscanf(line, "vt %f %f", &tc.x, &tc.y);
            add_tex_coord(&data, tc);
        }
        else if (line[0] == 'f' && line[1] == ' ') {
            int indices_v[16];   // Позиции
            int indices_vt[16];  // Текстурные координаты
            int indices_vn[16];  // Нормали
            int vertex_count_in_face = 0;

            char *ptr = line + 2; // Пропускаем "f "

            while (*ptr && vertex_count_in_face < 16) {
                int v = 0, vt = 0, vn = 0;
                int scanned = 0;

                if (sscanf(ptr, "%d/%d/%d%n", &v, &vt, &vn, &scanned) == 3) {
                    indices_v[vertex_count_in_face] = v;
                    indices_vt[vertex_count_in_face] = vt;
                    indices_vn[vertex_count_in_face] = vn;
                    vertex_count_in_face++;
                    ptr += scanned;
                }
                else if (sscanf(ptr, "%d/%d%n", &v, &vt, &scanned) == 2) {
                    indices_v[vertex_count_in_face] = v;
                    indices_vt[vertex_count_in_face] = vt;
                    indices_vn[vertex_count_in_face] = 0;
                    vertex_count_in_face++;
                    ptr += scanned;
                }
                else if (sscanf(ptr, "%d//%d%n", &v, &vn, &scanned) == 2) {
                    indices_v[vertex_count_in_face] = v;
                    indices_vt[vertex_count_in_face] = 0;
                    indices_vn[vertex_count_in_face] = vn;
                    vertex_count_in_face++;
                    ptr += scanned;
                }
                else if (sscanf(ptr, "%d%n", &v, &scanned) == 1) {
                    indices_v[vertex_count_in_face] = v;
                    indices_vt[vertex_count_in_face] = 0;
                    indices_vn[vertex_count_in_face] = 0;
                    vertex_count_in_face++;
                    ptr += scanned;
                }
                else {
                    break;
                }

                while (*ptr == ' ' || *ptr == '\t') ptr++;
            }

            // Триангулируем полигон методом веера (fan triangulation)
            // Для n вершин создаём (n-2) треугольника
            if (vertex_count_in_face >= 3) {
                for (int i = 1; i < vertex_count_in_face - 1; i++) {
                    OBJVertex verts[3];

                    for (int j = 0; j < 3; j++) {
                        int idx = (j == 0) ? 0 : (j == 1) ? i : i + 1;

                        verts[j].position = data.positions[indices_v[idx] - 1];

                        if (indices_vt[idx] > 0 && indices_vt[idx] <= data.tex_coord_count) {
                            verts[j].tex_coord = data.tex_coords[indices_vt[idx] - 1];
                        } else {
                            verts[j].tex_coord = (Vector2){0.0f, 0.0f};
                        }
                    }

                    add_vertex(&data, verts[0]);
                    add_vertex(&data, verts[1]);
                    add_vertex(&data, verts[2]);
                }
            }
        }
    }

    fclose(file);

    model->vertices = data.vertices;
    model->vertex_count = data.vertex_count;
    model->vao = 0;
    model->vbo = 0;

     printf("✅ Загружено из %s:\n", filename);
    printf("   - Позиций: %d\n", data.position_count);
    printf("   - Текстурных координат: %d\n", data.tex_coord_count);
    printf("   - Вершин (после триангуляции): %d\n", model->vertex_count);
    printf("   - Треугольников: %d\n", model->vertex_count / 3);

    // Освобождаем временные данные
    free(data.positions);
    free(data.tex_coords);

    return true;
}

void free_obj_model(OBJModel *model) {
    if (model->vertices) {
        free(model->vertices);
        model->vertices = NULL;
    }

    if (model->vao) {
        glDeleteVertexArrays(1, &model->vao);
        model->vao = 0;
    }

    if (model->vbo) {
        glDeleteBuffers(1, &model->vbo);
        model->vbo = 0;
    }

    model->vertex_count = 0;
}

void setup_obj_model_buffers(OBJModel *model) {
    glGenVertexArrays(1, &model->vao);
    glGenBuffers(1, &model->vbo);

    glBindVertexArray(model->vao);
    glBindBuffer(GL_ARRAY_BUFFER, model->vbo);

    glBufferData(GL_ARRAY_BUFFER,
                 model->vertex_count * sizeof(OBJVertex),
                 model->vertices,
                 GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                         sizeof(OBJVertex),
                         (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                         sizeof(OBJVertex),
                         (void*)(sizeof(Vector3)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

