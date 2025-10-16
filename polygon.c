#include "polygon.h"
#include "stack.h"
#include "utils.h"

float line_thickness = 3.0f;
float vertex_radius  = 7.0f;

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

void polygon_draw(Polygon p, bool highlighted) {
    const VECTOR_TYPE(Point) vert = p.vertices;

    if (!vert.len) return;

    Color line_color = (p.convex ? PINK : (highlighted ? GRAY : DARKGRAY));

    Point prev = vector_get(vert, 0);
    for (size_t i = 1; i < vert.len; i++) {
        Point cur = vector_get(vert, i);
        Vector2 start = {prev.x, prev.y};
        Vector2 end = {cur.x, cur.y};
        DrawLineEx(start, end, line_thickness, line_color);
        DrawCircle(prev.x, prev.y, vertex_radius, BLACK);
        DrawCircle(cur.x, cur.y, vertex_radius, BLACK);
        prev = cur;
    }

    Point beg = vector_get(vert, 0);
    Point end = vector_get(vert, vert.len - 1);

    DrawLineEx((Vector2){beg.x, beg.y}, (Vector2){end.x, end.y}, line_thickness, line_color);
    DrawCircle(beg.x, beg.y, vertex_radius, highlighted ? RED : MAROON);
    DrawCircle(end.x, end.y, vertex_radius, highlighted ? LIME : DARKGREEN);
}

bool _polygon_is_convex(Polygon p) {
    const VECTOR_TYPE(Point) vert = p.vertices;
    int n = vert.len;
    if (n < 3) return true;

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

int _left_turn(Point a, Point b, Point c) {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

bool _is_valid_diagonal(Point current, Point top, Point last) {
    return _left_turn(current, top, last) > 0;  // Левый поворот = валидно
}

int _cmp_point_y(const void* a, const void* b) {
    Point arg1 = *(const Point*)a;
    Point arg2 = *(const Point*)b;

    if (arg1.y < arg2.y) return 1;
    if (arg1.y > arg2.y) return -1;
    return 0;
}

int _cmp_not_point_y(const void* a, const void* b) {
    return -_cmp_point_y(a, b);
}
// Minimal-fix version of your polygon_triangulate that keeps your style but fixes bugs.
// Assumptions:
// - Point has fields x,y
// - p->vertices is a vector of Point in polygon order
// - VECTOR_TYPE(Diagonal) *diagonals is an initialized vector to append to
// - _is_valid_diagonal(Point a, Point b, Point c) exists and returns bool
// - stack(Point*)/stack_init/stack_push/stack_pop/stack_top behave as in your code
// - vector_* helpers behave as in your code

typedef struct {
    Point p;
    int orig_idx; // index in original polygon vertex order
    int chain;    // 0 = left chain, 1 = right chain (assigned below)
} SPoint;

static int _cmp_sp_desc_y(const void *a, const void *b) {
    const SPoint *A = a;
    const SPoint *B = b;
    if (A->p.y < B->p.y) return 1;
    if (A->p.y > B->p.y) return -1;
    if (A->p.x < B->p.x) return -1;
    if (A->p.x > B->p.x) return 1;
    return 0;
}

void polygon_triangulate(Polygon* p, VECTOR_TYPE(Diagonal)* diagonals) {
    size_t n = p->vertices.len;
    if (n < 3) return;

    SPoint *arr = malloc(sizeof(SPoint) * n);
    if (!arr) return;
    for (size_t i = 0; i < n; ++i) {
        arr[i].p = vector_get(p->vertices, i);
        arr[i].orig_idx = (int)i;
        arr[i].chain = -1;
    }

    int top = 0, bottom = 0;
    for (int i = 1; i < (int)n; ++i) {
        Point pi = vector_get(p->vertices, i);
        Point ptop = vector_get(p->vertices, top);
        Point pbot = vector_get(p->vertices, bottom);
        if (pi.y > ptop.y || (pi.y == ptop.y && pi.x < ptop.x)) top = i;
        if (pi.y < pbot.y || (pi.y == pbot.y && pi.x > pbot.x)) bottom = i;
    }

    int idx = top;
    while (idx != bottom) {
        arr[idx].chain = 0;
        idx = (idx + 1) % (int)n;
    }
    arr[bottom].chain = 0;

    idx = top;
    while (idx != bottom) {
        arr[idx].chain = 1;
        idx = (idx - 1 + (int)n) % (int)n;
    }
    arr[bottom].chain = 1;

    SPoint *sorted = malloc(sizeof(SPoint) * n);
    if (!sorted) { free(arr); return; }
    for (size_t i = 0; i < n; ++i) sorted[i] = arr[i];
    qsort(sorted, n, sizeof(SPoint), _cmp_sp_desc_y);

    stack(SPoint) st;
    stack_init(st);

    stack_push(st, &sorted[0]);
    stack_push(st, &sorted[1]);

    for (size_t j = 2; j < n - 1; ++j) {
        SPoint *vj = &sorted[j];
        SPoint *peek = stack_top(st);

        if (vj->chain != peek->chain) {
            while (st.top != -1) {
                if (st.top != 0) {
                    Diagonal diag;
                    diag.p1 = vj->p;
                    diag.p2 = stack_top(st)->p;
                    vector_append(*diagonals, diag);
                }
                stack_pop(st);
            }
            stack_push(st, &sorted[j - 1]);
            stack_push(st, vj);
        } else {
            SPoint *last = stack_pop(st);
            while (st.top != -1) {
                SPoint *second = stack_top(st);
                if (_is_valid_diagonal(vj->p, second->p, last->p)) {
                    Diagonal diag;
                    diag.p1 = vj->p;
                    diag.p2 = second->p;
                    vector_append(*diagonals, diag);

                    last = stack_pop(st);
                    if (last == NULL) break;
                } else {
                    break;
                }
            }
            stack_push(st, last);
            stack_push(st, vj);
        }
    }

    stack_pop(st);
    while (st.top != -1) {
        if (st.top != 0) {
            Diagonal diag;
            diag.p1 = sorted[n - 1].p;
            diag.p2 = stack_top(st)->p;
            vector_append(*diagonals, diag);
        }
        stack_pop(st);
    }

    free(arr);
    free(sorted);
}
