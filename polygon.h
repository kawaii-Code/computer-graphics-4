#ifndef POLYGON_H
#define POLYGON_H

#include "vector.h"
#include "third_party/include/raylib.h"

typedef Vector2 Point;

typedef struct Polygon {
    vector(Point) vertices;
    bool convex;
} Polygon;

typedef struct Diagonal {
    Point p1;
    Point p2;
} Diagonal;

vector(Polygon);

Polygon polygon_create();
void polygon_free(Polygon p);

void polygon_draw(Polygon p, bool highlighted);
void polygon_add_vertice();

bool polygon_do_edges_intersect(Polygon p);
bool polygon_do_edges_intersect_new(Polygon p, Point pt);
bool polygon_contains(Polygon p, Point pt);

vector(Diagonal);
void polygon_triangulate(Polygon* p, VECTOR_TYPE(Diagonal)* diagonals);

#endif //POLYGON_H
