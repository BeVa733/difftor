#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "tree.h"
#include "difftor.h"
#include "difftor_dump.h"

void tree_dot(FILE* dot_file, node_t* node)
{
    if (node == NULL)
        return;

    char* type   = (char*)calloc(STR_MAX_LEN, sizeof(char));
    char* label = (char*)calloc(STR_MAX_LEN, sizeof(char));
    char temp_str[STR_MAX_LEN] = "";

    switch (node->type)
    {
        case NUMBER:
            strcpy(type, "number");
            sprintf(temp_str, "%.2lf", node->data.number);
            strcpy(label, temp_str);
            break;
        case OPERATOR:
            strcpy(type, "operator");
            switch (node->data.oper)
            {
                case ADD: strcpy(label, "+");  break;
                case SUB: strcpy(label, "-");  break;
                case MUL: strcpy(label, "*");  break;
                case DIV: strcpy(label, "/");  break;
                case POW: strcpy(label, "^");  break;
                case SIN: strcpy(label, "sin"); break;
                case COS: strcpy(label, "cos"); break;
                case LN:  strcpy(label, "ln");  break;
                case EXP: strcpy(label, "exp"); break;
                default: strcpy(label, "?");
            }
            break;
        case VARIABLE:
            strcpy(type, "variable");
            strcpy(label, node->data.variable);
            break;
    }

    fprintf(dot_file, "    node%p [label=\"{{Parent: %p}|{<f0>PTR: %p}|{type: %s}|{%s}|{{<fl> %p}|{<fr> %p}}}\", shape=Mrecord];\n",
                    node, node->parent, node, type, label, node->left, node->right);

    if (node->left)
    {
        fprintf(dot_file, "    node%p:<fl> -> node%p;\n", node, node->left);
        tree_dot(dot_file, node->left);
    }

    if (node->right)
    {
        fprintf(dot_file, "    node%p:<fr> -> node%p;\n", node, node->right);
        tree_dot(dot_file, node->right);
    }

    free(label);
    free(type);
}

void tree_graph_to_file(node_t* node, const char* filename_prefix)
{
    if (!node)
        return;

    char dot_filename[STR_MAX_LEN] = "";
    char png_filename[STR_MAX_LEN] = "";

    sprintf(dot_filename, "%s.dot", filename_prefix);
    sprintf(png_filename, "%s.png", filename_prefix);

    FILE* dot_file = fopen(dot_filename, "w");
    if (!dot_file)
    {
        fprintf(stderr, "Cannot open file %s\n", dot_filename);
        return;
    }

    fprintf(dot_file, "digraph G {\n");
    tree_dot(dot_file, node);
    fprintf(dot_file, "}\n");
    fclose(dot_file);

    char command[STR_MAX_LEN * 2] = "";
    sprintf(command, "dot -Tpng %s -o %s", dot_filename, png_filename);

    if (system(command))
        printf("Error: graph not created\n");
}

FILE* tex_dump_init(const char* filename_prefix)
{
    char tex_filename[STR_MAX_LEN] = "";
    sprintf(tex_filename, "%s.tex", filename_prefix);

    FILE* tex_file = fopen(tex_filename, "w");
    if (!tex_file)
    {
        fprintf(stderr, "Cannot open file %s\n", tex_filename);
        return NULL;
    }

    make_tex_preamble(tex_file);

    fprintf(tex_file, "\\begin{document}\n");

    fflush(tex_file);

    return tex_file;
}

void tex_dump_compile(FILE* tex_file, const char* filename_prefix)
{
    fprintf(tex_file, "\\end{document}\n");
    fflush(tex_file);

    char command[STR_MAX_LEN] = "";
    sprintf(command, "pdflatex -interaction=nonstopmode -jobname=%s %s.tex > NUL 2>&1",
            filename_prefix, filename_prefix);

    if (!system(command))
        printf("Error: pdf not created\n");

    char aux_files[STR_MAX_LEN] = "";
    sprintf(aux_files, "rm %s.aux %s.log > NUL 2>&1",
            filename_prefix, filename_prefix);

    if (system(aux_files))
        printf("Error: extra files not deleted\n");

    fclose(tex_file);
}

void tex_dump_append_tree(node_t* node, FILE* tex_file)
{
    if (!node)
        return;

    fprintf(tex_file, "$$\n");
    tree_tex(tex_file, node);
    fprintf(tex_file, "\n$$\n");

    fflush(tex_file);
}

void tree_tex(FILE* tex_file, node_t* node)
{
    if (node == NULL) return;

    switch (node->type)
    {
        case NUMBER:
            fprintf(tex_file, "%.2lf", node->data.number);
            break;

        case VARIABLE:
            fprintf(tex_file, "%s", node->data.variable);
            break;

        case OPERATOR:
            switch (node->data.oper)
            {
                case ADD:
                    tree_tex(tex_file, node->left);
                    fprintf(tex_file, " + ");
                    tree_tex(tex_file, node->right);
                    break;

                case SUB:
                    tree_tex(tex_file, node->left);
                    fprintf(tex_file, " - ");
                    tree_tex(tex_file, node->right);
                    break;

                case MUL:
                    if (node->left->type == OPERATOR && (node->left->data.oper == ADD || node->left->data.oper == SUB))
                    {
                        fprintf(tex_file, "\\left(");
                        tree_tex(tex_file, node->left);
                        fprintf(tex_file, "\\right)");
                    }
                    else
                        tree_tex(tex_file, node->left);

                    fprintf(tex_file, " \\cdot ");

                    if (node->right->type == OPERATOR && (node->right->data.oper == ADD || node->right->data.oper == SUB))
                    {
                        fprintf(tex_file, "\\left(");
                        tree_tex(tex_file, node->right);
                        fprintf(tex_file, "\\right)");
                    }
                    else
                        tree_tex(tex_file, node->right);

                    break;

                case DIV:
                    fprintf(tex_file, "\\frac{");

                    tree_tex(tex_file, node->left);

                    fprintf(tex_file, "}{");

                    tree_tex(tex_file, node->right);

                    fprintf(tex_file, "}");
                    break;

                case POW:
                    if (node->left->type == OPERATOR &&
                       (node->left->data.oper == ADD || node->left->data.oper == SUB ||
                        node->left->data.oper == MUL || node->left->data.oper == DIV))
                    {
                        fprintf(tex_file, "\\left(");
                        tree_tex(tex_file, node->left);
                        fprintf(tex_file, "\\right)");
                    }
                    else
                        tree_tex(tex_file, node->left);

                    fprintf(tex_file, "^{");
                    tree_tex(tex_file, node->right);
                    fprintf(tex_file, "}");
                    break;

                case SIN:
                    fprintf(tex_file, "\\sin\\left(");
                    tree_tex(tex_file, node->right);
                    fprintf(tex_file, "\\right)");
                    break;

                case COS:
                    fprintf(tex_file, "\\cos\\left(");
                    tree_tex(tex_file, node->right);
                    fprintf(tex_file, "\\right)");
                    break;

                case LN:
                    fprintf(tex_file, "\\ln\\left(");
                    tree_tex(tex_file, node->right);
                    fprintf(tex_file, "\\right)");
                    break;

                case EXP:
                    fprintf(tex_file, "e^{");
                    tree_tex(tex_file, node->right);
                    fprintf(tex_file, "}");
                    break;

                default:
                    fprintf(tex_file, "?");
                    break;
            }
            break;
    }
}

void make_tex_preamble(FILE* tex_file)
{
    fprintf(tex_file, "\\documentclass[a4paper,12pt]{article}\n");
    fprintf(tex_file, "\\usepackage{geometry}\n");
    fprintf(tex_file, "\\geometry{top=2cm, bottom=3cm, left=3cm, right=3cm}\n");
    fprintf(tex_file, "\\setlength{\\parindent}{0pt}\n");

    fprintf(tex_file, "\\usepackage{graphicx}\n");
    fprintf(tex_file, "\\usepackage{wrapfig}\n");
    fprintf(tex_file, "\\usepackage{caption}\n");
    fprintf(tex_file, "\\usepackage{subcaption}\n");
    fprintf(tex_file, "\\usepackage[english,russian]{babel}\n");
    fprintf(tex_file, "\\usepackage{amsmath, float, amssymb, amsthm, mathtools}\n");
    fprintf(tex_file, "\\usepackage{amsfonts}\n");
}
