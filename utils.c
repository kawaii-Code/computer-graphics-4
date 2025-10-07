#include "utils.h"
#include <math.h>

Intersection edge_intersection(Edge e1, Edge e2) {
    float dx1 = e1.p2.x - e1.p1.x, dy1 = e1.p2.y - e1.p1.y;
    float dx2 = e2.p2.x - e2.p1.x, dy2 = e2.p2.y - e2.p1.y;

    float det = dx1 * dy2 - dx2 * dy1;
    if (fabs(det) < 1e-6f) return (Intersection) { { 0, 0 }, false };

    float dx = e2.p1.x - e1.p1.x, dy = e2.p1.y - e1.p1.y;
    float t = (dx * dy2 - dy * dx2) / det;
    float u = (dx * dy1 - dy * dx1) / det;

    if (t >= 0.0f && t <= 1.0f && u >= 0.0f && u <= 1.0f) {
        return (Intersection) { { e1.p1.x + t * dx1, e1.p1.y + t * dy1 }, true };
    }

    return (Intersection) { { 0, 0 }, false };
}

bool edge_is_point_right(Edge e, Point pt) {
    Vector2 edge = { e.p2.x - e.p1.x, e.p2.y - e.p1.y };
    Vector2 to_point = { pt.x - e.p1.x, pt.y - e.p1.y };

    float cross = edge.x * to_point.y - edge.y * to_point.x;
    return cross < 0;
}
