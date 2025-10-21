#include "L-system.h"

void turtle_stack_init(TurtleStack* stack) {
    stack->top = -1;
}

bool turtle_stack_push(TurtleStack* stack, TurtleState state) {
    if (stack->top >= MAX_STACK_SIZE - 1) {
        return false;
    }
    stack->data[++stack->top] = state;
    return true;
}

TurtleState turtle_stack_pop(TurtleStack* stack) {
    if (stack->top < 0) {
        TurtleState empty = { {0, 0}, 0 };
        return empty;
    }
    return stack->data[stack->top--];
}

bool turtle_stack_is_empty(TurtleStack* stack) {
    return stack->top < 0;
}

void lsystem_init(LSystem* ls) {
    strcpy(ls->axiom, "F");
    ls->angle = 25.0f;
    ls->initial_angle = -90.0f;
    ls->step = 10.0f;
    ls->rule_count = 0;
    ls->pos = (Vector2){ 0.0f, 0.0f };
    ls->max_iter = 4;
    ls->angle_variation = 9.0f;    
    ls->angle_variations = NULL; 
    ls->variations_count = 0;
    ls->use_random = false;
    turtle_stack_init(&ls->stack);
}

bool lsystem_load_from_file(LSystem* ls, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error: Cannot open file %s\n", filename);
        return false;
    }

    char line[256];
    ls->rule_count = 0;

    if (fgets(line, sizeof(line), file)) {
        char axiom[10];
        float angle, initial_angle;
        float pos_x, pos_y, step;
        int iter;

        if (sscanf(line, "%s %f %f %f %f %f %d", axiom, &angle, &initial_angle, &step, &pos_x, &pos_y,&iter) == 7) {
            strcpy(ls->axiom, axiom);
            ls->angle = angle;
            ls->initial_angle = initial_angle;
            ls->step = step;
            ls->pos = (Vector2){ pos_x,pos_y };
            ls->max_iter = iter;
        }
    }
    
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;

        char* arrow = strstr(line, "->");
        if (arrow && ls->rule_count < MAX_RULES) {
            char predecessor = line[0];
            char* successor = arrow + 2; 

            ls->rules[ls->rule_count].predecessor = predecessor;
            strcpy(ls->rules[ls->rule_count].successor, successor);
            ls->rule_count++;
        }
    }

    fclose(file);
    return true;
}

void lsystem_generate_string(LSystem* ls, char* result, int iterations) {
    char* current = (char*)malloc(MAX_STRING_LENGTH * sizeof(char));
    char* next = (char*)malloc(MAX_STRING_LENGTH * sizeof(char));

    if (!current || !next) {
        printf("Error: Memory allocation failed!\n");
        if (current) free(current);
        if (next) free(next);
        return;
    }

    strcpy(current, ls->axiom);

    for (int iter = 0; iter < iterations; iter++) {
        next[0] = '\0';

        for (int i = 0; i < strlen(current); i++) {
            char symbol = current[i];
            bool rule_applied = false;

            for (int r = 0; r < ls->rule_count; r++) {
                if (ls->rules[r].predecessor == symbol) {
                    strcat(next, ls->rules[r].successor);
                    rule_applied = true;
                    break;
                }
            }

            if (!rule_applied) {
                strncat(next, &symbol, 1);
            }
        }

        strcpy(current, next);
    }

    strcpy(result, current);
}

float my_random_range(float min, float max) {
    return min + ((float)rand() / RAND_MAX) * (max - min);
}

void lsystem_regenerate_variations(LSystem* ls, const char* lstring) {
    if (ls->angle_variations) {
        free(ls->angle_variations);
        ls->angle_variations = NULL;
    }

    if (!lstring || strlen(lstring) == 0) {
        ls->variations_count = 0;
        return;
    }

    int len = strlen(lstring);
    ls->angle_variations = (float*)malloc(len * sizeof(float));

    if (!ls->angle_variations) {
        ls->variations_count = 0;
        return;
    }

    ls->variations_count = len;

    for (int i = 0; i < len; i++) {
        ls->angle_variations[i] = my_random_range(-ls->angle_variation, ls->angle_variation);
    }

}

void lsystem_draw(LSystem* ls, const char* lstring, int width, int height) {
    if (strlen(lstring) == 0) {
        return;
    }

    float base_step = ls->step;
    Vector2 current_pos = ls->pos;
    float current_angle = ls->initial_angle;

    turtle_stack_init(&ls->stack);
    int variation_index = 0;

    for (int i = 0; i < strlen(lstring); i++) {
        char symbol = lstring[i];

        float angle_var = 0.0f;

        if (ls->use_random &&
            ls->angle_variations != NULL &&
            variation_index < ls->variations_count &&
            (symbol == '+' || symbol == '-')) {

            angle_var = ls->angle_variations[variation_index];
            variation_index++;
        }


        switch (symbol) {
        case 'F': 
        {
            Vector2 new_pos = {
                current_pos.x + cosf(current_angle * DEG2RAD) * base_step,
                current_pos.y + sinf(current_angle * DEG2RAD) * base_step
            };

            DrawLineEx(current_pos, new_pos, 3.0f, BLACK);
            current_pos = new_pos;
            break;
        }

        case 'G': 
        {
            Vector2 new_pos = {
                current_pos.x + cosf(current_angle * DEG2RAD) * base_step,
                current_pos.y + sinf(current_angle * DEG2RAD) * base_step
            };
            current_pos = new_pos;
            break;
        }
        
        case '+': 
            current_angle += ls->angle + angle_var;
            break;

        case '-': 
            current_angle -= ls->angle + angle_var;
            break;

        case '[': 
        {
            TurtleState state;
            state.position = current_pos;
            state.angle = current_angle;
            turtle_stack_push(&ls->stack, state);
            break;
        }

        case ']': 
        {
            if (!turtle_stack_is_empty(&ls->stack)) {
                TurtleState state = turtle_stack_pop(&ls->stack);
                current_pos = state.position;
                current_angle = state.angle;
            }
            break;
        }

        default:
            break;
        }
    }
}

void lsystem_free(LSystem* ls) {
    if (ls->angle_variations) {
        free(ls->angle_variations);
        ls->angle_variations = NULL;
    }
    ls->variations_count = 0;
}