#ifndef LSYSTEM_H
#define LSYSTEM_H

#include "third_party/include/raylib.h"
#include "stack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>

#define MAX_STRING_LENGTH 50000
#define MAX_RULES 10
#define MAX_STACK_SIZE 1000

typedef struct {
    char predecessor;
    char successor[100];
} ProductionRule;

typedef struct {
    Vector2 position;
    float angle;
} TurtleState;

typedef struct {
    TurtleState data[MAX_STACK_SIZE];
    int top;
} TurtleStack;

typedef struct {
    char axiom[100];
    float angle;
    float initial_angle;
    float step;
    ProductionRule rules[MAX_RULES];
    int rule_count;
    TurtleStack stack;
} LSystem;

void lsystem_init(LSystem* ls);
bool lsystem_load_from_file(LSystem* ls, const char* filename);
void lsystem_generate_string(LSystem* ls, char* result, int iterations);
void lsystem_draw(LSystem* ls, const char* lstring, int width, int height);
void lsystem_free(LSystem* ls);

void turtle_stack_init(TurtleStack* stack);
bool turtle_stack_push(TurtleStack* stack, TurtleState state);
TurtleState turtle_stack_pop(TurtleStack* stack);
bool turtle_stack_is_empty(TurtleStack* stack);

#endif