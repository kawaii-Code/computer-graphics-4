#include "polygon.h"
#include "utils.h"

Polygon polygon_create() {
    Polygon p;
    vector_init(p.vertices);
    vector_reserve(p.vertices, 3);
    p.convex = true;

    return p;
}

void polygon_free(Polygon p) {
    vector_free(p.vertices);
}

void polygon_draw(Polygon p) {
    const VECTOR_TYPE(Point) vert = p.vertices;

    if (!vert.len) return;

    Point prev = vector_get(vert, 0);
    for (size_t i = 1; i < vert.len; i++) {
        Point cur = vector_get(vert, i);
        DrawLine(prev.x, prev.y, cur.x, cur.y, GRAY);
        DrawCircle(prev.x, prev.y, 4, BLACK);
        DrawCircle(cur.x, cur.y, 4, BLACK);
        prev = cur;
    }

    Point beg = vector_get(vert, 0);
    Point end = vector_get(vert, vert.len - 1);

    DrawLine(beg.x, beg.y, end.x, end.y, GRAY);
    DrawCircle(beg.x, beg.y, 4, BLACK);
    DrawCircle(end.x, end.y, 4, BLACK);
}

bool _polygon_is_convex(Polygon p) {
    const VECTOR_TYPE(Point) vert = p.vertices;
    int n = vert.len;
    if (n < 3) return false;

    int sign = 0;
    for (int i = 0; i < n; i++) {
        Point a = vector_get(vert, i);
        Point b = vector_get(vert, (i + 1) % n);
        Point c = vector_get(vert, (i + 2) % n);

        Edge edge = { a, b };
        bool is_right = edge_is_point_right(edge, c);
        int current_sign = is_right ? -1 : 1;

        if (sign == 0) sign = current_sign;
        else if (sign != current_sign) return false;
    }
    return true;
}

void polygon_add_vertice(Polygon* p, Point pt) {
    vector_append(p->vertices, pt);
    if (p->convex && !_polygon_is_convex(*p)) {
        p->convex = false;
    }
}

//bool polygon_do_edges_intersect(Polygon p) {
//    const VECTOR_TYPE(Point) vert = p.vertices;
//
//    if (vert.len < 4) return false;
//
//    Edge prev = (Edge){vector_get(vert, 0), vector_get(vert, 1)};
//    for (size_t i = 2; i < vert.len; i++) {
//        Edge cur = (Edge){vector_get(vert, i - 1), vector_get(vert, i)};
//        if (edge_intersection(prev, cur).flag) {
//            return true;
//        }
//    }
//
//    return false;
//}

//bool polygon_do_edges_intersect_new(Polygon p, Point pt) {
//    const VECTOR_TYPE(Point) vert = p.vertices;
//
//    if (vert.len < 3) return false;
//
//    Edge newEdge = (Edge){vector_get(vert, vert.len-1), pt};
//    for (size_t i = 1; i < vert.len; i++) {
//        Edge cur = (Edge){vector_get(vert, i-1), vector_get(vert, i)};
//        if (edge_intersection(newEdge, cur).flag) {
//            return true;
//        }
//    }
//
//    return false;
//}

bool polygon_do_edges_intersect_new(Polygon p, Point pt) {
    const VECTOR_TYPE(Point) vert = p.vertices;

    if (vert.len < 2) return false;

    Edge newEdge = { vector_get(vert, vert.len - 1), pt };

    for (size_t i = 0; i < vert.len - 2; i++) {
        Edge existingEdge = { vector_get(vert, i), vector_get(vert, i + 1) };

        if (edge_intersection(newEdge, existingEdge).flag) {
            return true;
        }
    }

    if (vert.len >= 3) {
        Edge closingEdge = { pt, vector_get(vert, 0) };
        for (size_t i = 1; i < vert.len - 1; i++) {
            Edge existingEdge = { vector_get(vert, i), vector_get(vert, i + 1) };
            if (edge_intersection(closingEdge, existingEdge).flag) {
                return true;
            }
        }
    }

    return false;
}

bool _polygon_contains_convex(Polygon p, Point pt) {
    const VECTOR_TYPE(Point) vert = p.vertices;
    int n = vert.len;
    if (n < 3) return false;

    int sign = 0;
    for (int i = 0; i < n; i++) {
        Point a = vector_get(vert, i);
        Point b = vector_get(vert, (i + 1) % n);

        Edge edge = { a, b };
        bool is_right = edge_is_point_right(edge, pt);
        int current_sign = is_right ? -1 : 1;

        if (sign == 0) sign = current_sign;
        else if (sign != current_sign) return false;
    }
    return true;
}

bool _polygon_contains_not_convex(Polygon p, Point pt) {
    const VECTOR_TYPE(Point) vert = p.vertices;
    int n = vert.len;
    if (n < 3) return false;

    bool inside = false;
    for (int i = 0, j = n - 1; i < n; j = i++) {
        Point vi = vector_get(vert, i);
        Point vj = vector_get(vert, j);

        if (((vi.y > pt.y) != (vj.y > pt.y)) &&
            (pt.x < (vj.x - vi.x) * (pt.y - vi.y) / (vj.y - vi.y) + vi.x)) {
            inside = !inside;
        }
    }
    return inside;
}

bool polygon_contains(Polygon p, Point pt) {
    if (p.convex) {
        return _polygon_contains_convex(p, pt);
    }

    return _polygon_contains_not_convex(p, pt);
}