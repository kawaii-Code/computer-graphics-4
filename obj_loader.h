#ifndef OBJ_LOADER_H
#define OBJ_LOADER_H

#include <stdbool.h>
#include "linalg.h"

typedef struct {
    Vector3 position;
    Vector2 tex_coord;
} OBJVertex;

typedef struct {
    OBJVertex *vertices;
    int vertex_count;
    unsigned int vao;
    unsigned int vbo;
} OBJModel;

// Загружает модель из OBJ файла
bool load_obj_model(const char *filename, OBJModel *model);

// Освобождает память модели
void free_obj_model(OBJModel *model);

// Создаёт VAO/VBO для модели
void setup_obj_model_buffers(OBJModel *model);

#endif // OBJ_LOADER_H

