#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#include "tree.h"
#include "difftor.h"
#include "difftor_dump.h"

extern node_t* HEAD;
extern FILE* DUMP_FILE;


double tree_sum(node_t* node, var_t** variables, int var_count, bool need_var_value)
{
    if (node == NULL)
        return NAN;

    if (var_count > 0 && isnan(variables[0]->value) && need_var_value)
        request_variable_values(variables, var_count);


    if (node->type == NUMBER)
        return node->data.number;



    if (node->type == VARIABLE)
    {
        if (need_var_value)
            return get_variable_value(node->data.variable, variables, var_count);
        else
            return NAN;
    }

    if (node->type == OPERATOR)
    {
        switch(node->data.oper)
        {
            case ADD:
                return tree_sum(node->left, variables, var_count) + tree_sum(node->right, variables, var_count);
            case SUB:
                return tree_sum(node->left, variables, var_count) - tree_sum(node->right, variables, var_count);
            case MUL:
                return tree_sum(node->left, variables, var_count) * tree_sum(node->right, variables, var_count);
            case DIV:
                {
                    double right_val = tree_sum(node->right, variables, var_count);
                    if (right_val == 0)
                        return NAN;

                    return tree_sum(node->left, variables, var_count) / right_val;
                }
            case POW:
                return pow(tree_sum(node->left, variables, var_count), tree_sum(node->right, variables, var_count));
            // case SIN:
            //     return sin(tree_sum(node->right, variables, var_count));
            // case COS:
            //     return cos(tree_sum(node->right, variables, var_count));
            // case LN:
            //     {
            //         double arg = tree_sum(node->right, variables, var_count);
            //         if (arg <= 0)
            //             return NAN;
            //         return log(arg);
            //     }
            // case EXP:
            //     return exp(tree_sum(node->right, variables, var_count));
            default:
                return NAN;
        }
    }

    return NAN;
}

node_t* tree_copy(node_t* node)
{
    if (node == NULL)
        return NULL;

    node_t* new_node = (node_t*)calloc(1, sizeof(node_t));
    if (!new_node) return NULL;

    new_node->type = node->type;
    new_node->left = NULL;
    new_node->right = NULL;
    new_node->parent = NULL;

    switch (node->type)
    {
        case NUMBER:
            new_node->data.number = node->data.number;
            break;

        case OPERATOR:
            new_node->data.oper = node->data.oper;
            break;

        case VARIABLE:
            if (node->data.variable)
            {
                size_t len = strlen(node->data.variable);
                new_node->data.variable = (char*)calloc(len + 1, sizeof(char));
                if (new_node->data.variable)
                {
                    strcpy(new_node->data.variable, node->data.variable);
                }
            }
            else
            {
                new_node->data.variable = NULL;
            }
            break;
    }

    new_node->left = tree_copy(node->left);
    if (new_node->left != NULL)
        new_node->left->parent = new_node;

    new_node->right = tree_copy(node->right);
    if (new_node->right != NULL)
    {
        new_node->right->parent = new_node;
    }

    return new_node;
}

node_t* create_number_node(double value)
{
    char buffer[64];
    sprintf(buffer, "%.2lf", value);
    return node_ctor(NUMBER, buffer);
}

node_t* create_operator_node(const char* oper, node_t* left, node_t* right)
{
    return make_new_node_with_sons(OPERATOR, oper, left, right);
}

#define d(tree, var) \
    tree_diff(tree, var)

#define c(tree) \
    tree_copy(tree)

#define L \
    node->left

#define R \
    node->right

node_t* tree_diff(node_t* node, const char* var)
{
    assert(node);

    if (node->type == NUMBER)
        return create_number_node(0);

    else if (node->type == VARIABLE)
    {
        if (strcmp(var, node->data.variable) == 0)
            return create_number_node(1);
        else
            return create_number_node(0);
    }

    else if (node->type == OPERATOR)
    {
        switch(node->data.oper)
        {
            case MUL:
                return ADD_(MUL_(d(L, var), c(R)), MUL_(d(R, var), c(L)));

            case ADD:
                return ADD_(d(L, var), d(R, var));

            case SUB:
                return SUB_(d(L, var), d(R, var));

            case DIV:
                return DIV_(SUB_(MUL_(d(L, var), c(R)), MUL_(c(L), d(R, var))), MUL_(c(R), c(R)));

            case POW:
                {
                    if (!is_variable_in_tree(node->right, var))
                    {
                        node_t* node1 = create_number_node(1);
                        return MUL_(MUL_(c(R), POW_(c(L), SUB_(c(R), node1))), d(L, var));
                    }

                    else if (!is_variable_in_tree(node->left, var))
                    {
                        node_t* ln_node = create_operator_node("ln", NULL, c(L));
                        return MUL_(MUL_(POW_(c(L), c(R)), ln_node), d(R, var));
                    }

                    else
                    {
                        node_t* ln_node = create_operator_node("ln", NULL, c(L));
                        return MUL_(POW_(c(L), c(R)), ADD_(MUL_(d(R, var), ln_node), MUL_(c(R), DIV_(d(L, var), c(L)))));
                    }
                }

            case SIN:
                {
                    node_t* cos_node = create_operator_node("cos", NULL, c(R));
                    return MUL_(cos_node, d(R, var));
                }

            case COS:
                {
                    node_t* sin_node = create_operator_node("sin", NULL, c(R));
                    node_t* neg_sin = MUL_(create_number_node(-1), sin_node);
                    return MUL_(neg_sin, d(R, var));
                }

            case LN:
                {
                    node_t* ln_node = DIV_(create_number_node(1), c(R));
                    return MUL_(ln_node, d(R, var));
                }

            case EXP:
                {
                    node_t* exp_node = create_operator_node("exp", NULL, c(R));
                    return MUL_(exp_node, d(R, var));
                }

            default:
                return create_number_node(0);
        }
    }

    return create_number_node(0);
}

// #undef ADD_
// #undef SUB_
// #undef MUL_
// #undef DIV_
#undef c
#undef d
#undef L
#undef R

node_t* make_new_node_with_sons(enum node_type type, const char* data, node_t* left_son, node_t* right_son)
{
    node_t* new_node = node_ctor(type, data);

    new_node->left  = left_son;
    new_node->right = right_son;

    if (left_son)
        new_node->left->parent = new_node;

    if (right_son)
        new_node->right->parent = new_node;

    return new_node;
}

bool is_variable_in_tree (node_t* node, const char* var)
{
    if (node->type == VARIABLE)
    {
        if (strcmp(node->data.variable, var) == 0 || strcmp(var, "all") == 0)
            return true;
    }

    if (node->left)
    {
        if (is_variable_in_tree(node->left, var))
            return true;
    }

    if (node->right)
    {
        if (is_variable_in_tree(node->right, var))
            return true;
    }

    return false;
}

bool simplify_tree(node_t* node)
{
    if (!node) return false;

    bool changed = false;
    bool made_change = false;
    int safety_counter = 0;
    const int MAX_ITERATIONS = 1000;

    do {
        changed = false;
        safety_counter++;

        if (node->left)
        {
            if (simplify_tree(node->left))
            {
                changed = true;
                made_change = true;
                continue;
            }
        }

        if (node->right)
        {
            if (simplify_tree(node->right))
            {
                changed = true;
                made_change = true;
                continue;
            }
        }

        if (constant_folding(node))
        {
            changed = true;
            made_change = true;
            continue;
        }

        if (delete_extra_nodes(node))
        {
            changed = true;
            made_change = true;
            continue;
        }

    } while (changed && safety_counter < MAX_ITERATIONS);

    if (safety_counter >= MAX_ITERATIONS)
    {
        fprintf(stderr, "Warning: simplify_tree reached max iterations\n");
    }

    return made_change;
}

bool constant_folding(node_t* node)
{
    if (!node || node->type != OPERATOR) return false;

    double tree_value = tree_sum(node, NULL, 0, false);
    if (!isnan(tree_value)) {
        node_to_number(node, tree_value);
        return true;
    }

    return false;
}

bool delete_extra_nodes(node_t* node)
{
    if (!node || node->type != OPERATOR) return false;

    bool changed = false;

    switch(node->data.oper)
    {
        case MUL:
            if ((node->left && node->left->type == NUMBER && fabs(node->left->data.number) < LOW_NUMBER) ||
                (node->right && node->right->type == NUMBER && fabs(node->right->data.number) < LOW_NUMBER))
            {
                node_to_number(node, 0);
                return true;
            }
            if (node->left && node->left->type == NUMBER && fabs(node->left->data.number - 1) < LOW_NUMBER)
            {
                replace_with_child(node, node->right);
                return true;
            }
            if (node->right && node->right->type == NUMBER && fabs(node->right->data.number - 1) < LOW_NUMBER)
            {
                replace_with_child(node, node->left);
                return true;
            }
            break;

        case ADD:
            if (node->left && node->left->type == NUMBER && fabs(node->left->data.number) < LOW_NUMBER)
            {
                replace_with_child(node, node->right);
                return true;
            }
            if (node->right && node->right->type == NUMBER && fabs(node->right->data.number) < LOW_NUMBER)
            {
                replace_with_child(node, node->left);
                return true;
            }
            break;

        case SUB:
            if (node->right && node->right->type == NUMBER && fabs(node->right->data.number) < LOW_NUMBER)
            {
                replace_with_child(node, node->left);
                return true;
            }
            break;

        case DIV:
            if (node->right && node->right->type == NUMBER && fabs(node->right->data.number - 1) < LOW_NUMBER)
            {
                replace_with_child(node, node->left);
                return true;
            }
            // 0 / что-то = 0
            if (node->left && node->left->type == NUMBER && fabs(node->left->data.number) < LOW_NUMBER)
            {
                node_to_number(node, 0);
                return true;
            }
            break;

        case POW:
            if (node->right && node->right->type == NUMBER && fabs(node->right->data.number) < LOW_NUMBER)
            {
                node_to_number(node, 1);
                return true;
            }
            if (node->right && node->right->type == NUMBER && fabs(node->right->data.number - 1) < LOW_NUMBER)
            {
                replace_with_child(node, node->left);
                return true;
            }
            break;

        default:
            break;
    }

    return false;
}

void replace_with_child(node_t* node, node_t* child)
{
    if (!node || !child) return;

    node_t* parent = node->parent;

    node->type = child->type;

    switch (child->type)
    {
        case NUMBER:
            node->data.number = child->data.number;
            break;
        case OPERATOR:
            node->data.oper = child->data.oper;
            break;
        case VARIABLE:
            if (node->data.variable)
                free(node->data.variable);

            if (child->data.variable)
                node->data.variable = strdup(child->data.variable);

            else
                node->data.variable = NULL;

            break;
    }

    node_t* old_left = node->left;
    node_t* old_right = node->right;

    node->left = child->left ? tree_copy(child->left) : NULL;
    node->right = child->right ? tree_copy(child->right) : NULL;

    if (node->left) node->left->parent = node;
    if (node->right) node->right->parent = node;

    node->parent = parent;

    if (old_left)
        tree_dtor(old_left);
    if (old_right)
        tree_dtor(old_right);
}

void node_to_number(node_t* node, double value)
{
    if (!node)
        return;

    if (node->type == VARIABLE && node->data.variable) {
        free(node->data.variable);
    }

    node->type = NUMBER;
    node->data.number = value;

    if (node->left)
    {
        tree_dtor(node->left);
        node->left = NULL;
    }

    if (node->right)
    {
        tree_dtor(node->right);
        node->right = NULL;
    }
}
