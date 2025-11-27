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

    if (var_count > 0 && need_var_value)
    {
        bool has_nan = false;
        for (int i = 0; i < var_count; i++)
        {
            if (isnan(variables[i]->value))
            {
                has_nan = true;
                break;
            }
        }

        if (has_nan)
            request_variable_values(variables, var_count);
    }

    switch(node->type)
    {
        case NUMBER:
            return node->data.number;

        case VARIABLE:
            if (need_var_value)
                return get_variable_value(node->data.variable, variables, var_count);
            else
                return NAN;

        case OPERATOR:
            return funcs[node->data.oper].calc_func(node, variables, var_count, need_var_value);
    }

    return NAN;
}

void request_variable_values(var_t** variables, int var_count)
{
    printf("Please enter values for variables:\n");
    for (int i = 0; i < var_count; i++)
    {
        printf("%s = ", variables[i]->name);

        if (scanf("%lf", &variables[i]->value) != 1)
            printf("Variable value is not found\n");
    }
}

double get_variable_value(const char* var_name, var_t** variables, int var_count)
{
    for (int i = 0; i < var_count; i++)
    {
        if (strcmp(variables[i]->name, var_name) == 0)
        {
            return variables[i]->value;
        }
    }

    return NAN;
}

void var_dtor(var_t** var, int var_count)
{
    for(int i = 0; i < var_count; i++)
    {
        if (var[i])
        {
            free(var[i]->name);
            free(var[i]);
        }
    }
}

double calc_add(node_t* node, var_t** variables, int var_count, bool need_var_value)
{
    return tree_sum(node->left, variables, var_count, need_var_value) + tree_sum(node->right, variables, var_count, need_var_value);
}

double calc_sub(node_t* node, var_t** variables, int var_count, bool need_var_value)
{
    return tree_sum(node->left, variables, var_count, need_var_value) - tree_sum(node->right, variables, var_count, need_var_value);
}

double calc_mul(node_t* node, var_t** variables, int var_count, bool need_var_value)
{
    return tree_sum(node->left, variables, var_count, need_var_value) * tree_sum(node->right, variables, var_count, need_var_value);
}

double calc_div(node_t* node, var_t** variables, int var_count, bool need_var_value)
{
    double right_val = tree_sum(node->right, variables, var_count, need_var_value);
    if (right_val == 0)
        return NAN;

    return tree_sum(node->left, variables, var_count, need_var_value) / right_val;
}

double calc_pow(node_t* node, var_t** variables, int var_count, bool need_var_value)
{
    return pow(tree_sum(node->left, variables, var_count, need_var_value), tree_sum(node->right, variables, var_count, need_var_value));
}

double calc_sin(node_t* node, var_t** variables, int var_count, bool need_var_value)
{
    return sin(tree_sum(node->right, variables, var_count, need_var_value));
}

double calc_cos(node_t* node, var_t** variables, int var_count, bool need_var_value)
{
    return cos(tree_sum(node->right, variables, var_count, need_var_value));
}

double calc_ln(node_t* node, var_t** variables, int var_count, bool need_var_value)
{
    double arg = tree_sum(node->right, variables, var_count, need_var_value);
    if (arg <= 0)
        return NAN;

    return log(arg);
}

double calc_exp(node_t* node, var_t** variables, int var_count, bool need_var_value)
{
    return exp(tree_sum(node->right, variables, var_count, need_var_value));
}

