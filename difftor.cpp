// #include <TXLib.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "tree.h"
#include "difftor.h"
#include "difftor_dump.h"

node_t* HEAD = NULL;
FILE* DUMP_FILE = NULL;

int main()
{
    // var_t* variables[MAX_VARS] = {};
    // int var_count = 0;

    FILE* tex_file = tex_dump_init("test");

    DUMP_FILE = tex_file;

    node_t* head = read_file("calc.txt");
    tree_graph_to_file(head, "tree");
    tex_dump_append_tree(head, tex_file);

    // double result = tree_sum (head, NULL, 0);
    // printf("Result: %.02lf\n", result);

    // node_t* diff_tree = tree_diff(head, "x");
    // tree_graph_to_file(diff_tree, "tree");
    // tex_dump_append_tree(diff_tree, tex_file);
//
//     HEAD = diff_tree;
//
//
//
//     simplify_tree(diff_tree);
//
//
//
//     tree_graph_to_file(diff_tree, "result");
//     tex_dump_append_tree(diff_tree, tex_file);

    tex_dump_compile(tex_file, "test");

    // var_dtor(variables, var_count);

    tree_dtor(head);
    // tree_dtor(diff_tree);

    return 0;
}

void request_variable_values(var_t** variables, int var_count)
{
    printf("Please enter values for variables:\n");
    for (int i = 0; i < var_count; i++)
    {
        printf("%s = ", variables[i]->name);

        if (scanf("%lf", &variables[i]->value))
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
            free(var[i]);
        }
    }
}
