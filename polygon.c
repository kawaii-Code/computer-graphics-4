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

typedef struct {
    Point p;
    int orig_idx;
    int chain;
} SPoint;

int _left_turn(Point a, Point b, Point c) {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

static int _cmp_sp_desc_y(const void *a, const void *b) {
    const SPoint *A = a;
    const SPoint *B = b;
    if (A->p.y != B->p.y) return (A->p.y > B->p.y) ? -1 : 1;
    if (A->chain != B->chain) return A->chain - B->chain;
    return (A->p.x < B->p.x) ? -1 : 1;
}

bool _is_valid_diagonal_new(Point a, Point b, Point c, int chain) {
    int turn = _left_turn(a, b, c);

    bool valid_turn = (chain == 0) ? (turn < 0) : (turn > 0);

    Polygon tmp;
    vector_init(tmp.vertices);
    vector_append(tmp.vertices, a);
    vector_append(tmp.vertices, b);
    vector_append(tmp.vertices, c);
    bool convex_local = _polygon_is_convex(tmp);
    vector_free(tmp.vertices);

    return valid_turn && convex_local;
}

void ensure_counter_clockwise(Polygon* p) {
    size_t n = p->vertices.len;
    if (n < 3) return;

    double area = 0;
    for (size_t i = 0; i < n; i++) {
        Point p1 = vector_get(p->vertices, i);
        Point p2 = vector_get(p->vertices, (i + 1) % n);
        area += (p1.x * p2.y - p2.x * p1.y);
    }

    // If clockwise (negative area), reverse the vertices
    if (area < 0) {
        for (size_t i = 0; i < n / 2; i++) {
            Point temp = vector_get(p->vertices, i);
            p->vertices.head[i] = p->vertices.head[n - i - 1];
            p->vertices.head[n - i - 1] = temp;
        }
    }
}

void polygon_triangulate(Polygon* p, VECTOR_TYPE(Diagonal)* diagonals) {
    size_t n = p->vertices.len;
    if (n < 3) return;

    double area = 0;
    for (size_t i = 0; i < n; i++) {
        Point p1 = vector_get(p->vertices, i);
        Point p2 = vector_get(p->vertices, (i + 1) % n);
        area += (p1.x * p2.y - p2.x * p1.y);
    }

    SPoint *arr = malloc(sizeof(SPoint) * n);
    if (area < 0) {
        // Clockwise - reverse order
        for (size_t i = 0; i < n; ++i) {
            arr[i].p = vector_get(p->vertices, n - 1 - i);
            arr[i].orig_idx = n - 1 - i;
            arr[i].chain = -1;
        }
    } else {
        // Counter-clockwise - keep original order
        for (size_t i = 0; i < n; ++i) {
            arr[i].p = vector_get(p->vertices, i);
            arr[i].orig_idx = i;
            arr[i].chain = -1;
        }
    }

    int top = 0, bottom = 0;
    for (int i = 1; i < (int)n; ++i) {
        Point pi = arr[i].p;
        Point ptop = arr[top].p;
        if (pi.y > ptop.y || (pi.y == ptop.y && pi.x < ptop.x)) top = i;
    }
    for (int i = 1; i < (int)n; ++i) {
        Point pi = arr[i].p;
        Point pbot = arr[bottom].p;
        if (pi.y < pbot.y || (pi.y == pbot.y && pi.x > pbot.x)) bottom = i;
    }

    arr[top].chain = 0;
    int current = (top + 1) % (int)n;
    while (current != bottom) {
        arr[current].chain = 0;
        current = (current + 1) % (int)n;
    }

    current = (top - 1 + (int)n) % (int)n;
    while (current != bottom) {
        arr[current].chain = 1;
        current = (current - 1 + (int)n) % (int)n;
    }
    arr[bottom].chain = 1;

    SPoint *sorted = malloc(sizeof(SPoint) * n);
    memcpy(sorted, arr, sizeof(SPoint) * n);
    qsort(sorted, n, sizeof(SPoint), _cmp_sp_desc_y);

    stack(SPoint) st;
    stack_init(st);
    stack_push(st, &sorted[0]);
    stack_push(st, &sorted[1]);

    for (size_t j = 2; j < n; ++j) {
        SPoint *vj = &sorted[j];

        SPoint *last = stack_top(st);
        if (vj->chain != last->chain) {
            while (st.top > 0) {
                SPoint *u = stack_pop(st);
                Diagonal diag = { vj->p, u->p };
                vector_append(*diagonals, diag);
            }
            stack_pop(st);
            stack_push(st, &sorted[j - 1]);
            stack_push(st, vj);
        } else {
            /* same chain */
            SPoint *tmp = stack_pop(st);
            bool did_connect;
            do {
                did_connect = false;
                if (st.top >= 0) {
                    SPoint *top_sp = stack_top(st);
                    if (_is_valid_diagonal_new(tmp->p, top_sp->p, vj->p, vj->chain)) {
                        Diagonal diag = { vj->p, top_sp->p };
                        vector_append(*diagonals, diag);
                        tmp = stack_pop(st);
                        did_connect = true;
                    }
                }
            } while (did_connect && st.top >= 0);

            stack_push(st, tmp);
            stack_push(st, vj);
        }
    }

    while (st.top > 0) {
        SPoint *u = stack_pop(st);
        Diagonal diag = { sorted[n - 1].p, u->p };
        vector_append(*diagonals, diag);
    }

    free(arr);
    free(sorted);
}

