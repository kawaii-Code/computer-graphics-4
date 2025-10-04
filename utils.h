#ifndef UTILS_H
#define UTILS_H

#include "polygon.h"

typedef struct Edge {
    Point p1;
    Point p2;
} Edge;

typedef struct Intersection {
    Point intersection;
    bool flag;
} Intersection;

Intersection edge_intersection(Edge e1, Edge e2);


bool edge_is_point_right(Edge e, Point pt);

#endif //UTILS_H
